#ifndef INI_H
#define INI_H

#include <exception>
#include <vector>
#include <sstream>
#include <string>
#include "scan.h"
#include "ini.h"

using namespace std;

namespace ini {

  class duplicate: exception {
  private:
    string label;
    string part;
  public:
    duplicate(string a, string p): label(a), part(p) {}
    virtual ~duplicate() throw() {}
    virtual const char* what() const throw() {
      ostringstream str;
      str << "duplicate " << part << ": " << label;
      return str.str().c_str();
    }
  };

  class not_found: exception {
  private:
    string label;
    string section;
  public:
    not_found(string s, string a): section(s), label(a) {}
    virtual ~not_found() throw() {}
    virtual const char* what() const throw() {
      ostringstream str;
      str << section << ": option " << label << " not found";
      return str.str().c_str();
    }
  };

  class unexpected_token: exception {
  private:
    token_type want;
    token_type got;
    position pos;
  public:
    unexpected_token(position p, token_type w, token_type g): pos(p), want(w), got(g) {}
    virtual ~unexpected_token() throw() {}
    virtual const char* what() const throw() {
      ostringstream str;
      str << ": unexpected token! want " << to_string(want);
      str << ", got " << to_string(got);
      return str.str().c_str();
    }
  };

  struct option {
    string section;
    string name;
    string value;
  };

  class config {
  private:
    vector<option> options;
    vector<option> parse(string file);
    option get_option(string section, string name);

  public:
    config(string file);

    string get_string(string option);
    string get_string(string section, string option);
    int get_int(string option);
    int get_int(string section, string option);
    double get_double(string option);
    double get_double(string section, string option);
    bool get_bool(string option);
    bool get_bool(string section, string option);

    bool has_section(string section);
    bool has_option(string section, string name);

    vector<option> all() {
      return options;
    }
  };
}

#endif // INI_H
