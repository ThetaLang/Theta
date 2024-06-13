#ifndef TOKEN_H
#define TOKEN_H

#import <string>
#import <vector>
#import <sstream>

using namespace std;

class Token {
    private:
        string text;
        int line;
        int column;
        string type;

    public:
        Token();
        Token(string tokenType, string tokenText);

        string getType();

        void setType(string tokenType);

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