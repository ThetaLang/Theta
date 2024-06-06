#import <string>

using namespace std;

class Token {
    private:
        string text;
        int startLocation;
        string type;

    public:
        Token(int tokenStartLocation, string tokenType, string tokenText) {
            text = tokenText;
            startLocation = tokenStartLocation;
            type = tokenType;
        }

        string getType() { return type; }

        void setType(string tokenType) { type = tokenType; }

        string getText() { return text; }

        void setText(string tokenText) { text = tokenText;  }

        void appendText(char character) { text += character; }

        void appendText(string appendableText) { text += appendableText; }

        int getStartLocation() { return startLocation; }

        void setStartLocation(int tokenStartLocation) { startLocation = tokenStartLocation; }
};