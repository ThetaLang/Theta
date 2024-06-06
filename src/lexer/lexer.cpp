#include <vector>
#include <string>
#include <iostream>
#include "token.cpp"

using namespace std;

class ThetaLexer {
    public:
        vector<Token> tokens = {};
        
        vector<Token> lex(string source) {
            int i = 0;

            // Iterate over the whole source
            while (i < source.length()) {
                char currentChar = source[i];
                char nextChar = source[i + 1];

                if (currentChar == '\'') {
                    tokens.push_back(accumulateUntilAnyOf("'", source, i, Token(currentLine, currentColumn, "string", "'"), "'"));
                } else if (currentChar == '<') {
                    tokens.push_back(accumulateUntilAnyOf(">", source, i, Token(currentLine, currentColumn, "type", "<"), ">"));
                } else if (currentChar == '/' && nextChar == '/') {
                    tokens.push_back(accumulateUntilAnyOf("\n", source, i, Token(currentLine, currentColumn, "comment", "/")));
                } else if (currentChar == '/' && nextChar == '-') {
                    tokens.push_back(accumulateUntilNext("-/", source, i, Token(currentLine, currentColumn, "comment", "/-"), "-/"));
                    i++;
                    currentColumn++;
                } else if (currentChar == '=' && nextChar == '=') {
                    tokens.push_back(Token(currentLine, currentColumn, "operator", "=="));
                    i++;
                    currentColumn++;
                } else if (currentChar == '=' && nextChar == '>') {
                    tokens.push_back(Token(currentLine, currentColumn, "operator", "=>"));
                    i++;
                    currentColumn++;
                } else if (currentChar == '=') {
                    tokens.push_back(Token(currentLine, currentColumn, "operator", "="));
                } else if (currentChar == '+' && nextChar == '=') {
                    tokens.push_back(Token(currentLine, currentColumn, "operator", "+="));
                    i++;
                    currentColumn++;
                } else if (currentChar == '+') {
                    tokens.push_back(Token(currentLine, currentColumn, "operator", "+"));
                } else if (currentChar == '-' && nextChar == '=') {
                    tokens.push_back(Token(currentLine, currentColumn, "operator", "-="));
                    i++;
                    currentColumn++;
                } else if (currentChar == '-' && nextChar == '>') {
                    tokens.push_back(Token(currentLine, currentColumn, "keyword", "->"));
                    i++;
                    currentColumn++;
                } else if (currentChar == '-') {
                    tokens.push_back(Token(currentLine, currentColumn, "operator", "-"));
                } else if (currentChar == '{') {
                    tokens.push_back(Token(currentLine, currentColumn, "brace", "{"));
                } else if (currentChar == '}') {
                    tokens.push_back(Token(currentLine, currentColumn, "brace", "}"));
                } else if (currentChar == ',') {
                    tokens.push_back(Token(currentLine, currentColumn, "comma", ","));
                } else if (currentChar == ':') {
                    tokens.push_back(Token(currentLine, currentColumn, "colon", ":"));
                } else if (currentChar == '\n') {
                    currentLine += 1;
                    currentColumn = 0;
                } else if (!isspace(currentChar)) {
                    // We default this to an identifier, but then change it later if we discover its actually a keyword or bool
                    Token genericToken = accumulateUntilAnyOf(
                        " <>=/\\!?@#$%^&*()~`|,.-+{}[]'\";:\n\r",
                        source,
                        i,
                        Token(currentLine, currentColumn, "identifier", { currentChar }),
                        "",
                        false
                    );

                    if (isLanguageKeyword(genericToken.getText())) {
                        genericToken.setType("keyword");
                    } else if (genericToken.getText() == "true" || genericToken.getText() == "false") {
                        genericToken.setType("boolean");
                    }

                    tokens.push_back(genericToken);
                }
                   
                i++;
                currentColumn++;
            }

            return tokens;
        }

    private:
        int currentLine = 1;
        int currentColumn = 1;

        Token accumulateUntilNext(string endChars, string source, int &i, Token token, string append = "", bool incrementAfter = true) {
            return accumulateUntilCondition(
                [endChars, source](int i) { return source.substr(i, endChars.length()) != endChars; },
                source,
                i,
                token,
                append,
                incrementAfter
            );
        }

        Token accumulateUntilAnyOf(string endChars, string source, int &i, Token token, string append = "", bool incrementAfter = true) {
            return accumulateUntilCondition(
                [endChars, source](int i) { return find(endChars.begin(), endChars.end(), source[i]) == endChars.end(); },
                source,
                i,
                token,
                append,
                incrementAfter
            );
        }

        Token accumulateUntilCondition(function<bool(int)> shouldContinue, string source, int &i, Token token, string append = "", bool incrementAfter = true) {
            // We need to jump forward one index because we're already on the start char
            i++;
            currentColumn++;

            // Just collect characters until we hit an endChar
            for (; i < source.length() && shouldContinue(i); i++) {
                token.appendText(source[i]);

                // We might hit newlines in multiline comments and strings. We need to keep line and column numbers correct
                if (source[i] == '\n') {
                    currentLine++;
                    currentColumn = 0;
                }

                currentColumn++;
            }

            token.appendText(append);

            // If incrementAfter is false, that means we want to roll back the index to the point right before we hit an endChar.
            // This is probably because the token we're parsing isn't a token thats enclosed in delimiters, so we want to
            // go back to processing the encChar we just stopped on, because it's going to be part of another token.
            if (!incrementAfter) {
                i--;
                currentColumn--;
            }

            return token;
        }

        bool isLanguageKeyword(string text) {
            vector<string> keywords = {
                "link", "capsule", "if", "else", "struct", "return"
            };

            return find(keywords.begin(), keywords.end(), text) != keywords.end();
        }
};