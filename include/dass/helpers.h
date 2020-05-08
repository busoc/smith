#ifndef __X_HELPERS_H__
#define __X_HELPERS_H__

#include <fstream>
#include <iostream>
#include <vector>
#include <string>

#include "ServiceDescriptor.h"
#include "UMISet.h"

using namespace std;

namespace dassutil {

  int get_mode(string label) {
    if (label == "ops") {
      return dass::ServiceDescriptor::MODE_OPS;
    } else if (label == "sim1") {
      return dass::ServiceDescriptor::MODE_SIM_1;
    } else if (label == "sim2") {
      return dass::ServiceDescriptor::MODE_SIM_2;
    } else if (label == "test") {
      return dass::ServiceDescriptor::MODE_TEST;
    } else {
      throw "unknown mode provided: "+label;
    }
  }

  int get_instance(string label) {
    if (label == "realtime" || label == "") {
      return dass::ServiceDescriptor::SERVICEMODE_REALTIME;
    } else if (label == "external") {
      return dass::ServiceDescriptor::SERVICEMODE_EXTERNALPLAYBACK;
    } else if (label == "dass") {
      return dass::ServiceDescriptor::SERVICEMODE_DASSPLAYBACK;
    } else {
      throw "unknown instance provided: "+label;
    }
  }

  dass::UMISet load_catalog(string file) {
    dass::UMISet set;
    ifstream in(file.c_str());
    if (!in) {
      throw "fail to open file "+file;
    }
    string code;
    while(getline(in, code)) {
      set.addUMI(dass::UMI(code));
    }
    in.close();
    return set;
  }

}
#endif // __X_HELPERS_H__
