#include "REPL.hpp"
#include <iostream>
#include <readline/readline.h>
#include <readline/history.h>
#include "compiler/Compiler.hpp"
#include "CLI.hpp"
#include "runtime/Runtime.hpp"
#include <map>

using namespace Theta;
using namespace std;

stack<char> REPL::delimeterStack;
int REPL::lineNumber = 0;

REPL::REPL() {
  cout << "Interactive Theta" << endl;
  CLI::printLanguageVersion();
  cout << "Report issues at " + CLI::makeLink("https://www.github.com/alexdovzhanyn/ThetaLang/issues") << endl;
  cout << "CTRL+D to exit" << endl << endl;
}

REPL::~REPL() {
  cout << endl << endl << "Exiting ITH..." << endl;
}

void REPL::readInput() {
  char* input;
  string accumulatedInput;

  rl_pre_input_hook = prefillIndentation;

  while ((input = readline(getPrompt().c_str())) != nullptr) {
    accumulatedInput += string(input);
    lineNumber++;

    // For multiline input. We keep a stack of the matching braces so we know once a statement is complete.
    for (char &c : string(input)) {
      if (c == '{' || c == '(' || c == '[') {
        delimeterStack.push(c);
      } else if (isMatchingDelimeter(c, delimeterStack)) {
        delimeterStack.pop();
      }
    }

    // If all of the delimeters weren't resolved, take in more user input
    if (!delimeterStack.empty()) {
      // Only add the newline in the case where we expect more input. Otherwise it's pointless
      accumulatedInput += "\n";
      free(input);

      string indents = "";
      for (int i = 0; i < delimeterStack.size(); ++i) {
        indents += "  ";
      }

      continue;
    }

    if (!accumulatedInput.empty()) execute(accumulatedInput);
    
    free(input);
    accumulatedInput.clear();
  }
}

void REPL::execute(string source) {
  add_history(source.c_str());

  vector<char> wasm = Compiler::getInstance().compileDirect(source);

  if (wasm.size() > 0) {
    ExecutionContext context = Runtime::getInstance().execute(wasm, "main0");
    
    cout << "\x1B[33m-----> " << context.stringifiedResult() << "\x1B[0m" << endl << endl;
  }

  Compiler::getInstance().clearExceptions();
}

int REPL::prefillIndentation(const char *text, int count) {
  string indents(delimeterStack.size() * 2, ' ');

  rl_insert_text(indents.c_str());
  rl_point = rl_end;
  rl_redisplay();
 
  return 0;
}

string REPL::getPrompt() { 
  string promptStyleSet = "\x1B[32m";
  string promptStyleClear = "\x1B[0m";

  string prompt = delimeterStack.size() == 0 ? "ith" : "...";
  string line = "(" + to_string(lineNumber) + ")"; 

  return promptStyleSet + prompt + line + " $> " + promptStyleClear;
}

bool REPL::isMatchingDelimeter(char &c, stack<char> delimeterStack) {
  if (delimeterStack.empty()) return false;
  
  map<char, char> delimeterPairs = {
    { '}', '{' },
    { ')', '(' },
    { ']', '['}
  };

  if (delimeterPairs.find(c) == delimeterPairs.end()) return false;
  
  return delimeterStack.top() == delimeterPairs.at(c);
}
