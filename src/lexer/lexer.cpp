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
            while (i <= source.length()) {
                char currentChar = source[i];
                char nextChar = source[i + 1];

                if (currentChar == '\'') {
                    tokens.push_back(accumulateUntilNext("'", source, i, Token(i, "string", "'"), "'"));
                } else if (currentChar == '<') {
                    tokens.push_back(accumulateUntilNext(">", source, i, Token(i, "type", "<"), ">"));
                } else if (currentChar == '=' && nextChar == '=') {
                    tokens.push_back(Token(i, "eq", "=="));
                    i++;
                } else if (currentChar == '=' && nextChar == '>') {
                    tokens.push_back(Token(i, "pipe", "=>"));
                    i++;
                } else if (currentChar == '=') {
                    tokens.push_back(Token(i, "assign", "="));
                } else if (currentChar == '+' && nextChar == '=') {
                    tokens.push_back(Token(i, "increment_assign", "+="));
                    i++;
                } else if (currentChar == '+') {
                    tokens.push_back(Token(i, "plus", "+"));
                } else if (currentChar == '-' && nextChar == '-') {
                    tokens.push_back(Token(i, "decrement_assign", "-="));
                    i++;
                } else if (currentChar == '-') {
                    tokens.push_back(Token(i, "minus", "-"));
                } else if (currentChar == '{') {
                    tokens.push_back(Token(i, "block_open", "{"));
                } else if (currentChar == '}') {
                    tokens.push_back(Token(i, "block_close", "}"));
                } else if (!isspace(currentChar)) {
                    tokens.push_back(accumulateUntilNext(
                        " <>=/\\!?@#$%^&*()~`|,.-+{}[]'\";:",
                        source,
                        i,
                        Token(i, "identifier", { currentChar }),
                        "",
                        false
                    ));
                }
                   
                i++;
            }

            return tokens;
        }

    private:
        // We take in a reference to the iterator thats being used in the main lexation loop.
        // This way we can increase the iterator position during out sub-lexation function here
        Token accumulateUntilNext(string endChars, string source, int &i, Token token, string append = "", bool incrementAfter = true) {
            // We need to jump forward one index because we're already on the start char
            i++;

            // Just collect characters until we hit an endChar
            for (; i <= source.length() && (find(endChars.begin(), endChars.end(), source[i]) == endChars.end()); i++) {
                token.appendText(source[i]);
            }

            token.appendText(append);

            // If incrementAfter is false, that means we want to roll back the index to the point right before we hit an endChar.
            // This is probably because the token we're parsing isn't a token thats enclosed in delimiters, so we want to
            // go back to processing the encChar we just stopped on, because it's going to be part of another token.
            if (!incrementAfter) {
                i--;
            }

            return token;
        }
};