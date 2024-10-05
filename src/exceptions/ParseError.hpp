#pragma once

#include <string>

using namespace std;

namespace Theta {
  class ParseError : public exception {
  public:
    ParseError(string typ) : errorParseType(typ) {}

    string getErrorParseType() { return errorParseType; }

  private:
    string errorParseType;
  };
}
