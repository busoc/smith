#ifndef __X_USOC_H__
#define __X_USOC_H__

#include <string>

using namespace std;

namespace usoc {
  class client
  {
  private:
    int fd;
    int port;
    string addr;

    void dial();
    bool is_connected()
    {
      return fd != -1;
    };

  public:
    client(string addr, int port);
    void kill();
    void forward(const char* data);
    void generate();
  };
}

#endif // __X_USOC_H__
