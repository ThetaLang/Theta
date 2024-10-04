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
    
    #ifdef __APPLE__
    static int preInputHook(const char *text, int count) {
      prefillIndentation();
      return 0;
    }
    #else
    static int preInputHook() {
      prefillIndentation();
      return 0;
    }
    #endif

    static void prefillIndentation();

  private:
    static stack<char> delimeterStack;
    static int lineNumber;

    bool isMatchingDelimeter(char &c, stack<char> delimeterStack);

    string getPrompt();

    void execute(string source);
  };
}
