#pragma once

#include <string>
#include <map>

using namespace std;

// Token types
enum class Tokens {
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

    // Whitespace
    NEWLINE,
    WHITESPACE,

    UNHANDLED
};

inline string tokenTypeToString(Tokens token) {
    static map<Tokens, string> tokensMap = {
        { Tokens::STRING, "STRING" },
        { Tokens::NUMBER, "NUMBER" },
        { Tokens::BOOLEAN, "BOOLEAN" },
        { Tokens::KEYWORD, "KEYWORD" },
        { Tokens::IDENTIFIER, "IDENTIFIER" },
        { Tokens::COMMENT, "COMMENT" },
        { Tokens::MULTILINE_COMMENT, "MULTILINE_COMMENT" },
        { Tokens::OPERATOR, "OPERATOR" },
        { Tokens::ASSIGNMENT, "ASSIGNMENT" },
        { Tokens::FUNC_DECLARATION, "FUNC_DECLARATION" },
        { Tokens::BRACE_OPEN, "BRACE_OPEN" },
        { Tokens::BRACE_CLOSE, "BRACE_CLOSE" },
        { Tokens::PAREN_OPEN, "PAREN_OPEN" },
        { Tokens::PAREN_CLOSE, "PAREN_CLOSE" },
        { Tokens::ANGLE_BRACKET_OPEN, "ANGLE_BRACKET_OPEN" },
        { Tokens::ANGLE_BRACKET_CLOSE, "ANGLE_BRACKET_CLOSE" },
        { Tokens::BRACKET_OPEN, "BRACKET_OPEN" },
        { Tokens::BRACKET_CLOSE, "BRACKET_CLOSE" },
        { Tokens::COMMA, "COMMA" },
        { Tokens::COLON, "COLON" },
        { Tokens::NEWLINE, "NEWLINE" },
        { Tokens::WHITESPACE, "WHITESPACE" },
        { Tokens::UNHANDLED, "UNHANDLED" }
    };

    auto it = tokensMap.find(token);
    if (it != tokensMap.end()) {
        return it->second;
    } else {
        return "UNKNOWN";
    }
}

namespace Symbols {
    // String Delimiters
    const string STRING_DELIMITER = "'";

    // Comment Delimiters
    const string COMMENT = "//";
    const string MULTILINE_COMMENT_DELIMITER_START = "/-";
    const string MULTILINE_COMMENT_DELIMITER_END = "-/";

    // Brackets
    const string BRACE_OPEN = "{";
    const string BRACE_CLOSE = "}";
    const string PAREN_OPEN = "(";
    const string PAREN_CLOSE = ")";
    const string ANGLE_BRACKET_OPEN = "<";
    const string ANGLE_BRACKET_CLOSE = ">";
    const string BRACKET_OPEN = "[";
    const string BRACKET_CLOSE = "]";

    // Punctuation
    const string COMMA = ",";
    const string COLON = ":";
    const string NEWLINE = "\n";

    // Arithmetic Operators
    const string DIVISION = "/";
    const string PLUS = "+";
    const string MINUS = "-";
    const string TIMES = "*";
    const string POWER = "**";

    // Boolean Operators
    const string AND = "&&";
    const string OR = "||";
    const string NOT = "!";

    // Assignment Operators
    const string ASSIGNMENT = "=";
    const string PLUS_EQUALS = "+=";
    const string MINUS_EQUALS = "-=";
    const string TIMES_EQUALS = "*=";

    // Comparison Operators
    const string EQUALITY = "==";

    // Function Declaration
    const string FUNC_DECLARATION = "->";
    const string PIPE = "=>";

    // Language keywords
    const string LINK = "link";
    const string CAPSULE = "capsule";
    const string IF = "if";
    const string ELSE = "else";
    const string STRUCT = "struct";
    const string RETURN = "return";
    const string TRUE = "true";
    const string FALSE = "false";
}