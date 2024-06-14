#ifndef TOKEN_H
#define TOKEN_H

#import <string>
#import <vector>
#import <sstream>
#import "../util/tokens.hpp"

using namespace std;

class Token {
    private:
        string text;
        int line;
        int column;
        Tokens type;

    public:
        Token();
        Token(Tokens tokenType, string tokenText);

        Tokens getType();

        void setType(Tokens tokenType);

        string getText();

        void setText(string tokenText);

        void appendText(char character);

        void appendText(string appendableText);

        vector<int> getStartLocation();

        string getStartLocationString();

        void setStartLine(int start);

        void setStartColumn(int start);

        string toJSON();
};

#endif