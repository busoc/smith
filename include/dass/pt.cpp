#include "pt.h"
#include "ini/ini.h"

namespace pt {
  int run(ini::config& cfg) {
    return 0;
  }

  client::client(ini::config& cfg): Util::TimedEvent(0) {

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

  }

  void client::receivePathTMResponse(dass::client::PathTMService& service, dass::PathTMResponse& response) {

  }

  int serialize(char* buffer, dass::PathTM* data) {
    return 0;
  }
}
