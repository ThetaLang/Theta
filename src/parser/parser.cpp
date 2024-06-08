#include <vector>
#include <string>
#include <iostream>
#include "../lexer/token.hpp"
#include "../util/exceptions.hpp"

using namespace std;

class ThetaParser {
    public:
        void parse(vector<Token> tokens, string source, string fileName) {
            for (int i = 0; i <= tokens.size(); i++) {
                if (tokens[i].getType() == "identifier") {
                    try {
                        validateIdentifier(tokens[i]);
                    } catch (SyntaxError &e) {
                        ExceptionFormatter::displayFormattedError("SyntaxError", e, source, fileName, tokens[i]);
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