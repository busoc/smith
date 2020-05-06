#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>
#include "usoc.h"

namespace usoc {

  void to_bytes(int i, char* arr)
  {
    int z = (int) sizeof(i);
    int x = z-1;
    for (int j = 0; j < z; j++)
    {
      int s = (x-j) * 8;
      arr[j] = (char)((i >> s) & 0xFF);
    }
  }

  void to_bytes(short i, char* arr)
  {
    int z = (int) sizeof(i);
    int x = z-1;
    for (int j = 0; j < z; j++)
    {
      int s = (x-j) * 8;
      arr[j] = (char)((i >> s) & 0xFF);
    }
  }

  client::client(string a, int p)
  {
    fd = -1;
    addr = a;
    port = p;

    dial();
  }

  void client::dial()
  {
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0)
    {
      throw "fail to create socket";
    }

    sockaddr_in a;
    a.sin_family = AF_INET;
    a.sin_port = htons(port);
    inet_aton(addr.c_str(), &a.sin_addr);

    if (connect(fd, (sockaddr*)&a, sizeof(a)) < 0)
    {
      throw "fail to connect to remote address";
    }
  }

  void client::kill()
  {
    shutdown(fd, 1);
  }

  void client::generate()
  {
    char buffer[512] = {0};
    int i = 0;
    short s = 0;
    while(1)
    {
      char* ptr = buffer;
      to_bytes(i, ptr);
      to_bytes(s, ptr+4);

      forward(buffer);
      i++;
      s++;
      sleep(1000);
    }
  }

  void client::forward(const char* data)
  {
    if (send(fd, data, 6, 0) < 0)
    {
      throw "fail to send message";
    }
  }
}
