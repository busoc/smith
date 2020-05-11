#include "helpers.h"
#include "pt.h"
#include "ini/ini.h"

namespace pt {

  int ccsds_len = 6;
  int buffer_len = 1<<16;
  int request = 4;
  char tag = 0x0B;

  int run(ini::config& cfg) {
    try {
      client* ptr = new client(cfg);
      dass::client::ClientConnection::StartService(0);

      dass::client::PathTMService* service = ptr->get_service();
      dass::client::PathTMClientConnection* conn = ptr->get_conn();

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

    string instance = cfg.get_string("data", "instance");
    string mode = cfg.get_string("data", "mode");
    conn = new dass::client::PathTMClientConnection(dassutil::get_mode(mode), dassutil::get_instance(instance));
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
        conn->startPathTMService(*this, spec);
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

  void client::receivePathTMData(dass::client::PathTMService& service, dass::PathTM* data) {
    int written = 0;

    dass::CCSDSUTime now;
    char buffer[buffer_len];

    memcpy(buffer, &tag, sizeof(tag));
    written += sizeof(tag);

    int coarse = htonl(now.getEpochTime());
    memcpy(buffer+written, &coarse, sizeof(coarse));
    written += sizeof(coarse);

    char fine = now.getMilliSeconds();
    memcpy(buffer+written, &fine, sizeof(fine));
    written += sizeof(fine);

    memcpy(buffer+written, &request, sizeof(request));
    written += sizeof(request);

    // written += serialize(buffer+written, data);

    delete data;
    worker->forward(buffer, written);
  }

  void client::receivePathTMResponse(dass::client::PathTMService& service, dass::PathTMResponse& response) {
    // do nothing
  }

  int serialize(char* buffer, dass::PathTM* data) {
    return 0;
  }
}
