#include <string>
#include <algorithm>
#include "scan.h"

using namespace std;

namespace ini {

  scanner::scanner(string file) {
    ifstream in(file.c_str(), ios::in);
    if (!in) {
      throw "fail to open file "+file;
    }
    buffer << in.rdbuf();
    in.close();

    string str = buffer.str();
    str.erase(remove(str.begin(), str.end(), mk::carriage), str.end());
    buffer.str(str);

    line = 1;
    column = 0;
    length = 0;
    last = 0;
    value = false;

    get();
  }

  bool scanner::done() {
    return buffer.eof();
  };

  position scanner::where() {
    // position pos {line, column};
    position pos;
    pos.line = line;
    pos.column = column;
    return pos;
  };

  token scanner::scan() {
    skip_lines();
    skip_blanks();

    token tok;
    if (done()) {
      tok.type = eof;
      return tok;
    }
    tok.pos = where(); // {line, column};

    switch (last) {
      case mk::semicolon:
      case mk::pound:
      scan_comment(tok);
      break;
      case mk::equal:
      value = true;
      tok.literal = "=";
      tok.type = assign;
      break;
      case mk::lsquare:
      scan_header(tok);
      break;
      case mk::dquote:
      case mk::squote:
      if (!value) {
        throw scanner_exception("scan", "only value can be quoted");
      }
      scan_quote(tok, last);
      value = false;
      break;
      default:
      if (value) {
        scan_value(tok);
        value = false;
      } else {
        scan_ident(tok);
      }
      break;
    }
    get();
    return tok;
  }

  void scanner::scan_ident(token& tok) {
    ostringstream str;
    if (!mk::is_letter(last)) {
      throw scanner_exception("ident", "invalid character "+last);
    }
    while(last != mk::equal) {
      if (done()) {
        throw scanner_exception("ident", "unexpected end of file");
      }
      if (mk::is_blank(last)) {
        break;
      }
      if (!mk::is_ident(last)) {
        throw scanner_exception("ident", "invalid character");
      }
      str.put(last);
      get();
    }
    tok.literal = str.str();
    tok.type = ident;
    if (last == mk::equal) {
      unget();
    }
  }

  void scanner::scan_quote(token& tok, char quote) {
    get();
    ostringstream str;
    while(last != quote) {
      if (done()) {
        throw scanner_exception("quoted", "unexpected end of file");
      }
      str.put(last);
      get();
    }
    tok.literal = str.str();
    tok.type = literal;
  }

  void scanner::scan_value(token& tok) {
    ostringstream str;
    str.put(last);
    while(!done()) {
      get();
      if (mk::is_newline(last) || mk::is_comment(last) || mk::is_blank(last)) {
        break;
      }
      str.put(last);
    }
    tok.literal = str.str();
    tok.type = literal;
  }

  void scanner::scan_header(token& tok) {
    get();

    ostringstream str;
    skip_blanks();
    if (!mk::is_letter(last)) {
      throw scanner_exception("header", "invalid character");
    }
    while (last != mk::rsquare) {
      if (done()) {
        throw scanner_exception("header", "unexpected end of file");
      }
      if (mk::is_blank(last)) {
        skip_blanks();
        break;
      }
      if (!mk::is_ident(last)) {
        throw scanner_exception("header", "invalid character");
      }
      str.put(last);
      get();
    }
    tok.literal = str.str();
    tok.type = header;
  }

  void scanner::scan_comment(token& tok) {
    ostringstream str;
    skip_blanks();
    while(last != mk::newline && !done()) {
      str.put(last);
      get();
    }
    tok.literal = str.str();
    tok.type = comment;
  }

  char scanner::peek() {
    return buffer.peek();
  }

  char scanner::get() {
    last = buffer.get();
    if (last == mk::newline) {
      line++;
      length = column;
      column = 0;
    } else {
      column++;
    }
    return last;
  }

  void scanner::unget() {
    if (last == mk::newline) {
      line--;
      column = length;

    } else {
      column--;
    }
    buffer.unget();
  }

  void scanner::skip_blanks() {
    if (!mk::is_blank(last)) {
      return;
    }
    while(mk::is_blank(get())) {}
  }

  void scanner::skip_lines() {
    if (!mk::is_newline(last)) {
      return;
    }
    while(get() == mk::newline) {}
  }
}
