#ifndef __X_PD_H__
#define __X_PD_H__

#include "ProcessedDataInterface.h"
#include "ProcessedDataClientConnection.h"
#include "StatusService.h"
#include "ServiceStatusResponse.h"
#include "ConnectionMonitor.h"
#include "ini/ini.h"
#include "usoc/usoc.h"

using namespace std;

namespace pd {
  int run(ini::config& cfg);

  int serialize(char* buffer, dass::ProcessedDataItem* item);

  class client: public dass::client::ProcessedDataInterface, dass::client::ConnectionMonitor, public dass::client::ServiceStatusInterface, public Util::TimedEvent {
  private:
    usoc::client* worker;

    dass::client::RealtimeProcessedDataSpecification spec;
    dass::client::ProcessedDataClientConnection* conn;
    dass::client::ProcessedDataService* service;
    dass::client::StatusService* status;
  public:
    client(ini::config& cfg);
    virtual ~client();

    virtual void receiveProcessedData(dass::client::ProcessedDataService& service, dass::ProcessedDataItemSet* data);
    virtual void receiveProcessedDataResponse(dass::client::ProcessedDataService& service, dass::ProcessedDataResponse& response);

    void sendRequest (int);

    virtual void Triggered() {
      if (conn->connected ()) {
        status = conn->startStatusService(*this);
      }
      if (status) {
        cout << "Started Service Status..." << endl;
      }
    }

    virtual void receiveServiceStatus (dass::client::StatusService& srv, dass::ServiceStatusResponse& response) {
      status = &srv;
      cout << "Received " << service->toString () << endl;
    }

    dass::client::StatusService * get_statusof() {
      return status;
    }

    dass::client::ProcessedDataService* get_service() {
      return service;
    }

    dass::client::StatusService* get_status() {
      return status;
    }

    dass::client::ProcessedDataClientConnection* get_conn() {
      return conn;
    }

    virtual void connected(dass::client::ClientConnection& c) {
      cout << "Connected...\n";
    }

    virtual void disconnected(dass::client::ClientConnection& c) {
      cout << "Disconnected...\n";
    }

    virtual void reconnecting(dass::client::ClientConnection& c) {
      cout << "Reconnecting...\n";
    }

    virtual void reconnected(dass::client::ClientConnection& c) {
      cout << "Reconnected...\n";
    }
    virtual void recovered(dass::client::ClientConnection& c) {
      cout << "Recovered...\n";
    }

    virtual void closed(dass::client::ClientConnection& c) {
      service = NULL;
      cout << "Closed...\n";
    }

    virtual void illegalPacket(dass::client::ClientConnection& c) {
      cout << "Illegal...\n";
    }

    virtual void serviceStarted(dass::client::Service& service) {
      cout << "Started " << service.toString ().c_str () << endl;
    }

    virtual void serviceStopped(dass::client::Service& service) {
      cout << "Stopped " << service.toString ().c_str () << endl;
    }

    virtual void serviceError(dass::client::Service& service, dass::client::ServiceError & error) {
      cout << "Error " << error.getErrorMessage () << ": service: " << service.toString() << endl;
    }
  };
}

#endif // __X_PD_H__
