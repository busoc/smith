#include <iostream>
#include <exception>
#include <vector>
#include "ini/ini.h"

#include "dass/pd.h"
#include "dass/pt.h"

using namespace std;

int main(int argc, char** argv) {
  if (argc != 2) {
    cerr << "invalid number of arguments given!" << endl;
    cerr << "usage: smith <config.ini>" << endl;
    return 1;
  }
  try {
    ini::config cfg(argv[1]);

    string item = cfg.get_string("type");
    int code = 0;
    if (item == "pd" || item == "PD") {
      code = pd::run(cfg);
    } else if (item == "pt" || item == "PT") {
      code = pt::run(cfg);
    } else {
      cerr << "unsupported item type "+item << endl;
      return 1;
    }
    return code;
  } catch (exception e) {
    cerr << "unexpected error: " << e.what() << endl;
    return 2;
  } catch(...) {
    cerr << "fatal error!!!" << endl;
    return 3;
  }
}
