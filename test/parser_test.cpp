#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch2/catch_amalgamated.hpp"
#include "../src/lexer/lexer.cpp"
#include "../src/parser/parser.cpp"

using namespace std;

TEST_CASE("ThetaParser") {
    ThetaLexer lexer;
    ThetaParser parser;
    shared_ptr<map<string, string>> filesByCapsuleName;

    // --------- LITERALS ----------
    SECTION("Can tokenize numbers with decimals") {
        string source = "5 * 12.23";
        lexer.lex(source);

        shared_ptr<SourceNode> parsedAST = dynamic_pointer_cast<SourceNode>(
            parser.parse(lexer.tokens, source, "fakeFile.th", filesByCapsuleName)
        );

        REQUIRE(parsedAST->getNodeType() == "Source");
        REQUIRE(parsedAST->getLinks().size() == 0);
        REQUIRE(parsedAST->getValue()->getNodeType() == "BinaryOperation");

        shared_ptr<BinaryOperationNode> binOpNode = dynamic_pointer_cast<BinaryOperationNode>(parsedAST->getValue());

        REQUIRE(binOpNode->getOperator() == "*");
        REQUIRE(binOpNode->getLeft()->getNodeType() == "NumberLiteral");
        REQUIRE(binOpNode->getRight()->getNodeType() == "NumberLiteral");

        shared_ptr<LiteralNode> leftNode = dynamic_pointer_cast<LiteralNode>(binOpNode->getLeft());
        shared_ptr<LiteralNode> rightNode = dynamic_pointer_cast<LiteralNode>(binOpNode->getRight());
        REQUIRE(leftNode->getLiteralValue() == "5");
        REQUIRE(rightNode->getLiteralValue() == "12.23");
    }

    SECTION("Numbers dont have multiple decimals") {
        string source = "5100 / 10.23.4.";
        lexer.lex(source);

    }

    SECTION("Can tokenize a string") {
        string source = "'And his name is John Cena!'";
        lexer.lex(source);

    }
    // --------- LITERALS ----------

    // --------- DATA STRUCTURES -----------
    SECTION("Can tokenize a list of strings") {
        string source = "['Alex', 'Tony', 'John', 'Denis']";
        lexer.lex(source);

    }

    SECTION("Can tokenize a nested list of strings") {
        string source = "[['alex', 'john', ['jeremy', 'pablo']], ['clarinda'], 'den' + 'is', 'jessica', 'ellis']";
        lexer.lex(source);

    }

    // --------- DATA STRUCTURES -----------

    // --------- ARITHMETIC OPERATORS ----------
    SECTION("Can tokenize addition") {
        string source = "100 + 7";
        lexer.lex(source);

    }

    SECTION("Can tokenize subtraction") {
        string source = "42 - 11";
        lexer.lex(source);

    }

    SECTION("Can tokenize multiplication") {
        string source = "10 * 11";
        lexer.lex(source);

    }

    SECTION("Can tokenize division") {
        string source = "100/5";
        lexer.lex(source);

    }

    SECTION("Can tokenize exponentiation") {
        string source = "2 ** 3";
        lexer.lex(source);

    }
    // --------- ARITHMETIC OPERATORS ----------

    // --------- BOOLEANS + BOOLEAN OPERATORS ----------
    SECTION("Can tokenize boolean logic") {
        string source = "true && !false";
        lexer.lex(source);

    }

    SECTION("Can tokenize boolean logic with identifiers") {
        string source = "!x || y";
        lexer.lex(source);

    }
    // --------- BOOLEANS + BOOLEAN OPERATORS ----------

    // --------- ASSIGNMENT ----------
    SECTION("Can tokenize a string assignment") {
        string source = "message<String> = 'Hello, World!'";
        lexer.lex(source);

    }

    SECTION("Can tokenize boolean assignment") {
        string source = "isOpen<Boolean> = true";
        lexer.lex(source);

    }

    SECTION("Can tokenize arithmetic assignment") {
        string source = "total<Number> = 5 + 3";
        lexer.lex(source);

    }
    // --------- ASSIGNMENT ----------

    // --------- CONTROL FLOW ---------
    SECTION("Can tokenize if statement") {
        string source = "if (x == 10) { return true }";
        lexer.lex(source);

    }
    // --------- CONTROL FLOW ---------

    // --------- FUNCTIONS ------------
    SECTION("Can tokenize inline function definition") {
        string source = "greet<String> = name<String> -> 'hello' + name";
        lexer.lex(source);

    }

    SECTION("Can tokenize function definition with multiple params") {
        string source = "greet<String> = greeting<String>, name<String> -> 'hello' + name";
        lexer.lex(source);

    }

    SECTION("Can tokenize function definition with no params") {
        string source = "greet<String> = () -> 'hello there'";
        lexer.lex(source);

    }
    
    SECTION("Can tokenize multiline function definition") {
        string source = 
            "greet<String> = names<List<String>> -> { "
            "    'hello' + concat_strings(names, ', ')"
            "}";

        lexer.lex(source);

    }
    // --------- FUNCTIONS ------------

    SECTION("Can tokenize with single line comments") {
        string source = "x<Number> = 5 + 3 // assignment";
        lexer.lex(source);

    }

    SECTION("Can tokenize with multi line comments") {
        string source = "x<Number> = 5 - /- comment -/ 3";
        lexer.lex(source);

    }

}