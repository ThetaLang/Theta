#include <vector>
#include <deque>
#include <string>
#include <iostream>
#include <algorithm>
#include "token.hpp"
#include "../util/tokens.hpp"

using namespace std;

/**
 * @class ThetaLexer
 * @brief A lexer for the Theta programming language that tokenizes source code into a deque of Token objects.
 */
class ThetaLexer {
    public:
        deque<Token> tokens = {};

        /**
         * @brief Tokenizes the given source code string.
         * @param source The source code to lex.
         */
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

                // We don't actually want to keep any whitespace related tokens or comments
                vector<Tokens> garbageTokens = { Tokens::NEWLINE, Tokens::WHITESPACE, Tokens::COMMENT, Tokens::MULTILINE_COMMENT };

                if (find(garbageTokens.begin(), garbageTokens.end(), newToken.getType()) == garbageTokens.end()) {
                    newToken.setStartLine(lineAtLexStart);
                    newToken.setStartColumn(columnAtLexStart);
                    tokens.push_back(newToken);
                }

                // Some tokens are more than one character. We want to advance the iterator and currentLine by however many
                // characters long the token was. We really only want to do this for any token that was not created by an
                // accumulateUntil function, since those already update the index internally.
                vector<Tokens> accumulatedTokens = {
                    Tokens::IDENTIFIER,
                    Tokens::KEYWORD,
                    Tokens::BOOLEAN,
                    Tokens::COMMENT,
                    Tokens::MULTILINE_COMMENT,
                    Tokens::STRING,
                    Tokens::NUMBER
                };

                if (find(accumulatedTokens.begin(), accumulatedTokens.end(), newToken.getType()) == accumulatedTokens.end()) {
                    i += newToken.getLexeme().length();
                    currentColumn += newToken.getLexeme().length();
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

        /**
         * @brief Creates a Token object based on the current and next characters in the source code.
         * @param currentChar The current character being processed.
         * @param nextChar The next character to be processed.
         * @param source The source code string.
         * @param i The current index in the source string.
         * @return The generated Token object.
         */
        Token makeToken(char currentChar, char nextChar, string source, int &i) {
            Token token;

            // Order matters here to ensure correct tokenization precedence.
            // We attempt to lex each potential token type in a specific order of priority.
            // Each attemptLex function call checks if the current and possibly next character
            // match the expected symbol(s) for the token type. If a match is found, it processes
            // and returns the corresponding token. If no match is found, it returns false, and
            // the next attemptLex in the sequence is checked.
            //
            // This sequential checking ensures that tokens with longer symbol representations
            // (e.g., multi-character operators) are prioritized over shorter ones, preventing
            // misinterpretations and ensuring correct tokenization of the source code.
            if (
                attemptLex(Symbols::STRING_DELIMITER, Tokens::STRING, token, currentChar, nextChar, source, i, Symbols::STRING_DELIMITER) ||
                attemptLex(Symbols::COMMENT, Tokens::COMMENT, token, currentChar, nextChar, source, i, Symbols::NEWLINE, false, false) ||
                attemptLex(Symbols::MULTILINE_COMMENT_DELIMITER_START, Tokens::MULTILINE_COMMENT, token, currentChar, nextChar, source, i, Symbols::MULTILINE_COMMENT_DELIMITER_END) ||
                attemptLex(Symbols::DIVISION, Tokens::OPERATOR, token, currentChar, nextChar, source, i) ||
                attemptLex(Symbols::EQUALITY, Tokens::OPERATOR, token, currentChar, nextChar, source, i) ||
                attemptLex(Symbols::INEQUALITY, Tokens::OPERATOR, token, currentChar, nextChar, source, i) ||
                attemptLex(Symbols::AND, Tokens::OPERATOR, token, currentChar, nextChar, source, i) ||
                attemptLex(Symbols::OR, Tokens::OPERATOR, token, currentChar, nextChar, source, i) ||
                attemptLex(Symbols::NOT, Tokens::OPERATOR, token, currentChar, nextChar, source, i) ||
                attemptLex(Symbols::PIPE, Tokens::OPERATOR, token, currentChar, nextChar, source, i) ||
                attemptLex(Symbols::ASSIGNMENT, Tokens::ASSIGNMENT, token, currentChar, nextChar, source, i) ||
                attemptLex(Symbols::PLUS_EQUALS, Tokens::OPERATOR, token, currentChar, nextChar, source, i) ||
                attemptLex(Symbols::PLUS, Tokens::OPERATOR, token, currentChar, nextChar, source, i) ||
                attemptLex(Symbols::MINUS_EQUALS, Tokens::OPERATOR, token, currentChar, nextChar, source, i) ||
                attemptLex(Symbols::FUNC_DECLARATION, Tokens::FUNC_DECLARATION, token, currentChar, nextChar, source, i) ||
                attemptLex(Symbols::MINUS, Tokens::OPERATOR, token, currentChar, nextChar, source, i) ||
                attemptLex(Symbols::TIMES_EQUALS, Tokens::OPERATOR, token, currentChar, nextChar, source, i) ||
                attemptLex(Symbols::POWER, Tokens::OPERATOR, token, currentChar, nextChar, source, i) ||
                attemptLex(Symbols::TIMES, Tokens::OPERATOR, token, currentChar, nextChar, source, i) ||
                attemptLex(Symbols::BRACE_OPEN, Tokens::BRACE_OPEN, token, currentChar, nextChar, source, i) ||
                attemptLex(Symbols::BRACE_CLOSE, Tokens::BRACE_CLOSE, token, currentChar, nextChar, source, i) ||
                attemptLex(Symbols::PAREN_OPEN, Tokens::PAREN_OPEN, token, currentChar, nextChar, source, i) ||
                attemptLex(Symbols::PAREN_CLOSE, Tokens::PAREN_CLOSE, token, currentChar, nextChar, source, i) ||
                attemptLex(Symbols::LTEQ, Tokens::OPERATOR, token, currentChar, nextChar, source, i) ||
                attemptLex(Symbols::LT, Tokens::OPERATOR, token, currentChar, nextChar, source, i) ||
                attemptLex(Symbols::GTEQ, Tokens::OPERATOR, token, currentChar, nextChar, source, i) ||
                attemptLex(Symbols::GT, Tokens::OPERATOR, token, currentChar, nextChar, source, i) ||
                attemptLex(Symbols::BRACKET_OPEN, Tokens::BRACKET_OPEN, token, currentChar, nextChar, source, i) ||
                attemptLex(Symbols::BRACKET_CLOSE, Tokens::BRACKET_CLOSE, token, currentChar, nextChar, source, i) ||
                attemptLex(Symbols::COMMA, Tokens::COMMA, token, currentChar, nextChar, source, i) ||
                attemptLex(Symbols::COLON, Tokens::COLON, token, currentChar, nextChar, source, i)
            ) {
                return token;
            }

            if (currentChar == '\n') {
                currentLine += 1;
                currentColumn = 0;
                return Token(Tokens::NEWLINE, Symbols::NEWLINE);
            } else if (isdigit(currentChar) && ((isdigit(nextChar) || isspace(nextChar) || nextChar == ']' || nextChar == ')' || nextChar == '}' || nextChar == '.' || nextChar == '\0' || nextChar == ',') || nextChar == ':')) {
                int countDecimals = 0;
                return accumulateUntilCondition(
                    [source, &countDecimals](int idx) {
                        if (source[idx] == '.') {
                            countDecimals++;
                        }

                        return isdigit(source[idx]) || (countDecimals <= 1 && source[idx] == '.');
                    },
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

                if (isLanguageKeyword(token.getLexeme())) {
                    token.setType(Tokens::KEYWORD);
                } else if (token.getLexeme() == "true" || token.getLexeme() == "false") {
                    token.setType(Tokens::BOOLEAN);
                }

                return token;
            } else if (isspace(currentChar)) {
                return Token(Tokens::WHITESPACE, { currentChar });
            } else {
                cout << "UNHANDLED CHAR: " << currentChar << " \n";
                return Token(Tokens::UNHANDLED, { currentChar });
            }
        }

        /**
         * @brief Attempts to lex a token from the source code based on the given symbol and token type.
         * @param symbol The symbol representing the token.
         * @param tokenType The type of token to create.
         * @param token The token object to update.
         * @param currentChar The current character being processed.
         * @param nextChar The next character to be processed.
         * @param source The source code string.
         * @param i The current index in the source string.
         * @param terminal The terminal character string to stop at (default is an empty string).
         * @param appendSymbol Whether to append the symbol to the token text (default is true).
         * @param incrementAfter Whether to increment the index after lexing (default is true).
         * @return True if the token was successfully lexed, false otherwise.
         */
        bool attemptLex(
            const string &symbol,
            Tokens tokenType,
            Token &token,
            char currentChar,
            char nextChar,
            const string &source,
            int& i,
            string terminal = "",
            bool appendSymbol = true,
            bool incrementAfter = true
        ) {
            if (currentChar == symbol[0] && (symbol.length() == 1 || nextChar == symbol[1])) {
                if (terminal != "") {
                    // With tokens like comment and multiline_comment, this will format them weirdly
                    // in the token text field, because the combination of an accumulator and a symbol
                    // that is different from the prefix we start at causes some weird insertion to happen.
                    // We could add extra logic to fix it, but we throw out comments anyway right after they are lexed,
                    // so its probably not worth the extra code to do so. Our line numbers and columns are still correct
                    // and the lexing otherwise succeeds. However, if we ever come across a token that is *not* a
                    // comment in the future that follows the same weird capture logic, we'll need to fix it.
                    token = accumulateUntilNext(
                        terminal,
                        source,
                        i,
                        Token(tokenType, symbol),
                        appendSymbol ? symbol : "",
                        incrementAfter
                    );
                } else {
                    token = Token(tokenType, symbol);
                }
                return true;
            }
            return false;
        }

        /**
         * @brief Accumulates characters from the source until all of the specified end characters are encountered as a substring.
         * @param endChars A string containing characters that mark the end of accumulation.
         * @param source The source code string to lex.
         * @param i The current index in the source string.
         * @param token The Token object to update with accumulated characters.
         * @param append Additional text to append to the accumulated token text (default is an empty string).
         * @param incrementAfter Whether to increment the index after accumulation (default is true).
         * @return The updated Token object after accumulation.
         */
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

        /**
         * @brief Accumulates characters from the source until any character from the specified endChars string is encountered.
         * @param endChars A string containing characters that mark the end of accumulation.
         * @param source The source code string to lex.
         * @param i The current index in the source string.
         * @param token The Token object to update with accumulated characters.
         * @param append Additional text to append to the accumulated token text (default is an empty string).
         * @param incrementAfter Whether to increment the index after accumulation (default is true).
         * @return The updated Token object after accumulation.
         */
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

        /**
         * @brief Generalized accumulation function that continues accumulating characters as long as the provided condition function returns true.
         * @param shouldContinue A function that takes an integer index and returns true if accumulation should continue.
         * @param source The source code string to lex.
         * @param i The current index in the source string.
         * @param token The Token object to update with accumulated characters.
         * @param append Additional text to append to the accumulated token text (default is an empty string).
         * @param incrementAfter Whether to increment the index after accumulation (default is true).
         * @return The updated Token object after accumulation.
         */
        Token accumulateUntilCondition(function<bool(int)> shouldContinue, string source, int &i, Token token, string append = "", bool incrementAfter = true) {
            // We need to jump forward one index because we're already on the start char
            i++;
            currentColumn++;

            // Just collect characters until we hit our end condition
            for (; i < source.length() && shouldContinue(i); i++) {
                token.appendLexeme(source[i]);

                // We might hit newlines in multiline comments and strings. We need to keep line and column numbers correct
                if (source[i] == '\n') {
                    currentLine++;
                    currentColumn = 0;
                }

                currentColumn++;
            }

            token.appendLexeme(append);

            // If incrementAfter is false, that means we want to roll back the index to the point right before we hit an endChar.
            // This is probably because the token we're parsing isn't a token thats enclosed in delimiters, so we want to
            // go back to processing the endChar we just stopped on, because it's going to be part of another token.
            if (!incrementAfter) {
                i--;
                currentColumn--;
            }

            return token;
        }

        /**
         * @brief Checks if a given text is a keyword in the Theta programming language.
         * @param text The text to check.
         * @return True if the text is a keyword, false otherwise.
         */
        bool isLanguageKeyword(string text) {
            vector<string> keywords = {
                Symbols::LINK,
                Symbols::CAPSULE,
                Symbols::IF,
                Symbols::ELSE,
                Symbols::STRUCT,
                Symbols::RETURN
            };

            return find(keywords.begin(), keywords.end(), text) != keywords.end();
        }
};
