#include "REPL.hpp"
#include <iostream>
#include <readline/readline.h>
#include <readline/history.h>
#include "compiler/Compiler.hpp"
#include "CLI.hpp"
#include "runtime/Runtime.hpp"

using namespace Theta;
using namespace std;

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

  while ((input = readline("\x1B[32mith $>\x1B[0m ")) != nullptr) {
    if (*input) add_history(input);

    vector<char> wasm = Compiler::getInstance().compileDirect(input);

    if (wasm.size() > 0) {
      ExecutionContext context = Runtime::getInstance().execute(wasm, "main0");
      
      cout << "\x1B[33m    -> " << context.stringifiedResult() << "\x1B[0m" << endl << endl;
    }

    Compiler::getInstance().clearExceptions();

    free(input);
  }
}

