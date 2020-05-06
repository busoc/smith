#ifndef SCAN_H
#define SCAN_H

#include <exception>
#include <fstream>
#include <sstream>
#include <string>

using namespace std;

namespace ini {
  namespace mk {
    const char space = ' ';
    const char tab = '\t';
    const char pound = '#';
    const char lsquare = '[';
    const char rsquare = ']';
    const char newline = '\n';
    const char equal = '=';
    const char hyphen = '-';
    const char underscore = '_';
    const char carriage = '\r';
    const char dquote = '"';
    const char squote = '\'';

    inline bool is_blank(char c) {
      return c == space || c == tab;
    }

    inline bool is_newline(char c) {
      return c == newline;
    }

    inline bool is_letter(char c) {
      return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
    }

    inline bool is_digit(char c) {
      return c >= '0' && c <= '9';
    }

    inline bool is_alpha(char c) {
      return is_digit(c) || is_letter(c);
    }

    inline bool is_ident(char c) {
      return is_alpha(c) || c == hyphen || c == underscore;
    }

    inline bool is_comment(char c) {
      return c == pound;
    }
  }

  enum token_type {eof, header, ident, literal, number, boolean, assign, comment};

  inline string to_string(token_type tt) {
    switch (tt) {
      case eof: return "eof";
      case header: return "header";
      case ident: return "ident";
      case number: return "number";
      case boolean: return "boolean";
      case literal: return "literal";
      case comment: return "comment";
      case assign: return "assign";
      default: return "unknown";
    }
  }

  struct position {
    int line;
    int column;
    string to_string() {
      ostringstream str;
      str << line;
      str << ":";
      str << column;

      return str.str();
    }
    bool is_valid() {
      return line > 0;
    }
  };

  struct token {
    token_type type;
    string literal;
    position pos;
  };

  class scanner_exception: public exception {
  private:
    string where;
    string reason;
  public:
    scanner_exception(string w, string r): where(w), reason(r) {}
    virtual ~scanner_exception() throw() {}
    virtual const char* what() const throw() {
      ostringstream str;
      str << "scanning " << where << ": " << reason << "!!!";
      return str.str().c_str();
    }
  };

  class scanner {
  public:
    scanner(string file);
    token scan();
    bool done();
    position where();

  private:
    stringstream buffer;
    int line;
    int column;
    int length;

    char last;
    bool value;

    void scan_header(token& tok);
    void scan_ident(token& tok);
    void scan_value(token& tok);
    void scan_quote(token& tok, char quote);
    void scan_comment(token& tok);

    char get();
    char peek();
    void unget();

    void skip_lines();
    void skip_blanks();
  };
}

#endif // SCAN_H
