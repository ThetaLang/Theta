#include "Token.hpp"
#include <sstream>

Theta::Token::Token() {}

Theta::Token::Token(Token::Types tokenType, string tokenLexeme) {
    lexeme = tokenLexeme;
    type = tokenType;
}

Theta::Token::Types Theta::Token::getType() { return type; }

void Theta::Token::setType(Theta::Token::Types tokenType) { type = tokenType; }

string Theta::Token::getLexeme() { return lexeme; }

void Theta::Token::setLexeme(string tokenText) { lexeme = tokenText;  }

void Theta::Token::appendLexeme(char character) { lexeme += character; }

void Theta::Token::appendLexeme(string appendableText) { lexeme += appendableText; }

vector<int> Theta::Token::getStartLocation() { return { line, column }; }

string Theta::Token::getStartLocationString() { return "line " + to_string(line) + ", column " + to_string(column); }

void Theta::Token::setStartLine(int start) { line = start; }

void Theta::Token::setStartColumn(int start) { column = start; }

string Theta::Token::toJSON() {
    ostringstream oss;

    oss << "{";
    oss << " \"type\": \"" << tokenTypeToString(type) << "\"";
    oss << ", \"lexeme\": \"" << lexeme << "\"";
    oss << ", \"location\": \"" << getStartLocationString() << "\"";
    oss << " }";

    return oss.str();
}
