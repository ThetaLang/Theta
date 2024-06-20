#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch2/catch_amalgamated.hpp"
#include "../src/lexer/lexer.cpp"

using namespace std;

void verifyTokens(deque<Token> &resultTokens, vector<pair<Tokens, string>> &expectedTokens) {
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

        vector<pair<Tokens, string>> expectedTokens = {
            { Tokens::NUMBER, "5" },
            { Tokens::OPERATOR, Symbols::TIMES },
            { Tokens::NUMBER, "12.23" }
        };

        verifyTokens(lexer.tokens, expectedTokens);
    }

    SECTION("Numbers dont have multiple decimals") {
        string source = "5100 / 10.23.4.";
        lexer.lex(source);

        vector<pair<Tokens, string>> expectedTokens = {
            { Tokens::NUMBER, "5100" },
            { Tokens::OPERATOR, Symbols::DIVISION },
            { Tokens::NUMBER, "10.23" },
            { Tokens::IDENTIFIER, ".4." }
        };

        verifyTokens(lexer.tokens, expectedTokens);
    }

    SECTION("Can tokenize a string") {
        string source = "'And his name is John Cena!'";
        lexer.lex(source);

        vector<pair<Tokens, string>> expectedTokens = {
            { Tokens::STRING, "'And his name is John Cena!'" }
        };

        verifyTokens(lexer.tokens, expectedTokens);
    }
    // --------- LITERALS ----------

    // --------- DATA STRUCTURES -----------
    SECTION("Can tokenize a list of strings") {
        string source = "['Alex', 'Tony', 'John', 'Denis']";
        lexer.lex(source);

        vector<pair<Tokens, string>> expectedTokens = {
            { Tokens::BRACKET_OPEN, Symbols::BRACKET_OPEN },
            { Tokens::STRING, "'Alex'" },
            { Tokens::COMMA, Symbols::COMMA },
            { Tokens::STRING, "'Tony'" },
            { Tokens::COMMA, Symbols::COMMA },
            { Tokens::STRING, "'John'" },
            { Tokens::COMMA, Symbols::COMMA },
            { Tokens::STRING, "'Denis'" },
            { Tokens::BRACKET_CLOSE, Symbols::BRACKET_CLOSE }
        };

        verifyTokens(lexer.tokens, expectedTokens);
    }

    SECTION("Can tokenize a nested list of strings") {
        string source = "[['alex', 'john', ['jeremy', 'pablo']], ['clarinda'], 'den' + 'is', 'jessica', 'ellis']";
        lexer.lex(source);

        vector<pair<Tokens, string>> expectedTokens = {
            { Tokens::BRACKET_OPEN, Symbols::BRACKET_OPEN },
            { Tokens::BRACKET_OPEN, Symbols::BRACKET_OPEN },
            { Tokens::STRING, "'alex'" },
            { Tokens::COMMA, Symbols::COMMA },
            { Tokens::STRING, "'john'" },
            { Tokens::COMMA, Symbols::COMMA },
            { Tokens::BRACKET_OPEN, Symbols::BRACKET_OPEN },
            { Tokens::STRING, "'jeremy'" },
            { Tokens::COMMA, Symbols::COMMA },
            { Tokens::STRING, "'pablo'" },
            { Tokens::BRACKET_CLOSE, Symbols::BRACKET_CLOSE },
            { Tokens::BRACKET_CLOSE, Symbols::BRACKET_CLOSE },
            { Tokens::COMMA, Symbols::COMMA },
            { Tokens::BRACKET_OPEN, Symbols::BRACKET_OPEN },
            { Tokens::STRING, "'clarinda'" },
            { Tokens::BRACKET_CLOSE, Symbols::BRACKET_CLOSE },
            { Tokens::COMMA, Symbols::COMMA },
            { Tokens::STRING, "'den'" },
            { Tokens::OPERATOR, Symbols::PLUS },
            { Tokens::STRING, "'is'" },
            { Tokens::COMMA, Symbols::COMMA },
            { Tokens::STRING, "'jessica'" },
            { Tokens::COMMA, Symbols::COMMA },
            { Tokens::STRING, "'ellis'" },
            { Tokens::BRACKET_CLOSE, Symbols::BRACKET_CLOSE }
        };

        verifyTokens(lexer.tokens, expectedTokens);
    }

    // --------- DATA STRUCTURES -----------

    // --------- ARITHMETIC OPERATORS ----------
    SECTION("Can tokenize addition") {
        string source = "100 + 7";
        lexer.lex(source);

        vector<pair<Tokens, string>> expectedTokens = {
            { Tokens::NUMBER, "100" },
            { Tokens::OPERATOR, Symbols::PLUS },
            { Tokens::NUMBER, "7" }
        };

        verifyTokens(lexer.tokens, expectedTokens);
    }

    SECTION("Can tokenize subtraction") {
        string source = "42 - 11";
        lexer.lex(source);

        vector<pair<Tokens, string>> expectedTokens = {
            { Tokens::NUMBER, "42" },
            { Tokens::OPERATOR, Symbols::MINUS },
            { Tokens::NUMBER, "11" }
        };

        verifyTokens(lexer.tokens, expectedTokens);
    }

    SECTION("Can tokenize multiplication") {
        string source = "10 * 11";
        lexer.lex(source);

        std::vector<std::pair<Tokens, std::string>> expectedTokens = {
            { Tokens::NUMBER, "10" },
            { Tokens::OPERATOR, Symbols::TIMES },
            { Tokens::NUMBER, "11" }
        };

        verifyTokens(lexer.tokens, expectedTokens);
    }

    SECTION("Can tokenize division") {
        string source = "100/5";
        lexer.lex(source);

        std::vector<std::pair<Tokens, std::string>> expectedTokens = {
            { Tokens::NUMBER, "100" },
            { Tokens::OPERATOR, Symbols::DIVISION },
            { Tokens::NUMBER, "5" }
        };

        verifyTokens(lexer.tokens, expectedTokens);
    }

    SECTION("Can tokenize exponentiation") {
        string source = "2 ** 3";
        lexer.lex(source);

        std::vector<std::pair<Tokens, std::string>> expectedTokens = {
            { Tokens::NUMBER, "2" },
            { Tokens::OPERATOR, Symbols::POWER },
            { Tokens::NUMBER, "3" }
        };

        verifyTokens(lexer.tokens, expectedTokens);
    }
    // --------- ARITHMETIC OPERATORS ----------

    // --------- BOOLEANS + BOOLEAN OPERATORS ----------
    SECTION("Can tokenize boolean logic") {
        string source = "true && !false";
        lexer.lex(source);

        REQUIRE(lexer.tokens.size() == 4);

        std::vector<std::pair<Tokens, std::string>> expectedTokens = {
            { Tokens::BOOLEAN, Symbols::TRUE },
            { Tokens::OPERATOR, Symbols::AND },
            { Tokens::OPERATOR, Symbols::NOT },
            { Tokens::BOOLEAN, Symbols::FALSE }
        };

        verifyTokens(lexer.tokens, expectedTokens);
    }

    SECTION("Can tokenize boolean logic with identifiers") {
        string source = "!x || y";
        lexer.lex(source);

        std::vector<std::pair<Tokens, std::string>> expectedTokens = {
            { Tokens::OPERATOR, Symbols::NOT },
            { Tokens::IDENTIFIER, "x" },
            { Tokens::OPERATOR, Symbols::OR },
            { Tokens::IDENTIFIER, "y" }
        };

        verifyTokens(lexer.tokens, expectedTokens);
    }
    // --------- BOOLEANS + BOOLEAN OPERATORS ----------

    // --------- ASSIGNMENT ----------
    SECTION("Can tokenize a string assignment") {
        string source = "message<String> = 'Hello, World!'";
        lexer.lex(source);

        std::vector<std::pair<Tokens, std::string>> expectedTokens = {
            { Tokens::IDENTIFIER, "message" },
            { Tokens::OPERATOR, Symbols::LT },
            { Tokens::IDENTIFIER, "String" },
            { Tokens::OPERATOR, Symbols::GT },
            { Tokens::ASSIGNMENT, Symbols::ASSIGNMENT },
            { Tokens::STRING, "'Hello, World!'" }
        };

        verifyTokens(lexer.tokens, expectedTokens);
    }

    SECTION("Can tokenize boolean assignment") {
        string source = "isOpen<Boolean> = true";
        lexer.lex(source);

        std::vector<std::pair<Tokens, std::string>> expectedTokens = {
            { Tokens::IDENTIFIER, "isOpen" },
            { Tokens::OPERATOR, Symbols::LT },
            { Tokens::IDENTIFIER, "Boolean" },
            { Tokens::OPERATOR, Symbols::GT },
            { Tokens::ASSIGNMENT, Symbols::ASSIGNMENT },
            { Tokens::BOOLEAN, Symbols::TRUE }
        };

        verifyTokens(lexer.tokens, expectedTokens);
    }

    SECTION("Can tokenize arithmetic assignment") {
        string source = "total<Number> = 5 + 3";
        lexer.lex(source);

        std::vector<std::pair<Tokens, std::string>> expectedTokens = {
            { Tokens::IDENTIFIER, "total" },
            { Tokens::OPERATOR, Symbols::LT },
            { Tokens::IDENTIFIER, "Number" },
            { Tokens::OPERATOR, Symbols::GT },
            { Tokens::ASSIGNMENT, Symbols::ASSIGNMENT },
            { Tokens::NUMBER, "5" },
            { Tokens::OPERATOR, Symbols::PLUS },
            { Tokens::NUMBER, "3" }
        };

        verifyTokens(lexer.tokens, expectedTokens);
    }
    // --------- ASSIGNMENT ----------

    // --------- CONTROL FLOW ---------
    SECTION("Can tokenize if statement") {
        string source = "if (x == 10) { return true }";
        lexer.lex(source);

        std::vector<std::pair<Tokens, std::string>> expectedTokens = {
            { Tokens::KEYWORD, Symbols::IF },
            { Tokens::PAREN_OPEN, Symbols::PAREN_OPEN },
            { Tokens::IDENTIFIER, "x" },
            { Tokens::OPERATOR, Symbols::EQUALITY },
            { Tokens::NUMBER, "10" },
            { Tokens::PAREN_CLOSE, Symbols::PAREN_CLOSE },
            { Tokens::BRACE_OPEN, Symbols::BRACE_OPEN },
            { Tokens::KEYWORD, Symbols::RETURN },
            { Tokens::BOOLEAN, Symbols::TRUE },
            { Tokens::BRACE_CLOSE, Symbols::BRACE_CLOSE }
        };

        verifyTokens(lexer.tokens, expectedTokens);
    }
    // --------- CONTROL FLOW ---------

    // --------- FUNCTIONS ------------
    SECTION("Can tokenize inline function definition") {
        string source = "greet<String> = name<String> -> 'hello' + name";
        lexer.lex(source);

        std::vector<std::pair<Tokens, std::string>> expectedTokens = {
            { Tokens::IDENTIFIER, "greet" },
            { Tokens::OPERATOR, Symbols::LT },
            { Tokens::IDENTIFIER, "String" },
            { Tokens::OPERATOR, Symbols::GT },
            { Tokens::ASSIGNMENT, Symbols::ASSIGNMENT },
            { Tokens::IDENTIFIER, "name" },
            { Tokens::OPERATOR, Symbols::LT },
            { Tokens::IDENTIFIER, "String"},
            { Tokens::OPERATOR, Symbols::GT },
            { Tokens::FUNC_DECLARATION, Symbols::FUNC_DECLARATION },
            { Tokens::STRING, "'hello'" },
            { Tokens::OPERATOR, Symbols::PLUS },
            { Tokens::IDENTIFIER, "name" }
        };

        verifyTokens(lexer.tokens, expectedTokens);
    }

    SECTION("Can tokenize function definition with multiple params") {
        string source = "greet<String> = greeting<String>, name<String> -> 'hello' + name";
        lexer.lex(source);

        std::vector<std::pair<Tokens, std::string>> expectedTokens = {
            { Tokens::IDENTIFIER, "greet" },
            { Tokens::OPERATOR, Symbols::LT },
            { Tokens::IDENTIFIER, "String" },
            { Tokens::OPERATOR, Symbols::GT },
            { Tokens::ASSIGNMENT, Symbols::ASSIGNMENT },
            { Tokens::IDENTIFIER, "greeting" },
            { Tokens::OPERATOR, Symbols::LT },
            { Tokens::IDENTIFIER, "String"},
            { Tokens::OPERATOR, Symbols::GT },
            { Tokens::COMMA, Symbols::COMMA },
            { Tokens::IDENTIFIER, "name" },
            { Tokens::OPERATOR, Symbols::LT },
            { Tokens::IDENTIFIER, "String"},
            { Tokens::OPERATOR, Symbols::GT },
            { Tokens::FUNC_DECLARATION, Symbols::FUNC_DECLARATION },
            { Tokens::STRING, "'hello'" },
            { Tokens::OPERATOR, Symbols::PLUS },
            { Tokens::IDENTIFIER, "name" }
        };

        verifyTokens(lexer.tokens, expectedTokens);
    }

    SECTION("Can tokenize function definition with no params") {
        string source = "greet<String> = () -> 'hello there'";
        lexer.lex(source);

        std::vector<std::pair<Tokens, std::string>> expectedTokens = {
            { Tokens::IDENTIFIER, "greet" },
            { Tokens::OPERATOR, Symbols::LT },
            { Tokens::IDENTIFIER, "String" },
            { Tokens::OPERATOR, Symbols::GT },
            { Tokens::ASSIGNMENT, Symbols::ASSIGNMENT },
            { Tokens::PAREN_OPEN, Symbols::PAREN_OPEN },
            { Tokens::PAREN_CLOSE, Symbols::PAREN_CLOSE },
            { Tokens::FUNC_DECLARATION, Symbols::FUNC_DECLARATION },
            { Tokens::STRING, "'hello there'" }
        };

        verifyTokens(lexer.tokens, expectedTokens);
    }

    SECTION("Can tokenize multiline function definition") {
        string source =
            "greet<String> = names<List<String>> -> { "
            "    'hello' + concat_strings(names, ', ')"
            "}";

        lexer.lex(source);

        std::vector<std::pair<Tokens, std::string>> expectedTokens = {
            { Tokens::IDENTIFIER, "greet" },
            { Tokens::OPERATOR, Symbols::LT },
            { Tokens::IDENTIFIER, "String" },
            { Tokens::OPERATOR, Symbols::GT },
            { Tokens::ASSIGNMENT, Symbols::ASSIGNMENT },
            { Tokens::IDENTIFIER, "names" },
            { Tokens::OPERATOR, Symbols::LT },
            { Tokens::IDENTIFIER, "List"},
            { Tokens::OPERATOR, Symbols::LT },
            { Tokens::IDENTIFIER, "String" },
            { Tokens::OPERATOR, Symbols::GT },
            { Tokens::OPERATOR, Symbols::GT },
            { Tokens::FUNC_DECLARATION, Symbols::FUNC_DECLARATION },
            { Tokens::BRACE_OPEN, Symbols::BRACE_OPEN },
            { Tokens::STRING, "'hello'" },
            { Tokens::OPERATOR, Symbols::PLUS },
            { Tokens::IDENTIFIER, "concat_strings" },
            { Tokens::PAREN_OPEN, Symbols::PAREN_OPEN },
            { Tokens::IDENTIFIER, "names" },
            { Tokens::COMMA, Symbols::COMMA },
            { Tokens::STRING, "', '" },
            { Tokens::PAREN_CLOSE, Symbols::PAREN_CLOSE },
            { Tokens::BRACE_CLOSE, Symbols::BRACE_CLOSE }
        };

        verifyTokens(lexer.tokens, expectedTokens);
    }
    // --------- FUNCTIONS ------------

    SECTION("Can tokenize with single line comments") {
        string source = "x<Number> = 5 + 3 // assignment";
        lexer.lex(source);

        std::vector<std::pair<Tokens, std::string>> expectedTokens = {
            { Tokens::IDENTIFIER, "x" },
            { Tokens::OPERATOR, Symbols::LT },
            { Tokens::IDENTIFIER, "Number" },
            { Tokens::OPERATOR, Symbols::GT },
            { Tokens::ASSIGNMENT, Symbols::ASSIGNMENT },
            { Tokens::NUMBER, "5" },
            { Tokens::OPERATOR, Symbols::PLUS },
            { Tokens::NUMBER, "3" }
        };

        verifyTokens(lexer.tokens, expectedTokens);
    }

    SECTION("Can tokenize with multi line comments") {
        string source = "x<Number> = 5 - /- comment -/ 3";
        lexer.lex(source);

        std::vector<std::pair<Tokens, std::string>> expectedTokens = {
            { Tokens::IDENTIFIER, "x" },
            { Tokens::OPERATOR, Symbols::LT },
            { Tokens::IDENTIFIER, "Number" },
            { Tokens::OPERATOR, Symbols::GT },
            { Tokens::ASSIGNMENT, Symbols::ASSIGNMENT },
            { Tokens::NUMBER, "5" },
            { Tokens::OPERATOR, Symbols::MINUS },
            { Tokens::NUMBER, "3" }
        };

        verifyTokens(lexer.tokens, expectedTokens);
    }

}
