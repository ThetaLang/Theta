#include "CLI.hpp"
#include <iostream>
#include <algorithm>
#include "../../version.h"
#include "../compiler/Compiler.hpp"
#include "REPL.hpp"

using namespace Theta;
using namespace std;

void CLI::parseCommand(int argc, char* argv[]) {
    bool isEmitTokens = false;
    bool isEmitAST = false;
    bool isEmitWAT = false;
    string sourceFile;
    string outFile;

    if (argc == 1) {
        REPL repl = REPL();
        repl.readInput();
    } else if (argc == 2) {
        string arg1 = argv[1];

        if (arg1 == "--version") return printLanguageVersion();
        if (arg1 == "--help") return printUsageInstructions();

        sourceFile = argv[1];
    } else {
        int i = 1;

        while (i < argc) {
            string arg = argv[i];

            if (arg == "-o") {
                outFile = argv[i + 1];
                i++;
            }
            else if (arg == "--emitTokens") isEmitTokens = true;
            else if (arg == "--emitAST") isEmitAST = true;
            else if (arg == "--emitWAT") isEmitWAT = true;
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

        outFile += ".wasm";
    }

    Theta::Compiler::getInstance().compile(sourceFile, outFile, isEmitTokens, isEmitAST, isEmitWAT);
}

string CLI::makeLink(string url, string text) {
    return "\x1B]8;;" +  url + "\x1B\\" + (text != "" ? text : url) + "\x1B]8;;\x1B\\";
}

void CLI::printLanguageVersion() {
    cout << "Theta v" << VERSION_MAJOR << '.' << VERSION_MINOR << '.' << VERSION_PATCH << endl;
}

void CLI::printUsageInstructions() {
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
    cout << "  --emitWAT                      Emit the WebAssembly Text format (WAT) representation produced." << endl;
    cout << "  --help                         Display this help message and exit." << endl;
    cout << "  --version                      Display the currently installed Theta language version and exit." << endl;
}

bool CLI::validateOption(string option) {
    vector<string> validOptions = {
        "--version",
        "--help",
        "--emitTokens",
        "--emitAST",
        "--emitWAT",
        "-o"
    };

    if (find(validOptions.begin(), validOptions.end(), option) == validOptions.end()) {
        cout << "Invalid option: " + option << endl;
        return false;
    }

    return true;
}
