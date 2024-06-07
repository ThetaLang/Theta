#ifndef TOKEN_H
#define TOKEN_H

#import <string>
#import <vector>

using namespace std;

class Token {
    private:
        string text;
        int line;
        int column;
        string type;

    public:
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
};

#endif