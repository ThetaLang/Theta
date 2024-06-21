#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch2/catch_amalgamated.hpp"
#include "../src/lexer/lexer.cpp"

using namespace std;

void verifyTokens(deque<Token> &resultTokens, vector<pair<Token::Types, string>> &expectedTokens) {
    REQUIRE(resultTokens.size() == expectedTokens.size());

    for (int i = 0; i < expectedTokens.size(); i++) {
        REQUIRE(resultTokens[i].getType() == expectedTokens[i].first);
        REQUIRE(resultTokens[i].getLexeme() == expectedTokens[i].second);
    }
}

TEST_CASE("ThetaLexer") {
    ThetaLexer lexer;

    // --------- LITERALS ----------
    SECTION("Can tokenize numbers with decimals") {
        string source = "5 * 12.23";
        lexer.lex(source);

        vector<pair<Token::Types, string>> expectedTokens = {
            { Token::Types::NUMBER, "5" },
            { Token::Types::OPERATOR, Symbols::TIMES },
            { Token::Types::NUMBER, "12.23" }
        };

        verifyTokens(lexer.tokens, expectedTokens);
    }

    SECTION("Numbers dont have multiple decimals") {
        string source = "5100 / 10.23.4.";
        lexer.lex(source);

        vector<pair<Token::Types, string>> expectedTokens = {
            { Token::Types::NUMBER, "5100" },
            { Token::Types::OPERATOR, Symbols::DIVISION },
            { Token::Types::NUMBER, "10.23" },
            { Token::Types::IDENTIFIER, ".4." }
        };

        verifyTokens(lexer.tokens, expectedTokens);
    }

    SECTION("Can tokenize a string") {
        string source = "'And his name is John Cena!'";
        lexer.lex(source);

        vector<pair<Token::Types, string>> expectedTokens = {
            { Token::Types::STRING, "'And his name is John Cena!'" }
        };

        verifyTokens(lexer.tokens, expectedTokens);
    }
    // --------- LITERALS ----------

    // --------- DATA STRUCTURES -----------
    SECTION("Can tokenize a list of strings") {
        string source = "['Alex', 'Tony', 'John', 'Denis']";
        lexer.lex(source);

        vector<pair<Token::Types, string>> expectedTokens = {
            { Token::Types::BRACKET_OPEN, Symbols::BRACKET_OPEN },
            { Token::Types::STRING, "'Alex'" },
            { Token::Types::COMMA, Symbols::COMMA },
            { Token::Types::STRING, "'Tony'" },
            { Token::Types::COMMA, Symbols::COMMA },
            { Token::Types::STRING, "'John'" },
            { Token::Types::COMMA, Symbols::COMMA },
            { Token::Types::STRING, "'Denis'" },
            { Token::Types::BRACKET_CLOSE, Symbols::BRACKET_CLOSE }
        };

        verifyTokens(lexer.tokens, expectedTokens);
    }

    SECTION("Can tokenize a nested list of strings") {
        string source = "[['alex', 'john', ['jeremy', 'pablo']], ['clarinda'], 'den' + 'is', 'jessica', 'ellis']";
        lexer.lex(source);

        vector<pair<Token::Types, string>> expectedTokens = {
            { Token::Types::BRACKET_OPEN, Symbols::BRACKET_OPEN },
            { Token::Types::BRACKET_OPEN, Symbols::BRACKET_OPEN },
            { Token::Types::STRING, "'alex'" },
            { Token::Types::COMMA, Symbols::COMMA },
            { Token::Types::STRING, "'john'" },
            { Token::Types::COMMA, Symbols::COMMA },
            { Token::Types::BRACKET_OPEN, Symbols::BRACKET_OPEN },
            { Token::Types::STRING, "'jeremy'" },
            { Token::Types::COMMA, Symbols::COMMA },
            { Token::Types::STRING, "'pablo'" },
            { Token::Types::BRACKET_CLOSE, Symbols::BRACKET_CLOSE },
            { Token::Types::BRACKET_CLOSE, Symbols::BRACKET_CLOSE },
            { Token::Types::COMMA, Symbols::COMMA },
            { Token::Types::BRACKET_OPEN, Symbols::BRACKET_OPEN },
            { Token::Types::STRING, "'clarinda'" },
            { Token::Types::BRACKET_CLOSE, Symbols::BRACKET_CLOSE },
            { Token::Types::COMMA, Symbols::COMMA },
            { Token::Types::STRING, "'den'" },
            { Token::Types::OPERATOR, Symbols::PLUS },
            { Token::Types::STRING, "'is'" },
            { Token::Types::COMMA, Symbols::COMMA },
            { Token::Types::STRING, "'jessica'" },
            { Token::Types::COMMA, Symbols::COMMA },
            { Token::Types::STRING, "'ellis'" },
            { Token::Types::BRACKET_CLOSE, Symbols::BRACKET_CLOSE }
        };

        verifyTokens(lexer.tokens, expectedTokens);
    }

    // --------- DATA STRUCTURES -----------

    // --------- ARITHMETIC OPERATORS ----------
    SECTION("Can tokenize addition") {
        string source = "100 + 7";
        lexer.lex(source);

        vector<pair<Token::Types, string>> expectedTokens = {
            { Token::Types::NUMBER, "100" },
            { Token::Types::OPERATOR, Symbols::PLUS },
            { Token::Types::NUMBER, "7" }
        };

        verifyTokens(lexer.tokens, expectedTokens);
    }

    SECTION("Can tokenize subtraction") {
        string source = "42 - 11";
        lexer.lex(source);

        vector<pair<Token::Types, string>> expectedTokens = {
            { Token::Types::NUMBER, "42" },
            { Token::Types::OPERATOR, Symbols::MINUS },
            { Token::Types::NUMBER, "11" }
        };

        verifyTokens(lexer.tokens, expectedTokens);
    }

    SECTION("Can tokenize multiplication") {
        string source = "10 * 11";
        lexer.lex(source);

        std::vector<std::pair<Token::Types, std::string>> expectedTokens = {
            { Token::Types::NUMBER, "10" },
            { Token::Types::OPERATOR, Symbols::TIMES },
            { Token::Types::NUMBER, "11" }
        };

        verifyTokens(lexer.tokens, expectedTokens);
    }

    SECTION("Can tokenize division") {
        string source = "100/5";
        lexer.lex(source);

        std::vector<std::pair<Token::Types, std::string>> expectedTokens = {
            { Token::Types::NUMBER, "100" },
            { Token::Types::OPERATOR, Symbols::DIVISION },
            { Token::Types::NUMBER, "5" }
        };

        verifyTokens(lexer.tokens, expectedTokens);
    }

    SECTION("Can tokenize exponentiation") {
        string source = "2 ** 3";
        lexer.lex(source);

        std::vector<std::pair<Token::Types, std::string>> expectedTokens = {
            { Token::Types::NUMBER, "2" },
            { Token::Types::OPERATOR, Symbols::EXPOONENT },
            { Token::Types::NUMBER, "3" }
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
            { Token::Types::BOOLEAN, Symbols::TRUE },
            { Token::Types::OPERATOR, Symbols::AND },
            { Token::Types::OPERATOR, Symbols::NOT },
            { Token::Types::BOOLEAN, Symbols::FALSE }
        };

        verifyTokens(lexer.tokens, expectedTokens);
    }

    SECTION("Can tokenize boolean logic with identifiers") {
        string source = "!x || y";
        lexer.lex(source);

        std::vector<std::pair<Token::Types, std::string>> expectedTokens = {
            { Token::Types::OPERATOR, Symbols::NOT },
            { Token::Types::IDENTIFIER, "x" },
            { Token::Types::OPERATOR, Symbols::OR },
            { Token::Types::IDENTIFIER, "y" }
        };

        verifyTokens(lexer.tokens, expectedTokens);
    }
    // --------- BOOLEANS + BOOLEAN OPERATORS ----------

    // --------- ASSIGNMENT ----------
    SECTION("Can tokenize a string assignment") {
        string source = "message<String> = 'Hello, World!'";
        lexer.lex(source);

        std::vector<std::pair<Token::Types, std::string>> expectedTokens = {
            { Token::Types::IDENTIFIER, "message" },
            { Token::Types::OPERATOR, Symbols::LT },
            { Token::Types::IDENTIFIER, "String" },
            { Token::Types::OPERATOR, Symbols::GT },
            { Token::Types::ASSIGNMENT, Symbols::ASSIGNMENT },
            { Token::Types::STRING, "'Hello, World!'" }
        };

        verifyTokens(lexer.tokens, expectedTokens);
    }

    SECTION("Can tokenize boolean assignment") {
        string source = "isOpen<Boolean> = true";
        lexer.lex(source);

        std::vector<std::pair<Token::Types, std::string>> expectedTokens = {
            { Token::Types::IDENTIFIER, "isOpen" },
            { Token::Types::OPERATOR, Symbols::LT },
            { Token::Types::IDENTIFIER, "Boolean" },
            { Token::Types::OPERATOR, Symbols::GT },
            { Token::Types::ASSIGNMENT, Symbols::ASSIGNMENT },
            { Token::Types::BOOLEAN, Symbols::TRUE }
        };

        verifyTokens(lexer.tokens, expectedTokens);
    }

    SECTION("Can tokenize arithmetic assignment") {
        string source = "total<Number> = 5 + 3";
        lexer.lex(source);

        std::vector<std::pair<Token::Types, std::string>> expectedTokens = {
            { Token::Types::IDENTIFIER, "total" },
            { Token::Types::OPERATOR, Symbols::LT },
            { Token::Types::IDENTIFIER, "Number" },
            { Token::Types::OPERATOR, Symbols::GT },
            { Token::Types::ASSIGNMENT, Symbols::ASSIGNMENT },
            { Token::Types::NUMBER, "5" },
            { Token::Types::OPERATOR, Symbols::PLUS },
            { Token::Types::NUMBER, "3" }
        };

        verifyTokens(lexer.tokens, expectedTokens);
    }
    // --------- ASSIGNMENT ----------

    // --------- CONTROL FLOW ---------
    SECTION("Can tokenize if statement") {
        string source = "if (x == 10) { return true }";
        lexer.lex(source);

        std::vector<std::pair<Token::Types, std::string>> expectedTokens = {
            { Token::Types::KEYWORD, Symbols::IF },
            { Token::Types::PAREN_OPEN, Symbols::PAREN_OPEN },
            { Token::Types::IDENTIFIER, "x" },
            { Token::Types::OPERATOR, Symbols::EQUALITY },
            { Token::Types::NUMBER, "10" },
            { Token::Types::PAREN_CLOSE, Symbols::PAREN_CLOSE },
            { Token::Types::BRACE_OPEN, Symbols::BRACE_OPEN },
            { Token::Types::KEYWORD, Symbols::RETURN },
            { Token::Types::BOOLEAN, Symbols::TRUE },
            { Token::Types::BRACE_CLOSE, Symbols::BRACE_CLOSE }
        };

        verifyTokens(lexer.tokens, expectedTokens);
    }
    // --------- CONTROL FLOW ---------

    // --------- FUNCTIONS ------------
    SECTION("Can tokenize inline function definition") {
        string source = "greet<String> = name<String> -> 'hello' + name";
        lexer.lex(source);

        std::vector<std::pair<Token::Types, std::string>> expectedTokens = {
            { Token::Types::IDENTIFIER, "greet" },
            { Token::Types::OPERATOR, Symbols::LT },
            { Token::Types::IDENTIFIER, "String" },
            { Token::Types::OPERATOR, Symbols::GT },
            { Token::Types::ASSIGNMENT, Symbols::ASSIGNMENT },
            { Token::Types::IDENTIFIER, "name" },
            { Token::Types::OPERATOR, Symbols::LT },
            { Token::Types::IDENTIFIER, "String"},
            { Token::Types::OPERATOR, Symbols::GT },
            { Token::Types::FUNC_DECLARATION, Symbols::FUNC_DECLARATION },
            { Token::Types::STRING, "'hello'" },
            { Token::Types::OPERATOR, Symbols::PLUS },
            { Token::Types::IDENTIFIER, "name" }
        };

        verifyTokens(lexer.tokens, expectedTokens);
    }

    SECTION("Can tokenize function definition with multiple params") {
        string source = "greet<String> = greeting<String>, name<String> -> 'hello' + name";
        lexer.lex(source);

        std::vector<std::pair<Token::Types, std::string>> expectedTokens = {
            { Token::Types::IDENTIFIER, "greet" },
            { Token::Types::OPERATOR, Symbols::LT },
            { Token::Types::IDENTIFIER, "String" },
            { Token::Types::OPERATOR, Symbols::GT },
            { Token::Types::ASSIGNMENT, Symbols::ASSIGNMENT },
            { Token::Types::IDENTIFIER, "greeting" },
            { Token::Types::OPERATOR, Symbols::LT },
            { Token::Types::IDENTIFIER, "String"},
            { Token::Types::OPERATOR, Symbols::GT },
            { Token::Types::COMMA, Symbols::COMMA },
            { Token::Types::IDENTIFIER, "name" },
            { Token::Types::OPERATOR, Symbols::LT },
            { Token::Types::IDENTIFIER, "String"},
            { Token::Types::OPERATOR, Symbols::GT },
            { Token::Types::FUNC_DECLARATION, Symbols::FUNC_DECLARATION },
            { Token::Types::STRING, "'hello'" },
            { Token::Types::OPERATOR, Symbols::PLUS },
            { Token::Types::IDENTIFIER, "name" }
        };

        verifyTokens(lexer.tokens, expectedTokens);
    }

    SECTION("Can tokenize function definition with no params") {
        string source = "greet<String> = () -> 'hello there'";
        lexer.lex(source);

        std::vector<std::pair<Token::Types, std::string>> expectedTokens = {
            { Token::Types::IDENTIFIER, "greet" },
            { Token::Types::OPERATOR, Symbols::LT },
            { Token::Types::IDENTIFIER, "String" },
            { Token::Types::OPERATOR, Symbols::GT },
            { Token::Types::ASSIGNMENT, Symbols::ASSIGNMENT },
            { Token::Types::PAREN_OPEN, Symbols::PAREN_OPEN },
            { Token::Types::PAREN_CLOSE, Symbols::PAREN_CLOSE },
            { Token::Types::FUNC_DECLARATION, Symbols::FUNC_DECLARATION },
            { Token::Types::STRING, "'hello there'" }
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
            { Token::Types::IDENTIFIER, "greet" },
            { Token::Types::OPERATOR, Symbols::LT },
            { Token::Types::IDENTIFIER, "String" },
            { Token::Types::OPERATOR, Symbols::GT },
            { Token::Types::ASSIGNMENT, Symbols::ASSIGNMENT },
            { Token::Types::IDENTIFIER, "names" },
            { Token::Types::OPERATOR, Symbols::LT },
            { Token::Types::IDENTIFIER, "List"},
            { Token::Types::OPERATOR, Symbols::LT },
            { Token::Types::IDENTIFIER, "String" },
            { Token::Types::OPERATOR, Symbols::GT },
            { Token::Types::OPERATOR, Symbols::GT },
            { Token::Types::FUNC_DECLARATION, Symbols::FUNC_DECLARATION },
            { Token::Types::BRACE_OPEN, Symbols::BRACE_OPEN },
            { Token::Types::STRING, "'hello'" },
            { Token::Types::OPERATOR, Symbols::PLUS },
            { Token::Types::IDENTIFIER, "concat_strings" },
            { Token::Types::PAREN_OPEN, Symbols::PAREN_OPEN },
            { Token::Types::IDENTIFIER, "names" },
            { Token::Types::COMMA, Symbols::COMMA },
            { Token::Types::STRING, "', '" },
            { Token::Types::PAREN_CLOSE, Symbols::PAREN_CLOSE },
            { Token::Types::BRACE_CLOSE, Symbols::BRACE_CLOSE }
        };

        verifyTokens(lexer.tokens, expectedTokens);
    }
    // --------- FUNCTIONS ------------

    SECTION("Can tokenize with single line comments") {
        string source = "x<Number> = 5 + 3 // assignment";
        lexer.lex(source);

        std::vector<std::pair<Token::Types, std::string>> expectedTokens = {
            { Token::Types::IDENTIFIER, "x" },
            { Token::Types::OPERATOR, Symbols::LT },
            { Token::Types::IDENTIFIER, "Number" },
            { Token::Types::OPERATOR, Symbols::GT },
            { Token::Types::ASSIGNMENT, Symbols::ASSIGNMENT },
            { Token::Types::NUMBER, "5" },
            { Token::Types::OPERATOR, Symbols::PLUS },
            { Token::Types::NUMBER, "3" }
        };

        verifyTokens(lexer.tokens, expectedTokens);
    }

    SECTION("Can tokenize with multi line comments") {
        string source = "x<Number> = 5 - /- comment -/ 3";
        lexer.lex(source);

        std::vector<std::pair<Token::Types, std::string>> expectedTokens = {
            { Token::Types::IDENTIFIER, "x" },
            { Token::Types::OPERATOR, Symbols::LT },
            { Token::Types::IDENTIFIER, "Number" },
            { Token::Types::OPERATOR, Symbols::GT },
            { Token::Types::ASSIGNMENT, Symbols::ASSIGNMENT },
            { Token::Types::NUMBER, "5" },
            { Token::Types::OPERATOR, Symbols::MINUS },
            { Token::Types::NUMBER, "3" }
        };

        verifyTokens(lexer.tokens, expectedTokens);
    }

}
