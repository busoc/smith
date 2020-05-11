#ifndef __X_HELPERS_H__
#define __X_HELPERS_H__
#include <string>

using namespace std;

namespace dassutil {

  int get_mode(string label);

  int get_instance(string label);

  dass::UMISet load_catalog(string file);

}
#endif // __X_HELPERS_H__
