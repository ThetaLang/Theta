#pragma once

#include <stack>
#include <string>

using namespace std;

namespace Theta {
  class REPL {
  public:
    REPL();
    ~REPL();

    void readInput();
    
    static int prefillIndentation(const char *text, int count);

  private:
    static stack<char> delimeterStack;
    static int lineNumber;

    bool isMatchingDelimeter(char &c, stack<char> delimeterStack);

    string getPrompt();

    void execute(string source);
  };
}
