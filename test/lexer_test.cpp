#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch2/catch_amalgamated.hpp"
#include "../src/lexer/lexer.cpp" 

TEST_CASE("ThetaLexer tests") {

    SECTION("Basic tokenization") {
        ThetaLexer lexer;
        std::string source = "if (x == 10) { return true; }";
        lexer.lex(source);

        REQUIRE(lexer.tokens.size() == 12);

        // Test specific tokens
        REQUIRE(lexer.tokens[0].getType() == Tokens::KEYWORD);
        REQUIRE(lexer.tokens[0].getText() == "if");

        REQUIRE(lexer.tokens[1].getType() == Tokens::PAREN_OPEN);
        REQUIRE(lexer.tokens[1].getText() == "(");

        REQUIRE(lexer.tokens[2].getType() == Tokens::IDENTIFIER);
        REQUIRE(lexer.tokens[2].getText() == "x");

        // Add more checks for other tokens as needed
    }

    SECTION("Handling whitespace and comments") {
        ThetaLexer lexer;
        std::string source = "x = 5 + /* comment */ 3; // assignment";
        lexer.lex(source);

        REQUIRE(lexer.tokens.size() == 9);

        // Test specific tokens
        REQUIRE(lexer.tokens[0].getType() == Tokens::IDENTIFIER);
        REQUIRE(lexer.tokens[0].getText() == "x");

        REQUIRE(lexer.tokens[1].getType() == Tokens::ASSIGNMENT);
        REQUIRE(lexer.tokens[1].getText() == "=");

        REQUIRE(lexer.tokens[4].getType() == Tokens::NUMBER);
        REQUIRE(lexer.tokens[4].getText() == "3");

        // Add more checks for other tokens as needed
    }

    SECTION("String tokenization") {
        ThetaLexer lexer;
        std::string source = "message = \"Hello, World!\";";
        lexer.lex(source);

        REQUIRE(lexer.tokens.size() == 5);

        // Test specific tokens
        REQUIRE(lexer.tokens[2].getType() == Tokens::STRING);
        REQUIRE(lexer.tokens[2].getText() == "\"Hello, World!\"");

        // Add more checks for other tokens as needed
    }

    // Add more sections and tests to cover various scenarios (numbers, operators, etc.)
}
