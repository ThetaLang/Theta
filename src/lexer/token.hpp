#ifndef TOKEN_H
#define TOKEN_H

#import <string>
#import <vector>
#import <sstream>
#import "../util/tokens.hpp"

using namespace std;

class Token {
    private:
        string lexeme;
        int line;
        int column;
        Tokens type;

    public:
        Token();
        Token(Tokens tokenType, string tokenLexeme);

        Tokens getType();

        void setType(Tokens tokenType);

        string getLexeme();

        void setLexeme(string tokenText);

        void appendLexeme(char character);

        void appendLexeme(string appendableText);

        vector<int> getStartLocation();

        string getStartLocationString();

        void setStartLine(int start);

        void setStartColumn(int start);

        string toJSON();
};

#endif
