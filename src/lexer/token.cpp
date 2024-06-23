#include "Token.hpp"

Token::Token() {}

Token::Token(Token::Types tokenType, string tokenLexeme) {
    lexeme = tokenLexeme;
    type = tokenType;
}

Token::Types Token::getType() { return type; }

void Token::setType(Token::Types tokenType) { type = tokenType; }

string Token::getLexeme() { return lexeme; }

void Token::setLexeme(string tokenText) { lexeme = tokenText;  }

void Token::appendLexeme(char character) { lexeme += character; }

void Token::appendLexeme(string appendableText) { lexeme += appendableText; }

vector<int> Token::getStartLocation() { return { line, column }; }

string Token::getStartLocationString() { return "line " + to_string(line) + ", column " + to_string(column); }

void Token::setStartLine(int start) { line = start; }

void Token::setStartColumn(int start) { column = start; }

string Token::toJSON() {
    ostringstream oss;

    oss << "{";
    oss << " \"type\": \"" << tokenTypeToString(type) << "\"";
    oss << ", \"lexeme\": \"" << lexeme << "\"";
    oss << ", \"location\": \"" << getStartLocationString() << "\"";
    oss << " }";

    return oss.str();
}
