#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch2/catch_amalgamated.hpp"
#include "../src/lexer/Lexer.cpp"

using namespace std;
using namespace Theta;

void verifyTokens(deque<Token> &resultTokens, vector<pair<Token::Types, string>> &expectedTokens) {
    REQUIRE(resultTokens.size() == expectedTokens.size());

    for (int i = 0; i < expectedTokens.size(); i++) {
        REQUIRE(resultTokens[i].getType() == expectedTokens[i].first);
        REQUIRE(resultTokens[i].getLexeme() == expectedTokens[i].second);
    }
}

TEST_CASE("Lexer") {
    Theta::Lexer lexer;

    // --------- LITERALS ----------
    SECTION("Can tokenize numbers with decimals") {
        string source = "5 * 12.23";
        lexer.lex(source);

        vector<pair<Token::Types, string>> expectedTokens = {
            { Token::NUMBER, "5" },
            { Token::OPERATOR, Lexemes::TIMES },
            { Token::NUMBER, "12.23" }
        };

        verifyTokens(lexer.tokens, expectedTokens);
    }

    SECTION("Numbers dont have multiple decimals") {
        string source = "5100 / 10.23.4.";
        lexer.lex(source);

        vector<pair<Token::Types, string>> expectedTokens = {
            { Token::NUMBER, "5100" },
            { Token::OPERATOR, Lexemes::DIVISION },
            { Token::NUMBER, "10.23" },
            { Token::IDENTIFIER, ".4." }
        };

        verifyTokens(lexer.tokens, expectedTokens);
    }

    SECTION("Can tokenize a string") {
        string source = "'And his name is John Cena!'";
        lexer.lex(source);

        vector<pair<Token::Types, string>> expectedTokens = {
            { Token::STRING, "'And his name is John Cena!'" }
        };

        verifyTokens(lexer.tokens, expectedTokens);
    }
    // --------- LITERALS ----------

    // --------- DATA STRUCTURES -----------
    SECTION("Can tokenize a list of strings") {
        string source = "['Alex', 'Tony', 'John', 'Denis']";
        lexer.lex(source);

        vector<pair<Token::Types, string>> expectedTokens = {
            { Token::BRACKET_OPEN, Lexemes::BRACKET_OPEN },
            { Token::STRING, "'Alex'" },
            { Token::COMMA, Lexemes::COMMA },
            { Token::STRING, "'Tony'" },
            { Token::COMMA, Lexemes::COMMA },
            { Token::STRING, "'John'" },
            { Token::COMMA, Lexemes::COMMA },
            { Token::STRING, "'Denis'" },
            { Token::BRACKET_CLOSE, Lexemes::BRACKET_CLOSE }
        };

        verifyTokens(lexer.tokens, expectedTokens);
    }

    SECTION("Can tokenize a nested list of strings") {
        string source = "[['alex', 'john', ['jeremy', 'pablo']], ['clarinda'], 'den' + 'is', 'jessica', 'ellis']";
        lexer.lex(source);

        vector<pair<Token::Types, string>> expectedTokens = {
            { Token::BRACKET_OPEN, Lexemes::BRACKET_OPEN },
            { Token::BRACKET_OPEN, Lexemes::BRACKET_OPEN },
            { Token::STRING, "'alex'" },
            { Token::COMMA, Lexemes::COMMA },
            { Token::STRING, "'john'" },
            { Token::COMMA, Lexemes::COMMA },
            { Token::BRACKET_OPEN, Lexemes::BRACKET_OPEN },
            { Token::STRING, "'jeremy'" },
            { Token::COMMA, Lexemes::COMMA },
            { Token::STRING, "'pablo'" },
            { Token::BRACKET_CLOSE, Lexemes::BRACKET_CLOSE },
            { Token::BRACKET_CLOSE, Lexemes::BRACKET_CLOSE },
            { Token::COMMA, Lexemes::COMMA },
            { Token::BRACKET_OPEN, Lexemes::BRACKET_OPEN },
            { Token::STRING, "'clarinda'" },
            { Token::BRACKET_CLOSE, Lexemes::BRACKET_CLOSE },
            { Token::COMMA, Lexemes::COMMA },
            { Token::STRING, "'den'" },
            { Token::OPERATOR, Lexemes::PLUS },
            { Token::STRING, "'is'" },
            { Token::COMMA, Lexemes::COMMA },
            { Token::STRING, "'jessica'" },
            { Token::COMMA, Lexemes::COMMA },
            { Token::STRING, "'ellis'" },
            { Token::BRACKET_CLOSE, Lexemes::BRACKET_CLOSE }
        };

        verifyTokens(lexer.tokens, expectedTokens);
    }

    // --------- DATA STRUCTURES -----------

    // --------- ARITHMETIC OPERATORS ----------
    SECTION("Can tokenize addition") {
        string source = "100 + 7";
        lexer.lex(source);

        vector<pair<Token::Types, string>> expectedTokens = {
            { Token::NUMBER, "100" },
            { Token::OPERATOR, Lexemes::PLUS },
            { Token::NUMBER, "7" }
        };

        verifyTokens(lexer.tokens, expectedTokens);
    }

    SECTION("Can tokenize subtraction") {
        string source = "42 - 11";
        lexer.lex(source);

        vector<pair<Token::Types, string>> expectedTokens = {
            { Token::NUMBER, "42" },
            { Token::OPERATOR, Lexemes::MINUS },
            { Token::NUMBER, "11" }
        };

        verifyTokens(lexer.tokens, expectedTokens);
    }

    SECTION("Can tokenize multiplication") {
        string source = "10 * 11";
        lexer.lex(source);

        std::vector<std::pair<Token::Types, std::string>> expectedTokens = {
            { Token::NUMBER, "10" },
            { Token::OPERATOR, Lexemes::TIMES },
            { Token::NUMBER, "11" }
        };

        verifyTokens(lexer.tokens, expectedTokens);
    }

    SECTION("Can tokenize division") {
        string source = "100/5";
        lexer.lex(source);

        std::vector<std::pair<Token::Types, std::string>> expectedTokens = {
            { Token::NUMBER, "100" },
            { Token::OPERATOR, Lexemes::DIVISION },
            { Token::NUMBER, "5" }
        };

        verifyTokens(lexer.tokens, expectedTokens);
    }

    SECTION("Can tokenize exponentiation") {
        string source = "2 ** 3";
        lexer.lex(source);

        std::vector<std::pair<Token::Types, std::string>> expectedTokens = {
            { Token::NUMBER, "2" },
            { Token::OPERATOR, Lexemes::EXPONENT },
            { Token::NUMBER, "3" }
        };

        verifyTokens(lexer.tokens, expectedTokens);
    }

    SECTION("Can tokenize modulo") {
        string source = "9 % 2";
        lexer.lex(source);

        std::vector<std::pair<Token::Types, std::string>> expectedTokens = {
            { Token::NUMBER, "9" },
            { Token::OPERATOR, Lexemes::MODULO },
            { Token::NUMBER, "2" }
        };

        verifyTokens(lexer.tokens, expectedTokens);
    }
    // --------- ARITHMETIC OPERATORS ----------

    // --------- BOOLEANS + BOOLEAN OPERATORS ----------
    SECTION("Can tokenize boolean logic") {
        string source = "true && !false";
        lexer.lex(source);

        REQUIRE(lexer.tokens.size() == 4);

        std::vector<std::pair<Token::Types, std::string>> expectedTokens = {
            { Token::BOOLEAN, Lexemes::TRUE },
            { Token::OPERATOR, Lexemes::AND },
            { Token::OPERATOR, Lexemes::NOT },
            { Token::BOOLEAN, Lexemes::FALSE }
        };

        verifyTokens(lexer.tokens, expectedTokens);
    }

    SECTION("Can tokenize boolean logic with identifiers") {
        string source = "!x || y";
        lexer.lex(source);

        std::vector<std::pair<Token::Types, std::string>> expectedTokens = {
            { Token::OPERATOR, Lexemes::NOT },
            { Token::IDENTIFIER, "x" },
            { Token::OPERATOR, Lexemes::OR },
            { Token::IDENTIFIER, "y" }
        };

        verifyTokens(lexer.tokens, expectedTokens);
    }
    // --------- BOOLEANS + BOOLEAN OPERATORS ----------

    // --------- ASSIGNMENT ----------
    SECTION("Can tokenize a string assignment") {
        string source = "message<String> = 'Hello, World!'";
        lexer.lex(source);

        std::vector<std::pair<Token::Types, std::string>> expectedTokens = {
            { Token::IDENTIFIER, "message" },
            { Token::OPERATOR, Lexemes::LT },
            { Token::IDENTIFIER, "String" },
            { Token::OPERATOR, Lexemes::GT },
            { Token::ASSIGNMENT, Lexemes::ASSIGNMENT },
            { Token::STRING, "'Hello, World!'" }
        };

        verifyTokens(lexer.tokens, expectedTokens);
    }

    SECTION("Can tokenize boolean assignment") {
        string source = "isOpen<Boolean> = true";
        lexer.lex(source);

        std::vector<std::pair<Token::Types, std::string>> expectedTokens = {
            { Token::IDENTIFIER, "isOpen" },
            { Token::OPERATOR, Lexemes::LT },
            { Token::IDENTIFIER, "Boolean" },
            { Token::OPERATOR, Lexemes::GT },
            { Token::ASSIGNMENT, Lexemes::ASSIGNMENT },
            { Token::BOOLEAN, Lexemes::TRUE }
        };

        verifyTokens(lexer.tokens, expectedTokens);
    }

    SECTION("Can tokenize arithmetic assignment") {
        string source = "total<Number> = 5 + 3";
        lexer.lex(source);

        std::vector<std::pair<Token::Types, std::string>> expectedTokens = {
            { Token::IDENTIFIER, "total" },
            { Token::OPERATOR, Lexemes::LT },
            { Token::IDENTIFIER, "Number" },
            { Token::OPERATOR, Lexemes::GT },
            { Token::ASSIGNMENT, Lexemes::ASSIGNMENT },
            { Token::NUMBER, "5" },
            { Token::OPERATOR, Lexemes::PLUS },
            { Token::NUMBER, "3" }
        };

        verifyTokens(lexer.tokens, expectedTokens);
    }
    // --------- ASSIGNMENT ----------

    // --------- CONTROL FLOW ---------
    SECTION("Can tokenize if statement") {
        string source = "if (x == 10) { return true }";
        lexer.lex(source);

        std::vector<std::pair<Token::Types, std::string>> expectedTokens = {
            { Token::KEYWORD, Lexemes::IF },
            { Token::PAREN_OPEN, Lexemes::PAREN_OPEN },
            { Token::IDENTIFIER, "x" },
            { Token::OPERATOR, Lexemes::EQUALITY },
            { Token::NUMBER, "10" },
            { Token::PAREN_CLOSE, Lexemes::PAREN_CLOSE },
            { Token::BRACE_OPEN, Lexemes::BRACE_OPEN },
            { Token::KEYWORD, Lexemes::RETURN },
            { Token::BOOLEAN, Lexemes::TRUE },
            { Token::BRACE_CLOSE, Lexemes::BRACE_CLOSE }
        };

        verifyTokens(lexer.tokens, expectedTokens);
    }
    // --------- CONTROL FLOW ---------

    // --------- FUNCTIONS ------------
    SECTION("Can tokenize inline function definition") {
        string source = "greet<String> = name<String> -> 'hello' + name";
        lexer.lex(source);

        std::vector<std::pair<Token::Types, std::string>> expectedTokens = {
            { Token::IDENTIFIER, "greet" },
            { Token::OPERATOR, Lexemes::LT },
            { Token::IDENTIFIER, "String" },
            { Token::OPERATOR, Lexemes::GT },
            { Token::ASSIGNMENT, Lexemes::ASSIGNMENT },
            { Token::IDENTIFIER, "name" },
            { Token::OPERATOR, Lexemes::LT },
            { Token::IDENTIFIER, "String"},
            { Token::OPERATOR, Lexemes::GT },
            { Token::FUNC_DECLARATION, Lexemes::FUNC_DECLARATION },
            { Token::STRING, "'hello'" },
            { Token::OPERATOR, Lexemes::PLUS },
            { Token::IDENTIFIER, "name" }
        };

        verifyTokens(lexer.tokens, expectedTokens);
    }

    SECTION("Can tokenize function definition with multiple params") {
        string source = "greet<String> = greeting<String>, name<String> -> 'hello' + name";
        lexer.lex(source);

        std::vector<std::pair<Token::Types, std::string>> expectedTokens = {
            { Token::IDENTIFIER, "greet" },
            { Token::OPERATOR, Lexemes::LT },
            { Token::IDENTIFIER, "String" },
            { Token::OPERATOR, Lexemes::GT },
            { Token::ASSIGNMENT, Lexemes::ASSIGNMENT },
            { Token::IDENTIFIER, "greeting" },
            { Token::OPERATOR, Lexemes::LT },
            { Token::IDENTIFIER, "String"},
            { Token::OPERATOR, Lexemes::GT },
            { Token::COMMA, Lexemes::COMMA },
            { Token::IDENTIFIER, "name" },
            { Token::OPERATOR, Lexemes::LT },
            { Token::IDENTIFIER, "String"},
            { Token::OPERATOR, Lexemes::GT },
            { Token::FUNC_DECLARATION, Lexemes::FUNC_DECLARATION },
            { Token::STRING, "'hello'" },
            { Token::OPERATOR, Lexemes::PLUS },
            { Token::IDENTIFIER, "name" }
        };

        verifyTokens(lexer.tokens, expectedTokens);
    }

    SECTION("Can tokenize function definition with no params") {
        string source = "greet<String> = () -> 'hello there'";
        lexer.lex(source);

        std::vector<std::pair<Token::Types, std::string>> expectedTokens = {
            { Token::IDENTIFIER, "greet" },
            { Token::OPERATOR, Lexemes::LT },
            { Token::IDENTIFIER, "String" },
            { Token::OPERATOR, Lexemes::GT },
            { Token::ASSIGNMENT, Lexemes::ASSIGNMENT },
            { Token::PAREN_OPEN, Lexemes::PAREN_OPEN },
            { Token::PAREN_CLOSE, Lexemes::PAREN_CLOSE },
            { Token::FUNC_DECLARATION, Lexemes::FUNC_DECLARATION },
            { Token::STRING, "'hello there'" }
        };

        verifyTokens(lexer.tokens, expectedTokens);
    }

    SECTION("Can tokenize multiline function definition") {
        string source =
            "greet<String> = names<List<String>> -> { "
            "    'hello' + concat_strings(names, ', ')"
            "}";

        lexer.lex(source);

        std::vector<std::pair<Token::Types, std::string>> expectedTokens = {
            { Token::IDENTIFIER, "greet" },
            { Token::OPERATOR, Lexemes::LT },
            { Token::IDENTIFIER, "String" },
            { Token::OPERATOR, Lexemes::GT },
            { Token::ASSIGNMENT, Lexemes::ASSIGNMENT },
            { Token::IDENTIFIER, "names" },
            { Token::OPERATOR, Lexemes::LT },
            { Token::IDENTIFIER, "List"},
            { Token::OPERATOR, Lexemes::LT },
            { Token::IDENTIFIER, "String" },
            { Token::OPERATOR, Lexemes::GT },
            { Token::OPERATOR, Lexemes::GT },
            { Token::FUNC_DECLARATION, Lexemes::FUNC_DECLARATION },
            { Token::BRACE_OPEN, Lexemes::BRACE_OPEN },
            { Token::STRING, "'hello'" },
            { Token::OPERATOR, Lexemes::PLUS },
            { Token::IDENTIFIER, "concat_strings" },
            { Token::PAREN_OPEN, Lexemes::PAREN_OPEN },
            { Token::IDENTIFIER, "names" },
            { Token::COMMA, Lexemes::COMMA },
            { Token::STRING, "', '" },
            { Token::PAREN_CLOSE, Lexemes::PAREN_CLOSE },
            { Token::BRACE_CLOSE, Lexemes::BRACE_CLOSE }
        };

        verifyTokens(lexer.tokens, expectedTokens);
    }
    // --------- FUNCTIONS ------------

    SECTION("Can tokenize with single line comments") {
        string source = "x<Number> = 5 + 3 // assignment";
        lexer.lex(source);

        std::vector<std::pair<Token::Types, std::string>> expectedTokens = {
            { Token::IDENTIFIER, "x" },
            { Token::OPERATOR, Lexemes::LT },
            { Token::IDENTIFIER, "Number" },
            { Token::OPERATOR, Lexemes::GT },
            { Token::ASSIGNMENT, Lexemes::ASSIGNMENT },
            { Token::NUMBER, "5" },
            { Token::OPERATOR, Lexemes::PLUS },
            { Token::NUMBER, "3" }
        };

        verifyTokens(lexer.tokens, expectedTokens);
    }

    SECTION("Can tokenize with multi line comments") {
        string source = "x<Number> = 5 - /- comment -/ 3";
        lexer.lex(source);

        std::vector<std::pair<Token::Types, std::string>> expectedTokens = {
            { Token::IDENTIFIER, "x" },
            { Token::OPERATOR, Lexemes::LT },
            { Token::IDENTIFIER, "Number" },
            { Token::OPERATOR, Lexemes::GT },
            { Token::ASSIGNMENT, Lexemes::ASSIGNMENT },
            { Token::NUMBER, "5" },
            { Token::OPERATOR, Lexemes::MINUS },
            { Token::NUMBER, "3" }
        };

        verifyTokens(lexer.tokens, expectedTokens);
    }

}
