#pragma once

#include <string>

using namespace std;

namespace Theta {
    namespace Lexemes {
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
        const string EXPONENT = "**";
        const string MODULO = "%";

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
        const string INEQUALITY = "!=";
        const string LT = "<";
        const string GT = ">";
        const string LTEQ = "<=";
        const string GTEQ = ">=";

        // Function Declaration
        const string FUNC_DECLARATION = "->";
        const string PIPE = "=>";

        // Language keywords
        const string LINK = "link";
        const string CAPSULE = "capsule";
        const string IF = "if";
        const string ELSE = "else";
        const string STRUCT = "struct";
        const string ENUM = "enum";
        const string RETURN = "return";
        const string TRUE = "true";
        const string FALSE = "false";
        const string AT = "@";
    }
}
