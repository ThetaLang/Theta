#ifndef TOKEN_H
#define TOKEN_H

#import <string>
#import <vector>
#import <map>

using namespace std;

namespace Theta {
    class Token {
        public:
            enum Types {
                // Literal Types
                STRING,
                NUMBER,
                BOOLEAN,

                // Identifiers and Keywords
                KEYWORD,
                IDENTIFIER,

                // Comments
                COMMENT,
                MULTILINE_COMMENT,

                // Operators and Assignments
                OPERATOR,
                ASSIGNMENT,
                FUNC_DECLARATION,

                // Delimiters
                BRACE_OPEN,
                BRACE_CLOSE,
                PAREN_OPEN,
                PAREN_CLOSE,
                ANGLE_BRACKET_OPEN,
                ANGLE_BRACKET_CLOSE,
                BRACKET_OPEN,
                BRACKET_CLOSE,
                COMMA,
                COLON,
                AT,

                // Whitespace
                NEWLINE,
                WHITESPACE,

                UNHANDLED
            };

            Token();
            Token(Token::Types tokenType, string tokenLexeme);

            Token::Types getType();

            void setType(Token::Types tokenType);

            string getLexeme();

            void setLexeme(string tokenText);

            void appendLexeme(char character);

            void appendLexeme(string appendableText);

            vector<int> getStartLocation();

            string getStartLocationString();

            void setStartLine(int start);

            void setStartColumn(int start);

            string toJSON();

            static string tokenTypeToString(Token::Types token) {
                static map<Token::Types, string> tokensMap = {
                    { Token::STRING, "STRING" },
                    { Token::NUMBER, "NUMBER" },
                    { Token::BOOLEAN, "BOOLEAN" },
                    { Token::KEYWORD, "KEYWORD" },
                    { Token::IDENTIFIER, "IDENTIFIER" },
                    { Token::COMMENT, "COMMENT" },
                    { Token::MULTILINE_COMMENT, "MULTILINE_COMMENT" },
                    { Token::OPERATOR, "OPERATOR" },
                    { Token::ASSIGNMENT, "ASSIGNMENT" },
                    { Token::FUNC_DECLARATION, "FUNC_DECLARATION" },
                    { Token::BRACE_OPEN, "BRACE_OPEN" },
                    { Token::BRACE_CLOSE, "BRACE_CLOSE" },
                    { Token::PAREN_OPEN, "PAREN_OPEN" },
                    { Token::PAREN_CLOSE, "PAREN_CLOSE" },
                    { Token::ANGLE_BRACKET_OPEN, "ANGLE_BRACKET_OPEN" },
                    { Token::ANGLE_BRACKET_CLOSE, "ANGLE_BRACKET_CLOSE" },
                    { Token::BRACKET_OPEN, "BRACKET_OPEN" },
                    { Token::BRACKET_CLOSE, "BRACKET_CLOSE" },
                    { Token::COMMA, "COMMA" },
                    { Token::COLON, "COLON" },
                    { Token::AT, "AT" },
                    { Token::NEWLINE, "NEWLINE" },
                    { Token::WHITESPACE, "WHITESPACE" },
                    { Token::UNHANDLED, "UNHANDLED" }
                };

                auto it = tokensMap.find(token);
                if (it != tokensMap.end()) {
                    return it->second;
                } else {
                    return "UNKNOWN";
                }
            }

        private:
            string lexeme;
            int line;
            int column;
            Token::Types type;
    };
}

#endif
