#include <vector>
#include <set>
#include <string>
#include <cstdlib>
#include <iostream>
#include "scan.h"
#include "ini.h"

using namespace std;

namespace ini {

  class parser {
  private:
    token curr;
    token peek;
    scanner scan;

  public:
    parser(string file): scan(file) {
        next();
        next();

    };
    vector<option> parse_ini();
  private:
    vector<option> parse_section(string label);
    option parse_option(string label);
    void next();
  };

  config::config(string file) {
    options = parse(file);
  }

  string config::get_string(string section, string name) {
    option o = get_option(section, name);
    return o.value;
  }

  string config::get_string(string name) {
    return get_string("", name);
  }

  int config::get_int(string section, string name) {
    option o = get_option(section, name);
    return atoi(o.value.c_str());
    // return stoi(o.value);
  }

  int config::get_int(string name) {
    return get_int("", name);
  }

  double config::get_double(string section, string name) {
    option o = get_option(section, name);
    return atof(o.value.c_str());
    // return stod(o.value);
  }

  double config::get_double(string name) {
    return get_double("", name);
  }

  bool config::get_bool(string section, string name) {
    option o = get_option(section, name);
    if (o.value == "" || o.value == "0" || o.value == "false") {
      return false;
    }
    return true;
  }

  bool config::get_bool(string name) {
    return get_bool("", name);
  }

  bool config::has_section(string section) {
    for (int i = 0; i < options.size(); i++) {
      option o = options[i];
      if (o.section == section) {
        return true;
      }
    }
    return false;
  }

  bool config::has_option(string section, string name) {
    for (int i = 0; i < options.size(); i++) {
      option o = options[i];
      if (o.section == section && o.name == name) {
        return true;
      }
    }
    return false;
  }

  option config::get_option(string section, string name) {
    for (int i = 0; i < options.size(); i++) {
      option o = options[i];
      if (o.section == section && o.name == name) {
        return o;
      }
    }
    throw not_found(section, name);
  }

  vector<option> config::parse(string file) {
    vector<option> options;
    parser ps(file);
    return ps.parse_ini();
  }

  vector<option> parser::parse_ini() {
    vector<option> options;
    set<string> sections;
    if (curr.type == ident) {
      vector<option> os = parse_section("default");
      options.insert(options.end(), os.begin(), os.end());
    }
    while(!scan.done()) {
      if (curr.type != header) {
        throw unexpected_token(curr.pos, header, curr.type);
      }
      string label = curr.literal;
      if (sections.find(label) != sections.end()) {
        throw duplicate(label, "section");
      }
      next();
      vector<option> os = parse_section(label);
      options.insert(options.end(), os.begin(), os.end());
    }
    return options;
  }

  vector<option> parser::parse_section(string label) {
    vector<option> options;
    set<string> labels;
    while (curr.type != header && !scan.done()) {
      option opt = parse_option(label);
      if (labels.find(opt.name) != labels.end()) {
        throw duplicate(opt.name, "option");
      }
      options.push_back(opt);
      next();
    }
    return options;
  }

  option parser::parse_option(string section) {
    option opt;
    if (curr.type != ident) {
      throw unexpected_token(curr.pos, ident, curr.type);
    }
    opt.section = section;
    opt.name = curr.literal;
    next();
    if (curr.type != assign) {
      throw unexpected_token(curr.pos, assign, curr.type);
    }
    next();
    if (curr.type != literal) {
      throw unexpected_token(curr.pos, literal, curr.type);
    }
    opt.value = curr.literal;

    return opt;
  }

  void parser::next() {
    curr = peek;
    peek = scan.scan();
    while (peek.type == comment) {
      peek = scan.scan();
    }
  };
}
