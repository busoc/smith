#include <fstream>
#include <iostream>
#include <vector>
#include <arpa/inet.h>

#include "helpers.h"
#include "pd.h"
#include "ini/ini.h"
#include "usoc/usoc.h"

#include "ProcessedDataInterface.h"
#include "ProcessedDataClientConnection.h"
#include "StatusService.h"
#include "ServiceStatusResponse.h"
#include "ConnectionMonitor.h"

using namespace std;

namespace pd {

  int serialize(char* buffer, dass::ProcessedDataItem* item);
  void hexdump(char* buffer, int written);

  char tag = 0x06;
  int request = 0x00;
  int preamble_len = 12;
  int header_len = 21;
  int buffer_len = (1<<16)-1;
  int umi_len = 6;

  int run(ini::config& cfg) {
    try {
      client* ptr = new client(cfg);
      dass::client::ClientConnection::StartService(0);

      dass::client::ProcessedDataService* service = ptr->get_service();
      dass::client::ProcessedDataClientConnection* conn = ptr->get_conn();

      if (service != NULL) {
        service->stop();
        cout << "Stopping request!" << endl;
      }
      dass::client::ClientConnection::StartService(5);

      if (ptr->get_statusof() != NULL) {
  			ptr->get_statusof()->stop();
  			cout << "Stopping status service request!" << endl;
      }
      dass::client::ClientConnection::StartService (5);

      if (conn != NULL && conn->connected()) {
        conn->disconnect();
        cout << "Closing connection!" << endl;
      }
      dass::client::ClientConnection::StartService(5);

      if (ptr != NULL) {
        delete ptr;
      }
    } catch(...) {
      return 2;
    }
    return 0;
  }

  client::client(ini::config& cfg): Util::TimedEvent(0) {
    worker = new usoc::client(cfg.get_string("usoc", "address"), cfg.get_int("usoc", "port"));

    string catalog = cfg.get_string("data", "catalog");
    dass::UMISet set = dassutil::load_catalog(catalog);
    spec = dass::client::RealtimeProcessedDataSpecification(1, set);

    string instance = cfg.get_string("data", "instance");
    string mode = cfg.get_string("data", "mode");
    conn = new dass::client::ProcessedDataClientConnection(dassutil::get_mode(mode), dassutil::get_instance(instance));
    conn->addConnectionMonitor(*this);
    conn->setSessionRetrievalTime(30);

    if (cfg.has_option("dass", "certificate")) {
      conn->setCredentials(cfg.get_string("dass", "certificate"));
    } else {
      conn->setCredentials(cfg.get_string("dass", "user"), cfg.get_string("dass", "passwd"));
    }

    try {
      conn->connect(cfg.get_string("dass", "address"), cfg.get_int("dass", "port"), 30);
      if (conn->connected()) {
        conn->startProcessedDataService(*this, spec);
        Set(Util::CTimeSpan::Second() * 5);
      } else {
        Clear();
        Set(Util::CTimeSpan::Second() * 10);
        // throw "fail to connect to remote server: "+cfg.get_string("dass", "address");
      }
    } catch (...) {
      Clear();
      cout << "Exception while starting PD service!" << endl;
      Set(Util::CTimeSpan::Second() * 10);
    }
  }

  client::~client() {
    if (service != NULL) {
      service->stop();
    }
    if (conn != NULL) {
      delete conn;
    }
    if (worker != NULL) {
      worker->kill();
      delete worker;
    }
  }

  void client::receiveProcessedData(dass::client::ProcessedDataService& service, dass::ProcessedDataItemSet* data) {
    dass::CCSDSUTime now;
    short count = htons(short(data->count()));
    int written = 0;
    char buffer[buffer_len];

    memcpy(buffer, &tag, sizeof(tag));
    written += sizeof(tag);

    int coarse = htonl(now.getEpochTime());
    memcpy(buffer+written, &coarse, sizeof(coarse));
    written += sizeof(coarse);

    char fine = now.getMilliSeconds();
    memcpy(buffer+written, &fine, sizeof(fine));
    written += sizeof(fine)

    memcpy(buffer, &request, sizeof(request));
    written += sizeof(request);

    memcpy(buffer+written, &count, sizeof(count));
    written += sizeof(count);


    for (int i = 0; i < count; i++) {
      written += serialize(buffer+written, data->getItem(i));
    }
    delete data;

    worker->forward(buffer, written);
  }

  void client::receiveProcessedDataResponse(dass::client::ProcessedDataService& service, dass::ProcessedDataResponse& response) {
    // do nothing
  }

  const string alphabet = "0123456789ABCDEF";
  void hexdump(char*buffer, int written) {
    ostringstream str;
    for (int i = 0; i < written; i++) {
      int high = int(buffer[i]) >> 4;
      str.put(alphabet[high]);

      int low = int(buffer[i]) & 0x0F;
      str.put(alphabet[low]);
      str.put(' ');
    }
    cerr << str.str() << endl;
  }

  int serialize(char* buffer, dass::ProcessedDataItem* item) {
    int written = 0;

    unsigned char state = item->getValueState();
    memcpy(buffer, &state, sizeof(state));
    written += sizeof(state);

    int bitfield = htonl(item->getStatusBitfield());
    memcpy(buffer+written, &bitfield, sizeof(bitfield));
    written += sizeof(bitfield);

    long long umi = item->getUMI().longdump();
    buffer[written]   = char((umi >> 40) & 0xFF);
    buffer[written+1] = char((umi >> 32) & 0xFF);
    buffer[written+2] = char((umi >> 24) & 0xFF);
    buffer[written+3] = char((umi >> 16) & 0xFF);
    buffer[written+4] = char((umi >> 8) & 0xFF);
    buffer[written+5] = char(umi & 0xFF);
    written += umi_len;

    unsigned char type = item->getDataType();
    memcpy(buffer+written, &type, sizeof(type));
    written += sizeof(type);

    unsigned short unit = htons(item->getUnitsIndex());
    memcpy(buffer+written, &unit, sizeof(unit));
    written += sizeof(unit);

    dass::CCSDSUTime time = item->getGeneratedTime();

    int coarse = htonl(time.getEpochTime());
    memcpy(buffer+written, &coarse, sizeof(coarse));
    written += sizeof(coarse);

    char fine = time.getMilliSeconds();
    memcpy(buffer+written, &fine, sizeof(fine));
    written += sizeof(fine);

    unsigned short length = htons(item->getRawValueLength());
    memcpy(buffer+written, &length, sizeof(length));
    written += sizeof(length);

    const unsigned char* raw = item->getRawValue();
    for (int i = 0; i < length; i++) {
      buffer[written+i] = raw[i];
    }
    // memcpy(buffer+written, raw, int(length));
    written += length;

    return written;
  }
}
