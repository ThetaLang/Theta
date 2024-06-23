#include <iostream>
#include <readline/readline.h>
#include <readline/history.h>
#include "../../version.h"
#include "../compiler/ThetaCompiler.hpp"

using namespace std;

class ThetaCLI {
    public:
        static void parseCommand(int argc, char* argv[]) {
            bool isEmitTokens = false;
            bool isEmitAST = false;
            string sourceFile;
            string outFile;

            if (argc == 1) {
                launchInteractiveShell();
            } else if (argc == 2) {
                string arg1 = argv[1];

                if (arg1 == "--version") return printLanguageVersion();
                if (arg1 == "--help") return printUsageInstructions();

                if (validateOption(arg1)) sourceFile = argv[1];
            } else {
                int i = 0;

                while (i < argc) {
                    string arg = argv[i];

                    if (arg == "-o") {
                        outFile = argv[i + 1];
                        i++;
                    }
                    else if (arg == "--emitTokens") isEmitTokens = true;
                    else if (arg == "--emitAST") isEmitAST = true;
                    else if (i == argc - 1) sourceFile = arg;
                    else validateOption(arg);

                    i++;
                }
            }

            if (sourceFile == "") return;

            if (outFile == "") {
                bool reachedDelimiter = false;
                for (int i = 0; !reachedDelimiter; i++) {
                    outFile += sourceFile[i];

                    if (sourceFile[i + 1] == '.') reachedDelimiter = true;
                }
            }

            ThetaCompiler::getInstance().compile(sourceFile, outFile, isEmitTokens, isEmitAST);
        }

   private:
        static void printUsageInstructions() {
            cout << "Theta Language Compiler CLI" << endl;
            cout << "============================" << endl;
            cout << endl;
            cout << "Usage:" << endl;
            cout << "  theta [options] <source_file>" << endl;
            cout << endl;
            cout << "Options:" << endl;
            cout << "  -o <output_file>               Specify the output file name." << endl;
            cout << "  --emitTokens                   Emit the tokenized representation of the source file produced by the lexer." << endl;
            cout << "  --emitAST                      Emit the Abstract Syntax Tree (AST) representation produced by the parser." << endl;
            cout << "  --help                         Display this help message and exit." << endl;
            cout << "  --version                      Display the currently installed Theta language version and exit." << endl;
        }

        static void printLanguageVersion() {
            cout << "Theta v" << VERSION_MAJOR << '.' << VERSION_MINOR << '.' << VERSION_PATCH << endl;
        }

        static bool validateOption(string option) {
            vector<string> validOptions = {
                "--version",
                "--help",
                "--emitTokens",
                "--emitAST",
                "-o"
            };

            if (find(validOptions.begin(), validOptions.end(), option) == validOptions.end()) {
                cout << "Invalid option: " + option << endl;
                return false;
            }

            return true;
        }

        static void launchInteractiveShell() {
            cout << "Interactive Theta" << endl << endl;

            char* input;

            while ((input = readline("ith $> ")) != nullptr) {
                if (*input) add_history(input);

                // TODO: Change this to get execution result once we get an interpreter built out
                shared_ptr<ASTNode> compiled = ThetaCompiler::getInstance().compileDirect(input);

                cout << "-> " + compiled->toJSON() << endl << endl;

                ThetaCompiler::getInstance().clearExceptions();

                free(input);
            }

            cout << "Exiting ITH..." << endl;
        }
};
