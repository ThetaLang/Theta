#include "token.hpp"

Token::Token(string tokenType, string tokenText) {
    text = tokenText;
    type = tokenType;
}

string Token::getType() { return type; }

void Token::setType(string tokenType) { type = tokenType; }

string Token::getText() { return text; }

void Token::setText(string tokenText) { text = tokenText;  }

void Token::appendText(char character) { text += character; }

void Token::appendText(string appendableText) { text += appendableText; }

vector<int> Token::getStartLocation() { return { line, column }; }

string Token::getStartLocationString() { return "line " + to_string(line) + ", column " + to_string(column); }

void Token::setStartLine(int start) { line = start; }

void Token::setStartColumn(int start) { column = start; }

string Token::toJSON() {
    ostringstream oss;

    oss << "{";
    oss << "\"type\": \"" << type << "\"";
    oss << ",\"text\": \"" << text << "\"";
    oss << ",\"location\": \"" << getStartLocationString() << "\"";
    oss << "}";

    return oss.str();
    // "{ type: " + lexer.tokens[i].getType() + ", text: " + lexer.tokens[i].getText() + ", location: " + lexer.tokens[i].getStartLocationString() + " }\n";
}