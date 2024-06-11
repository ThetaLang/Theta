#include <vector>
#include <deque>
#include <string>
#include <iostream>
#include <algorithm>
#include "token.hpp"

using namespace std;

class ThetaLexer {
    public:
        deque<Token> tokens = {};
        
        void lex(string source) {
            int i = 0;

            // Iterate over the whole source
            while (i < source.length()) {
                char currentChar = source[i];
                char nextChar = source[i + 1];

                // We want the line and column numbers for the beginning of the token, not the end. If we were to use
                // currentLine and currentColumn, it would give us the values for the end of the token, since some of
                // the makeToken function calls actually update currentLine and currentColumn while they iterate over
                // the token contents
                int lineAtLexStart = currentLine;
                int columnAtLexStart = currentColumn;

                Token newToken = makeToken(currentChar, nextChar, source, i);

                // We don't actually want to keep any whitespace related tokens
                if (newToken.getType() != "newline" && newToken.getType() != "whitespace") {
                    newToken.setStartLine(lineAtLexStart);
                    newToken.setStartColumn(columnAtLexStart);
                    tokens.push_back(newToken);
                }

                // Some tokens are more than one character. We want to advance the iterator and currentLine by however many
                // characters long the token was. We really only want to do this for any token that was not created by an
                // accumulateUntil function, since those already update the index internally.
                vector<string> accumulatedTokens = {
                    "identifier", "keyword", "boolean", "comment", "multiline_comment", "string", "number"
                };
                
                if (find(accumulatedTokens.begin(), accumulatedTokens.end(), newToken.getType()) == accumulatedTokens.end()) {
                    i += newToken.getText().length();
                    currentColumn += newToken.getText().length();
                } else if (newToken.getType() == "multiline_comment") {
                    i += 2;
                    currentColumn += 2;
                } else {
                    i++;
                    currentColumn++;
                }
            }
        }

    private:
        int currentLine = 1;
        int currentColumn = 1;

        Token makeToken(char currentChar, char nextChar, string source, int &i) {
            if (currentChar == '\'') return accumulateUntilNext("'", source, i, Token("string", "'"), "'");
            else if (currentChar == '/' && nextChar == '/') return accumulateUntilNext("\n", source, i, Token("comment", "/"), "", false);
            else if (currentChar == '/' && nextChar == '-') return accumulateUntilNext("-/", source, i, Token("multiline_comment", "/-"), "-/");
            else if (currentChar == '/') return Token("operator", "/");
            else if (currentChar == '=' && nextChar == '=')  return Token("operator", "==");
            else if (currentChar == '=' && nextChar == '>') return Token("operator", "=>");
            else if (currentChar == '=') return Token("assignment", "=");
            else if (currentChar == '+' && nextChar == '=') return Token("operator", "+=");
            else if (currentChar == '+') return Token("operator", "+");
            else if (currentChar == '-' && nextChar == '=') return Token("operator", "-=");
            else if (currentChar == '-' && nextChar == '>') return Token("operator", "->");
            else if (currentChar == '-') return Token("operator", "-");
            else if (currentChar == '*' && nextChar == '=') return Token("operator", "*=");
            else if (currentChar == '*' && nextChar == '*') return Token("operator", "**");
            else if (currentChar == '*') return Token("operator", "*");
            else if (currentChar == '{') return Token("brace_open", "{");
            else if (currentChar == '}') return Token("brace_close", "}");
            else if (currentChar == '(') return Token("paren_open", "(");
            else if (currentChar == ')') return Token("paren_close", ")");
            else if (currentChar == '<') return Token("angle_bracket_open", "<");
            else if (currentChar == '>') return Token("angle_bracket_close", ">");
            else if (currentChar == '[') return Token("bracket_open", "[");
            else if (currentChar == ']') return Token("bracket_close", "]");
            else if (currentChar == ',') return Token("comma", ",");
            else if (currentChar == ':') return Token("colon", ":");
            else if (currentChar == '\n') {
                currentLine += 1;
                currentColumn = 0;
                return Token("newline", "\n");
            } else if (isdigit(currentChar) && (isdigit(nextChar) || nextChar == '.' || isspace(nextChar) || !nextChar)) {
                return accumulateUntilCondition(
                    [source](int idx) { return isdigit(source[idx]) || source[idx] == '.'; },
                    source,
                    i,
                    Token("number", { currentChar }),
                    "",
                    false
                );
            } else if (!isspace(currentChar)) {
                // We default this to an identifier, but then change it later if we discover its actually a keyword or bool
                Token token = accumulateUntilAnyOf(
                    " <>=/\\!?@#$%^&*()~`|,.-+{}[]'\";:\n\r",
                    source,
                    i,
                    Token("identifier", { currentChar }),
                    "",
                    false
                );

                if (isLanguageKeyword(token.getText())) {
                    token.setType("keyword");
                } else if (token.getText() == "true" || token.getText() == "false") {
                    token.setType("boolean");
                }

                return token;
            } else if (isspace(currentChar)) {
                return Token("whitespace", { currentChar });
            } else {
                cout << "UNHANDLED CHAR: " << currentChar << " \n";
                return Token("unhandled", { currentChar });
            }
        }

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

            // Just collect characters until we hit our end condition
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
            // go back to processing the endChar we just stopped on, because it's going to be part of another token.
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