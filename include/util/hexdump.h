#ifndef __X_HEXDUMP_H__
#define __X_HEXDUMP_H__

#include <string>
#include <sstream>
#include <iostream>

using namespace std;

namespace hexdump {
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
}

#endif
