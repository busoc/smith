#ifndef __X_PT_H__
#define __X_PT_H__

#include <iostream>

#include "PathTMInterface.h"
#include "PathTMClientConnection.h"
#include "StatusService.h"
#include "ServiceStatusResponse.h"
#include "ConnectionMonitor.h"
#include "ini/ini.h"
#include "usoc/usoc.h"


namespace pt {
  int run(ini::config& cfg);

  int serialize(char* buffer, dass::PathTM* data);

  class client: public dass::client::PathTMInterface, public dass::client::ConnectionMonitor, public dass::client::ServiceStatusInterface, public Util::TimedEvent {
  private:
    usoc::client* worker;

    dass::client::RealtimePathTMSpecification spec;
    dass::client::PathTMClientConnection* conn;
    dass::client::PathTMService* service;
    dass::client::StatusService* status;
  public:
    client(string section, ini::config& cfg);
    virtual ~client();

    virtual void receivePathTMData (dass::client::PathTMService& service, dass::PathTM* data);
    virtual void receivePathTMResponse (dass::client::PathTMService& service, dass::PathTMResponse& response);

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

    dass::client::PathTMService* get_service() {
      return service;
    }

    dass::client::StatusService* get_statusof() {
      return status;
    };

    dass::client::PathTMClientConnection * get_conn() {
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

#endif // __X_PT_H__
