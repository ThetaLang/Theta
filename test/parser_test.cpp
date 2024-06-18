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
    SECTION("Can parse numbers with decimals") {
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

        REQUIRE_THROWS_AS(
            parser.parse(lexer.tokens, source, "fakeFile.th", filesByCapsuleName),
            ThetaCompilationError
        );

        REQUIRE(lexer.tokens.size() == 1);
        REQUIRE(lexer.tokens[0].getType() == Tokens::IDENTIFIER);
        REQUIRE(lexer.tokens[0].getLexeme() == ".4.");
    }

    SECTION("Can parse a string") {
        string source = "'And his name is John Cena!'";
        lexer.lex(source);

        shared_ptr<SourceNode> parsedAST = dynamic_pointer_cast<SourceNode>(
            parser.parse(lexer.tokens, source, "fakeFile.th", filesByCapsuleName)
        );

        REQUIRE(parsedAST->getNodeType() == "Source");
        REQUIRE(parsedAST->getLinks().size() == 0);
        REQUIRE(parsedAST->getValue()->getNodeType() == "StringLiteral");

        shared_ptr<LiteralNode> childNode = dynamic_pointer_cast<LiteralNode>(parsedAST->getValue());

        REQUIRE(childNode->getLiteralValue() == "'And his name is John Cena!'");
    }
    // --------- LITERALS ----------

    // --------- DATA STRUCTURES -----------
    SECTION("Can parse a list of strings") {
        string source = "['Alex', 'Tony', 'John', 'Denis']";
        lexer.lex(source);

        shared_ptr<SourceNode> parsedAST = dynamic_pointer_cast<SourceNode>(
            parser.parse(lexer.tokens, source, "fakeFile.th", filesByCapsuleName)
        );

        REQUIRE(parsedAST->getNodeType() == "Source");
        REQUIRE(parsedAST->getLinks().size() == 0);
        REQUIRE(parsedAST->getValue()->getNodeType() == "ListDefinition");

        shared_ptr<ListDefinitionNode> listNode = dynamic_pointer_cast<ListDefinitionNode>(parsedAST->getValue());

        REQUIRE(listNode->getElements().size() == 4);

        vector<string> expectedValues = { "'Alex'", "'Tony'", "'John'", "'Denis'" };

        for (int i = 0; i < listNode->getElements().size(); i++) {
            shared_ptr<LiteralNode> literalNode = dynamic_pointer_cast<LiteralNode>(listNode->getElements()[i]);

            REQUIRE(literalNode->getNodeType() == "StringLiteral");
            REQUIRE(literalNode->getLiteralValue() == expectedValues[i]);
        }
    }

    SECTION("Can parse a nested list of strings") {
        string source = "[['alex', 'john', ['jeremy', 'pablo']], ['clarinda'], 'den' + 'is', 'jessica', 'ellis']";
        lexer.lex(source);

        shared_ptr<SourceNode> parsedAST = dynamic_pointer_cast<SourceNode>(
            parser.parse(lexer.tokens, source, "fakeFile.th", filesByCapsuleName)
        );

        REQUIRE(parsedAST->getNodeType() == "Source");
        REQUIRE(parsedAST->getLinks().size() == 0);
        REQUIRE(parsedAST->getValue()->getNodeType() == "ListDefinition");

        shared_ptr<ListDefinitionNode> listNode = dynamic_pointer_cast<ListDefinitionNode>(parsedAST->getValue());

        REQUIRE(listNode->getElements().size() == 5);
        REQUIRE(listNode->getElements()[0]->getNodeType() == "ListDefinition");

        shared_ptr<ListDefinitionNode> listElement0 = dynamic_pointer_cast<ListDefinitionNode>(listNode->getElements()[0]);

        REQUIRE(listElement0->getElements().size() == 3);
        REQUIRE(listElement0->getElements()[0]->getNodeType() == "StringLiteral");
        REQUIRE(listElement0->getElements()[1]->getNodeType() == "StringLiteral");
        REQUIRE(listElement0->getElements()[2]->getNodeType() == "ListDefinition");
        REQUIRE(dynamic_pointer_cast<LiteralNode>(listElement0->getElements()[0])->getLiteralValue() == "'alex'");
        REQUIRE(dynamic_pointer_cast<LiteralNode>(listElement0->getElements()[1])->getLiteralValue() == "'john'");

        shared_ptr<ListDefinitionNode> listElement0Element2 = dynamic_pointer_cast<ListDefinitionNode>(listElement0->getElements()[2]);
        REQUIRE(listElement0Element2->getElements().size() == 2);
        REQUIRE(listElement0Element2->getElements()[0]->getNodeType() == "StringLiteral");
        REQUIRE(listElement0Element2->getElements()[1]->getNodeType() == "StringLiteral");
        REQUIRE(dynamic_pointer_cast<LiteralNode>(listElement0Element2->getElements()[0])->getLiteralValue() == "'jeremy'");
        REQUIRE(dynamic_pointer_cast<LiteralNode>(listElement0Element2->getElements()[1])->getLiteralValue() == "'pablo'");

        REQUIRE(listNode->getElements()[1]->getNodeType() == "ListDefinition");

        shared_ptr<ListDefinitionNode> listElement1 = dynamic_pointer_cast<ListDefinitionNode>(listNode->getElements()[1]);

        REQUIRE(listElement1->getElements().size() == 1);
        REQUIRE(listElement1->getElements()[0]->getNodeType() == "StringLiteral");
        REQUIRE(dynamic_pointer_cast<LiteralNode>(listElement1->getElements()[0])->getLiteralValue() == "'clarinda'");

        REQUIRE(listNode->getElements()[2]->getNodeType() == "BinaryOperation");

        shared_ptr<BinaryOperationNode> listElement2 = dynamic_pointer_cast<BinaryOperationNode>(listNode->getElements()[2]);
        REQUIRE(listElement2->getOperator() == "+");
        REQUIRE(listElement2->getLeft()->getNodeType() == "StringLiteral");
        REQUIRE(listElement2->getRight()->getNodeType() == "StringLiteral");
        REQUIRE(dynamic_pointer_cast<LiteralNode>(listElement2->getLeft())->getLiteralValue() == "'den'");
        REQUIRE(dynamic_pointer_cast<LiteralNode>(listElement2->getRight())->getLiteralValue() == "'is'");

        REQUIRE(listNode->getElements()[3]->getNodeType() == "StringLiteral");
        REQUIRE(listNode->getElements()[4]->getNodeType() == "StringLiteral");
        REQUIRE(dynamic_pointer_cast<LiteralNode>(listNode->getElements()[3])->getLiteralValue() == "'jessica'");
        REQUIRE(dynamic_pointer_cast<LiteralNode>(listNode->getElements()[4])->getLiteralValue() == "'ellis'");
    }

    // --------- DATA STRUCTURES -----------

    // --------- ARITHMETIC OPERATORS ----------
    SECTION("Can parse addition") {
        string source = "100 + 7";
        lexer.lex(source);

        shared_ptr<SourceNode> parsedAST = dynamic_pointer_cast<SourceNode>(
            parser.parse(lexer.tokens, source, "fakeFile.th", filesByCapsuleName)
        );

        REQUIRE(parsedAST->getNodeType() == "Source");
        REQUIRE(parsedAST->getLinks().size() == 0);
        REQUIRE(parsedAST->getValue()->getNodeType() == "BinaryOperation");

        shared_ptr<BinaryOperationNode> binOpNode = dynamic_pointer_cast<BinaryOperationNode>(parsedAST->getValue());

        REQUIRE(binOpNode->getOperator() == "+");
        REQUIRE(binOpNode->getLeft()->getNodeType() == "NumberLiteral");
        REQUIRE(binOpNode->getRight()->getNodeType() == "NumberLiteral");
        REQUIRE(dynamic_pointer_cast<LiteralNode>(binOpNode->getLeft())->getLiteralValue() == "100");
        REQUIRE(dynamic_pointer_cast<LiteralNode>(binOpNode->getRight())->getLiteralValue() == "7");
    }

    SECTION("Can parse subtraction") {
        string source = "42 - 11";
        lexer.lex(source);

        shared_ptr<SourceNode> parsedAST = dynamic_pointer_cast<SourceNode>(
            parser.parse(lexer.tokens, source, "fakeFile.th", filesByCapsuleName)
        );

        REQUIRE(parsedAST->getNodeType() == "Source");
        REQUIRE(parsedAST->getLinks().size() == 0);
        REQUIRE(parsedAST->getValue()->getNodeType() == "BinaryOperation");

        shared_ptr<BinaryOperationNode> binOpNode = dynamic_pointer_cast<BinaryOperationNode>(parsedAST->getValue());

        REQUIRE(binOpNode->getOperator() == "-");
        REQUIRE(binOpNode->getLeft()->getNodeType() == "NumberLiteral");
        REQUIRE(binOpNode->getRight()->getNodeType() == "NumberLiteral");
        REQUIRE(dynamic_pointer_cast<LiteralNode>(binOpNode->getLeft())->getLiteralValue() == "42");
        REQUIRE(dynamic_pointer_cast<LiteralNode>(binOpNode->getRight())->getLiteralValue() == "11");
    }

    SECTION("Can parse multiplication") {
        string source = "10 * 11";
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
        REQUIRE(dynamic_pointer_cast<LiteralNode>(binOpNode->getLeft())->getLiteralValue() == "10");
        REQUIRE(dynamic_pointer_cast<LiteralNode>(binOpNode->getRight())->getLiteralValue() == "11");
    }

    SECTION("Can parse division") {
        string source = "100/5";
        lexer.lex(source);

        shared_ptr<SourceNode> parsedAST = dynamic_pointer_cast<SourceNode>(
            parser.parse(lexer.tokens, source, "fakeFile.th", filesByCapsuleName)
        );

        REQUIRE(parsedAST->getNodeType() == "Source");
        REQUIRE(parsedAST->getLinks().size() == 0);
        REQUIRE(parsedAST->getValue()->getNodeType() == "BinaryOperation");

        shared_ptr<BinaryOperationNode> binOpNode = dynamic_pointer_cast<BinaryOperationNode>(parsedAST->getValue());

        REQUIRE(binOpNode->getOperator() == "/");
        REQUIRE(binOpNode->getLeft()->getNodeType() == "NumberLiteral");
        REQUIRE(binOpNode->getRight()->getNodeType() == "NumberLiteral");
        REQUIRE(dynamic_pointer_cast<LiteralNode>(binOpNode->getLeft())->getLiteralValue() == "100");
        REQUIRE(dynamic_pointer_cast<LiteralNode>(binOpNode->getRight())->getLiteralValue() == "5");
    }

    SECTION("Can parse exponentiation") {
        string source = "2 ** 3";
        lexer.lex(source);

        shared_ptr<SourceNode> parsedAST = dynamic_pointer_cast<SourceNode>(
            parser.parse(lexer.tokens, source, "fakeFile.th", filesByCapsuleName)
        );

        REQUIRE(parsedAST->getNodeType() == "Source");
        REQUIRE(parsedAST->getLinks().size() == 0);
        REQUIRE(parsedAST->getValue()->getNodeType() == "BinaryOperation");

        shared_ptr<BinaryOperationNode> binOpNode = dynamic_pointer_cast<BinaryOperationNode>(parsedAST->getValue());

        REQUIRE(binOpNode->getOperator() == "**");
        REQUIRE(binOpNode->getLeft()->getNodeType() == "NumberLiteral");
        REQUIRE(binOpNode->getRight()->getNodeType() == "NumberLiteral");
        REQUIRE(dynamic_pointer_cast<LiteralNode>(binOpNode->getLeft())->getLiteralValue() == "2");
        REQUIRE(dynamic_pointer_cast<LiteralNode>(binOpNode->getRight())->getLiteralValue() == "3");
    }

    SECTION("Can parse negative numbers") {
        string source = "100.4 - -24";
        lexer.lex(source);

        shared_ptr<SourceNode> parsedAST = dynamic_pointer_cast<SourceNode>(
            parser.parse(lexer.tokens, source, "fakeFile.th", filesByCapsuleName)
        );

        REQUIRE(parsedAST->getNodeType() == "Source");
        REQUIRE(parsedAST->getLinks().size() == 0);
        REQUIRE(parsedAST->getValue()->getNodeType() == "BinaryOperation");

        shared_ptr<BinaryOperationNode> binOpNode = dynamic_pointer_cast<BinaryOperationNode>(parsedAST->getValue());

        REQUIRE(binOpNode->getOperator() == "-");
        REQUIRE(binOpNode->getLeft()->getNodeType() == "NumberLiteral");
        REQUIRE(binOpNode->getRight()->getNodeType() == "UnaryOperation");
        REQUIRE(dynamic_pointer_cast<LiteralNode>(binOpNode->getLeft())->getLiteralValue() == "100.4");

        REQUIRE(binOpNode->getRight()->getValue()->getNodeType() == "NumberLiteral");
        REQUIRE(dynamic_pointer_cast<UnaryOperationNode>(binOpNode->getRight())->getOperator() == "-");
        REQUIRE(dynamic_pointer_cast<LiteralNode>(binOpNode->getRight()->getValue())->getLiteralValue() == "24");
    }
    // --------- ARITHMETIC OPERATORS ----------

    // --------- BOOLEANS + BOOLEAN OPERATORS ----------
    SECTION("Can parse boolean logic") {
        string source = "true && false";
        lexer.lex(source);

        shared_ptr<SourceNode> parsedAST = dynamic_pointer_cast<SourceNode>(
            parser.parse(lexer.tokens, source, "fakeFile.th", filesByCapsuleName)
        );

        REQUIRE(parsedAST->getNodeType() == "Source");
        REQUIRE(parsedAST->getLinks().size() == 0);
        REQUIRE(parsedAST->getValue()->getNodeType() == "BinaryOperation");

        shared_ptr<BinaryOperationNode> binOpNode = dynamic_pointer_cast<BinaryOperationNode>(parsedAST->getValue());

        REQUIRE(binOpNode->getOperator() == "&&");
        REQUIRE(binOpNode->getLeft()->getNodeType() == "BooleanLiteral");
        REQUIRE(binOpNode->getRight()->getNodeType() == "BooleanLiteral");
        REQUIRE(dynamic_pointer_cast<LiteralNode>(binOpNode->getLeft())->getLiteralValue() == "true");
        REQUIRE(dynamic_pointer_cast<LiteralNode>(binOpNode->getRight())->getLiteralValue() == "false");
    }

    SECTION("Can parse boolean with unary logic") {
        string source = "true && !false";
        lexer.lex(source);

        shared_ptr<SourceNode> parsedAST = dynamic_pointer_cast<SourceNode>(
            parser.parse(lexer.tokens, source, "fakeFile.th", filesByCapsuleName)
        );

        REQUIRE(parsedAST->getNodeType() == "Source");
        REQUIRE(parsedAST->getLinks().size() == 0);
        REQUIRE(parsedAST->getValue()->getNodeType() == "BinaryOperation");

        shared_ptr<BinaryOperationNode> binOpNode = dynamic_pointer_cast<BinaryOperationNode>(parsedAST->getValue());

        REQUIRE(binOpNode->getOperator() == "&&");
        REQUIRE(binOpNode->getLeft()->getNodeType() == "BooleanLiteral");
        REQUIRE(binOpNode->getRight()->getNodeType() == "UnaryOperation");
        REQUIRE(dynamic_pointer_cast<LiteralNode>(binOpNode->getLeft())->getLiteralValue() == "true");
        REQUIRE(dynamic_pointer_cast<UnaryOperationNode>(binOpNode->getRight())->getOperator() == "!");
        REQUIRE(dynamic_pointer_cast<LiteralNode>(binOpNode->getRight()->getValue())->getLiteralValue() == "false");
    }

    SECTION("Can parse boolean logic with identifiers") {
        string source = "!x || y";
        lexer.lex(source);

        shared_ptr<SourceNode> parsedAST = dynamic_pointer_cast<SourceNode>(
            parser.parse(lexer.tokens, source, "fakeFile.th", filesByCapsuleName)
        );

        REQUIRE(parsedAST->getNodeType() == "Source");
        REQUIRE(parsedAST->getLinks().size() == 0);
        REQUIRE(parsedAST->getValue()->getNodeType() == "BinaryOperation");

        shared_ptr<BinaryOperationNode> binOpNode = dynamic_pointer_cast<BinaryOperationNode>(parsedAST->getValue());

        REQUIRE(binOpNode->getOperator() == "||");
        REQUIRE(binOpNode->getLeft()->getNodeType() == "UnaryOperation");
        REQUIRE(binOpNode->getRight()->getNodeType() == "Identifier");
        REQUIRE(dynamic_pointer_cast<UnaryOperationNode>(binOpNode->getLeft())->getOperator() == "!");
        REQUIRE(dynamic_pointer_cast<IdentifierNode>(binOpNode->getLeft()->getValue())->getIdentifier() == "x");
        REQUIRE(dynamic_pointer_cast<IdentifierNode>(binOpNode->getRight())->getIdentifier() == "y");
    }
    // --------- BOOLEANS + BOOLEAN OPERATORS ----------

    // --------- ASSIGNMENT ----------
    SECTION("Can parse a string assignment") {
        string source = "message<String> = 'Hello, World!'";
        lexer.lex(source);

        shared_ptr<SourceNode> parsedAST = dynamic_pointer_cast<SourceNode>(
            parser.parse(lexer.tokens, source, "fakeFile.th", filesByCapsuleName)
        );

        REQUIRE(parsedAST->getNodeType() == "Source");
        REQUIRE(parsedAST->getLinks().size() == 0);
        REQUIRE(parsedAST->getValue()->getNodeType() == "Assignment");

        shared_ptr<AssignmentNode> assignmentNode = dynamic_pointer_cast<AssignmentNode>(parsedAST->getValue());
        REQUIRE(assignmentNode->getLeft()->getNodeType() == "Identifier");
        REQUIRE(assignmentNode->getRight()->getNodeType() == "StringLiteral");
        REQUIRE(dynamic_pointer_cast<IdentifierNode>(assignmentNode->getLeft())->getIdentifier() == "message");
        REQUIRE(dynamic_pointer_cast<TypeDeclarationNode>(assignmentNode->getLeft()->getValue())->getType() == "String");
        REQUIRE(dynamic_pointer_cast<LiteralNode>(assignmentNode->getRight())->getLiteralValue() == "'Hello, World!'");
    }

    SECTION("Can parse boolean assignment") {
        string source = "isOpen<Boolean> = true";
        lexer.lex(source);

        shared_ptr<SourceNode> parsedAST = dynamic_pointer_cast<SourceNode>(
            parser.parse(lexer.tokens, source, "fakeFile.th", filesByCapsuleName)
        );

        REQUIRE(parsedAST->getNodeType() == "Source");
        REQUIRE(parsedAST->getLinks().size() == 0);
        REQUIRE(parsedAST->getValue()->getNodeType() == "Assignment");

        shared_ptr<AssignmentNode> assignmentNode = dynamic_pointer_cast<AssignmentNode>(parsedAST->getValue());
        REQUIRE(assignmentNode->getLeft()->getNodeType() == "Identifier");
        REQUIRE(assignmentNode->getRight()->getNodeType() == "BooleanLiteral");
        REQUIRE(dynamic_pointer_cast<IdentifierNode>(assignmentNode->getLeft())->getIdentifier() == "isOpen");
        REQUIRE(dynamic_pointer_cast<TypeDeclarationNode>(assignmentNode->getLeft()->getValue())->getType() == "Boolean");
        REQUIRE(dynamic_pointer_cast<LiteralNode>(assignmentNode->getRight())->getLiteralValue() == "true");
    }

    SECTION("Can parse boolean assignment with unary") {
        string source = "isOpen<Boolean> = !true && false";
        lexer.lex(source);

        shared_ptr<SourceNode> parsedAST = dynamic_pointer_cast<SourceNode>(
            parser.parse(lexer.tokens, source, "fakeFile.th", filesByCapsuleName)
        );

        REQUIRE(parsedAST->getNodeType() == "Source");
        REQUIRE(parsedAST->getLinks().size() == 0);
        REQUIRE(parsedAST->getValue()->getNodeType() == "Assignment");

        shared_ptr<AssignmentNode> assignmentNode = dynamic_pointer_cast<AssignmentNode>(parsedAST->getValue());
        REQUIRE(assignmentNode->getLeft()->getNodeType() == "Identifier");
        REQUIRE(assignmentNode->getRight()->getNodeType() == "BinaryOperation");
        REQUIRE(dynamic_pointer_cast<IdentifierNode>(assignmentNode->getLeft())->getIdentifier() == "isOpen");
        REQUIRE(dynamic_pointer_cast<TypeDeclarationNode>(assignmentNode->getLeft()->getValue())->getType() == "Boolean");

        shared_ptr<BinaryOperationNode> binOpNode = dynamic_pointer_cast<BinaryOperationNode>(assignmentNode->getRight());

        REQUIRE(binOpNode->getOperator() == "&&");
        REQUIRE(binOpNode->getLeft()->getNodeType() == "UnaryOperation");
        REQUIRE(dynamic_pointer_cast<UnaryOperationNode>(binOpNode->getLeft())->getOperator() == "!");
        REQUIRE(binOpNode->getLeft()->getValue()->getNodeType() == "BooleanLiteral");
        REQUIRE(dynamic_pointer_cast<LiteralNode>(binOpNode->getLeft()->getValue())->getLiteralValue() == "true");
        REQUIRE(binOpNode->getRight()->getNodeType() == "BooleanLiteral");
        REQUIRE(dynamic_pointer_cast<LiteralNode>(binOpNode->getRight())->getLiteralValue() == "false");
    }

    SECTION("Can parse arithmetic assignment") {
        string source = "total<Number> = 5 + 3";
        lexer.lex(source);

        shared_ptr<SourceNode> parsedAST = dynamic_pointer_cast<SourceNode>(
            parser.parse(lexer.tokens, source, "fakeFile.th", filesByCapsuleName)
        );

        REQUIRE(parsedAST->getNodeType() == "Source");
        REQUIRE(parsedAST->getLinks().size() == 0);
        REQUIRE(parsedAST->getValue()->getNodeType() == "Assignment");

        shared_ptr<AssignmentNode> assignmentNode = dynamic_pointer_cast<AssignmentNode>(parsedAST->getValue());
        REQUIRE(assignmentNode->getLeft()->getNodeType() == "Identifier");
        REQUIRE(assignmentNode->getRight()->getNodeType() == "BinaryOperation");
        REQUIRE(dynamic_pointer_cast<IdentifierNode>(assignmentNode->getLeft())->getIdentifier() == "total");
        REQUIRE(dynamic_pointer_cast<TypeDeclarationNode>(assignmentNode->getLeft()->getValue())->getType() == "Number");

        shared_ptr<BinaryOperationNode> binOpNode = dynamic_pointer_cast<BinaryOperationNode>(assignmentNode->getRight());

        REQUIRE(binOpNode->getOperator() == "+");
        REQUIRE(binOpNode->getLeft()->getNodeType() == "NumberLiteral");
        REQUIRE(dynamic_pointer_cast<LiteralNode>(binOpNode->getLeft())->getLiteralValue() == "5");
        REQUIRE(binOpNode->getRight()->getNodeType() == "NumberLiteral");
        REQUIRE(dynamic_pointer_cast<LiteralNode>(binOpNode->getRight())->getLiteralValue() == "3");
    }

    SECTION("Can parse arithmetic assignment with unary") {
        string source = "total<Number> = 5 + -3";
        lexer.lex(source);

        shared_ptr<SourceNode> parsedAST = dynamic_pointer_cast<SourceNode>(
            parser.parse(lexer.tokens, source, "fakeFile.th", filesByCapsuleName)
        );

        REQUIRE(parsedAST->getNodeType() == "Source");
        REQUIRE(parsedAST->getLinks().size() == 0);
        REQUIRE(parsedAST->getValue()->getNodeType() == "Assignment");

        shared_ptr<AssignmentNode> assignmentNode = dynamic_pointer_cast<AssignmentNode>(parsedAST->getValue());
        REQUIRE(assignmentNode->getLeft()->getNodeType() == "Identifier");
        REQUIRE(assignmentNode->getRight()->getNodeType() == "BinaryOperation");
        REQUIRE(dynamic_pointer_cast<IdentifierNode>(assignmentNode->getLeft())->getIdentifier() == "total");
        REQUIRE(dynamic_pointer_cast<TypeDeclarationNode>(assignmentNode->getLeft()->getValue())->getType() == "Number");

        shared_ptr<BinaryOperationNode> binOpNode = dynamic_pointer_cast<BinaryOperationNode>(assignmentNode->getRight());

        REQUIRE(binOpNode->getOperator() == "+");
        REQUIRE(binOpNode->getLeft()->getNodeType() == "NumberLiteral");
        REQUIRE(dynamic_pointer_cast<LiteralNode>(binOpNode->getLeft())->getLiteralValue() == "5");
        REQUIRE(binOpNode->getRight()->getNodeType() == "UnaryOperation");
        REQUIRE(dynamic_pointer_cast<UnaryOperationNode>(binOpNode->getRight())->getOperator() == "-");
        REQUIRE(binOpNode->getRight()->getValue()->getNodeType() == "NumberLiteral");
        REQUIRE(dynamic_pointer_cast<LiteralNode>(binOpNode->getRight()->getValue())->getLiteralValue() == "3");
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
