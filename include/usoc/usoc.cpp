#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>
#include "usoc.h"

namespace usoc {

  client::client(string a, int p): fd(-1), port(p), addr(a) {
    dial();
  }

  void client::dial()
  {
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
      throw "fail to create socket";
    }

    sockaddr_in a;
    a.sin_family = AF_INET;
    a.sin_port = htons(port);
    inet_aton(addr.c_str(), &a.sin_addr);

    if (connect(fd, (sockaddr*)&a, sizeof(a)) < 0) {
      throw "fail to connect to remote address";
    }
  }

  void client::kill() {
    shutdown(fd, 1);
  }

  void client::forward(const char* data, int size) {
    if (size <= 0) {
      return;
    }
    if (send(fd, data, size, 0) < 0) {
      throw "fail to send message";
    }
  }
}
