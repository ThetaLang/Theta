#ifndef TOKEN_H
#define TOKEN_H

#import <string>
#import <vector>
#import <sstream>
#import <map>

using namespace std;

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
                { Token::Types::STRING, "STRING" },
                { Token::Types::NUMBER, "NUMBER" },
                { Token::Types::BOOLEAN, "BOOLEAN" },
                { Token::Types::KEYWORD, "KEYWORD" },
                { Token::Types::IDENTIFIER, "IDENTIFIER" },
                { Token::Types::COMMENT, "COMMENT" },
                { Token::Types::MULTILINE_COMMENT, "MULTILINE_COMMENT" },
                { Token::Types::OPERATOR, "OPERATOR" },
                { Token::Types::ASSIGNMENT, "ASSIGNMENT" },
                { Token::Types::FUNC_DECLARATION, "FUNC_DECLARATION" },
                { Token::Types::BRACE_OPEN, "BRACE_OPEN" },
                { Token::Types::BRACE_CLOSE, "BRACE_CLOSE" },
                { Token::Types::PAREN_OPEN, "PAREN_OPEN" },
                { Token::Types::PAREN_CLOSE, "PAREN_CLOSE" },
                { Token::Types::ANGLE_BRACKET_OPEN, "ANGLE_BRACKET_OPEN" },
                { Token::Types::ANGLE_BRACKET_CLOSE, "ANGLE_BRACKET_CLOSE" },
                { Token::Types::BRACKET_OPEN, "BRACKET_OPEN" },
                { Token::Types::BRACKET_CLOSE, "BRACKET_CLOSE" },
                { Token::Types::COMMA, "COMMA" },
                { Token::Types::COLON, "COLON" },
                { Token::Types::AT, "AT" },
                { Token::Types::NEWLINE, "NEWLINE" },
                { Token::Types::WHITESPACE, "WHITESPACE" },
                { Token::Types::UNHANDLED, "UNHANDLED" }
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

#endif
