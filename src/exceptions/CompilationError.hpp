#pragma once

#include <string>
#include <iostream>
#import "Error.hpp"
#import "lexer/Token.hpp"

using namespace std;

namespace Theta {
    class CompilationError : public Error {
        private:
            string errorType;
            string message;
            Token token;
            string source;
            string fileName;

        public:
            CompilationError(string type, string msg, Token tok, string &src, string file) : errorType(type), message(msg), token(tok), source(src), fileName(file) {};

            string what() {
                return message + " at line " + to_string(token.getStartLocation()[0]) + ", column " + to_string(token.getStartLocation()[1]);
            }

            void display() override {
                cout << "\n" + fileName << endl;
                cout << "  \033[1;31m" + errorType + "\033[0m: " << what() << ':' << endl;

                string contextPrevLine;
                string contextErrorLine;
                string contextNextLine;
                int line = 1;
                int charIdx = 0;

                // If the error isn't on the first line of the file, we want to add a prevLine to the log output, for context.
                if (token.getStartLocation()[0] > 1) {
                    for (; line < token.getStartLocation()[0] - 1; charIdx++) {
                        if (source[charIdx] == '\n') line++;
                    }

                    while (source[charIdx] != '\n') {
                        contextPrevLine += source[charIdx];
                        charIdx++;
                    }
                }

                // Skip to the line where the error is
                for (; line < token.getStartLocation()[0]; charIdx++) {
                    if (source[charIdx] == '\n') line++;
                }

                while (source[charIdx] != '\n' && charIdx <= source.length()) {
                    contextErrorLine += source[charIdx];
                    charIdx++;
                }

                // If there's another line after this one, add its contents to the nextLine for logging output
                if (charIdx++ < source.length()) {
                    while (source[charIdx] != '\n') {
                        contextNextLine += source[charIdx];
                        charIdx++;
                    }
                }

                string errorMarker(token.getStartLocation()[1] + to_string(token.getStartLocation()[0]).length() + 1, ' ');
                string errorPoint(token.getLexeme().length(), '^');

                if (contextPrevLine != "") {
                    cout << "    " + to_string(token.getStartLocation()[0] - 1) + ": " + contextPrevLine << endl;
                }

                cout << "    " + to_string(token.getStartLocation()[0]) + ": " +  contextErrorLine << endl;
                cout << "    " + errorMarker + "\033[31m" + errorPoint + "\033[0m" << endl;

                if (contextNextLine != "") {
                    cout << "    " + to_string(token.getStartLocation()[0] + 1) + ": " + contextNextLine << endl;
                }
            }
    };
}
