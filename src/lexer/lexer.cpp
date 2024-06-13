#include <vector>
#include <deque>
#include <string>
#include <iostream>
#include <algorithm>
#include "token.hpp"
#include "../util/tokens.hpp"

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
                if (newToken.getType() != Tokens::NEWLINE && newToken.getType() != Tokens::WHITESPACE) {
                    newToken.setStartLine(lineAtLexStart);
                    newToken.setStartColumn(columnAtLexStart);
                    tokens.push_back(newToken);
                }

                // Some tokens are more than one character. We want to advance the iterator and currentLine by however many
                // characters long the token was. We really only want to do this for any token that was not created by an
                // accumulateUntil function, since those already update the index internally.
                vector<string> accumulatedTokens = {
                    Tokens::IDENTIFIER,
                    Tokens::KEYWORD,
                    Tokens::BOOLEAN,
                    Tokens::COMMENT,
                    Tokens::MULTILINE_COMMENT,
                    Tokens::STRING,
                    Tokens::NUMBER
                };
                
                if (find(accumulatedTokens.begin(), accumulatedTokens.end(), newToken.getType()) == accumulatedTokens.end()) {
                    i += newToken.getText().length();
                    currentColumn += newToken.getText().length();
                } else if (newToken.getType() == Tokens::MULTILINE_COMMENT) {
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
            if (currentChar == '\'') return accumulateUntilNext("'", source, i, Token(Tokens::STRING, Symbols::STRING_DELIMITER), Symbols::STRING_DELIMITER);
            else if (currentChar == '/' && nextChar == '/') return accumulateUntilNext("\n", source, i, Token(Tokens::COMMENT, "/"), "", false);
            else if (currentChar == '/' && nextChar == '-') return accumulateUntilNext("-/", source, i, Token(Tokens::MULTILINE_COMMENT, Symbols::MULTILINE_COMMENT_DELIMITER_START), Symbols::MULTILINE_COMMENT_DELIMITER_END);
            else if (currentChar == '/') return Token(Tokens::OPERATOR, Symbols::DIVISION);
            else if (currentChar == '=' && nextChar == '=')  return Token(Tokens::OPERATOR, Symbols::EQUALITY);
            else if (currentChar == '=' && nextChar == '>') return Token(Tokens::OPERATOR, Symbols::PIPE);
            else if (currentChar == '=') return Token(Tokens::ASSIGNMENT, Symbols::ASSIGNMENT);
            else if (currentChar == '+' && nextChar == '=') return Token(Tokens::OPERATOR, Symbols::PLUS_EQUALS);
            else if (currentChar == '+') return Token(Tokens::OPERATOR, Symbols::PLUS);
            else if (currentChar == '-' && nextChar == '=') return Token(Tokens::OPERATOR, Symbols::MINUS_EQUALS);
            else if (currentChar == '-' && nextChar == '>') return Token(Tokens::FUNC_DECLARATION, Symbols::FUNC_DECLARATION);
            else if (currentChar == '-') return Token(Tokens::OPERATOR, Symbols::MINUS);
            else if (currentChar == '*' && nextChar == '=') return Token(Tokens::OPERATOR, Symbols::TIMES_EQUALS);
            else if (currentChar == '*' && nextChar == '*') return Token(Tokens::OPERATOR, Symbols::POWER);
            else if (currentChar == '*') return Token(Tokens::OPERATOR, Symbols::TIMES);
            else if (currentChar == '{') return Token(Tokens::BRACE_OPEN, Symbols::BRACE_OPEN);
            else if (currentChar == '}') return Token(Tokens::BRACE_CLOSE, Symbols::BRACE_CLOSE);
            else if (currentChar == '(') return Token(Tokens::PAREN_OPEN, Symbols::PAREN_OPEN);
            else if (currentChar == ')') return Token(Tokens::PAREN_CLOSE, Symbols::PAREN_CLOSE);
            else if (currentChar == '<') return Token(Tokens::ANGLE_BRACKET_OPEN, Symbols::ANGLE_BRACKET_OPEN);
            else if (currentChar == '>') return Token(Tokens::ANGLE_BRACKET_CLOSE, Symbols::ANGLE_BRACKET_CLOSE);
            else if (currentChar == '[') return Token(Tokens::BRACKET_OPEN, Symbols::BRACKET_OPEN);
            else if (currentChar == ']') return Token(Tokens::BRACKET_CLOSE, Symbols::BRACKET_CLOSE);
            else if (currentChar == ',') return Token(Tokens::COMMA, Symbols::COMMA);
            else if (currentChar == ':') return Token(Tokens::COLON, Symbols::COLON);
            else if (currentChar == '\n') {
                currentLine += 1;
                currentColumn = 0;
                return Token(Tokens::NEWLINE, Symbols::NEWLINE);
            } else if (isdigit(currentChar) && (isdigit(nextChar) || isspace(nextChar) || nextChar == ']' || nextChar == ')' || nextChar == '}' || nextChar == '.')) {
                return accumulateUntilCondition(
                    [source](int idx) { return isdigit(source[idx]) || source[idx] == '.'; },
                    source,
                    i,
                    Token(Tokens::NUMBER, { currentChar }),
                    "",
                    false
                );
            } else if (!isspace(currentChar)) {
                // We default this to an identifier, but then change it later if we discover its actually a keyword or bool
                Token token = accumulateUntilAnyOf(
                    " <>=/\\!?@#$%^&*()~`|,-+{}[]'\";:\n\r",
                    source,
                    i,
                    Token(Tokens::IDENTIFIER, { currentChar }),
                    "",
                    false
                );

                if (isLanguageKeyword(token.getText())) {
                    token.setType(Tokens::KEYWORD);
                } else if (token.getText() == "true" || token.getText() == "false") {
                    token.setType(Tokens::BOOLEAN);
                }

                return token;
            } else if (isspace(currentChar)) {
                return Token(Tokens::WHITESPACE, { currentChar });
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