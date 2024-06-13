#pragma once

#include <string>

using namespace std;

// Token types
namespace Tokens {
    // Literal Types
    const string STRING = "string";
    const string NUMBER = "number";
    const string BOOLEAN = "boolean";

    // Identifiers and Keywords
    const string KEYWORD = "keyword";
    const string IDENTIFIER = "identifier";

    // Comments
    const string COMMENT = "comment";
    const string MULTILINE_COMMENT = "multiline_comment";

    // Operators and Assignments
    const string OPERATOR = "operator";
    const string ASSIGNMENT = "assignment";
    const string FUNC_DECLARATION = "func_declaration";

    // Delimiters
    const string BRACE_OPEN = "brace_open";
    const string BRACE_CLOSE = "brace_close";
    const string PAREN_OPEN = "paren_open";
    const string PAREN_CLOSE = "paren_close";
    const string ANGLE_BRACKET_OPEN = "angle_bracket_open";
    const string ANGLE_BRACKET_CLOSE = "angle_bracket_close";
    const string BRACKET_OPEN = "bracket_open";
    const string BRACKET_CLOSE = "bracket_close";
    const string COMMA = "comma";
    const string COLON = "colon";

    // Whitespace
    const string NEWLINE = "newline";
    const string WHITESPACE = "whitespace";
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
}