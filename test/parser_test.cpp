#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch2/catch_amalgamated.hpp"
#include "../src/lexer/lexer.cpp"
#include "../src/parser/parser.cpp"
#include "../src/compiler/compiler.hpp"

using namespace std;

TEST_CASE("ThetaParser") {
    ThetaLexer lexer;
    ThetaParser parser;

    shared_ptr<map<string, string>> filesByCapsuleName = ThetaCompiler::getInstance().filesByCapsuleName;

    // --------- PRIMITIVES ----------
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

        parser.parse(lexer.tokens, source, "fakeFile.th", filesByCapsuleName);

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

    SECTION("Can parse symbols") {
        string source = ":atomic_nuclei";
        lexer.lex(source);

        shared_ptr<SourceNode> parsedAST = dynamic_pointer_cast<SourceNode>(
            parser.parse(lexer.tokens, source, "fakeFile.th", filesByCapsuleName)
        );

        REQUIRE(parsedAST->getNodeType() == "Source");
        REQUIRE(parsedAST->getLinks().size() == 0);
        REQUIRE(parsedAST->getValue()->getNodeType() == "Symbol");

        shared_ptr<SymbolNode> symbolNode = dynamic_pointer_cast<SymbolNode>(parsedAST->getValue());
        REQUIRE(symbolNode->getSymbol() == ":atomic_nuclei");
    }
    // --------- PRIMITIVES ----------

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

    SECTION("Can complex arithmetic expressions") {
        string source = "12 + (23 - -1) * 7 ** 2.4";
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
        REQUIRE(dynamic_pointer_cast<LiteralNode>(binOpNode->getLeft())->getLiteralValue() == "12");

        REQUIRE(binOpNode->getRight()->getNodeType() == "BinaryOperation");

        shared_ptr<BinaryOperationNode> binOpNode2 = dynamic_pointer_cast<BinaryOperationNode>(binOpNode->getRight());

        REQUIRE(binOpNode2->getOperator() == "*");
        REQUIRE(binOpNode2->getLeft()->getNodeType() == "BinaryOperation");
        REQUIRE(binOpNode2->getRight()->getNodeType() == "BinaryOperation");

        shared_ptr<BinaryOperationNode> binOpNode2LeftBinOpNode = dynamic_pointer_cast<BinaryOperationNode>(binOpNode2->getLeft());
        shared_ptr<BinaryOperationNode> binOpNode2RightBinOpNode = dynamic_pointer_cast<BinaryOperationNode>(binOpNode2->getRight());

        REQUIRE(binOpNode2LeftBinOpNode->getOperator() == "-");
        REQUIRE(binOpNode2LeftBinOpNode->getLeft()->getNodeType() == "NumberLiteral");
        REQUIRE(dynamic_pointer_cast<LiteralNode>(binOpNode2LeftBinOpNode->getLeft())->getLiteralValue() == "23");
        REQUIRE(binOpNode2LeftBinOpNode->getRight()->getNodeType() == "UnaryOperation");

        shared_ptr<UnaryOperationNode> unaryOpNode = dynamic_pointer_cast<UnaryOperationNode>(binOpNode2LeftBinOpNode->getRight());

        REQUIRE(unaryOpNode->getOperator() == "-");
        REQUIRE(unaryOpNode->getValue()->getNodeType() == "NumberLiteral");
        REQUIRE(dynamic_pointer_cast<LiteralNode>(unaryOpNode->getValue())->getLiteralValue() == "1");

        REQUIRE(binOpNode2RightBinOpNode->getOperator() == "**");
        REQUIRE(binOpNode2RightBinOpNode->getLeft()->getNodeType() == "NumberLiteral");
        REQUIRE(binOpNode2RightBinOpNode->getRight()->getNodeType() == "NumberLiteral");
        REQUIRE(dynamic_pointer_cast<LiteralNode>(binOpNode2RightBinOpNode->getLeft())->getLiteralValue() == "7");
        REQUIRE(dynamic_pointer_cast<LiteralNode>(binOpNode2RightBinOpNode->getRight())->getLiteralValue() == "2.4");
    }
    // --------- ARITHMETIC OPERATORS ----------

    // --------- LOGICAL OPERATORS ----------
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

    SECTION("Can parse equality logic with identifiers") {
        string source = "x == y";
        lexer.lex(source);

        shared_ptr<SourceNode> parsedAST = dynamic_pointer_cast<SourceNode>(
            parser.parse(lexer.tokens, source, "fakeFile.th", filesByCapsuleName)
        );

        REQUIRE(parsedAST->getNodeType() == "Source");
        REQUIRE(parsedAST->getLinks().size() == 0);
        REQUIRE(parsedAST->getValue()->getNodeType() == "BinaryOperation");

        shared_ptr<BinaryOperationNode> binOpNode = dynamic_pointer_cast<BinaryOperationNode>(parsedAST->getValue());

        REQUIRE(binOpNode->getOperator() == "==");
        REQUIRE(binOpNode->getLeft()->getNodeType() == "Identifier");
        REQUIRE(binOpNode->getRight()->getNodeType() == "Identifier");
        REQUIRE(dynamic_pointer_cast<IdentifierNode>(binOpNode->getLeft())->getIdentifier() == "x");
        REQUIRE(dynamic_pointer_cast<IdentifierNode>(binOpNode->getRight())->getIdentifier() == "y");
    }

    SECTION("Can parse inequality logic with identifiers") {
        string source = "true != false";
        lexer.lex(source);

        shared_ptr<SourceNode> parsedAST = dynamic_pointer_cast<SourceNode>(
            parser.parse(lexer.tokens, source, "fakeFile.th", filesByCapsuleName)
        );

        REQUIRE(parsedAST->getNodeType() == "Source");
        REQUIRE(parsedAST->getLinks().size() == 0);
        REQUIRE(parsedAST->getValue()->getNodeType() == "BinaryOperation");

        shared_ptr<BinaryOperationNode> binOpNode = dynamic_pointer_cast<BinaryOperationNode>(parsedAST->getValue());

        REQUIRE(binOpNode->getOperator() == "!=");
        REQUIRE(binOpNode->getLeft()->getNodeType() == "BooleanLiteral");
        REQUIRE(binOpNode->getRight()->getNodeType() == "BooleanLiteral");
        REQUIRE(dynamic_pointer_cast<LiteralNode>(binOpNode->getLeft())->getLiteralValue() == "true");
        REQUIRE(dynamic_pointer_cast<LiteralNode>(binOpNode->getRight())->getLiteralValue() == "false");
    }
    // --------- LOGICAL OPERATORS ----------

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

    SECTION("Can parse list assignment with expressions") {
        string source = "x<List<Boolean>> = [x == y, y + 5 > 9, 'meow' != 'cow', !gross]";
        lexer.lex(source);

        shared_ptr<SourceNode> parsedAST = dynamic_pointer_cast<SourceNode>(
            parser.parse(lexer.tokens, source, "fakeFile.th", filesByCapsuleName)
        );

        REQUIRE(parsedAST->getNodeType() == "Source");
        REQUIRE(parsedAST->getLinks().size() == 0);
        REQUIRE(parsedAST->getValue()->getNodeType() == "Assignment");

        shared_ptr<AssignmentNode> assignmentNode = dynamic_pointer_cast<AssignmentNode>(parsedAST->getValue());
        REQUIRE(assignmentNode->getLeft()->getNodeType() == "Identifier");
        REQUIRE(assignmentNode->getRight()->getNodeType() == "ListDefinition");
        REQUIRE(dynamic_pointer_cast<IdentifierNode>(assignmentNode->getLeft())->getIdentifier() == "x");

        shared_ptr<ListDefinitionNode> listDefNode = dynamic_pointer_cast<ListDefinitionNode>(assignmentNode->getRight());
        REQUIRE(listDefNode->getElements().size() == 4);

        // Checking first element: x == y
        shared_ptr<BinaryOperationNode> binOpNode1 = dynamic_pointer_cast<BinaryOperationNode>(listDefNode->getElements()[0]);
        REQUIRE(binOpNode1->getOperator() == "==");
        REQUIRE(binOpNode1->getLeft()->getNodeType() == "Identifier");
        REQUIRE(dynamic_pointer_cast<IdentifierNode>(binOpNode1->getLeft())->getIdentifier() == "x");
        REQUIRE(binOpNode1->getRight()->getNodeType() == "Identifier");
        REQUIRE(dynamic_pointer_cast<IdentifierNode>(binOpNode1->getRight())->getIdentifier() == "y");

        // Checking second element: y + 5 > 9
        shared_ptr<BinaryOperationNode> binOpNode2 = dynamic_pointer_cast<BinaryOperationNode>(listDefNode->getElements()[1]);
        REQUIRE(binOpNode2->getOperator() == ">");
        shared_ptr<BinaryOperationNode> innerBinOpNode = dynamic_pointer_cast<BinaryOperationNode>(binOpNode2->getLeft());
        REQUIRE(innerBinOpNode->getOperator() == "+");
        REQUIRE(innerBinOpNode->getLeft()->getNodeType() == "Identifier");
        REQUIRE(dynamic_pointer_cast<IdentifierNode>(innerBinOpNode->getLeft())->getIdentifier() == "y");
        REQUIRE(innerBinOpNode->getRight()->getNodeType() == "NumberLiteral");
        REQUIRE(dynamic_pointer_cast<LiteralNode>(innerBinOpNode->getRight())->getLiteralValue() == "5");
        REQUIRE(binOpNode2->getRight()->getNodeType() == "NumberLiteral");
        REQUIRE(dynamic_pointer_cast<LiteralNode>(binOpNode2->getRight())->getLiteralValue() == "9");

        // Checking third element: 'meow' != 'cow'
        shared_ptr<BinaryOperationNode> binOpNode3 = dynamic_pointer_cast<BinaryOperationNode>(listDefNode->getElements()[2]);
        REQUIRE(binOpNode3->getOperator() == "!=");
        REQUIRE(binOpNode3->getLeft()->getNodeType() == "StringLiteral");
        REQUIRE(dynamic_pointer_cast<LiteralNode>(binOpNode3->getLeft())->getLiteralValue() == "'meow'");
        REQUIRE(binOpNode3->getRight()->getNodeType() == "StringLiteral");
        REQUIRE(dynamic_pointer_cast<LiteralNode>(binOpNode3->getRight())->getLiteralValue() == "'cow'");

        // Checking fourth element: !gross
        shared_ptr<UnaryOperationNode> unaryOpNode = dynamic_pointer_cast<UnaryOperationNode>(listDefNode->getElements()[3]);
        REQUIRE(unaryOpNode->getOperator() == "!");
        REQUIRE(unaryOpNode->getValue()->getNodeType() == "Identifier");
        REQUIRE(dynamic_pointer_cast<IdentifierNode>(unaryOpNode->getValue())->getIdentifier() == "gross");
    }

    SECTION("Can parse dict assignment with nested dicts") {
        string source = "x<Dict> = { bob: { age: 40, wife: 'Janet', bald: true }, mike: { age: 20, wife: '', bald: false }}";
        lexer.lex(source);

        shared_ptr<SourceNode> parsedAST = dynamic_pointer_cast<SourceNode>(
            parser.parse(lexer.tokens, source, "fakeFile.th", filesByCapsuleName)
        );

        REQUIRE(parsedAST->getNodeType() == "Source");
        REQUIRE(parsedAST->getLinks().size() == 0);
        REQUIRE(parsedAST->getValue()->getNodeType() == "Assignment");

        shared_ptr<AssignmentNode> assignmentNode = dynamic_pointer_cast<AssignmentNode>(parsedAST->getValue());
        REQUIRE(assignmentNode->getLeft()->getNodeType() == "Identifier");
        REQUIRE(assignmentNode->getRight()->getNodeType() == "DictDefinition");
        REQUIRE(dynamic_pointer_cast<IdentifierNode>(assignmentNode->getLeft())->getIdentifier() == "x");

        shared_ptr<TypeDeclarationNode> typeDeclNode = dynamic_pointer_cast<TypeDeclarationNode>(assignmentNode->getLeft()->getValue());
        REQUIRE(typeDeclNode->getType() == "Dict");

        shared_ptr<DictDefinitionNode> dictDefNode = dynamic_pointer_cast<DictDefinitionNode>(assignmentNode->getRight());
        REQUIRE(dictDefNode->getElements().size() == 2);

        // Checking first dict element: bob
        shared_ptr<TupleNode> tupleNodeBob = dynamic_pointer_cast<TupleNode>(dictDefNode->getElements()[0]);
        REQUIRE(dynamic_pointer_cast<SymbolNode>(tupleNodeBob->getLeft())->getSymbol() == ":bob");
        shared_ptr<DictDefinitionNode> bobDictNode = dynamic_pointer_cast<DictDefinitionNode>(tupleNodeBob->getRight());
        REQUIRE(bobDictNode->getElements().size() == 3);

        // Checking bob's age
        shared_ptr<TupleNode> bobAgeNode = dynamic_pointer_cast<TupleNode>(bobDictNode->getElements()[0]);
        REQUIRE(dynamic_pointer_cast<SymbolNode>(bobAgeNode->getLeft())->getSymbol() == ":age");
        REQUIRE(dynamic_pointer_cast<LiteralNode>(bobAgeNode->getRight())->getLiteralValue() == "40");

        // Checking bob's wife
        shared_ptr<TupleNode> bobWifeNode = dynamic_pointer_cast<TupleNode>(bobDictNode->getElements()[1]);
        REQUIRE(dynamic_pointer_cast<SymbolNode>(bobWifeNode->getLeft())->getSymbol() == ":wife");
        REQUIRE(dynamic_pointer_cast<LiteralNode>(bobWifeNode->getRight())->getLiteralValue() == "'Janet'");

        // Checking bob's bald
        shared_ptr<TupleNode> bobBaldNode = dynamic_pointer_cast<TupleNode>(bobDictNode->getElements()[2]);
        REQUIRE(dynamic_pointer_cast<SymbolNode>(bobBaldNode->getLeft())->getSymbol() == ":bald");
        REQUIRE(dynamic_pointer_cast<LiteralNode>(bobBaldNode->getRight())->getLiteralValue() == "true");

        // Checking second dict element: mike
        shared_ptr<TupleNode> tupleNodeMike = dynamic_pointer_cast<TupleNode>(dictDefNode->getElements()[1]);
        REQUIRE(dynamic_pointer_cast<SymbolNode>(tupleNodeMike->getLeft())->getSymbol() == ":mike");
        shared_ptr<DictDefinitionNode> mikeDictNode = dynamic_pointer_cast<DictDefinitionNode>(tupleNodeMike->getRight());
        REQUIRE(mikeDictNode->getElements().size() == 3);

        // Checking mike's age
        shared_ptr<TupleNode> mikeAgeNode = dynamic_pointer_cast<TupleNode>(mikeDictNode->getElements()[0]);
        REQUIRE(dynamic_pointer_cast<SymbolNode>(mikeAgeNode->getLeft())->getSymbol() == ":age");
        REQUIRE(dynamic_pointer_cast<LiteralNode>(mikeAgeNode->getRight())->getLiteralValue() == "20");

        // Checking mike's wife
        shared_ptr<TupleNode> mikeWifeNode = dynamic_pointer_cast<TupleNode>(mikeDictNode->getElements()[1]);
        REQUIRE(dynamic_pointer_cast<SymbolNode>(mikeWifeNode->getLeft())->getSymbol() == ":wife");
        REQUIRE(dynamic_pointer_cast<LiteralNode>(mikeWifeNode->getRight())->getLiteralValue() == "''");

        // Checking mike's bald
        shared_ptr<TupleNode> mikeBaldNode = dynamic_pointer_cast<TupleNode>(mikeDictNode->getElements()[2]);
        REQUIRE(dynamic_pointer_cast<SymbolNode>(mikeBaldNode->getLeft())->getSymbol() == ":bald");
        REQUIRE(dynamic_pointer_cast<LiteralNode>(mikeBaldNode->getRight())->getLiteralValue() == "false");
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

    SECTION("Can parse capsules") {
        string source = R"(
            capsule TextCapsule {
                x<Dict> = { bob: 'saget' }
                y<Number> = 432.11
            }
        )";
        lexer.lex(source);

        shared_ptr<SourceNode> parsedAST = dynamic_pointer_cast<SourceNode>(
            parser.parse(lexer.tokens, source, "fakeFile.th", filesByCapsuleName)
        );

        REQUIRE(parsedAST->getNodeType() == "Source");
        REQUIRE(parsedAST->getLinks().size() == 0);
        REQUIRE(parsedAST->getValue()->getNodeType() == "Capsule");

        shared_ptr<CapsuleNode> capsuleNode = dynamic_pointer_cast<CapsuleNode>(parsedAST->getValue());
        REQUIRE(capsuleNode->getName() == "TextCapsule");

        shared_ptr<BlockNode> blockNode = dynamic_pointer_cast<BlockNode>(capsuleNode->getValue());
        REQUIRE(blockNode->getNodeType() == "Block");
        vector<shared_ptr<ASTNode>> blockExpressions = blockNode->getBlockExpressions();
        REQUIRE(blockExpressions.size() == 2);

        // Check the first assignment within the block
        shared_ptr<AssignmentNode> assignmentNodeX = dynamic_pointer_cast<AssignmentNode>(blockExpressions[0]);
        REQUIRE(assignmentNodeX->getLeft()->getNodeType() == "Identifier");
        REQUIRE(dynamic_pointer_cast<IdentifierNode>(assignmentNodeX->getLeft())->getIdentifier() == "x");
        REQUIRE(dynamic_pointer_cast<TypeDeclarationNode>(assignmentNodeX->getLeft()->getValue())->getType() == "Dict");
        REQUIRE(assignmentNodeX->getRight()->getNodeType() == "DictDefinition");

        shared_ptr<DictDefinitionNode> dictNode = dynamic_pointer_cast<DictDefinitionNode>(assignmentNodeX->getRight());
        REQUIRE(dictNode->getElements().size() == 1);
        shared_ptr<TupleNode> tupleNode = dynamic_pointer_cast<TupleNode>(dictNode->getElements()[0]);
        REQUIRE(tupleNode->getLeft()->getNodeType() == "Symbol");
        REQUIRE(dynamic_pointer_cast<SymbolNode>(tupleNode->getLeft())->getSymbol() == ":bob");
        REQUIRE(tupleNode->getRight()->getNodeType() == "StringLiteral");
        REQUIRE(dynamic_pointer_cast<LiteralNode>(tupleNode->getRight())->getLiteralValue() == "'saget'");

        // Check the second assignment within the block
        shared_ptr<AssignmentNode> assignmentNodeY = dynamic_pointer_cast<AssignmentNode>(blockExpressions[1]);
        REQUIRE(assignmentNodeY->getLeft()->getNodeType() == "Identifier");
        REQUIRE(dynamic_pointer_cast<IdentifierNode>(assignmentNodeY->getLeft())->getIdentifier() == "y");
        REQUIRE(dynamic_pointer_cast<TypeDeclarationNode>(assignmentNodeY->getLeft()->getValue())->getType() == "Number");
        REQUIRE(assignmentNodeY->getRight()->getNodeType() == "NumberLiteral");
        REQUIRE(dynamic_pointer_cast<LiteralNode>(assignmentNodeY->getRight())->getLiteralValue() == "432.11");
    }

    SECTION("Can resolve files with links correctly") {
        string source = R"(
            link Theta.StringUtil
            link Theta.StringTraversal

            capsule MyTestCapsule {
                x<String> = Theta.StringUtil.name
            }
        )";
        lexer.lex(source);

        shared_ptr<SourceNode> parsedAST = dynamic_pointer_cast<SourceNode>(
            parser.parse(lexer.tokens, source, "fakeFile.th", filesByCapsuleName)
        );

        REQUIRE(parsedAST->getNodeType() == "Source");
        REQUIRE(parsedAST->getLinks().size() == 2);

        // Check first link
        shared_ptr<LinkNode> linkNode1 = dynamic_pointer_cast<LinkNode>(parsedAST->getLinks()[0]);
        REQUIRE(linkNode1->getNodeType() == "Link");
        REQUIRE(linkNode1->capsule == "Theta.StringUtil");

        shared_ptr<SourceNode> linkedSource1 = dynamic_pointer_cast<SourceNode>(linkNode1->getValue());
        REQUIRE(linkedSource1->getNodeType() == "Source");
        REQUIRE(linkedSource1->getLinks().size() == 0);

        shared_ptr<CapsuleNode> linkedCapsule1 = dynamic_pointer_cast<CapsuleNode>(linkedSource1->getValue());
        REQUIRE(linkedCapsule1->getNodeType() == "Capsule");
        REQUIRE(linkedCapsule1->name == "Theta.StringUtil");

        shared_ptr<BlockNode> linkedBlock1 = dynamic_pointer_cast<BlockNode>(linkedCapsule1->getValue());
        REQUIRE(linkedBlock1->getNodeType() == "Block");
        REQUIRE(linkedBlock1->getBlockExpressions().size() == 1);

        shared_ptr<AssignmentNode> assignmentNode1 = dynamic_pointer_cast<AssignmentNode>(linkedBlock1->getBlockExpressions()[0]);
        REQUIRE(assignmentNode1->getLeft()->getNodeType() == "Identifier");
        REQUIRE(assignmentNode1->getRight()->getNodeType() == "StringLiteral");

        shared_ptr<IdentifierNode> leftNode1 = dynamic_pointer_cast<IdentifierNode>(assignmentNode1->getLeft());
        REQUIRE(leftNode1->getIdentifier() == "name");
        REQUIRE(dynamic_pointer_cast<TypeDeclarationNode>(leftNode1->getValue())->getType() == "String");

        shared_ptr<LiteralNode> rightNode1 = dynamic_pointer_cast<LiteralNode>(assignmentNode1->getRight());
        REQUIRE(rightNode1->getLiteralValue() == "'Bobby'");

        // Check second link
        shared_ptr<LinkNode> linkNode2 = dynamic_pointer_cast<LinkNode>(parsedAST->getLinks()[1]);
        REQUIRE(linkNode2->getNodeType() == "Link");
        REQUIRE(linkNode2->capsule == "Theta.StringTraversal");

        shared_ptr<SourceNode> linkedSource2 = dynamic_pointer_cast<SourceNode>(linkNode2->getValue());
        REQUIRE(linkedSource2->getNodeType() == "Source");
        REQUIRE(linkedSource2->getLinks().size() == 1);

        shared_ptr<LinkNode> nestedLinkNode2 = dynamic_pointer_cast<LinkNode>(linkedSource2->getLinks()[0]);
        REQUIRE(nestedLinkNode2->getNodeType() == "Link");
        REQUIRE(nestedLinkNode2->capsule == "Theta.StringUtil");

        shared_ptr<SourceNode> nestedLinkedSource2 = dynamic_pointer_cast<SourceNode>(nestedLinkNode2->getValue());
        REQUIRE(nestedLinkedSource2->getNodeType() == "Source");
        REQUIRE(nestedLinkedSource2->getLinks().size() == 0);

        shared_ptr<CapsuleNode> nestedLinkedCapsule2 = dynamic_pointer_cast<CapsuleNode>(nestedLinkedSource2->getValue());
        REQUIRE(nestedLinkedCapsule2->getNodeType() == "Capsule");
        REQUIRE(nestedLinkedCapsule2->name == "Theta.StringUtil");

        shared_ptr<BlockNode> nestedLinkedBlock2 = dynamic_pointer_cast<BlockNode>(nestedLinkedCapsule2->getValue());
        REQUIRE(nestedLinkedBlock2->getNodeType() == "Block");
        REQUIRE(nestedLinkedBlock2->getBlockExpressions().size() == 1);

        shared_ptr<AssignmentNode> nestedAssignmentNode2 = dynamic_pointer_cast<AssignmentNode>(nestedLinkedBlock2->getBlockExpressions()[0]);
        REQUIRE(nestedAssignmentNode2->getLeft()->getNodeType() == "Identifier");
        REQUIRE(nestedAssignmentNode2->getRight()->getNodeType() == "StringLiteral");

        shared_ptr<IdentifierNode> nestedLeftNode2 = dynamic_pointer_cast<IdentifierNode>(nestedAssignmentNode2->getLeft());
        REQUIRE(nestedLeftNode2->getIdentifier() == "name");
        REQUIRE(dynamic_pointer_cast<TypeDeclarationNode>(nestedLeftNode2->getValue())->getType() == "String");

        shared_ptr<LiteralNode> nestedRightNode2 = dynamic_pointer_cast<LiteralNode>(nestedAssignmentNode2->getRight());
        REQUIRE(nestedRightNode2->getLiteralValue() == "'Bobby'");

        // Check MyTestCapsule
        shared_ptr<CapsuleNode> mainCapsule = dynamic_pointer_cast<CapsuleNode>(parsedAST->getValue());
        REQUIRE(mainCapsule->getNodeType() == "Capsule");
        REQUIRE(mainCapsule->name == "MyTestCapsule");

        shared_ptr<BlockNode> mainBlock = dynamic_pointer_cast<BlockNode>(mainCapsule->getValue());
        REQUIRE(mainBlock->getNodeType() == "Block");
        REQUIRE(mainBlock->getBlockExpressions().size() == 1);

        shared_ptr<AssignmentNode> mainAssignmentNode = dynamic_pointer_cast<AssignmentNode>(mainBlock->getBlockExpressions()[0]);
        REQUIRE(mainAssignmentNode->getLeft()->getNodeType() == "Identifier");
        REQUIRE(mainAssignmentNode->getRight()->getNodeType() == "Identifier");

        shared_ptr<IdentifierNode> mainLeftNode = dynamic_pointer_cast<IdentifierNode>(mainAssignmentNode->getLeft());
        REQUIRE(mainLeftNode->getIdentifier() == "x");
        REQUIRE(dynamic_pointer_cast<TypeDeclarationNode>(mainLeftNode->getValue())->getType() == "String");

        shared_ptr<IdentifierNode> mainRightNode = dynamic_pointer_cast<IdentifierNode>(mainAssignmentNode->getRight());
        REQUIRE(mainRightNode->getIdentifier() == "Theta.StringUtil.name");
    }


    SECTION("Can tokenize with single line comments") {
        string source = "x<Number> = 5 + 3 // assignment";
        lexer.lex(source);

    }

    SECTION("Can tokenize with multi line comments") {
        string source = "x<Number> = 5 - /- comment -/ 3";
        lexer.lex(source);

    }

}
