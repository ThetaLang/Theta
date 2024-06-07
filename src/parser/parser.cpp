#include <vector>
#include <string>
#include <iostream>
#include "../lexer/token.hpp"
#include "../errors/syntaxerror.hpp"

using namespace std;

class ThetaParser {
    public:
        void parse(vector<Token> tokens, string source) {
            for (int i = 0; i <= tokens.size(); i++) {
                if (tokens[i].getType() == "identifier") {
                    try {
                        validateIdentifier(tokens[i]);
                    } catch (SyntaxError e) {
                        cout << "\n\033[1;31mSyntaxError\033[0m: " << e.what() << ':' << endl;
                        
                        string contextLine;
                        string errorMarker(e.contextLocation()[1] - 1, ' ');
                        string errorPoint(tokens[i].getText().length(), '^');

                        int l = 1;
                        int c = 0;

                        for (; l < e.contextLocation()[0]; c++) {
                            if (source[c] == '\n') l++;
                        }

                        while (source[c] != '\n') {
                            contextLine += source[c];
                            c++;
                        }

                        cout << contextLine << endl;
                        cout << errorMarker + "\033[31m" + errorPoint + "\033[0m" << endl;
                    }
                }
            }
        }

    private:
        void validateIdentifier(Token token) {
            string disallowedIdentifierChars = "!@#$%^&*()-=+/<>{}[]|?.,`~";
            
            for (int i = 0; i < token.getText().length(); i++) {
                char identChar = tolower(token.getText()[i]);

                bool isDisallowedChar = find(disallowedIdentifierChars.begin(), disallowedIdentifierChars.end(), identChar) != disallowedIdentifierChars.end();
                bool isStartsWithDigit = i == 0 && isdigit(identChar);

                if (isStartsWithDigit || isDisallowedChar) {
                    throw SyntaxError(
                        "Invalid identifier \"" + token.getText() + "\"",
                        token.getStartLocation()
                    );
                }
            }
        }
};