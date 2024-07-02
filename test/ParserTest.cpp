#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch2/catch_amalgamated.hpp"
#include "../src/lexer/Lexer.cpp"
#include "../src/parser/Parser.cpp"
#include "../src/compiler/Compiler.hpp"

using namespace std;
using namespace Theta;

TEST_CASE("Parser") {
    Theta::Lexer lexer;
    Theta::Parser parser;

    shared_ptr<map<string, string>> filesByCapsuleName = Theta::Compiler::getInstance().filesByCapsuleName;

    // --------- PRIMITIVES ----------
    SECTION("Can parse numbers with decimals") {
        string source = "5 * 12.23";
        lexer.lex(source);

        shared_ptr<SourceNode> parsedAST = dynamic_pointer_cast<SourceNode>(
            parser.parse(lexer.tokens, source, "fakeFile.th", filesByCapsuleName)
        );

        REQUIRE(parsedAST->getNodeType() == ASTNode::Types::SOURCE);
        REQUIRE(parsedAST->getLinks().size() == 0);
        REQUIRE(parsedAST->getValue()->getNodeType() == ASTNode::Types::BINARY_OPERATION);

        shared_ptr<BinaryOperationNode> binOpNode = dynamic_pointer_cast<BinaryOperationNode>(parsedAST->getValue());

        REQUIRE(binOpNode->getOperator() == "*");
        REQUIRE(binOpNode->getLeft()->getNodeType() == ASTNode::Types::NUMBER_LITERAL);
        REQUIRE(binOpNode->getRight()->getNodeType() == ASTNode::Types::NUMBER_LITERAL);

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
        REQUIRE(lexer.tokens[0].getType() == Token::Types::IDENTIFIER);
        REQUIRE(lexer.tokens[0].getLexeme() == ".4.");
    }

    SECTION("Can parse a string") {
        string source = "'And his name is John Cena!'";
        lexer.lex(source);

        shared_ptr<SourceNode> parsedAST = dynamic_pointer_cast<SourceNode>(
            parser.parse(lexer.tokens, source, "fakeFile.th", filesByCapsuleName)
        );

        REQUIRE(parsedAST->getNodeType() == ASTNode::Types::SOURCE);
        REQUIRE(parsedAST->getLinks().size() == 0);
        REQUIRE(parsedAST->getValue()->getNodeType() == ASTNode::Types::STRING_LITERAL);

        shared_ptr<LiteralNode> childNode = dynamic_pointer_cast<LiteralNode>(parsedAST->getValue());

        REQUIRE(childNode->getLiteralValue() == "'And his name is John Cena!'");
    }

    SECTION("Can parse symbols") {
        string source = ":atomic_nuclei";
        lexer.lex(source);

        shared_ptr<SourceNode> parsedAST = dynamic_pointer_cast<SourceNode>(
            parser.parse(lexer.tokens, source, "fakeFile.th", filesByCapsuleName)
        );

        REQUIRE(parsedAST->getNodeType() == ASTNode::Types::SOURCE);
        REQUIRE(parsedAST->getLinks().size() == 0);
        REQUIRE(parsedAST->getValue()->getNodeType() == ASTNode::Types::SYMBOL);

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

        REQUIRE(parsedAST->getNodeType() == ASTNode::Types::SOURCE);
        REQUIRE(parsedAST->getLinks().size() == 0);
        REQUIRE(parsedAST->getValue()->getNodeType() == ASTNode::Types::LIST);

        shared_ptr<ListNode> listNode = dynamic_pointer_cast<ListNode>(parsedAST->getValue());

        REQUIRE(listNode->getElements().size() == 4);

        vector<string> expectedValues = { "'Alex'", "'Tony'", "'John'", "'Denis'" };

        for (int i = 0; i < listNode->getElements().size(); i++) {
            shared_ptr<LiteralNode> literalNode = dynamic_pointer_cast<LiteralNode>(listNode->getElements()[i]);

            REQUIRE(literalNode->getNodeType() == ASTNode::Types::STRING_LITERAL);
            REQUIRE(literalNode->getLiteralValue() == expectedValues[i]);
        }
    }

    SECTION("Can parse a nested list of strings") {
        string source = "[['alex', 'john', ['jeremy', 'pablo']], ['clarinda'], 'den' + 'is', 'jessica', 'ellis']";
        lexer.lex(source);

        shared_ptr<SourceNode> parsedAST = dynamic_pointer_cast<SourceNode>(
            parser.parse(lexer.tokens, source, "fakeFile.th", filesByCapsuleName)
        );

        REQUIRE(parsedAST->getNodeType() == ASTNode::Types::SOURCE);
        REQUIRE(parsedAST->getLinks().size() == 0);
        REQUIRE(parsedAST->getValue()->getNodeType() == ASTNode::Types::LIST);

        shared_ptr<ListNode> listNode = dynamic_pointer_cast<ListNode>(parsedAST->getValue());

        REQUIRE(listNode->getElements().size() == 5);
        REQUIRE(listNode->getElements()[0]->getNodeType() == ASTNode::Types::LIST);

        shared_ptr<ListNode> listElement0 = dynamic_pointer_cast<ListNode>(listNode->getElements()[0]);

        REQUIRE(listElement0->getElements().size() == 3);
        REQUIRE(listElement0->getElements()[0]->getNodeType() == ASTNode::Types::STRING_LITERAL);
        REQUIRE(listElement0->getElements()[1]->getNodeType() == ASTNode::Types::STRING_LITERAL);
        REQUIRE(listElement0->getElements()[2]->getNodeType() == ASTNode::Types::LIST);
        REQUIRE(dynamic_pointer_cast<LiteralNode>(listElement0->getElements()[0])->getLiteralValue() == "'alex'");
        REQUIRE(dynamic_pointer_cast<LiteralNode>(listElement0->getElements()[1])->getLiteralValue() == "'john'");

        shared_ptr<ListNode> listElement0Element2 = dynamic_pointer_cast<ListNode>(listElement0->getElements()[2]);
        REQUIRE(listElement0Element2->getElements().size() == 2);
        REQUIRE(listElement0Element2->getElements()[0]->getNodeType() == ASTNode::Types::STRING_LITERAL);
        REQUIRE(listElement0Element2->getElements()[1]->getNodeType() == ASTNode::Types::STRING_LITERAL);
        REQUIRE(dynamic_pointer_cast<LiteralNode>(listElement0Element2->getElements()[0])->getLiteralValue() == "'jeremy'");
        REQUIRE(dynamic_pointer_cast<LiteralNode>(listElement0Element2->getElements()[1])->getLiteralValue() == "'pablo'");

        REQUIRE(listNode->getElements()[1]->getNodeType() == ASTNode::Types::LIST);

        shared_ptr<ListNode> listElement1 = dynamic_pointer_cast<ListNode>(listNode->getElements()[1]);

        REQUIRE(listElement1->getElements().size() == 1);
        REQUIRE(listElement1->getElements()[0]->getNodeType() == ASTNode::Types::STRING_LITERAL);
        REQUIRE(dynamic_pointer_cast<LiteralNode>(listElement1->getElements()[0])->getLiteralValue() == "'clarinda'");

        REQUIRE(listNode->getElements()[2]->getNodeType() == ASTNode::Types::BINARY_OPERATION);

        shared_ptr<BinaryOperationNode> listElement2 = dynamic_pointer_cast<BinaryOperationNode>(listNode->getElements()[2]);
        REQUIRE(listElement2->getOperator() == "+");
        REQUIRE(listElement2->getLeft()->getNodeType() == ASTNode::Types::STRING_LITERAL);
        REQUIRE(listElement2->getRight()->getNodeType() == ASTNode::Types::STRING_LITERAL);
        REQUIRE(dynamic_pointer_cast<LiteralNode>(listElement2->getLeft())->getLiteralValue() == "'den'");
        REQUIRE(dynamic_pointer_cast<LiteralNode>(listElement2->getRight())->getLiteralValue() == "'is'");

        REQUIRE(listNode->getElements()[3]->getNodeType() == ASTNode::Types::STRING_LITERAL);
        REQUIRE(listNode->getElements()[4]->getNodeType() == ASTNode::Types::STRING_LITERAL);
        REQUIRE(dynamic_pointer_cast<LiteralNode>(listNode->getElements()[3])->getLiteralValue() == "'jessica'");
        REQUIRE(dynamic_pointer_cast<LiteralNode>(listNode->getElements()[4])->getLiteralValue() == "'ellis'");
    }

    SECTION("Can parse tuple") {
        string source = "{ :ok, 'Success' }";
        lexer.lex(source);

        shared_ptr<SourceNode> parsedAST = dynamic_pointer_cast<SourceNode>(
            parser.parse(lexer.tokens, source, "fakeFile.th", filesByCapsuleName)
        );

        REQUIRE(parsedAST->getNodeType() == ASTNode::Types::SOURCE);
        REQUIRE(parsedAST->getLinks().size() == 0);

        shared_ptr<TupleNode> tupleNode = dynamic_pointer_cast<TupleNode>(parsedAST->getValue());
        REQUIRE(tupleNode != nullptr);
        REQUIRE(tupleNode->getNodeType() == ASTNode::Types::TUPLE);

        shared_ptr<SymbolNode> firstNode = dynamic_pointer_cast<SymbolNode>(tupleNode->getLeft());
        REQUIRE(firstNode != nullptr);
        REQUIRE(firstNode->getNodeType() == ASTNode::Types::SYMBOL);
        REQUIRE(firstNode->getSymbol() == ":ok");

        shared_ptr<LiteralNode> secondNode = dynamic_pointer_cast<LiteralNode>(tupleNode->getRight());
        REQUIRE(secondNode != nullptr);
        REQUIRE(secondNode->getNodeType() == ASTNode::Types::STRING_LITERAL);
        REQUIRE(secondNode->getLiteralValue() == "'Success'");
    }
    // --------- DATA STRUCTURES -----------

    // --------- ARITHMETIC OPERATORS ----------
    SECTION("Can parse addition") {
        string source = "100 + 7";
        lexer.lex(source);

        shared_ptr<SourceNode> parsedAST = dynamic_pointer_cast<SourceNode>(
            parser.parse(lexer.tokens, source, "fakeFile.th", filesByCapsuleName)
        );

        REQUIRE(parsedAST->getNodeType() == ASTNode::Types::SOURCE);
        REQUIRE(parsedAST->getLinks().size() == 0);
        REQUIRE(parsedAST->getValue()->getNodeType() == ASTNode::Types::BINARY_OPERATION);

        shared_ptr<BinaryOperationNode> binOpNode = dynamic_pointer_cast<BinaryOperationNode>(parsedAST->getValue());

        REQUIRE(binOpNode->getOperator() == "+");
        REQUIRE(binOpNode->getLeft()->getNodeType() == ASTNode::Types::NUMBER_LITERAL);
        REQUIRE(binOpNode->getRight()->getNodeType() == ASTNode::Types::NUMBER_LITERAL);
        REQUIRE(dynamic_pointer_cast<LiteralNode>(binOpNode->getLeft())->getLiteralValue() == "100");
        REQUIRE(dynamic_pointer_cast<LiteralNode>(binOpNode->getRight())->getLiteralValue() == "7");
    }

    SECTION("Can parse subtraction") {
        string source = "42 - 11";
        lexer.lex(source);

        shared_ptr<SourceNode> parsedAST = dynamic_pointer_cast<SourceNode>(
            parser.parse(lexer.tokens, source, "fakeFile.th", filesByCapsuleName)
        );

        REQUIRE(parsedAST->getNodeType() == ASTNode::Types::SOURCE);
        REQUIRE(parsedAST->getLinks().size() == 0);
        REQUIRE(parsedAST->getValue()->getNodeType() == ASTNode::Types::BINARY_OPERATION);

        shared_ptr<BinaryOperationNode> binOpNode = dynamic_pointer_cast<BinaryOperationNode>(parsedAST->getValue());

        REQUIRE(binOpNode->getOperator() == "-");
        REQUIRE(binOpNode->getLeft()->getNodeType() == ASTNode::Types::NUMBER_LITERAL);
        REQUIRE(binOpNode->getRight()->getNodeType() == ASTNode::Types::NUMBER_LITERAL);
        REQUIRE(dynamic_pointer_cast<LiteralNode>(binOpNode->getLeft())->getLiteralValue() == "42");
        REQUIRE(dynamic_pointer_cast<LiteralNode>(binOpNode->getRight())->getLiteralValue() == "11");
    }

    SECTION("Can parse multiplication") {
        string source = "10 * 11";
        lexer.lex(source);

        shared_ptr<SourceNode> parsedAST = dynamic_pointer_cast<SourceNode>(
            parser.parse(lexer.tokens, source, "fakeFile.th", filesByCapsuleName)
        );

        REQUIRE(parsedAST->getNodeType() == ASTNode::Types::SOURCE);
        REQUIRE(parsedAST->getLinks().size() == 0);
        REQUIRE(parsedAST->getValue()->getNodeType() == ASTNode::Types::BINARY_OPERATION);

        shared_ptr<BinaryOperationNode> binOpNode = dynamic_pointer_cast<BinaryOperationNode>(parsedAST->getValue());

        REQUIRE(binOpNode->getOperator() == "*");
        REQUIRE(binOpNode->getLeft()->getNodeType() == ASTNode::Types::NUMBER_LITERAL);
        REQUIRE(binOpNode->getRight()->getNodeType() == ASTNode::Types::NUMBER_LITERAL);
        REQUIRE(dynamic_pointer_cast<LiteralNode>(binOpNode->getLeft())->getLiteralValue() == "10");
        REQUIRE(dynamic_pointer_cast<LiteralNode>(binOpNode->getRight())->getLiteralValue() == "11");
    }

    SECTION("Can parse division") {
        string source = "100/5";
        lexer.lex(source);

        shared_ptr<SourceNode> parsedAST = dynamic_pointer_cast<SourceNode>(
            parser.parse(lexer.tokens, source, "fakeFile.th", filesByCapsuleName)
        );

        REQUIRE(parsedAST->getNodeType() == ASTNode::Types::SOURCE);
        REQUIRE(parsedAST->getLinks().size() == 0);
        REQUIRE(parsedAST->getValue()->getNodeType() == ASTNode::Types::BINARY_OPERATION);

        shared_ptr<BinaryOperationNode> binOpNode = dynamic_pointer_cast<BinaryOperationNode>(parsedAST->getValue());

        REQUIRE(binOpNode->getOperator() == "/");
        REQUIRE(binOpNode->getLeft()->getNodeType() == ASTNode::Types::NUMBER_LITERAL);
        REQUIRE(binOpNode->getRight()->getNodeType() == ASTNode::Types::NUMBER_LITERAL);
        REQUIRE(dynamic_pointer_cast<LiteralNode>(binOpNode->getLeft())->getLiteralValue() == "100");
        REQUIRE(dynamic_pointer_cast<LiteralNode>(binOpNode->getRight())->getLiteralValue() == "5");
    }

    SECTION("Can parse exponentiation") {
        string source = "2 ** 3";
        lexer.lex(source);

        shared_ptr<SourceNode> parsedAST = dynamic_pointer_cast<SourceNode>(
            parser.parse(lexer.tokens, source, "fakeFile.th", filesByCapsuleName)
        );

        REQUIRE(parsedAST->getNodeType() == ASTNode::Types::SOURCE);
        REQUIRE(parsedAST->getLinks().size() == 0);
        REQUIRE(parsedAST->getValue()->getNodeType() == ASTNode::Types::BINARY_OPERATION);

        shared_ptr<BinaryOperationNode> binOpNode = dynamic_pointer_cast<BinaryOperationNode>(parsedAST->getValue());

        REQUIRE(binOpNode->getOperator() == "**");
        REQUIRE(binOpNode->getLeft()->getNodeType() == ASTNode::Types::NUMBER_LITERAL);
        REQUIRE(binOpNode->getRight()->getNodeType() == ASTNode::Types::NUMBER_LITERAL);
        REQUIRE(dynamic_pointer_cast<LiteralNode>(binOpNode->getLeft())->getLiteralValue() == "2");
        REQUIRE(dynamic_pointer_cast<LiteralNode>(binOpNode->getRight())->getLiteralValue() == "3");
    }

    SECTION("Can parse modulo") {
        string source = "9 % 2";
        lexer.lex(source);

        shared_ptr<SourceNode> parsedAST = dynamic_pointer_cast<SourceNode>(
            parser.parse(lexer.tokens, source, "fakeFile.th", filesByCapsuleName)
        );

        REQUIRE(parsedAST->getNodeType() == ASTNode::Types::SOURCE);
        REQUIRE(parsedAST->getLinks().size() == 0);
        REQUIRE(parsedAST->getValue()->getNodeType() == ASTNode::Types::BINARY_OPERATION);

        shared_ptr<BinaryOperationNode> binOpNode = dynamic_pointer_cast<BinaryOperationNode>(parsedAST->getValue());

        REQUIRE(binOpNode->getOperator() == "%");
        REQUIRE(binOpNode->getLeft()->getNodeType() == ASTNode::Types::NUMBER_LITERAL);
        REQUIRE(binOpNode->getRight()->getNodeType() == ASTNode::Types::NUMBER_LITERAL);
        REQUIRE(dynamic_pointer_cast<LiteralNode>(binOpNode->getLeft())->getLiteralValue() == "9");
        REQUIRE(dynamic_pointer_cast<LiteralNode>(binOpNode->getRight())->getLiteralValue() == "2");
    }

    SECTION("Can parse negative numbers") {
        string source = "100.4 - -24";
        lexer.lex(source);

        shared_ptr<SourceNode> parsedAST = dynamic_pointer_cast<SourceNode>(
            parser.parse(lexer.tokens, source, "fakeFile.th", filesByCapsuleName)
        );

        REQUIRE(parsedAST->getNodeType() == ASTNode::Types::SOURCE);
        REQUIRE(parsedAST->getLinks().size() == 0);
        REQUIRE(parsedAST->getValue()->getNodeType() == ASTNode::Types::BINARY_OPERATION);

        shared_ptr<BinaryOperationNode> binOpNode = dynamic_pointer_cast<BinaryOperationNode>(parsedAST->getValue());

        REQUIRE(binOpNode->getOperator() == "-");
        REQUIRE(binOpNode->getLeft()->getNodeType() == ASTNode::Types::NUMBER_LITERAL);
        REQUIRE(binOpNode->getRight()->getNodeType() == ASTNode::Types::UNARY_OPERATION);
        REQUIRE(dynamic_pointer_cast<LiteralNode>(binOpNode->getLeft())->getLiteralValue() == "100.4");

        REQUIRE(binOpNode->getRight()->getValue()->getNodeType() == ASTNode::Types::NUMBER_LITERAL);
        REQUIRE(dynamic_pointer_cast<UnaryOperationNode>(binOpNode->getRight())->getOperator() == "-");
        REQUIRE(dynamic_pointer_cast<LiteralNode>(binOpNode->getRight()->getValue())->getLiteralValue() == "24");
    }

    SECTION("Can complex arithmetic expressions") {
        string source = "12 + (23 - -1) * 7 ** 2.4";
        lexer.lex(source);

        shared_ptr<SourceNode> parsedAST = dynamic_pointer_cast<SourceNode>(
            parser.parse(lexer.tokens, source, "fakeFile.th", filesByCapsuleName)
        );

        REQUIRE(parsedAST->getNodeType() == ASTNode::Types::SOURCE);
        REQUIRE(parsedAST->getLinks().size() == 0);
        REQUIRE(parsedAST->getValue()->getNodeType() == ASTNode::Types::BINARY_OPERATION);

        shared_ptr<BinaryOperationNode> binOpNode = dynamic_pointer_cast<BinaryOperationNode>(parsedAST->getValue());

        REQUIRE(binOpNode->getOperator() == "+");
        REQUIRE(binOpNode->getLeft()->getNodeType() == ASTNode::Types::NUMBER_LITERAL);
        REQUIRE(dynamic_pointer_cast<LiteralNode>(binOpNode->getLeft())->getLiteralValue() == "12");

        REQUIRE(binOpNode->getRight()->getNodeType() == ASTNode::Types::BINARY_OPERATION);

        shared_ptr<BinaryOperationNode> binOpNode2 = dynamic_pointer_cast<BinaryOperationNode>(binOpNode->getRight());

        REQUIRE(binOpNode2->getOperator() == "*");
        REQUIRE(binOpNode2->getLeft()->getNodeType() == ASTNode::Types::BINARY_OPERATION);
        REQUIRE(binOpNode2->getRight()->getNodeType() == ASTNode::Types::BINARY_OPERATION);

        shared_ptr<BinaryOperationNode> binOpNode2LeftBinOpNode = dynamic_pointer_cast<BinaryOperationNode>(binOpNode2->getLeft());
        shared_ptr<BinaryOperationNode> binOpNode2RightBinOpNode = dynamic_pointer_cast<BinaryOperationNode>(binOpNode2->getRight());

        REQUIRE(binOpNode2LeftBinOpNode->getOperator() == "-");
        REQUIRE(binOpNode2LeftBinOpNode->getLeft()->getNodeType() == ASTNode::Types::NUMBER_LITERAL);
        REQUIRE(dynamic_pointer_cast<LiteralNode>(binOpNode2LeftBinOpNode->getLeft())->getLiteralValue() == "23");
        REQUIRE(binOpNode2LeftBinOpNode->getRight()->getNodeType() == ASTNode::Types::UNARY_OPERATION);

        shared_ptr<UnaryOperationNode> unaryOpNode = dynamic_pointer_cast<UnaryOperationNode>(binOpNode2LeftBinOpNode->getRight());

        REQUIRE(unaryOpNode->getOperator() == "-");
        REQUIRE(unaryOpNode->getValue()->getNodeType() == ASTNode::Types::NUMBER_LITERAL);
        REQUIRE(dynamic_pointer_cast<LiteralNode>(unaryOpNode->getValue())->getLiteralValue() == "1");

        REQUIRE(binOpNode2RightBinOpNode->getOperator() == "**");
        REQUIRE(binOpNode2RightBinOpNode->getLeft()->getNodeType() == ASTNode::Types::NUMBER_LITERAL);
        REQUIRE(binOpNode2RightBinOpNode->getRight()->getNodeType() == ASTNode::Types::NUMBER_LITERAL);
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

        REQUIRE(parsedAST->getNodeType() == ASTNode::Types::SOURCE);
        REQUIRE(parsedAST->getLinks().size() == 0);
        REQUIRE(parsedAST->getValue()->getNodeType() == ASTNode::Types::BINARY_OPERATION);

        shared_ptr<BinaryOperationNode> binOpNode = dynamic_pointer_cast<BinaryOperationNode>(parsedAST->getValue());

        REQUIRE(binOpNode->getOperator() == "&&");
        REQUIRE(binOpNode->getLeft()->getNodeType() == ASTNode::Types::BOOLEAN_LITERAL);
        REQUIRE(binOpNode->getRight()->getNodeType() == ASTNode::Types::BOOLEAN_LITERAL);
        REQUIRE(dynamic_pointer_cast<LiteralNode>(binOpNode->getLeft())->getLiteralValue() == "true");
        REQUIRE(dynamic_pointer_cast<LiteralNode>(binOpNode->getRight())->getLiteralValue() == "false");
    }

    SECTION("Can parse boolean with unary logic") {
        string source = "true && !false";
        lexer.lex(source);

        shared_ptr<SourceNode> parsedAST = dynamic_pointer_cast<SourceNode>(
            parser.parse(lexer.tokens, source, "fakeFile.th", filesByCapsuleName)
        );

        REQUIRE(parsedAST->getNodeType() == ASTNode::Types::SOURCE);
        REQUIRE(parsedAST->getLinks().size() == 0);
        REQUIRE(parsedAST->getValue()->getNodeType() == ASTNode::Types::BINARY_OPERATION);

        shared_ptr<BinaryOperationNode> binOpNode = dynamic_pointer_cast<BinaryOperationNode>(parsedAST->getValue());

        REQUIRE(binOpNode->getOperator() == "&&");
        REQUIRE(binOpNode->getLeft()->getNodeType() == ASTNode::Types::BOOLEAN_LITERAL);
        REQUIRE(binOpNode->getRight()->getNodeType() == ASTNode::Types::UNARY_OPERATION);
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

        REQUIRE(parsedAST->getNodeType() == ASTNode::Types::SOURCE);
        REQUIRE(parsedAST->getLinks().size() == 0);
        REQUIRE(parsedAST->getValue()->getNodeType() == ASTNode::Types::BINARY_OPERATION);

        shared_ptr<BinaryOperationNode> binOpNode = dynamic_pointer_cast<BinaryOperationNode>(parsedAST->getValue());

        REQUIRE(binOpNode->getOperator() == "||");
        REQUIRE(binOpNode->getLeft()->getNodeType() == ASTNode::Types::UNARY_OPERATION);
        REQUIRE(binOpNode->getRight()->getNodeType() == ASTNode::Types::IDENTIFIER);
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

        REQUIRE(parsedAST->getNodeType() == ASTNode::Types::SOURCE);
        REQUIRE(parsedAST->getLinks().size() == 0);
        REQUIRE(parsedAST->getValue()->getNodeType() == ASTNode::Types::BINARY_OPERATION);

        shared_ptr<BinaryOperationNode> binOpNode = dynamic_pointer_cast<BinaryOperationNode>(parsedAST->getValue());

        REQUIRE(binOpNode->getOperator() == "==");
        REQUIRE(binOpNode->getLeft()->getNodeType() == ASTNode::Types::IDENTIFIER);
        REQUIRE(binOpNode->getRight()->getNodeType() == ASTNode::Types::IDENTIFIER);
        REQUIRE(dynamic_pointer_cast<IdentifierNode>(binOpNode->getLeft())->getIdentifier() == "x");
        REQUIRE(dynamic_pointer_cast<IdentifierNode>(binOpNode->getRight())->getIdentifier() == "y");
    }

    SECTION("Can parse inequality logic with identifiers") {
        string source = "true != false";
        lexer.lex(source);

        shared_ptr<SourceNode> parsedAST = dynamic_pointer_cast<SourceNode>(
            parser.parse(lexer.tokens, source, "fakeFile.th", filesByCapsuleName)
        );

        REQUIRE(parsedAST->getNodeType() == ASTNode::Types::SOURCE);
        REQUIRE(parsedAST->getLinks().size() == 0);
        REQUIRE(parsedAST->getValue()->getNodeType() == ASTNode::Types::BINARY_OPERATION);

        shared_ptr<BinaryOperationNode> binOpNode = dynamic_pointer_cast<BinaryOperationNode>(parsedAST->getValue());

        REQUIRE(binOpNode->getOperator() == "!=");
        REQUIRE(binOpNode->getLeft()->getNodeType() == ASTNode::Types::BOOLEAN_LITERAL);
        REQUIRE(binOpNode->getRight()->getNodeType() == ASTNode::Types::BOOLEAN_LITERAL);
        REQUIRE(dynamic_pointer_cast<LiteralNode>(binOpNode->getLeft())->getLiteralValue() == "true");
        REQUIRE(dynamic_pointer_cast<LiteralNode>(binOpNode->getRight())->getLiteralValue() == "false");
    }
    // --------- LOGICAL OPERATORS ----------

    // --------- PIPELINE OPERATOR ----------
    SECTION("Can parse pipeline operator with missing lefthand binary operation") {
        string source = "'hello' => + ' mike' => + ' how are you'";
        lexer.lex(source);

        shared_ptr<SourceNode> parsedAST = dynamic_pointer_cast<SourceNode>(
            parser.parse(lexer.tokens, source, "fakeFile.th", filesByCapsuleName)
        );

        REQUIRE(parsedAST->getNodeType() == ASTNode::Types::SOURCE);
        REQUIRE(parsedAST->getLinks().size() == 0);
        REQUIRE(parsedAST->getValue()->getNodeType() == ASTNode::Types::BINARY_OPERATION);

        shared_ptr<BinaryOperationNode> firstPipelineOp = dynamic_pointer_cast<BinaryOperationNode>(parsedAST->getValue());
        REQUIRE(firstPipelineOp->getOperator() == "=>");
        REQUIRE(firstPipelineOp->getRight()->getNodeType() == ASTNode::Types::BINARY_OPERATION);

        shared_ptr<BinaryOperationNode> rightConcatOp = dynamic_pointer_cast<BinaryOperationNode>(firstPipelineOp->getRight());
        REQUIRE(rightConcatOp->getOperator() == "+");
        REQUIRE(rightConcatOp->getLeft() == nullptr);
        REQUIRE(rightConcatOp->getRight()->getNodeType() == ASTNode::Types::STRING_LITERAL);
        REQUIRE(dynamic_pointer_cast<LiteralNode>(rightConcatOp->getRight())->getLiteralValue() == "' how are you'");

        shared_ptr<BinaryOperationNode> secondPipelineOp = dynamic_pointer_cast<BinaryOperationNode>(firstPipelineOp->getLeft());
        REQUIRE(secondPipelineOp->getOperator() == "=>");
        REQUIRE(secondPipelineOp->getRight()->getNodeType() == ASTNode::Types::BINARY_OPERATION);

        shared_ptr<BinaryOperationNode> leftConcatOp = dynamic_pointer_cast<BinaryOperationNode>(secondPipelineOp->getRight());
        REQUIRE(leftConcatOp->getOperator() == "+");
        REQUIRE(leftConcatOp->getLeft() == nullptr);
        REQUIRE(leftConcatOp->getRight()->getNodeType() == ASTNode::Types::STRING_LITERAL);
        REQUIRE(dynamic_pointer_cast<LiteralNode>(leftConcatOp->getRight())->getLiteralValue() == "' mike'");

        REQUIRE(secondPipelineOp->getLeft()->getNodeType() == ASTNode::Types::STRING_LITERAL);
        REQUIRE(dynamic_pointer_cast<LiteralNode>(secondPipelineOp->getLeft())->getLiteralValue() == "'hello'");
    }
    // --------- PIPELINE OPERATOR ----------

    // --------- ASSIGNMENT ----------
    SECTION("Can parse a string assignment") {
        string source = "message<String> = 'Hello, World!'";
        lexer.lex(source);

        shared_ptr<SourceNode> parsedAST = dynamic_pointer_cast<SourceNode>(
            parser.parse(lexer.tokens, source, "fakeFile.th", filesByCapsuleName)
        );

        REQUIRE(parsedAST->getNodeType() == ASTNode::Types::SOURCE);
        REQUIRE(parsedAST->getLinks().size() == 0);
        REQUIRE(parsedAST->getValue()->getNodeType() == ASTNode::Types::ASSIGNMENT);

        shared_ptr<AssignmentNode> assignmentNode = dynamic_pointer_cast<AssignmentNode>(parsedAST->getValue());
        REQUIRE(assignmentNode->getLeft()->getNodeType() == ASTNode::Types::IDENTIFIER);
        REQUIRE(assignmentNode->getRight()->getNodeType() == ASTNode::Types::STRING_LITERAL);
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

        REQUIRE(parsedAST->getNodeType() == ASTNode::Types::SOURCE);
        REQUIRE(parsedAST->getLinks().size() == 0);
        REQUIRE(parsedAST->getValue()->getNodeType() == ASTNode::Types::ASSIGNMENT);

        shared_ptr<AssignmentNode> assignmentNode = dynamic_pointer_cast<AssignmentNode>(parsedAST->getValue());
        REQUIRE(assignmentNode->getLeft()->getNodeType() == ASTNode::Types::IDENTIFIER);
        REQUIRE(assignmentNode->getRight()->getNodeType() == ASTNode::Types::BOOLEAN_LITERAL);
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

        REQUIRE(parsedAST->getNodeType() == ASTNode::Types::SOURCE);
        REQUIRE(parsedAST->getLinks().size() == 0);
        REQUIRE(parsedAST->getValue()->getNodeType() == ASTNode::Types::ASSIGNMENT);

        shared_ptr<AssignmentNode> assignmentNode = dynamic_pointer_cast<AssignmentNode>(parsedAST->getValue());
        REQUIRE(assignmentNode->getLeft()->getNodeType() == ASTNode::Types::IDENTIFIER);
        REQUIRE(assignmentNode->getRight()->getNodeType() == ASTNode::Types::BINARY_OPERATION);
        REQUIRE(dynamic_pointer_cast<IdentifierNode>(assignmentNode->getLeft())->getIdentifier() == "isOpen");
        REQUIRE(dynamic_pointer_cast<TypeDeclarationNode>(assignmentNode->getLeft()->getValue())->getType() == "Boolean");

        shared_ptr<BinaryOperationNode> binOpNode = dynamic_pointer_cast<BinaryOperationNode>(assignmentNode->getRight());

        REQUIRE(binOpNode->getOperator() == "&&");
        REQUIRE(binOpNode->getLeft()->getNodeType() == ASTNode::Types::UNARY_OPERATION);
        REQUIRE(dynamic_pointer_cast<UnaryOperationNode>(binOpNode->getLeft())->getOperator() == "!");
        REQUIRE(binOpNode->getLeft()->getValue()->getNodeType() == ASTNode::Types::BOOLEAN_LITERAL);
        REQUIRE(dynamic_pointer_cast<LiteralNode>(binOpNode->getLeft()->getValue())->getLiteralValue() == "true");
        REQUIRE(binOpNode->getRight()->getNodeType() == ASTNode::Types::BOOLEAN_LITERAL);
        REQUIRE(dynamic_pointer_cast<LiteralNode>(binOpNode->getRight())->getLiteralValue() == "false");
    }

    SECTION("Can parse arithmetic assignment") {
        string source = "total<Number> = 5 + 3";
        lexer.lex(source);

        shared_ptr<SourceNode> parsedAST = dynamic_pointer_cast<SourceNode>(
            parser.parse(lexer.tokens, source, "fakeFile.th", filesByCapsuleName)
        );

        REQUIRE(parsedAST->getNodeType() == ASTNode::Types::SOURCE);
        REQUIRE(parsedAST->getLinks().size() == 0);
        REQUIRE(parsedAST->getValue()->getNodeType() == ASTNode::Types::ASSIGNMENT);

        shared_ptr<AssignmentNode> assignmentNode = dynamic_pointer_cast<AssignmentNode>(parsedAST->getValue());
        REQUIRE(assignmentNode->getLeft()->getNodeType() == ASTNode::Types::IDENTIFIER);
        REQUIRE(assignmentNode->getRight()->getNodeType() == ASTNode::Types::BINARY_OPERATION);
        REQUIRE(dynamic_pointer_cast<IdentifierNode>(assignmentNode->getLeft())->getIdentifier() == "total");
        REQUIRE(dynamic_pointer_cast<TypeDeclarationNode>(assignmentNode->getLeft()->getValue())->getType() == "Number");

        shared_ptr<BinaryOperationNode> binOpNode = dynamic_pointer_cast<BinaryOperationNode>(assignmentNode->getRight());

        REQUIRE(binOpNode->getOperator() == "+");
        REQUIRE(binOpNode->getLeft()->getNodeType() == ASTNode::Types::NUMBER_LITERAL);
        REQUIRE(dynamic_pointer_cast<LiteralNode>(binOpNode->getLeft())->getLiteralValue() == "5");
        REQUIRE(binOpNode->getRight()->getNodeType() == ASTNode::Types::NUMBER_LITERAL);
        REQUIRE(dynamic_pointer_cast<LiteralNode>(binOpNode->getRight())->getLiteralValue() == "3");
    }

    SECTION("Can parse arithmetic assignment with unary") {
        string source = "total<Number> = 5 + -3";
        lexer.lex(source);

        shared_ptr<SourceNode> parsedAST = dynamic_pointer_cast<SourceNode>(
            parser.parse(lexer.tokens, source, "fakeFile.th", filesByCapsuleName)
        );

        REQUIRE(parsedAST->getNodeType() == ASTNode::Types::SOURCE);
        REQUIRE(parsedAST->getLinks().size() == 0);
        REQUIRE(parsedAST->getValue()->getNodeType() == ASTNode::Types::ASSIGNMENT);

        shared_ptr<AssignmentNode> assignmentNode = dynamic_pointer_cast<AssignmentNode>(parsedAST->getValue());
        REQUIRE(assignmentNode->getLeft()->getNodeType() == ASTNode::Types::IDENTIFIER);
        REQUIRE(assignmentNode->getRight()->getNodeType() == ASTNode::Types::BINARY_OPERATION);
        REQUIRE(dynamic_pointer_cast<IdentifierNode>(assignmentNode->getLeft())->getIdentifier() == "total");
        REQUIRE(dynamic_pointer_cast<TypeDeclarationNode>(assignmentNode->getLeft()->getValue())->getType() == "Number");

        shared_ptr<BinaryOperationNode> binOpNode = dynamic_pointer_cast<BinaryOperationNode>(assignmentNode->getRight());

        REQUIRE(binOpNode->getOperator() == "+");
        REQUIRE(binOpNode->getLeft()->getNodeType() == ASTNode::Types::NUMBER_LITERAL);
        REQUIRE(dynamic_pointer_cast<LiteralNode>(binOpNode->getLeft())->getLiteralValue() == "5");
        REQUIRE(binOpNode->getRight()->getNodeType() == ASTNode::Types::UNARY_OPERATION);
        REQUIRE(dynamic_pointer_cast<UnaryOperationNode>(binOpNode->getRight())->getOperator() == "-");
        REQUIRE(binOpNode->getRight()->getValue()->getNodeType() == ASTNode::Types::NUMBER_LITERAL);
        REQUIRE(dynamic_pointer_cast<LiteralNode>(binOpNode->getRight()->getValue())->getLiteralValue() == "3");
    }

    SECTION("Can parse list assignment with expressions") {
        string source = "x<List<Boolean>> = [x == y, y + 5 > 9, 'meow' != 'cow', !gross]";
        lexer.lex(source);

        shared_ptr<SourceNode> parsedAST = dynamic_pointer_cast<SourceNode>(
            parser.parse(lexer.tokens, source, "fakeFile.th", filesByCapsuleName)
        );

        REQUIRE(parsedAST->getNodeType() == ASTNode::Types::SOURCE);
        REQUIRE(parsedAST->getLinks().size() == 0);
        REQUIRE(parsedAST->getValue()->getNodeType() == ASTNode::Types::ASSIGNMENT);

        shared_ptr<AssignmentNode> assignmentNode = dynamic_pointer_cast<AssignmentNode>(parsedAST->getValue());
        REQUIRE(assignmentNode->getLeft()->getNodeType() == ASTNode::Types::IDENTIFIER);
        REQUIRE(assignmentNode->getRight()->getNodeType() == ASTNode::Types::LIST);
        REQUIRE(dynamic_pointer_cast<IdentifierNode>(assignmentNode->getLeft())->getIdentifier() == "x");

        shared_ptr<ListNode> listDefNode = dynamic_pointer_cast<ListNode>(assignmentNode->getRight());
        REQUIRE(listDefNode->getElements().size() == 4);

        // Checking first element: x == y
        shared_ptr<BinaryOperationNode> binOpNode1 = dynamic_pointer_cast<BinaryOperationNode>(listDefNode->getElements()[0]);
        REQUIRE(binOpNode1->getOperator() == "==");
        REQUIRE(binOpNode1->getLeft()->getNodeType() == ASTNode::Types::IDENTIFIER);
        REQUIRE(dynamic_pointer_cast<IdentifierNode>(binOpNode1->getLeft())->getIdentifier() == "x");
        REQUIRE(binOpNode1->getRight()->getNodeType() == ASTNode::Types::IDENTIFIER);
        REQUIRE(dynamic_pointer_cast<IdentifierNode>(binOpNode1->getRight())->getIdentifier() == "y");

        // Checking second element: y + 5 > 9
        shared_ptr<BinaryOperationNode> binOpNode2 = dynamic_pointer_cast<BinaryOperationNode>(listDefNode->getElements()[1]);
        REQUIRE(binOpNode2->getOperator() == ">");
        shared_ptr<BinaryOperationNode> innerBinOpNode = dynamic_pointer_cast<BinaryOperationNode>(binOpNode2->getLeft());
        REQUIRE(innerBinOpNode->getOperator() == "+");
        REQUIRE(innerBinOpNode->getLeft()->getNodeType() == ASTNode::Types::IDENTIFIER);
        REQUIRE(dynamic_pointer_cast<IdentifierNode>(innerBinOpNode->getLeft())->getIdentifier() == "y");
        REQUIRE(innerBinOpNode->getRight()->getNodeType() == ASTNode::Types::NUMBER_LITERAL);
        REQUIRE(dynamic_pointer_cast<LiteralNode>(innerBinOpNode->getRight())->getLiteralValue() == "5");
        REQUIRE(binOpNode2->getRight()->getNodeType() == ASTNode::Types::NUMBER_LITERAL);
        REQUIRE(dynamic_pointer_cast<LiteralNode>(binOpNode2->getRight())->getLiteralValue() == "9");

        // Checking third element: 'meow' != 'cow'
        shared_ptr<BinaryOperationNode> binOpNode3 = dynamic_pointer_cast<BinaryOperationNode>(listDefNode->getElements()[2]);
        REQUIRE(binOpNode3->getOperator() == "!=");
        REQUIRE(binOpNode3->getLeft()->getNodeType() == ASTNode::Types::STRING_LITERAL);
        REQUIRE(dynamic_pointer_cast<LiteralNode>(binOpNode3->getLeft())->getLiteralValue() == "'meow'");
        REQUIRE(binOpNode3->getRight()->getNodeType() == ASTNode::Types::STRING_LITERAL);
        REQUIRE(dynamic_pointer_cast<LiteralNode>(binOpNode3->getRight())->getLiteralValue() == "'cow'");

        // Checking fourth element: !gross
        shared_ptr<UnaryOperationNode> unaryOpNode = dynamic_pointer_cast<UnaryOperationNode>(listDefNode->getElements()[3]);
        REQUIRE(unaryOpNode->getOperator() == "!");
        REQUIRE(unaryOpNode->getValue()->getNodeType() == ASTNode::Types::IDENTIFIER);
        REQUIRE(dynamic_pointer_cast<IdentifierNode>(unaryOpNode->getValue())->getIdentifier() == "gross");
    }

    SECTION("Can parse dict assignment with nested dicts") {
        string source = "x<Dict<Person>> = { bob: { age: 40, wife: 'Janet', bald: true }, mike: { age: 20, wife: '', bald: false }}";
        lexer.lex(source);

        shared_ptr<SourceNode> parsedAST = dynamic_pointer_cast<SourceNode>(
            parser.parse(lexer.tokens, source, "fakeFile.th", filesByCapsuleName)
        );

        REQUIRE(parsedAST->getNodeType() == ASTNode::Types::SOURCE);
        REQUIRE(parsedAST->getLinks().size() == 0);
        REQUIRE(parsedAST->getValue()->getNodeType() == ASTNode::Types::ASSIGNMENT);

        shared_ptr<AssignmentNode> assignmentNode = dynamic_pointer_cast<AssignmentNode>(parsedAST->getValue());
        REQUIRE(assignmentNode->getLeft()->getNodeType() == ASTNode::Types::IDENTIFIER);
        REQUIRE(assignmentNode->getRight()->getNodeType() == ASTNode::Types::DICTIONARY);
        REQUIRE(dynamic_pointer_cast<IdentifierNode>(assignmentNode->getLeft())->getIdentifier() == "x");

        shared_ptr<TypeDeclarationNode> typeDeclNode = dynamic_pointer_cast<TypeDeclarationNode>(assignmentNode->getLeft()->getValue());
        REQUIRE(typeDeclNode->getType() == "Dict");

        shared_ptr<DictionaryNode> dictDefNode = dynamic_pointer_cast<DictionaryNode>(assignmentNode->getRight());
        REQUIRE(dictDefNode->getElements().size() == 2);

        // Checking first dict element: bob
        shared_ptr<TupleNode> tupleNodeBob = dynamic_pointer_cast<TupleNode>(dictDefNode->getElements()[0]);
        REQUIRE(dynamic_pointer_cast<SymbolNode>(tupleNodeBob->getLeft())->getSymbol() == ":bob");
        shared_ptr<DictionaryNode> bobDictNode = dynamic_pointer_cast<DictionaryNode>(tupleNodeBob->getRight());
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
        shared_ptr<DictionaryNode> mikeDictNode = dynamic_pointer_cast<DictionaryNode>(tupleNodeMike->getRight());
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

    SECTION("Can parse pipeline assignment") {
        string source = "x<String> = 'hello' => reverse => capitalize => print";
        lexer.lex(source);

        shared_ptr<SourceNode> parsedAST = dynamic_pointer_cast<SourceNode>(
            parser.parse(lexer.tokens, source, "fakeFile.th", filesByCapsuleName)
        );

        REQUIRE(parsedAST->getNodeType() == ASTNode::Types::SOURCE);
        REQUIRE(parsedAST->getLinks().size() == 0);
        REQUIRE(parsedAST->getValue()->getNodeType() == ASTNode::Types::ASSIGNMENT);

        shared_ptr<AssignmentNode> assignmentNode = dynamic_pointer_cast<AssignmentNode>(parsedAST->getValue());
        REQUIRE(assignmentNode->getLeft()->getNodeType() == ASTNode::Types::IDENTIFIER);
        REQUIRE(dynamic_pointer_cast<IdentifierNode>(assignmentNode->getLeft())->getIdentifier() == "x");
        REQUIRE(dynamic_pointer_cast<TypeDeclarationNode>(assignmentNode->getLeft()->getValue())->getType() == "String");

        shared_ptr<BinaryOperationNode> firstPipelineOp = dynamic_pointer_cast<BinaryOperationNode>(assignmentNode->getRight());
        REQUIRE(firstPipelineOp->getOperator() == "=>");
        REQUIRE(firstPipelineOp->getRight()->getNodeType() == ASTNode::Types::IDENTIFIER);
        REQUIRE(dynamic_pointer_cast<IdentifierNode>(firstPipelineOp->getRight())->getIdentifier() == "print");

        shared_ptr<BinaryOperationNode> secondPipelineOp = dynamic_pointer_cast<BinaryOperationNode>(firstPipelineOp->getLeft());
        REQUIRE(secondPipelineOp->getOperator() == "=>");
        REQUIRE(secondPipelineOp->getRight()->getNodeType() == ASTNode::Types::IDENTIFIER);
        REQUIRE(dynamic_pointer_cast<IdentifierNode>(secondPipelineOp->getRight())->getIdentifier() == "capitalize");

        shared_ptr<BinaryOperationNode> thirdPipelineOp = dynamic_pointer_cast<BinaryOperationNode>(secondPipelineOp->getLeft());
        REQUIRE(thirdPipelineOp->getOperator() == "=>");
        REQUIRE(thirdPipelineOp->getRight()->getNodeType() == ASTNode::Types::IDENTIFIER);
        REQUIRE(dynamic_pointer_cast<IdentifierNode>(thirdPipelineOp->getRight())->getIdentifier() == "reverse");

        REQUIRE(thirdPipelineOp->getLeft()->getNodeType() == ASTNode::Types::STRING_LITERAL);
        REQUIRE(dynamic_pointer_cast<LiteralNode>(thirdPipelineOp->getLeft())->getLiteralValue() == "'hello'");
    }

    SECTION("Can parse control flow assignment") {
        string source = R"(
            x<String> = if (name == 'Bob') 'hello' else 'goodbye'
        )";
        lexer.lex(source);

        shared_ptr<SourceNode> parsedAST = dynamic_pointer_cast<SourceNode>(
            parser.parse(lexer.tokens, source, "fakeFile.th", filesByCapsuleName)
        );

        REQUIRE(parsedAST->getNodeType() == ASTNode::Types::SOURCE);
        REQUIRE(parsedAST->getLinks().size() == 0);
        REQUIRE(parsedAST->getValue()->getNodeType() == ASTNode::Types::ASSIGNMENT);

        shared_ptr<AssignmentNode> assignmentNode = dynamic_pointer_cast<AssignmentNode>(parsedAST->getValue());
        REQUIRE(assignmentNode != nullptr);

        shared_ptr<ASTNode> leftAssign = assignmentNode->getLeft();
        shared_ptr<ASTNode> rightAssign = assignmentNode->getRight();

        REQUIRE(leftAssign->getNodeType() == ASTNode::Types::IDENTIFIER);
        shared_ptr<IdentifierNode> leftAssignIdentifier = dynamic_pointer_cast<IdentifierNode>(leftAssign);
        REQUIRE(leftAssignIdentifier->getIdentifier() == "x");
        REQUIRE(dynamic_pointer_cast<TypeDeclarationNode>(leftAssignIdentifier->getValue())->getType() == "String");

        REQUIRE(rightAssign->getNodeType() == ASTNode::Types::CONTROL_FLOW);
        shared_ptr<ControlFlowNode> controlFlowNode = dynamic_pointer_cast<ControlFlowNode>(rightAssign);

        REQUIRE(controlFlowNode != nullptr);

        vector<pair<shared_ptr<ASTNode>, shared_ptr<ASTNode>>> conditionExpressionPairs = controlFlowNode->getConditionExpressionPairs();
        REQUIRE(conditionExpressionPairs.size() == 2);

        // Check the first condition-expression pair
        shared_ptr<ASTNode> condition1 = conditionExpressionPairs[0].first;
        shared_ptr<ASTNode> expression1 = conditionExpressionPairs[0].second;

        REQUIRE(condition1->getNodeType() == ASTNode::Types::BINARY_OPERATION);
        shared_ptr<BinaryOperationNode> binaryOperationNode1 = dynamic_pointer_cast<BinaryOperationNode>(condition1);

        REQUIRE(binaryOperationNode1->getOperator() == "==");

        shared_ptr<ASTNode> left1 = binaryOperationNode1->getLeft();
        shared_ptr<ASTNode> right1 = binaryOperationNode1->getRight();

        REQUIRE(left1->getNodeType() == ASTNode::Types::IDENTIFIER);
        REQUIRE(right1->getNodeType() == ASTNode::Types::STRING_LITERAL);

        shared_ptr<IdentifierNode> leftIdentifier1 = dynamic_pointer_cast<IdentifierNode>(left1);
        shared_ptr<LiteralNode> rightString1 = dynamic_pointer_cast<LiteralNode>(right1);

        REQUIRE(leftIdentifier1->getIdentifier() == "name");
        REQUIRE(rightString1->getLiteralValue() == "'Bob'");

        REQUIRE(expression1->getNodeType() == ASTNode::Types::STRING_LITERAL);
        shared_ptr<LiteralNode> expressionString1 = dynamic_pointer_cast<LiteralNode>(expression1);
        REQUIRE(expressionString1->getLiteralValue() == "'hello'");

        // Check the second condition-expression pair
        shared_ptr<ASTNode> condition2 = conditionExpressionPairs[1].first;
        shared_ptr<ASTNode> expression2 = conditionExpressionPairs[1].second;

        REQUIRE(condition2 == nullptr);

        REQUIRE(expression2->getNodeType() == ASTNode::Types::STRING_LITERAL);
        shared_ptr<LiteralNode> expressionString2 = dynamic_pointer_cast<LiteralNode>(expression2);
        REQUIRE(expressionString2->getLiteralValue() == "'goodbye'");
    }

    SECTION("Can parse tuple assignment") {
        string source = R"(
            x<Tuple<Symbol, String>> = { :ok, 'Success' }
        )";
        lexer.lex(source);

        shared_ptr<SourceNode> parsedAST = dynamic_pointer_cast<SourceNode>(
            parser.parse(lexer.tokens, source, "fakeFile.th", filesByCapsuleName)
        );

        REQUIRE(parsedAST->getNodeType() == ASTNode::Types::SOURCE);
        REQUIRE(parsedAST->getLinks().size() == 0);
        REQUIRE(parsedAST->getValue()->getNodeType() == ASTNode::Types::ASSIGNMENT);

        shared_ptr<AssignmentNode> assignmentNode = dynamic_pointer_cast<AssignmentNode>(parsedAST->getValue());
        REQUIRE(assignmentNode != nullptr);
        REQUIRE(assignmentNode->getLeft()->getNodeType() == ASTNode::Types::IDENTIFIER);
        REQUIRE(dynamic_pointer_cast<IdentifierNode>(assignmentNode->getLeft())->getIdentifier() == "x");

        shared_ptr<TypeDeclarationNode> typeNode = dynamic_pointer_cast<TypeDeclarationNode>(dynamic_pointer_cast<IdentifierNode>(assignmentNode->getLeft())->getValue());
        REQUIRE(typeNode != nullptr);
        REQUIRE(typeNode->getNodeType() == ASTNode::Types::TYPE_DECLARATION);
        REQUIRE(typeNode->getType() == "Tuple");

        shared_ptr<TypeDeclarationNode> leftTypeNode = dynamic_pointer_cast<TypeDeclarationNode>(typeNode->getLeft());
        REQUIRE(leftTypeNode != nullptr);
        REQUIRE(leftTypeNode->getType() == "Symbol");

        shared_ptr<TypeDeclarationNode> rightTypeNode = dynamic_pointer_cast<TypeDeclarationNode>(typeNode->getRight());
        REQUIRE(rightTypeNode != nullptr);
        REQUIRE(rightTypeNode->getType() == "String");

        shared_ptr<TupleNode> tupleNode = dynamic_pointer_cast<TupleNode>(assignmentNode->getRight());
        REQUIRE(tupleNode != nullptr);
        REQUIRE(tupleNode->getNodeType() == ASTNode::Types::TUPLE);

        shared_ptr<SymbolNode> firstNode = dynamic_pointer_cast<SymbolNode>(tupleNode->getLeft());
        REQUIRE(firstNode != nullptr);
        REQUIRE(firstNode->getNodeType() == ASTNode::Types::SYMBOL);
        REQUIRE(firstNode->getSymbol() == ":ok");

        shared_ptr<LiteralNode> secondNode = dynamic_pointer_cast<LiteralNode>(tupleNode->getRight());
        REQUIRE(secondNode != nullptr);
        REQUIRE(secondNode->getNodeType() == ASTNode::Types::STRING_LITERAL);
        REQUIRE(secondNode->getLiteralValue() == "'Success'");
    }
    // --------- ASSIGNMENT ----------

    // --------- CONTROL FLOW ---------
    SECTION("Can parse if statement with parentheses and block") {
        string source = R"(
            if (isOpen) {
                x == y
            }
        )";
        lexer.lex(source);

        shared_ptr<SourceNode> parsedAST = dynamic_pointer_cast<SourceNode>(
            parser.parse(lexer.tokens, source, "fakeFile.th", filesByCapsuleName)
        );

        REQUIRE(parsedAST->getNodeType() == ASTNode::Types::SOURCE);
        REQUIRE(parsedAST->getLinks().size() == 0);
        REQUIRE(parsedAST->getValue()->getNodeType() == ASTNode::Types::CONTROL_FLOW);

        shared_ptr<ControlFlowNode> controlFlowNode = dynamic_pointer_cast<ControlFlowNode>(parsedAST->getValue());

        REQUIRE(controlFlowNode != nullptr);

        vector<pair<shared_ptr<ASTNode>, shared_ptr<ASTNode>>> conditionExpressionPairs = controlFlowNode->getConditionExpressionPairs();
        REQUIRE(conditionExpressionPairs.size() == 1);

        shared_ptr<ASTNode> condition = conditionExpressionPairs[0].first;
        shared_ptr<ASTNode> block = conditionExpressionPairs[0].second;

        REQUIRE(condition->getNodeType() == ASTNode::Types::IDENTIFIER);
        shared_ptr<IdentifierNode> conditionIdentifier = dynamic_pointer_cast<IdentifierNode>(condition);
        REQUIRE(conditionIdentifier->getIdentifier() == "isOpen");

        REQUIRE(block->getNodeType() == ASTNode::Types::BLOCK);
        shared_ptr<BlockNode> blockNode = dynamic_pointer_cast<BlockNode>(block);

        vector<shared_ptr<ASTNode>> blockExpressions = blockNode->getElements();
        REQUIRE(blockExpressions.size() == 1);

        shared_ptr<ASTNode> binaryOperation = blockExpressions[0];
        REQUIRE(binaryOperation->getNodeType() == ASTNode::Types::BINARY_OPERATION);
        shared_ptr<BinaryOperationNode> binaryOperationNode = dynamic_pointer_cast<BinaryOperationNode>(binaryOperation);

        REQUIRE(binaryOperationNode->getOperator() == "==");

        shared_ptr<ASTNode> left = binaryOperationNode->getLeft();
        shared_ptr<ASTNode> right = binaryOperationNode->getRight();

        REQUIRE(left->getNodeType() == ASTNode::Types::IDENTIFIER);
        REQUIRE(right->getNodeType() == ASTNode::Types::IDENTIFIER);

        shared_ptr<IdentifierNode> leftIdentifier = dynamic_pointer_cast<IdentifierNode>(left);
        shared_ptr<IdentifierNode> rightIdentifier = dynamic_pointer_cast<IdentifierNode>(right);

        REQUIRE(leftIdentifier->getIdentifier() == "x");
        REQUIRE(rightIdentifier->getIdentifier() == "y");
    }

    SECTION("Can parse if statement without parentheses, with block") {
        string source = R"(
            if x == 10 {
                true
            }
        )";
        lexer.lex(source);

        shared_ptr<SourceNode> parsedAST = dynamic_pointer_cast<SourceNode>(
            parser.parse(lexer.tokens, source, "fakeFile.th", filesByCapsuleName)
        );

        REQUIRE(parsedAST->getNodeType() == ASTNode::Types::SOURCE);
        REQUIRE(parsedAST->getLinks().size() == 0);
        REQUIRE(parsedAST->getValue()->getNodeType() == ASTNode::Types::CONTROL_FLOW);

        shared_ptr<ControlFlowNode> controlFlowNode = dynamic_pointer_cast<ControlFlowNode>(parsedAST->getValue());

        REQUIRE(controlFlowNode != nullptr);

        vector<pair<shared_ptr<ASTNode>, shared_ptr<ASTNode>>> conditionExpressionPairs = controlFlowNode->getConditionExpressionPairs();
        REQUIRE(conditionExpressionPairs.size() == 1);

        shared_ptr<ASTNode> condition = conditionExpressionPairs[0].first;
        shared_ptr<ASTNode> block = conditionExpressionPairs[0].second;

        REQUIRE(condition->getNodeType() == ASTNode::Types::BINARY_OPERATION);
        shared_ptr<BinaryOperationNode> binaryOperationNode = dynamic_pointer_cast<BinaryOperationNode>(condition);

        REQUIRE(binaryOperationNode->getOperator() == "==");

        shared_ptr<ASTNode> left = binaryOperationNode->getLeft();
        shared_ptr<ASTNode> right = binaryOperationNode->getRight();

        REQUIRE(left->getNodeType() == ASTNode::Types::IDENTIFIER);
        REQUIRE(right->getNodeType() == ASTNode::Types::NUMBER_LITERAL);

        shared_ptr<IdentifierNode> leftIdentifier = dynamic_pointer_cast<IdentifierNode>(left);
        shared_ptr<LiteralNode> rightNumber = dynamic_pointer_cast<LiteralNode>(right);

        REQUIRE(leftIdentifier->getIdentifier() == "x");
        REQUIRE(rightNumber->getLiteralValue() == "10");

        REQUIRE(block->getNodeType() == ASTNode::Types::BLOCK);
        shared_ptr<BlockNode> blockNode = dynamic_pointer_cast<BlockNode>(block);

        vector<shared_ptr<ASTNode>> blockExpressions = blockNode->getElements();
        REQUIRE(blockExpressions.size() == 1);

        shared_ptr<ASTNode> booleanLiteral = blockExpressions[0];
        REQUIRE(booleanLiteral->getNodeType() == ASTNode::Types::BOOLEAN_LITERAL);

        shared_ptr<LiteralNode> booleanLiteralNode = dynamic_pointer_cast<LiteralNode>(booleanLiteral);
        REQUIRE(booleanLiteralNode->getLiteralValue() == "true");
    }

    SECTION("Can parse if statement with parentheses and expression") {
        string source = R"(
            if ('hello' == world) false
        )";
        lexer.lex(source);

        shared_ptr<SourceNode> parsedAST = dynamic_pointer_cast<SourceNode>(
            parser.parse(lexer.tokens, source, "fakeFile.th", filesByCapsuleName)
        );

        REQUIRE(parsedAST->getNodeType() == ASTNode::Types::SOURCE);
        REQUIRE(parsedAST->getLinks().size() == 0);
        REQUIRE(parsedAST->getValue()->getNodeType() == ASTNode::Types::CONTROL_FLOW);

        shared_ptr<ControlFlowNode> controlFlowNode = dynamic_pointer_cast<ControlFlowNode>(parsedAST->getValue());

        REQUIRE(controlFlowNode != nullptr);

        vector<pair<shared_ptr<ASTNode>, shared_ptr<ASTNode>>> conditionExpressionPairs = controlFlowNode->getConditionExpressionPairs();
        REQUIRE(conditionExpressionPairs.size() == 1);

        shared_ptr<ASTNode> condition = conditionExpressionPairs[0].first;
        shared_ptr<ASTNode> expression = conditionExpressionPairs[0].second;

        REQUIRE(condition->getNodeType() == ASTNode::Types::BINARY_OPERATION);
        shared_ptr<BinaryOperationNode> binaryOperationNode = dynamic_pointer_cast<BinaryOperationNode>(condition);

        REQUIRE(binaryOperationNode->getOperator() == "==");

        shared_ptr<ASTNode> left = binaryOperationNode->getLeft();
        shared_ptr<ASTNode> right = binaryOperationNode->getRight();

        REQUIRE(left->getNodeType() == ASTNode::Types::STRING_LITERAL);
        REQUIRE(right->getNodeType() == ASTNode::Types::IDENTIFIER);

        shared_ptr<LiteralNode> leftString = dynamic_pointer_cast<LiteralNode>(left);
        shared_ptr<IdentifierNode> rightIdentifier = dynamic_pointer_cast<IdentifierNode>(right);

        REQUIRE(leftString->getLiteralValue() == "'hello'");
        REQUIRE(rightIdentifier->getIdentifier() == "world");

        REQUIRE(expression->getNodeType() == ASTNode::Types::BOOLEAN_LITERAL);
        shared_ptr<LiteralNode> booleanLiteralNode = dynamic_pointer_cast<LiteralNode>(expression);
        REQUIRE(booleanLiteralNode->getLiteralValue() == "false");
    }

    SECTION("Can parse if statement with no parentheses and expression") {
        string source = R"(
            if true false
        )";
        lexer.lex(source);

        shared_ptr<SourceNode> parsedAST = dynamic_pointer_cast<SourceNode>(
            parser.parse(lexer.tokens, source, "fakeFile.th", filesByCapsuleName)
        );

        REQUIRE(parsedAST->getNodeType() == ASTNode::Types::SOURCE);
        REQUIRE(parsedAST->getLinks().size() == 0);
        REQUIRE(parsedAST->getValue()->getNodeType() == ASTNode::Types::CONTROL_FLOW);

        shared_ptr<ControlFlowNode> controlFlowNode = dynamic_pointer_cast<ControlFlowNode>(parsedAST->getValue());

        REQUIRE(controlFlowNode != nullptr);

        vector<pair<shared_ptr<ASTNode>, shared_ptr<ASTNode>>> conditionExpressionPairs = controlFlowNode->getConditionExpressionPairs();
        REQUIRE(conditionExpressionPairs.size() == 1);

        shared_ptr<ASTNode> condition = conditionExpressionPairs[0].first;
        shared_ptr<ASTNode> expression = conditionExpressionPairs[0].second;

        REQUIRE(condition->getNodeType() == ASTNode::Types::BOOLEAN_LITERAL);
        shared_ptr<LiteralNode> conditionBoolean = dynamic_pointer_cast<LiteralNode>(condition);
        REQUIRE(conditionBoolean->getLiteralValue() == "true");

        REQUIRE(expression->getNodeType() == ASTNode::Types::BOOLEAN_LITERAL);
        shared_ptr<LiteralNode> expressionBoolean = dynamic_pointer_cast<LiteralNode>(expression);
        REQUIRE(expressionBoolean->getLiteralValue() == "false");
    }

    SECTION("Can parse if/else-if/else statement with blocks") {
        string source = R"(
            if (name == 'Mike') {
                lastName<String> = 'Wazowski'
            } else if (name == 'Michael') {
                lastName<String> = 'Jordan'
            } else {
                lastName<String> = 'Unknown'
            }
        )";
        lexer.lex(source);

        shared_ptr<SourceNode> parsedAST = dynamic_pointer_cast<SourceNode>(
            parser.parse(lexer.tokens, source, "fakeFile.th", filesByCapsuleName)
        );

        REQUIRE(parsedAST->getNodeType() == ASTNode::Types::SOURCE);
        REQUIRE(parsedAST->getLinks().size() == 0);
        REQUIRE(parsedAST->getValue()->getNodeType() == ASTNode::Types::CONTROL_FLOW);

        shared_ptr<ControlFlowNode> controlFlowNode = dynamic_pointer_cast<ControlFlowNode>(parsedAST->getValue());

        REQUIRE(controlFlowNode != nullptr);

        vector<pair<shared_ptr<ASTNode>, shared_ptr<ASTNode>>> conditionExpressionPairs = controlFlowNode->getConditionExpressionPairs();
        REQUIRE(conditionExpressionPairs.size() == 3);

        // Check the first condition-expression pair
        shared_ptr<ASTNode> condition1 = conditionExpressionPairs[0].first;
        shared_ptr<ASTNode> block1 = conditionExpressionPairs[0].second;

        REQUIRE(condition1->getNodeType() == ASTNode::Types::BINARY_OPERATION);
        shared_ptr<BinaryOperationNode> binaryOperationNode1 = dynamic_pointer_cast<BinaryOperationNode>(condition1);

        REQUIRE(binaryOperationNode1->getOperator() == "==");

        shared_ptr<ASTNode> left1 = binaryOperationNode1->getLeft();
        shared_ptr<ASTNode> right1 = binaryOperationNode1->getRight();

        REQUIRE(left1->getNodeType() == ASTNode::Types::IDENTIFIER);
        REQUIRE(right1->getNodeType() == ASTNode::Types::STRING_LITERAL);

        shared_ptr<IdentifierNode> leftIdentifier1 = dynamic_pointer_cast<IdentifierNode>(left1);
        shared_ptr<LiteralNode> rightString1 = dynamic_pointer_cast<LiteralNode>(right1);

        REQUIRE(leftIdentifier1->getIdentifier() == "name");
        REQUIRE(rightString1->getLiteralValue() == "'Mike'");

        REQUIRE(block1->getNodeType() == ASTNode::Types::BLOCK);
        shared_ptr<BlockNode> blockNode1 = dynamic_pointer_cast<BlockNode>(block1);

        vector<shared_ptr<ASTNode>> blockExpressions1 = blockNode1->getElements();
        REQUIRE(blockExpressions1.size() == 1);

        shared_ptr<ASTNode> assignment1 = blockExpressions1[0];
        REQUIRE(assignment1->getNodeType() == ASTNode::Types::ASSIGNMENT);

        shared_ptr<AssignmentNode> assignmentNode1 = dynamic_pointer_cast<AssignmentNode>(assignment1);
        shared_ptr<ASTNode> leftAssign1 = assignmentNode1->getLeft();
        shared_ptr<ASTNode> rightAssign1 = assignmentNode1->getRight();

        REQUIRE(leftAssign1->getNodeType() == ASTNode::Types::IDENTIFIER);
        REQUIRE(rightAssign1->getNodeType() == ASTNode::Types::STRING_LITERAL);

        shared_ptr<IdentifierNode> leftAssignIdentifier1 = dynamic_pointer_cast<IdentifierNode>(leftAssign1);
        shared_ptr<LiteralNode> rightAssignString1 = dynamic_pointer_cast<LiteralNode>(rightAssign1);

        REQUIRE(leftAssignIdentifier1->getIdentifier() == "lastName");
        REQUIRE(dynamic_pointer_cast<TypeDeclarationNode>(leftAssignIdentifier1->getValue())->getType() == "String");
        REQUIRE(rightAssignString1->getLiteralValue() == "'Wazowski'");

        // Check the second condition-expression pair
        shared_ptr<ASTNode> condition2 = conditionExpressionPairs[1].first;
        shared_ptr<ASTNode> block2 = conditionExpressionPairs[1].second;

        REQUIRE(condition2->getNodeType() == ASTNode::Types::BINARY_OPERATION);
        shared_ptr<BinaryOperationNode> binaryOperationNode2 = dynamic_pointer_cast<BinaryOperationNode>(condition2);

        REQUIRE(binaryOperationNode2->getOperator() == "==");

        shared_ptr<ASTNode> left2 = binaryOperationNode2->getLeft();
        shared_ptr<ASTNode> right2 = binaryOperationNode2->getRight();

        REQUIRE(left2->getNodeType() == ASTNode::Types::IDENTIFIER);
        REQUIRE(right2->getNodeType() == ASTNode::Types::STRING_LITERAL);

        shared_ptr<IdentifierNode> leftIdentifier2 = dynamic_pointer_cast<IdentifierNode>(left2);
        shared_ptr<LiteralNode> rightString2 = dynamic_pointer_cast<LiteralNode>(right2);

        REQUIRE(leftIdentifier2->getIdentifier() == "name");
        REQUIRE(rightString2->getLiteralValue() == "'Michael'");

        REQUIRE(block2->getNodeType() == ASTNode::Types::BLOCK);
        shared_ptr<BlockNode> blockNode2 = dynamic_pointer_cast<BlockNode>(block2);

        vector<shared_ptr<ASTNode>> blockExpressions2 = blockNode2->getElements();
        REQUIRE(blockExpressions2.size() == 1);

        shared_ptr<ASTNode> assignment2 = blockExpressions2[0];
        REQUIRE(assignment2->getNodeType() == ASTNode::Types::ASSIGNMENT);

        shared_ptr<AssignmentNode> assignmentNode2 = dynamic_pointer_cast<AssignmentNode>(assignment2);
        shared_ptr<ASTNode> leftAssign2 = assignmentNode2->getLeft();
        shared_ptr<ASTNode> rightAssign2 = assignmentNode2->getRight();

        REQUIRE(leftAssign2->getNodeType() == ASTNode::Types::IDENTIFIER);
        REQUIRE(rightAssign2->getNodeType() == ASTNode::Types::STRING_LITERAL);

        shared_ptr<IdentifierNode> leftAssignIdentifier2 = dynamic_pointer_cast<IdentifierNode>(leftAssign2);
        shared_ptr<LiteralNode> rightAssignString2 = dynamic_pointer_cast<LiteralNode>(rightAssign2);

        REQUIRE(leftAssignIdentifier2->getIdentifier() == "lastName");
        REQUIRE(dynamic_pointer_cast<TypeDeclarationNode>(leftAssignIdentifier2->getValue())->getType() == "String");
        REQUIRE(rightAssignString2->getLiteralValue() == "'Jordan'");

        // Check the third condition-expression pair
        shared_ptr<ASTNode> condition3 = conditionExpressionPairs[2].first;
        shared_ptr<ASTNode> block3 = conditionExpressionPairs[2].second;

        REQUIRE(condition3 == nullptr);

        REQUIRE(block3->getNodeType() == ASTNode::Types::BLOCK);
        shared_ptr<BlockNode> blockNode3 = dynamic_pointer_cast<BlockNode>(block3);

        vector<shared_ptr<ASTNode>> blockExpressions3 = blockNode3->getElements();
        REQUIRE(blockExpressions3.size() == 1);

        shared_ptr<ASTNode> assignment3 = blockExpressions3[0];
        REQUIRE(assignment3->getNodeType() == ASTNode::Types::ASSIGNMENT);

        shared_ptr<AssignmentNode> assignmentNode3 = dynamic_pointer_cast<AssignmentNode>(assignment3);
        shared_ptr<ASTNode> leftAssign3 = assignmentNode3->getLeft();
        shared_ptr<ASTNode> rightAssign3 = assignmentNode3->getRight();

        REQUIRE(leftAssign3->getNodeType() == ASTNode::Types::IDENTIFIER);
        REQUIRE(rightAssign3->getNodeType() == ASTNode::Types::STRING_LITERAL);

        shared_ptr<IdentifierNode> leftAssignIdentifier3 = dynamic_pointer_cast<IdentifierNode>(leftAssign3);
        shared_ptr<LiteralNode> rightAssignString3 = dynamic_pointer_cast<LiteralNode>(rightAssign3);

        REQUIRE(leftAssignIdentifier3->getIdentifier() == "lastName");
        REQUIRE(dynamic_pointer_cast<TypeDeclarationNode>(leftAssignIdentifier3->getValue())->getType() == "String");
        REQUIRE(rightAssignString3->getLiteralValue() == "'Unknown'");
    }
    // --------- CONTROL FLOW ---------

    // --------- FUNCTIONS ------------
    SECTION("Can parse function declaration with no parens, single parameter, and expression definition") {
        string source = R"(
            greet<String> = name<String> -> 'hello' + name
        )";
        lexer.lex(source);

        shared_ptr<SourceNode> parsedAST = dynamic_pointer_cast<SourceNode>(
            parser.parse(lexer.tokens, source, "fakeFile.th", filesByCapsuleName)
        );

        REQUIRE(parsedAST->getNodeType() == ASTNode::Types::SOURCE);
        REQUIRE(parsedAST->getLinks().size() == 0);
        REQUIRE(parsedAST->getValue()->getNodeType() == ASTNode::Types::ASSIGNMENT);

        shared_ptr<AssignmentNode> assignmentNode = dynamic_pointer_cast<AssignmentNode>(parsedAST->getValue());
        REQUIRE(assignmentNode->getLeft()->getNodeType() == ASTNode::Types::IDENTIFIER);
        REQUIRE(assignmentNode->getRight()->getNodeType() == ASTNode::Types::FUNCTION_DECLARATION);
        REQUIRE(dynamic_pointer_cast<IdentifierNode>(assignmentNode->getLeft())->getIdentifier() == "greet");
        REQUIRE(dynamic_pointer_cast<TypeDeclarationNode>(assignmentNode->getLeft()->getValue())->getType() == "String");

        shared_ptr<FunctionDeclarationNode> funcDeclNode = dynamic_pointer_cast<FunctionDeclarationNode>(assignmentNode->getRight());

        REQUIRE(funcDeclNode->getParameters()->getElements().size() == 1);

        shared_ptr<IdentifierNode> param = dynamic_pointer_cast<IdentifierNode>(funcDeclNode->getParameters()->getElements()[0]);
        REQUIRE(param->getNodeType() == ASTNode::Types::IDENTIFIER);
        REQUIRE(param->getIdentifier() == "name");
        REQUIRE(dynamic_pointer_cast<TypeDeclarationNode>(param->getValue())->getType() == "String");

        shared_ptr<BinaryOperationNode> binOpNode = dynamic_pointer_cast<BinaryOperationNode>(funcDeclNode->getDefinition());
        REQUIRE(binOpNode->getOperator() == "+");
        REQUIRE(binOpNode->getLeft()->getNodeType() == ASTNode::Types::STRING_LITERAL);
        REQUIRE(binOpNode->getRight()->getNodeType() == ASTNode::Types::IDENTIFIER);

        shared_ptr<LiteralNode> leftNode = dynamic_pointer_cast<LiteralNode>(binOpNode->getLeft());
        REQUIRE(leftNode->getLiteralValue() == "'hello'");

        shared_ptr<IdentifierNode> rightNode = dynamic_pointer_cast<IdentifierNode>(binOpNode->getRight());
        REQUIRE(rightNode->getIdentifier() == "name");
    }

    SECTION("Can parse function declaration with no parens, single parameter, and empty block definition") {
        string source = R"(
            x<Boolean> = meow<String> -> {}
        )";
        lexer.lex(source);

        shared_ptr<SourceNode> parsedAST = dynamic_pointer_cast<SourceNode>(
            parser.parse(lexer.tokens, source, "fakeFile.th", filesByCapsuleName)
        );

        REQUIRE(parsedAST->getNodeType() == ASTNode::Types::SOURCE);
        REQUIRE(parsedAST->getLinks().size() == 0);
        REQUIRE(parsedAST->getValue()->getNodeType() == ASTNode::Types::ASSIGNMENT);

        shared_ptr<AssignmentNode> assignmentNode = dynamic_pointer_cast<AssignmentNode>(parsedAST->getValue());
        REQUIRE(assignmentNode->getLeft()->getNodeType() == ASTNode::Types::IDENTIFIER);
        REQUIRE(assignmentNode->getRight()->getNodeType() == ASTNode::Types::FUNCTION_DECLARATION);
        REQUIRE(dynamic_pointer_cast<IdentifierNode>(assignmentNode->getLeft())->getIdentifier() == "x");
        REQUIRE(dynamic_pointer_cast<TypeDeclarationNode>(assignmentNode->getLeft()->getValue())->getType() == "Boolean");

        shared_ptr<FunctionDeclarationNode> funcDeclNode = dynamic_pointer_cast<FunctionDeclarationNode>(assignmentNode->getRight());

        REQUIRE(funcDeclNode->getParameters()->getElements().size() == 1);

        shared_ptr<IdentifierNode> param = dynamic_pointer_cast<IdentifierNode>(funcDeclNode->getParameters()->getElements()[0]);
        REQUIRE(param->getNodeType() == ASTNode::Types::IDENTIFIER);
        REQUIRE(param->getIdentifier() == "meow");
        REQUIRE(dynamic_pointer_cast<TypeDeclarationNode>(param->getValue())->getType() == "String");

        shared_ptr<BlockNode> blockNode = dynamic_pointer_cast<BlockNode>(funcDeclNode->getDefinition());
        REQUIRE(blockNode->getNodeType() == ASTNode::Types::BLOCK);
        REQUIRE(blockNode->getElements().size() == 0);
    }

    SECTION("Can parse function declaration with single parameter in parentheses and empty block definition") {
        string source = R"(
            x<Boolean> = (meow<String>) -> {}
        )";
        lexer.lex(source);

        shared_ptr<SourceNode> parsedAST = dynamic_pointer_cast<SourceNode>(
            parser.parse(lexer.tokens, source, "fakeFile.th", filesByCapsuleName)
        );

        REQUIRE(parsedAST->getNodeType() == ASTNode::Types::SOURCE);
        REQUIRE(parsedAST->getLinks().size() == 0);
        REQUIRE(parsedAST->getValue()->getNodeType() == ASTNode::Types::ASSIGNMENT);

        shared_ptr<AssignmentNode> assignmentNode = dynamic_pointer_cast<AssignmentNode>(parsedAST->getValue());
        REQUIRE(assignmentNode->getLeft()->getNodeType() == ASTNode::Types::IDENTIFIER);
        REQUIRE(assignmentNode->getRight()->getNodeType() == ASTNode::Types::FUNCTION_DECLARATION);
        REQUIRE(dynamic_pointer_cast<IdentifierNode>(assignmentNode->getLeft())->getIdentifier() == "x");
        REQUIRE(dynamic_pointer_cast<TypeDeclarationNode>(assignmentNode->getLeft()->getValue())->getType() == "Boolean");

        shared_ptr<FunctionDeclarationNode> funcDeclNode = dynamic_pointer_cast<FunctionDeclarationNode>(assignmentNode->getRight());

        REQUIRE(funcDeclNode->getParameters()->getElements().size() == 1);

        shared_ptr<IdentifierNode> param = dynamic_pointer_cast<IdentifierNode>(funcDeclNode->getParameters()->getElements()[0]);
        REQUIRE(param->getNodeType() == ASTNode::Types::IDENTIFIER);
        REQUIRE(param->getIdentifier() == "meow");
        REQUIRE(dynamic_pointer_cast<TypeDeclarationNode>(param->getValue())->getType() == "String");

        shared_ptr<BlockNode> blockNode = dynamic_pointer_cast<BlockNode>(funcDeclNode->getDefinition());
        REQUIRE(blockNode->getNodeType() == ASTNode::Types::BLOCK);
        REQUIRE(blockNode->getElements().size() == 0);
    }

    SECTION("Can parse function declaration with multiple parameters in parentheses and empty block definition") {
        string source = R"(
            x<Boolean> = (meow<String>, cow<String>) -> {}
        )";
        lexer.lex(source);

        shared_ptr<SourceNode> parsedAST = dynamic_pointer_cast<SourceNode>(
            parser.parse(lexer.tokens, source, "fakeFile.th", filesByCapsuleName)
        );

        REQUIRE(parsedAST->getNodeType() == ASTNode::Types::SOURCE);
        REQUIRE(parsedAST->getLinks().size() == 0);
        REQUIRE(parsedAST->getValue()->getNodeType() == ASTNode::Types::ASSIGNMENT);

        shared_ptr<AssignmentNode> assignmentNode = dynamic_pointer_cast<AssignmentNode>(parsedAST->getValue());
        REQUIRE(assignmentNode->getLeft()->getNodeType() == ASTNode::Types::IDENTIFIER);
        REQUIRE(assignmentNode->getRight()->getNodeType() == ASTNode::Types::FUNCTION_DECLARATION);
        REQUIRE(dynamic_pointer_cast<IdentifierNode>(assignmentNode->getLeft())->getIdentifier() == "x");
        REQUIRE(dynamic_pointer_cast<TypeDeclarationNode>(assignmentNode->getLeft()->getValue())->getType() == "Boolean");

        shared_ptr<FunctionDeclarationNode> funcDeclNode = dynamic_pointer_cast<FunctionDeclarationNode>(assignmentNode->getRight());

        REQUIRE(funcDeclNode->getParameters()->getElements().size() == 2);

        shared_ptr<IdentifierNode> param1 = dynamic_pointer_cast<IdentifierNode>(funcDeclNode->getParameters()->getElements()[0]);
        REQUIRE(param1->getNodeType() == ASTNode::Types::IDENTIFIER);
        REQUIRE(param1->getIdentifier() == "meow");
        REQUIRE(dynamic_pointer_cast<TypeDeclarationNode>(param1->getValue())->getType() == "String");

        shared_ptr<IdentifierNode> param2 = dynamic_pointer_cast<IdentifierNode>(funcDeclNode->getParameters()->getElements()[1]);
        REQUIRE(param2->getNodeType() == ASTNode::Types::IDENTIFIER);
        REQUIRE(param2->getIdentifier() == "cow");
        REQUIRE(dynamic_pointer_cast<TypeDeclarationNode>(param2->getValue())->getType() == "String");

        shared_ptr<BlockNode> blockNode = dynamic_pointer_cast<BlockNode>(funcDeclNode->getDefinition());
        REQUIRE(blockNode->getNodeType() == ASTNode::Types::BLOCK);
        REQUIRE(blockNode->getElements().size() == 0);
    }

    SECTION("Can parse function declaration with empty parameter list and empty block definition") {
        string source = R"(
            x<Boolean> = () -> {}
        )";
        lexer.lex(source);

        shared_ptr<SourceNode> parsedAST = dynamic_pointer_cast<SourceNode>(
            parser.parse(lexer.tokens, source, "fakeFile.th", filesByCapsuleName)
        );

        REQUIRE(parsedAST->getNodeType() == ASTNode::Types::SOURCE);
        REQUIRE(parsedAST->getLinks().size() == 0);
        REQUIRE(parsedAST->getValue()->getNodeType() == ASTNode::Types::ASSIGNMENT);

        shared_ptr<AssignmentNode> assignmentNode = dynamic_pointer_cast<AssignmentNode>(parsedAST->getValue());
        REQUIRE(assignmentNode->getLeft()->getNodeType() == ASTNode::Types::IDENTIFIER);
        REQUIRE(assignmentNode->getRight()->getNodeType() == ASTNode::Types::FUNCTION_DECLARATION);
        REQUIRE(dynamic_pointer_cast<IdentifierNode>(assignmentNode->getLeft())->getIdentifier() == "x");
        REQUIRE(dynamic_pointer_cast<TypeDeclarationNode>(assignmentNode->getLeft()->getValue())->getType() == "Boolean");

        shared_ptr<FunctionDeclarationNode> funcDeclNode = dynamic_pointer_cast<FunctionDeclarationNode>(assignmentNode->getRight());

        REQUIRE(funcDeclNode->getParameters()->getElements().size() == 0);

        shared_ptr<BlockNode> blockNode = dynamic_pointer_cast<BlockNode>(funcDeclNode->getDefinition());
        REQUIRE(blockNode->getNodeType() == ASTNode::Types::BLOCK);
        REQUIRE(blockNode->getElements().size() == 0);
    }

    SECTION("Can parse function declaration with empty parameter list using shorthand syntax and empty block definition") {
        string source = R"(
            x<Boolean> = -> {}
        )";
        lexer.lex(source);

        shared_ptr<SourceNode> parsedAST = dynamic_pointer_cast<SourceNode>(
            parser.parse(lexer.tokens, source, "fakeFile.th", filesByCapsuleName)
        );

        REQUIRE(parsedAST->getNodeType() == ASTNode::Types::SOURCE);
        REQUIRE(parsedAST->getLinks().size() == 0);
        REQUIRE(parsedAST->getValue()->getNodeType() == ASTNode::Types::ASSIGNMENT);

        shared_ptr<AssignmentNode> assignmentNode = dynamic_pointer_cast<AssignmentNode>(parsedAST->getValue());
        REQUIRE(assignmentNode->getLeft()->getNodeType() == ASTNode::Types::IDENTIFIER);
        REQUIRE(assignmentNode->getRight()->getNodeType() == ASTNode::Types::FUNCTION_DECLARATION);
        REQUIRE(dynamic_pointer_cast<IdentifierNode>(assignmentNode->getLeft())->getIdentifier() == "x");
        REQUIRE(dynamic_pointer_cast<TypeDeclarationNode>(assignmentNode->getLeft()->getValue())->getType() == "Boolean");

        shared_ptr<FunctionDeclarationNode> funcDeclNode = dynamic_pointer_cast<FunctionDeclarationNode>(assignmentNode->getRight());

        REQUIRE(funcDeclNode->getParameters()->getElements().size() == 0);

        shared_ptr<BlockNode> blockNode = dynamic_pointer_cast<BlockNode>(funcDeclNode->getDefinition());
        REQUIRE(blockNode->getNodeType() == ASTNode::Types::BLOCK);
        REQUIRE(blockNode->getElements().size() == 0);
    }

    SECTION("Can parse function declaration with empty parameter list and binary operation definition") {
        string source = R"(
            x<Boolean> = () -> y - 5 == 0
        )";
        lexer.lex(source);

        shared_ptr<SourceNode> parsedAST = dynamic_pointer_cast<SourceNode>(
            parser.parse(lexer.tokens, source, "fakeFile.th", filesByCapsuleName)
        );

        REQUIRE(parsedAST->getNodeType() == ASTNode::Types::SOURCE);
        REQUIRE(parsedAST->getLinks().size() == 0);
        REQUIRE(parsedAST->getValue()->getNodeType() == ASTNode::Types::ASSIGNMENT);

        shared_ptr<AssignmentNode> assignmentNode = dynamic_pointer_cast<AssignmentNode>(parsedAST->getValue());
        REQUIRE(assignmentNode->getLeft()->getNodeType() == ASTNode::Types::IDENTIFIER);
        REQUIRE(assignmentNode->getRight()->getNodeType() == ASTNode::Types::FUNCTION_DECLARATION);
        REQUIRE(dynamic_pointer_cast<IdentifierNode>(assignmentNode->getLeft())->getIdentifier() == "x");
        REQUIRE(dynamic_pointer_cast<TypeDeclarationNode>(assignmentNode->getLeft()->getValue())->getType() == "Boolean");

        shared_ptr<FunctionDeclarationNode> funcDeclNode = dynamic_pointer_cast<FunctionDeclarationNode>(assignmentNode->getRight());

        REQUIRE(funcDeclNode->getParameters()->getElements().size() == 0);

        shared_ptr<BinaryOperationNode> binOpNode = dynamic_pointer_cast<BinaryOperationNode>(funcDeclNode->getDefinition());
        REQUIRE(binOpNode->getNodeType() == ASTNode::Types::BINARY_OPERATION);
        REQUIRE(binOpNode->getOperator() == "==");

        shared_ptr<BinaryOperationNode> leftBinOpNode = dynamic_pointer_cast<BinaryOperationNode>(binOpNode->getLeft());
        REQUIRE(leftBinOpNode->getOperator() == "-");
        REQUIRE(leftBinOpNode->getLeft()->getNodeType() == ASTNode::Types::IDENTIFIER);
        REQUIRE(leftBinOpNode->getRight()->getNodeType() == ASTNode::Types::NUMBER_LITERAL);

        shared_ptr<IdentifierNode> identifierNode = dynamic_pointer_cast<IdentifierNode>(leftBinOpNode->getLeft());
        REQUIRE(identifierNode->getIdentifier() == "y");

        shared_ptr<LiteralNode> leftLiteralNode = dynamic_pointer_cast<LiteralNode>(leftBinOpNode->getRight());
        REQUIRE(leftLiteralNode->getLiteralValue() == "5");

        shared_ptr<LiteralNode> rightLiteralNode = dynamic_pointer_cast<LiteralNode>(binOpNode->getRight());
        REQUIRE(rightLiteralNode->getLiteralValue() == "0");
    }

    SECTION("Can parse function declaration with parens, multiple parameters, and block definition containing binary operation") {
        string source = R"(
            x<Boolean> = (meow<String>, cow<String>) -> { meow == cow }
        )";
        lexer.lex(source);

        shared_ptr<SourceNode> parsedAST = dynamic_pointer_cast<SourceNode>(
            parser.parse(lexer.tokens, source, "fakeFile.th", filesByCapsuleName)
        );

        REQUIRE(parsedAST->getNodeType() == ASTNode::Types::SOURCE);
        REQUIRE(parsedAST->getLinks().size() == 0);
        REQUIRE(parsedAST->getValue()->getNodeType() == ASTNode::Types::ASSIGNMENT);

        shared_ptr<AssignmentNode> assignmentNode = dynamic_pointer_cast<AssignmentNode>(parsedAST->getValue());
        REQUIRE(assignmentNode->getLeft()->getNodeType() == ASTNode::Types::IDENTIFIER);
        REQUIRE(assignmentNode->getRight()->getNodeType() == ASTNode::Types::FUNCTION_DECLARATION);
        REQUIRE(dynamic_pointer_cast<IdentifierNode>(assignmentNode->getLeft())->getIdentifier() == "x");
        REQUIRE(dynamic_pointer_cast<TypeDeclarationNode>(assignmentNode->getLeft()->getValue())->getType() == "Boolean");

        shared_ptr<FunctionDeclarationNode> funcDeclNode = dynamic_pointer_cast<FunctionDeclarationNode>(assignmentNode->getRight());

        REQUIRE(funcDeclNode->getParameters()->getElements().size() == 2);

        shared_ptr<IdentifierNode> param1 = dynamic_pointer_cast<IdentifierNode>(funcDeclNode->getParameters()->getElements()[0]);
        shared_ptr<IdentifierNode> param2 = dynamic_pointer_cast<IdentifierNode>(funcDeclNode->getParameters()->getElements()[1]);

        REQUIRE(param1->getIdentifier() == "meow");
        REQUIRE(dynamic_pointer_cast<TypeDeclarationNode>(param1->getValue())->getType() == "String");

        REQUIRE(param2->getIdentifier() == "cow");
        REQUIRE(dynamic_pointer_cast<TypeDeclarationNode>(param2->getValue())->getType() == "String");

        shared_ptr<BlockNode> blockNode = dynamic_pointer_cast<BlockNode>(funcDeclNode->getDefinition());
        REQUIRE(blockNode->getNodeType() == ASTNode::Types::BLOCK);
        REQUIRE(blockNode->getElements().size() == 1);

        shared_ptr<BinaryOperationNode> binOpNode = dynamic_pointer_cast<BinaryOperationNode>(blockNode->getElements()[0]);
        REQUIRE(binOpNode->getNodeType() == ASTNode::Types::BINARY_OPERATION);
        REQUIRE(binOpNode->getOperator() == "==");

        shared_ptr<IdentifierNode> leftIdentifierNode = dynamic_pointer_cast<IdentifierNode>(binOpNode->getLeft());
        shared_ptr<IdentifierNode> rightIdentifierNode = dynamic_pointer_cast<IdentifierNode>(binOpNode->getRight());

        REQUIRE(leftIdentifierNode->getIdentifier() == "meow");
        REQUIRE(rightIdentifierNode->getIdentifier() == "cow");
    }

    SECTION("Can parse curried function declarations with block definition") {
        string source = R"(
            x<Boolean> = x<String> -> y<String> -> {}
        )";
        lexer.lex(source);

        shared_ptr<SourceNode> parsedAST = dynamic_pointer_cast<SourceNode>(
            parser.parse(lexer.tokens, source, "fakeFile.th", filesByCapsuleName)
        );

        REQUIRE(parsedAST->getNodeType() == ASTNode::Types::SOURCE);
        REQUIRE(parsedAST->getLinks().size() == 0);
        REQUIRE(parsedAST->getValue()->getNodeType() == ASTNode::Types::ASSIGNMENT);

        shared_ptr<AssignmentNode> assignmentNode = dynamic_pointer_cast<AssignmentNode>(parsedAST->getValue());
        REQUIRE(assignmentNode->getLeft()->getNodeType() == ASTNode::Types::IDENTIFIER);
        REQUIRE(assignmentNode->getRight()->getNodeType() == ASTNode::Types::FUNCTION_DECLARATION);
        REQUIRE(dynamic_pointer_cast<IdentifierNode>(assignmentNode->getLeft())->getIdentifier() == "x");
        REQUIRE(dynamic_pointer_cast<TypeDeclarationNode>(assignmentNode->getLeft()->getValue())->getType() == "Boolean");

        shared_ptr<FunctionDeclarationNode> outerFuncDeclNode = dynamic_pointer_cast<FunctionDeclarationNode>(assignmentNode->getRight());

        REQUIRE(outerFuncDeclNode->getParameters()->getElements().size() == 1);

        shared_ptr<IdentifierNode> outerParam = dynamic_pointer_cast<IdentifierNode>(outerFuncDeclNode->getParameters()->getElements()[0]);
        REQUIRE(outerParam->getIdentifier() == "x");
        REQUIRE(dynamic_pointer_cast<TypeDeclarationNode>(outerParam->getValue())->getType() == "String");

        shared_ptr<FunctionDeclarationNode> innerFuncDeclNode = dynamic_pointer_cast<FunctionDeclarationNode>(outerFuncDeclNode->getDefinition());
        REQUIRE(innerFuncDeclNode->getNodeType() == ASTNode::Types::FUNCTION_DECLARATION);

        REQUIRE(innerFuncDeclNode->getParameters()->getElements().size() == 1);

        shared_ptr<IdentifierNode> innerParam = dynamic_pointer_cast<IdentifierNode>(innerFuncDeclNode->getParameters()->getElements()[0]);
        REQUIRE(innerParam->getIdentifier() == "y");
        REQUIRE(dynamic_pointer_cast<TypeDeclarationNode>(innerParam->getValue())->getType() == "String");

        shared_ptr<BlockNode> blockNode = dynamic_pointer_cast<BlockNode>(innerFuncDeclNode->getDefinition());
        REQUIRE(blockNode->getNodeType() == ASTNode::Types::BLOCK);
        REQUIRE(blockNode->getElements().size() == 0);
    }

    SECTION("Can parse function invocation with no arguments") {
        string source = R"(
            greet()
        )";
        lexer.lex(source);

        shared_ptr<SourceNode> parsedAST = dynamic_pointer_cast<SourceNode>(
            parser.parse(lexer.tokens, source, "fakeFile.th", filesByCapsuleName)
        );

        REQUIRE(parsedAST->getNodeType() == ASTNode::Types::SOURCE);
        REQUIRE(parsedAST->getLinks().size() == 0);
        REQUIRE(parsedAST->getValue()->getNodeType() == ASTNode::Types::FUNCTION_INVOCATION);

        shared_ptr<FunctionInvocationNode> funcInvocationNode = dynamic_pointer_cast<FunctionInvocationNode>(parsedAST->getValue());

        REQUIRE(funcInvocationNode != nullptr);

        shared_ptr<IdentifierNode> functionIdentifier = dynamic_pointer_cast<IdentifierNode>(funcInvocationNode->getIdentifier());
        REQUIRE(functionIdentifier->getIdentifier() == "greet");

        shared_ptr<ASTNodeList> arguments = funcInvocationNode->getParameters();
        REQUIRE(arguments->getElements().size() == 0);
    }

    SECTION("Can parse function invocation with a single argument") {
        string source = R"(
            greet(names)
        )";
        lexer.lex(source);

        shared_ptr<SourceNode> parsedAST = dynamic_pointer_cast<SourceNode>(
            parser.parse(lexer.tokens, source, "fakeFile.th", filesByCapsuleName)
        );

        REQUIRE(parsedAST->getNodeType() == ASTNode::Types::SOURCE);
        REQUIRE(parsedAST->getLinks().size() == 0);
        REQUIRE(parsedAST->getValue()->getNodeType() == ASTNode::Types::FUNCTION_INVOCATION);

        shared_ptr<FunctionInvocationNode> funcInvocationNode = dynamic_pointer_cast<FunctionInvocationNode>(parsedAST->getValue());

        REQUIRE(funcInvocationNode != nullptr);

        shared_ptr<IdentifierNode> functionIdentifier = dynamic_pointer_cast<IdentifierNode>(funcInvocationNode->getIdentifier());
        REQUIRE(functionIdentifier->getIdentifier() == "greet");

        shared_ptr<ASTNodeList> arguments = funcInvocationNode->getParameters();
        REQUIRE(arguments->getElements().size() == 1);

        shared_ptr<IdentifierNode> argument = dynamic_pointer_cast<IdentifierNode>(arguments->getElements()[0]);
        REQUIRE(argument->getIdentifier() == "names");
    }

    SECTION("Can parse function invocation with multiple arguments") {
        string source = R"(
            greet(names, 'Hello, ')
        )";
        lexer.lex(source);

        shared_ptr<SourceNode> parsedAST = dynamic_pointer_cast<SourceNode>(
            parser.parse(lexer.tokens, source, "fakeFile.th", filesByCapsuleName)
        );

        REQUIRE(parsedAST->getNodeType() == ASTNode::Types::SOURCE);
        REQUIRE(parsedAST->getLinks().size() == 0);
        REQUIRE(parsedAST->getValue()->getNodeType() == ASTNode::Types::FUNCTION_INVOCATION);

        shared_ptr<FunctionInvocationNode> funcInvocationNode = dynamic_pointer_cast<FunctionInvocationNode>(parsedAST->getValue());

        REQUIRE(funcInvocationNode != nullptr);

        shared_ptr<IdentifierNode> functionIdentifier = dynamic_pointer_cast<IdentifierNode>(funcInvocationNode->getIdentifier());
        REQUIRE(functionIdentifier->getIdentifier() == "greet");

        shared_ptr<ASTNodeList> arguments = funcInvocationNode->getParameters();
        REQUIRE(arguments->getElements().size() == 2);

        shared_ptr<IdentifierNode> firstArgument = dynamic_pointer_cast<IdentifierNode>(arguments->getElements()[0]);
        REQUIRE(firstArgument->getIdentifier() == "names");

        shared_ptr<LiteralNode> secondArgument = dynamic_pointer_cast<LiteralNode>(arguments->getElements()[1]);
        REQUIRE(secondArgument->getLiteralValue() == "'Hello, '");
    }

    SECTION("Can parse function invocation with a list argument") {
        string source = R"(
            greet(['alex', 'mike', 'denis'])
        )";
        lexer.lex(source);

        shared_ptr<SourceNode> parsedAST = dynamic_pointer_cast<SourceNode>(
            parser.parse(lexer.tokens, source, "fakeFile.th", filesByCapsuleName)
        );

        REQUIRE(parsedAST->getNodeType() == ASTNode::Types::SOURCE);
        REQUIRE(parsedAST->getLinks().size() == 0);
        REQUIRE(parsedAST->getValue()->getNodeType() == ASTNode::Types::FUNCTION_INVOCATION);

        shared_ptr<FunctionInvocationNode> funcInvocationNode = dynamic_pointer_cast<FunctionInvocationNode>(parsedAST->getValue());

        REQUIRE(funcInvocationNode != nullptr);

        shared_ptr<IdentifierNode> functionIdentifier = dynamic_pointer_cast<IdentifierNode>(funcInvocationNode->getIdentifier());
        REQUIRE(functionIdentifier->getIdentifier() == "greet");

        shared_ptr<ASTNodeList> arguments = funcInvocationNode->getParameters();
        REQUIRE(arguments->getElements().size() == 1);

        shared_ptr<ListNode> listArgument = dynamic_pointer_cast<ListNode>(arguments->getElements()[0]);
        REQUIRE(listArgument->getNodeType() == ASTNode::Types::LIST);

        vector<shared_ptr<ASTNode>> listElements = listArgument->getElements();
        REQUIRE(listElements.size() == 3);

        shared_ptr<LiteralNode> firstElement = dynamic_pointer_cast<LiteralNode>(listElements[0]);
        REQUIRE(firstElement->getLiteralValue() == "'alex'");

        shared_ptr<LiteralNode> secondElement = dynamic_pointer_cast<LiteralNode>(listElements[1]);
        REQUIRE(secondElement->getLiteralValue() == "'mike'");

        shared_ptr<LiteralNode> thirdElement = dynamic_pointer_cast<LiteralNode>(listElements[2]);
        REQUIRE(thirdElement->getLiteralValue() == "'denis'");
    }

    SECTION("Can parse function invocation with multiple list arguments") {
        string source = R"(
            greet(['alex', 'mike', 'denis'], ['Hello', 'Hi'])
        )";
        lexer.lex(source);

        shared_ptr<SourceNode> parsedAST = dynamic_pointer_cast<SourceNode>(
            parser.parse(lexer.tokens, source, "fakeFile.th", filesByCapsuleName)
        );

        REQUIRE(parsedAST->getNodeType() == ASTNode::Types::SOURCE);
        REQUIRE(parsedAST->getLinks().size() == 0);
        REQUIRE(parsedAST->getValue()->getNodeType() == ASTNode::Types::FUNCTION_INVOCATION);

        shared_ptr<FunctionInvocationNode> funcInvocationNode = dynamic_pointer_cast<FunctionInvocationNode>(parsedAST->getValue());

        REQUIRE(funcInvocationNode != nullptr);

        shared_ptr<IdentifierNode> functionIdentifier = dynamic_pointer_cast<IdentifierNode>(funcInvocationNode->getIdentifier());
        REQUIRE(functionIdentifier->getIdentifier() == "greet");

        shared_ptr<ASTNodeList> arguments = funcInvocationNode->getParameters();
        REQUIRE(arguments->getElements().size() == 2);

        // First list argument
        shared_ptr<ListNode> listArgument1 = dynamic_pointer_cast<ListNode>(arguments->getElements()[0]);
        REQUIRE(listArgument1->getNodeType() == ASTNode::Types::LIST);

        vector<shared_ptr<ASTNode>> listElements1 = listArgument1->getElements();
        REQUIRE(listElements1.size() == 3);

        shared_ptr<LiteralNode> firstElement1 = dynamic_pointer_cast<LiteralNode>(listElements1[0]);
        REQUIRE(firstElement1->getLiteralValue() == "'alex'");

        shared_ptr<LiteralNode> secondElement1 = dynamic_pointer_cast<LiteralNode>(listElements1[1]);
        REQUIRE(secondElement1->getLiteralValue() == "'mike'");

        shared_ptr<LiteralNode> thirdElement1 = dynamic_pointer_cast<LiteralNode>(listElements1[2]);
        REQUIRE(thirdElement1->getLiteralValue() == "'denis'");

        // Second list argument
        shared_ptr<ListNode> listArgument2 = dynamic_pointer_cast<ListNode>(arguments->getElements()[1]);
        REQUIRE(listArgument2->getNodeType() == ASTNode::Types::LIST);

        vector<shared_ptr<ASTNode>> listElements2 = listArgument2->getElements();
        REQUIRE(listElements2.size() == 2);

        shared_ptr<LiteralNode> firstElement2 = dynamic_pointer_cast<LiteralNode>(listElements2[0]);
        REQUIRE(firstElement2->getLiteralValue() == "'Hello'");

        shared_ptr<LiteralNode> secondElement2 = dynamic_pointer_cast<LiteralNode>(listElements2[1]);
        REQUIRE(secondElement2->getLiteralValue() == "'Hi'");
    }

    SECTION("Can parse function invocation with dictionary argument") {
        string source = R"(
            greet({ name: 'Alex' })
        )";
        lexer.lex(source);

        shared_ptr<SourceNode> parsedAST = dynamic_pointer_cast<SourceNode>(
            parser.parse(lexer.tokens, source, "fakeFile.th", filesByCapsuleName)
        );

        REQUIRE(parsedAST->getNodeType() == ASTNode::Types::SOURCE);
        REQUIRE(parsedAST->getLinks().size() == 0);
        REQUIRE(parsedAST->getValue()->getNodeType() == ASTNode::Types::FUNCTION_INVOCATION);

        shared_ptr<FunctionInvocationNode> funcInvocationNode = dynamic_pointer_cast<FunctionInvocationNode>(parsedAST->getValue());

        REQUIRE(funcInvocationNode != nullptr);

        shared_ptr<IdentifierNode> functionIdentifier = dynamic_pointer_cast<IdentifierNode>(funcInvocationNode->getIdentifier());
        REQUIRE(functionIdentifier->getIdentifier() == "greet");

        shared_ptr<ASTNodeList> arguments = funcInvocationNode->getParameters();
        REQUIRE(arguments->getElements().size() == 1);

        // Dictionary argument
        shared_ptr<DictionaryNode> dictArgument = dynamic_pointer_cast<DictionaryNode>(arguments->getElements()[0]);
        REQUIRE(dictArgument->getNodeType() == ASTNode::Types::DICTIONARY);

        vector<shared_ptr<ASTNode>> dictElements = dictArgument->getElements();
        REQUIRE(dictElements.size() == 1);

        shared_ptr<TupleNode> tupleElement = dynamic_pointer_cast<TupleNode>(dictElements[0]);
        REQUIRE(tupleElement->getNodeType() == ASTNode::Types::TUPLE);

        shared_ptr<SymbolNode> firstElement = dynamic_pointer_cast<SymbolNode>(tupleElement->getLeft());
        REQUIRE(firstElement->getNodeType() == ASTNode::Types::SYMBOL);
        REQUIRE(firstElement->getSymbol() == ":name");

        shared_ptr<LiteralNode> secondElement = dynamic_pointer_cast<LiteralNode>(tupleElement->getRight());
        REQUIRE(secondElement->getNodeType() == ASTNode::Types::STRING_LITERAL);
        REQUIRE(secondElement->getLiteralValue() == "'Alex'");
    }

    SECTION("Can parse function invocation with function declaration argument") {
        string source = R"(
            greet(name, greeting<String> -> greeting + name)
        )";
        lexer.lex(source);

        shared_ptr<SourceNode> parsedAST = dynamic_pointer_cast<SourceNode>(
            parser.parse(lexer.tokens, source, "fakeFile.th", filesByCapsuleName)
        );

        REQUIRE(parsedAST->getNodeType() == ASTNode::Types::SOURCE);
        REQUIRE(parsedAST->getLinks().size() == 0);
        REQUIRE(parsedAST->getValue()->getNodeType() == ASTNode::Types::FUNCTION_INVOCATION);

        shared_ptr<FunctionInvocationNode> funcInvocationNode = dynamic_pointer_cast<FunctionInvocationNode>(parsedAST->getValue());

        REQUIRE(funcInvocationNode != nullptr);

        shared_ptr<IdentifierNode> functionIdentifier = dynamic_pointer_cast<IdentifierNode>(funcInvocationNode->getIdentifier());
        REQUIRE(functionIdentifier->getIdentifier() == "greet");

        shared_ptr<ASTNodeList> arguments = funcInvocationNode->getParameters();
        REQUIRE(arguments->getElements().size() == 2);

        // First argument (name)
        shared_ptr<IdentifierNode> firstArg = dynamic_pointer_cast<IdentifierNode>(arguments->getElements()[0]);
        REQUIRE(firstArg->getNodeType() == ASTNode::Types::IDENTIFIER);
        REQUIRE(firstArg->getIdentifier() == "name");

        // Second argument (function declaration)
        shared_ptr<FunctionDeclarationNode> funcDeclArg = dynamic_pointer_cast<FunctionDeclarationNode>(arguments->getElements()[1]);
        REQUIRE(funcDeclArg->getNodeType() == ASTNode::Types::FUNCTION_DECLARATION);

        shared_ptr<ASTNodeList> parameters = funcDeclArg->getParameters();
        REQUIRE(parameters->getElements().size() == 1);

        shared_ptr<IdentifierNode> param = dynamic_pointer_cast<IdentifierNode>(parameters->getElements()[0]);
        REQUIRE(param->getNodeType() == ASTNode::Types::IDENTIFIER);
        REQUIRE(param->getIdentifier() == "greeting");
        REQUIRE(dynamic_pointer_cast<TypeDeclarationNode>(param->getValue())->getType() == "String");

        shared_ptr<BinaryOperationNode> binOpNode = dynamic_pointer_cast<BinaryOperationNode>(funcDeclArg->getDefinition());
        REQUIRE(binOpNode->getNodeType() == ASTNode::Types::BINARY_OPERATION);
        REQUIRE(binOpNode->getOperator() == "+");

        shared_ptr<IdentifierNode> leftNode = dynamic_pointer_cast<IdentifierNode>(binOpNode->getLeft());
        REQUIRE(leftNode->getNodeType() == ASTNode::Types::IDENTIFIER);
        REQUIRE(leftNode->getIdentifier() == "greeting");

        shared_ptr<IdentifierNode> rightNode = dynamic_pointer_cast<IdentifierNode>(binOpNode->getRight());
        REQUIRE(rightNode->getNodeType() == ASTNode::Types::IDENTIFIER);
        REQUIRE(rightNode->getIdentifier() == "name");
    }

    SECTION("Can parse function invocation with function declaration argument having multiple parameters and a binary operation definition") {
        string source = R"(
            greet(name, (greeting<String>, punctuation<String>) -> greeting + name + punctuation)
        )";
        lexer.lex(source);

        shared_ptr<SourceNode> parsedAST = dynamic_pointer_cast<SourceNode>(
            parser.parse(lexer.tokens, source, "fakeFile.th", filesByCapsuleName)
        );

        REQUIRE(parsedAST->getNodeType() == ASTNode::Types::SOURCE);
        REQUIRE(parsedAST->getLinks().size() == 0);
        REQUIRE(parsedAST->getValue()->getNodeType() == ASTNode::Types::FUNCTION_INVOCATION);

        shared_ptr<FunctionInvocationNode> funcInvocationNode = dynamic_pointer_cast<FunctionInvocationNode>(parsedAST->getValue());

        REQUIRE(funcInvocationNode != nullptr);

        shared_ptr<IdentifierNode> functionIdentifier = dynamic_pointer_cast<IdentifierNode>(funcInvocationNode->getIdentifier());
        REQUIRE(functionIdentifier->getIdentifier() == "greet");

        shared_ptr<ASTNodeList> arguments = funcInvocationNode->getParameters();
        REQUIRE(arguments->getElements().size() == 2);

        // First argument (name)
        shared_ptr<IdentifierNode> firstArg = dynamic_pointer_cast<IdentifierNode>(arguments->getElements()[0]);
        REQUIRE(firstArg->getNodeType() == ASTNode::Types::IDENTIFIER);
        REQUIRE(firstArg->getIdentifier() == "name");

        // Second argument (function declaration)
        shared_ptr<FunctionDeclarationNode> funcDeclArg = dynamic_pointer_cast<FunctionDeclarationNode>(arguments->getElements()[1]);
        REQUIRE(funcDeclArg->getNodeType() == ASTNode::Types::FUNCTION_DECLARATION);

        shared_ptr<ASTNodeList> parameters = funcDeclArg->getParameters();
        REQUIRE(parameters->getElements().size() == 2);

        shared_ptr<IdentifierNode> param1 = dynamic_pointer_cast<IdentifierNode>(parameters->getElements()[0]);
        REQUIRE(param1->getNodeType() == ASTNode::Types::IDENTIFIER);
        REQUIRE(param1->getIdentifier() == "greeting");
        REQUIRE(dynamic_pointer_cast<TypeDeclarationNode>(param1->getValue())->getType() == "String");

        shared_ptr<IdentifierNode> param2 = dynamic_pointer_cast<IdentifierNode>(parameters->getElements()[1]);
        REQUIRE(param2->getNodeType() == ASTNode::Types::IDENTIFIER);
        REQUIRE(param2->getIdentifier() == "punctuation");
        REQUIRE(dynamic_pointer_cast<TypeDeclarationNode>(param2->getValue())->getType() == "String");

        shared_ptr<BinaryOperationNode> outerBinOpNode = dynamic_pointer_cast<BinaryOperationNode>(funcDeclArg->getDefinition());
        REQUIRE(outerBinOpNode->getNodeType() == ASTNode::Types::BINARY_OPERATION);
        REQUIRE(outerBinOpNode->getOperator() == "+");

        shared_ptr<BinaryOperationNode> innerBinOpNode = dynamic_pointer_cast<BinaryOperationNode>(outerBinOpNode->getLeft());
        REQUIRE(innerBinOpNode->getNodeType() == ASTNode::Types::BINARY_OPERATION);
        REQUIRE(innerBinOpNode->getOperator() == "+");

        shared_ptr<IdentifierNode> leftNode = dynamic_pointer_cast<IdentifierNode>(innerBinOpNode->getLeft());
        REQUIRE(leftNode->getNodeType() == ASTNode::Types::IDENTIFIER);
        REQUIRE(leftNode->getIdentifier() == "greeting");

        shared_ptr<IdentifierNode> middleNode = dynamic_pointer_cast<IdentifierNode>(innerBinOpNode->getRight());
        REQUIRE(middleNode->getNodeType() == ASTNode::Types::IDENTIFIER);
        REQUIRE(middleNode->getIdentifier() == "name");

        shared_ptr<IdentifierNode> rightNode = dynamic_pointer_cast<IdentifierNode>(outerBinOpNode->getRight());
        REQUIRE(rightNode->getNodeType() == ASTNode::Types::IDENTIFIER);
        REQUIRE(rightNode->getIdentifier() == "punctuation");
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

        REQUIRE(parsedAST->getNodeType() == ASTNode::Types::SOURCE);
        REQUIRE(parsedAST->getLinks().size() == 0);
        REQUIRE(parsedAST->getValue()->getNodeType() == ASTNode::Types::CAPSULE);

        shared_ptr<CapsuleNode> capsuleNode = dynamic_pointer_cast<CapsuleNode>(parsedAST->getValue());
        REQUIRE(capsuleNode->getName() == "TextCapsule");

        shared_ptr<BlockNode> blockNode = dynamic_pointer_cast<BlockNode>(capsuleNode->getValue());
        REQUIRE(blockNode->getNodeType() == ASTNode::Types::BLOCK);
        vector<shared_ptr<ASTNode>> blockExpressions = blockNode->getElements();
        REQUIRE(blockExpressions.size() == 2);

        // Check the first assignment within the block
        shared_ptr<AssignmentNode> assignmentNodeX = dynamic_pointer_cast<AssignmentNode>(blockExpressions[0]);
        REQUIRE(assignmentNodeX->getLeft()->getNodeType() == ASTNode::Types::IDENTIFIER);
        REQUIRE(dynamic_pointer_cast<IdentifierNode>(assignmentNodeX->getLeft())->getIdentifier() == "x");
        REQUIRE(dynamic_pointer_cast<TypeDeclarationNode>(assignmentNodeX->getLeft()->getValue())->getType() == "Dict");
        REQUIRE(assignmentNodeX->getRight()->getNodeType() == ASTNode::Types::DICTIONARY);

        shared_ptr<DictionaryNode> dictNode = dynamic_pointer_cast<DictionaryNode>(assignmentNodeX->getRight());
        REQUIRE(dictNode->getElements().size() == 1);
        shared_ptr<TupleNode> tupleNode = dynamic_pointer_cast<TupleNode>(dictNode->getElements()[0]);
        REQUIRE(tupleNode->getLeft()->getNodeType() == ASTNode::Types::SYMBOL);
        REQUIRE(dynamic_pointer_cast<SymbolNode>(tupleNode->getLeft())->getSymbol() == ":bob");
        REQUIRE(tupleNode->getRight()->getNodeType() == ASTNode::Types::STRING_LITERAL);
        REQUIRE(dynamic_pointer_cast<LiteralNode>(tupleNode->getRight())->getLiteralValue() == "'saget'");

        // Check the second assignment within the block
        shared_ptr<AssignmentNode> assignmentNodeY = dynamic_pointer_cast<AssignmentNode>(blockExpressions[1]);
        REQUIRE(assignmentNodeY->getLeft()->getNodeType() == ASTNode::Types::IDENTIFIER);
        REQUIRE(dynamic_pointer_cast<IdentifierNode>(assignmentNodeY->getLeft())->getIdentifier() == "y");
        REQUIRE(dynamic_pointer_cast<TypeDeclarationNode>(assignmentNodeY->getLeft()->getValue())->getType() == "Number");
        REQUIRE(assignmentNodeY->getRight()->getNodeType() == ASTNode::Types::NUMBER_LITERAL);
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

        REQUIRE(parsedAST->getNodeType() == ASTNode::Types::SOURCE);
        REQUIRE(parsedAST->getLinks().size() == 2);

        // Check first link
        shared_ptr<LinkNode> linkNode1 = dynamic_pointer_cast<LinkNode>(parsedAST->getLinks()[0]);
        REQUIRE(linkNode1->getNodeType() == ASTNode::Types::LINK);
        REQUIRE(linkNode1->capsule == "Theta.StringUtil");

        shared_ptr<SourceNode> linkedSource1 = dynamic_pointer_cast<SourceNode>(linkNode1->getValue());
        REQUIRE(linkedSource1->getNodeType() == ASTNode::Types::SOURCE);
        REQUIRE(linkedSource1->getLinks().size() == 0);

        shared_ptr<CapsuleNode> linkedCapsule1 = dynamic_pointer_cast<CapsuleNode>(linkedSource1->getValue());
        REQUIRE(linkedCapsule1->getNodeType() == ASTNode::Types::CAPSULE);
        REQUIRE(linkedCapsule1->name == "Theta.StringUtil");

        shared_ptr<BlockNode> linkedBlock1 = dynamic_pointer_cast<BlockNode>(linkedCapsule1->getValue());
        REQUIRE(linkedBlock1->getNodeType() == ASTNode::Types::BLOCK);
        REQUIRE(linkedBlock1->getElements().size() == 1);

        shared_ptr<AssignmentNode> assignmentNode1 = dynamic_pointer_cast<AssignmentNode>(linkedBlock1->getElements()[0]);
        REQUIRE(assignmentNode1->getLeft()->getNodeType() == ASTNode::Types::IDENTIFIER);
        REQUIRE(assignmentNode1->getRight()->getNodeType() == ASTNode::Types::STRING_LITERAL);

        shared_ptr<IdentifierNode> leftNode1 = dynamic_pointer_cast<IdentifierNode>(assignmentNode1->getLeft());
        REQUIRE(leftNode1->getIdentifier() == "name");
        REQUIRE(dynamic_pointer_cast<TypeDeclarationNode>(leftNode1->getValue())->getType() == "String");

        shared_ptr<LiteralNode> rightNode1 = dynamic_pointer_cast<LiteralNode>(assignmentNode1->getRight());
        REQUIRE(rightNode1->getLiteralValue() == "'Bobby'");

        // Check second link
        shared_ptr<LinkNode> linkNode2 = dynamic_pointer_cast<LinkNode>(parsedAST->getLinks()[1]);
        REQUIRE(linkNode2->getNodeType() == ASTNode::Types::LINK);
        REQUIRE(linkNode2->capsule == "Theta.StringTraversal");

        shared_ptr<SourceNode> linkedSource2 = dynamic_pointer_cast<SourceNode>(linkNode2->getValue());
        REQUIRE(linkedSource2->getNodeType() == ASTNode::Types::SOURCE);
        REQUIRE(linkedSource2->getLinks().size() == 1);

        shared_ptr<LinkNode> nestedLinkNode2 = dynamic_pointer_cast<LinkNode>(linkedSource2->getLinks()[0]);
        REQUIRE(nestedLinkNode2->getNodeType() == ASTNode::Types::LINK);
        REQUIRE(nestedLinkNode2->capsule == "Theta.StringUtil");

        shared_ptr<SourceNode> nestedLinkedSource2 = dynamic_pointer_cast<SourceNode>(nestedLinkNode2->getValue());
        REQUIRE(nestedLinkedSource2->getNodeType() == ASTNode::Types::SOURCE);
        REQUIRE(nestedLinkedSource2->getLinks().size() == 0);

        shared_ptr<CapsuleNode> nestedLinkedCapsule2 = dynamic_pointer_cast<CapsuleNode>(nestedLinkedSource2->getValue());
        REQUIRE(nestedLinkedCapsule2->getNodeType() == ASTNode::Types::CAPSULE);
        REQUIRE(nestedLinkedCapsule2->name == "Theta.StringUtil");

        shared_ptr<BlockNode> nestedLinkedBlock2 = dynamic_pointer_cast<BlockNode>(nestedLinkedCapsule2->getValue());
        REQUIRE(nestedLinkedBlock2->getNodeType() == ASTNode::Types::BLOCK);
        REQUIRE(nestedLinkedBlock2->getElements().size() == 1);

        shared_ptr<AssignmentNode> nestedAssignmentNode2 = dynamic_pointer_cast<AssignmentNode>(nestedLinkedBlock2->getElements()[0]);
        REQUIRE(nestedAssignmentNode2->getLeft()->getNodeType() == ASTNode::Types::IDENTIFIER);
        REQUIRE(nestedAssignmentNode2->getRight()->getNodeType() == ASTNode::Types::STRING_LITERAL);

        shared_ptr<IdentifierNode> nestedLeftNode2 = dynamic_pointer_cast<IdentifierNode>(nestedAssignmentNode2->getLeft());
        REQUIRE(nestedLeftNode2->getIdentifier() == "name");
        REQUIRE(dynamic_pointer_cast<TypeDeclarationNode>(nestedLeftNode2->getValue())->getType() == "String");

        shared_ptr<LiteralNode> nestedRightNode2 = dynamic_pointer_cast<LiteralNode>(nestedAssignmentNode2->getRight());
        REQUIRE(nestedRightNode2->getLiteralValue() == "'Bobby'");

        // Check MyTestCapsule
        shared_ptr<CapsuleNode> mainCapsule = dynamic_pointer_cast<CapsuleNode>(parsedAST->getValue());
        REQUIRE(mainCapsule->getNodeType() == ASTNode::Types::CAPSULE);
        REQUIRE(mainCapsule->name == "MyTestCapsule");

        shared_ptr<BlockNode> mainBlock = dynamic_pointer_cast<BlockNode>(mainCapsule->getValue());
        REQUIRE(mainBlock->getNodeType() == ASTNode::Types::BLOCK);
        REQUIRE(mainBlock->getElements().size() == 1);

        shared_ptr<AssignmentNode> mainAssignmentNode = dynamic_pointer_cast<AssignmentNode>(mainBlock->getElements()[0]);
        REQUIRE(mainAssignmentNode->getLeft()->getNodeType() == ASTNode::Types::IDENTIFIER);
        REQUIRE(mainAssignmentNode->getRight()->getNodeType() == ASTNode::Types::IDENTIFIER);

        shared_ptr<IdentifierNode> mainLeftNode = dynamic_pointer_cast<IdentifierNode>(mainAssignmentNode->getLeft());
        REQUIRE(mainLeftNode->getIdentifier() == "x");
        REQUIRE(dynamic_pointer_cast<TypeDeclarationNode>(mainLeftNode->getValue())->getType() == "String");

        shared_ptr<IdentifierNode> mainRightNode = dynamic_pointer_cast<IdentifierNode>(mainAssignmentNode->getRight());
        REQUIRE(mainRightNode->getIdentifier() == "Theta.StringUtil.name");
    }

    SECTION("Can parse struct declarations inside a capsule") {
        string source = R"(
            capsule Math {
                struct Point {
                    x<Number>
                    y<Number>
                }
            }
        )";
        lexer.lex(source);

        shared_ptr<SourceNode> parsedAST = dynamic_pointer_cast<SourceNode>(
            parser.parse(lexer.tokens, source, "fakeFile.th", filesByCapsuleName)
        );

        REQUIRE(parsedAST->getNodeType() == ASTNode::Types::SOURCE);
        REQUIRE(parsedAST->getLinks().size() == 0);
        REQUIRE(parsedAST->getValue()->getNodeType() == ASTNode::Types::CAPSULE);

        shared_ptr<CapsuleNode> capsuleNode = dynamic_pointer_cast<CapsuleNode>(parsedAST->getValue());
        REQUIRE(capsuleNode != nullptr);
        REQUIRE(capsuleNode->getName() == "Math");

        shared_ptr<BlockNode> blockNode = dynamic_pointer_cast<BlockNode>(capsuleNode->getValue());
        REQUIRE(blockNode != nullptr);
        REQUIRE(blockNode->getNodeType() == ASTNode::Types::BLOCK);
        REQUIRE(blockNode->getElements().size() == 1);

        shared_ptr<StructDefinitionNode> structNode = dynamic_pointer_cast<StructDefinitionNode>(blockNode->getElements()[0]);
        REQUIRE(structNode != nullptr);
        REQUIRE(structNode->getNodeType() == ASTNode::Types::STRUCT_DEFINITION);
        REQUIRE(structNode->name == "Point");

        REQUIRE(structNode->getElements().size() == 2);

        shared_ptr<IdentifierNode> xNode = dynamic_pointer_cast<IdentifierNode>(structNode->getElements()[0]);
        REQUIRE(xNode != nullptr);
        REQUIRE(xNode->getNodeType() == ASTNode::Types::IDENTIFIER);
        REQUIRE(xNode->getIdentifier() == "x");
        REQUIRE(dynamic_pointer_cast<TypeDeclarationNode>(xNode->getValue())->getType() == "Number");

        shared_ptr<IdentifierNode> yNode = dynamic_pointer_cast<IdentifierNode>(structNode->getElements()[1]);
        REQUIRE(yNode != nullptr);
        REQUIRE(yNode->getNodeType() == ASTNode::Types::IDENTIFIER);
        REQUIRE(yNode->getIdentifier() == "y");
        REQUIRE(dynamic_pointer_cast<TypeDeclarationNode>(yNode->getValue())->getType() == "Number");
    }

    SECTION("Can parse struct declaration with dictionary values") {
        string source = R"(
            @Point { x: 5, y: 3 }
        )";
        lexer.lex(source);

        shared_ptr<SourceNode> parsedAST = dynamic_pointer_cast<SourceNode>(
            parser.parse(lexer.tokens, source, "fakeFile.th", filesByCapsuleName)
        );

        REQUIRE(parsedAST->getNodeType() == ASTNode::Types::SOURCE);
        REQUIRE(parsedAST->getLinks().size() == 0);
        REQUIRE(parsedAST->getValue()->getNodeType() == ASTNode::Types::STRUCT_DECLARATION);

        shared_ptr<StructDeclarationNode> structDeclNode = dynamic_pointer_cast<StructDeclarationNode>(parsedAST->getValue());
        REQUIRE(structDeclNode != nullptr);
        REQUIRE(structDeclNode->getStructType() == "Point");

        shared_ptr<DictionaryNode> dictNode = dynamic_pointer_cast<DictionaryNode>(structDeclNode->getValue());
        REQUIRE(dictNode != nullptr);
        REQUIRE(dictNode->getNodeType() == ASTNode::Types::DICTIONARY);
        REQUIRE(dictNode->getElements().size() == 2);

        shared_ptr<TupleNode> tuple1 = dynamic_pointer_cast<TupleNode>(dictNode->getElements()[0]);
        REQUIRE(tuple1 != nullptr);
        REQUIRE(tuple1->getNodeType() == ASTNode::Types::TUPLE);
        REQUIRE(tuple1->getLeft()->getNodeType() == ASTNode::Types::SYMBOL);
        REQUIRE(tuple1->getRight()->getNodeType() == ASTNode::Types::NUMBER_LITERAL);

        shared_ptr<SymbolNode> symbol1 = dynamic_pointer_cast<SymbolNode>(tuple1->getLeft());
        REQUIRE(symbol1 != nullptr);
        REQUIRE(symbol1->getSymbol() == ":x");

        shared_ptr<LiteralNode> number1 = dynamic_pointer_cast<LiteralNode>(tuple1->getRight());
        REQUIRE(number1 != nullptr);
        REQUIRE(number1->getLiteralValue() == "5");

        shared_ptr<TupleNode> tuple2 = dynamic_pointer_cast<TupleNode>(dictNode->getElements()[1]);
        REQUIRE(tuple2 != nullptr);
        REQUIRE(tuple2->getNodeType() == ASTNode::Types::TUPLE);
        REQUIRE(tuple2->getLeft()->getNodeType() == ASTNode::Types::SYMBOL);
        REQUIRE(tuple2->getRight()->getNodeType() == ASTNode::Types::NUMBER_LITERAL);

        shared_ptr<SymbolNode> symbol2 = dynamic_pointer_cast<SymbolNode>(tuple2->getLeft());
        REQUIRE(symbol2 != nullptr);
        REQUIRE(symbol2->getSymbol() == ":y");

        shared_ptr<LiteralNode> number2 = dynamic_pointer_cast<LiteralNode>(tuple2->getRight());
        REQUIRE(number2 != nullptr);
        REQUIRE(number2->getLiteralValue() == "3");
    }

    SECTION("Can parse enum declaration") {
        string source = R"(
            enum Level {
                :LOW
                :MEDIUM
                :HIGH
            }
        )";
        lexer.lex(source);

        shared_ptr<SourceNode> parsedAST = dynamic_pointer_cast<SourceNode>(
            parser.parse(lexer.tokens, source, "fakeFile.th", filesByCapsuleName)
        );

        REQUIRE(parsedAST->getNodeType() == ASTNode::Types::SOURCE);
        REQUIRE(parsedAST->getLinks().size() == 0);
        REQUIRE(parsedAST->getValue()->getNodeType() == ASTNode::Types::ENUM);

        shared_ptr<EnumNode> enumNode = dynamic_pointer_cast<EnumNode>(parsedAST->getValue());
        REQUIRE(enumNode != nullptr);
        REQUIRE(enumNode->getNodeType() == ASTNode::Types::ENUM);

        shared_ptr<IdentifierNode> identifierNode = dynamic_pointer_cast<IdentifierNode>(enumNode->getIdentifier());
        REQUIRE(identifierNode != nullptr);
        REQUIRE(identifierNode->getIdentifier() == "Level");

        vector<shared_ptr<ASTNode>> elements = enumNode->getElements();
        REQUIRE(elements.size() == 3);

        // Check :LOW
        shared_ptr<SymbolNode> symbol1 = dynamic_pointer_cast<SymbolNode>(elements[0]);
        REQUIRE(symbol1 != nullptr);
        REQUIRE(symbol1->getNodeType() == ASTNode::Types::SYMBOL);
        REQUIRE(symbol1->getSymbol() == ":LOW");

        // Check :MEDIUM
        shared_ptr<SymbolNode> symbol2 = dynamic_pointer_cast<SymbolNode>(elements[1]);
        REQUIRE(symbol2 != nullptr);
        REQUIRE(symbol2->getNodeType() == ASTNode::Types::SYMBOL);
        REQUIRE(symbol2->getSymbol() == ":MEDIUM");

        // Check :HIGH
        shared_ptr<SymbolNode> symbol3 = dynamic_pointer_cast<SymbolNode>(elements[2]);
        REQUIRE(symbol3 != nullptr);
        REQUIRE(symbol3->getNodeType() == ASTNode::Types::SYMBOL);
        REQUIRE(symbol3->getSymbol() == ":HIGH");
    }

    SECTION("Can parse capsule with function declaration containing a return statement") {
        string source = R"(
            capsule Math {
                x<String> = () -> {
                    return true
                }
            }
        )";
        lexer.lex(source);

        shared_ptr<SourceNode> parsedAST = dynamic_pointer_cast<SourceNode>(
            parser.parse(lexer.tokens, source, "fakeFile.th", filesByCapsuleName)
        );

        REQUIRE(parsedAST->getNodeType() == ASTNode::Types::SOURCE);
        REQUIRE(parsedAST->getLinks().size() == 0);
        REQUIRE(parsedAST->getValue()->getNodeType() == ASTNode::Types::CAPSULE);

        shared_ptr<CapsuleNode> capsuleNode = dynamic_pointer_cast<CapsuleNode>(parsedAST->getValue());
        REQUIRE(capsuleNode != nullptr);
        REQUIRE(capsuleNode->getName() == "Math");

        shared_ptr<BlockNode> blockNode = dynamic_pointer_cast<BlockNode>(capsuleNode->getValue());
        REQUIRE(blockNode->getNodeType() == ASTNode::Types::BLOCK);
        REQUIRE(blockNode->getElements().size() == 1);

        shared_ptr<AssignmentNode> assignmentNode = dynamic_pointer_cast<AssignmentNode>(blockNode->getElements()[0]);
        REQUIRE(assignmentNode != nullptr);
        REQUIRE(assignmentNode->getLeft()->getNodeType() == ASTNode::Types::IDENTIFIER);
        REQUIRE(dynamic_pointer_cast<IdentifierNode>(assignmentNode->getLeft())->getIdentifier() == "x");
        REQUIRE(dynamic_pointer_cast<TypeDeclarationNode>(assignmentNode->getLeft()->getValue())->getType() == "String");

        shared_ptr<FunctionDeclarationNode> funcDeclNode = dynamic_pointer_cast<FunctionDeclarationNode>(assignmentNode->getRight());
        REQUIRE(funcDeclNode != nullptr);
        REQUIRE(funcDeclNode->getParameters()->getElements().size() == 0);

        shared_ptr<BlockNode> funcBlockNode = dynamic_pointer_cast<BlockNode>(funcDeclNode->getDefinition());
        REQUIRE(funcBlockNode->getNodeType() == ASTNode::Types::BLOCK);
        REQUIRE(funcBlockNode->getElements().size() == 1);

        shared_ptr<ReturnNode> returnNode = dynamic_pointer_cast<ReturnNode>(funcBlockNode->getElements()[0]);
        REQUIRE(returnNode != nullptr);
        REQUIRE(returnNode->getNodeType() == ASTNode::Types::RETURN);

        shared_ptr<LiteralNode> returnValueNode = dynamic_pointer_cast<LiteralNode>(returnNode->getValue());
        REQUIRE(returnValueNode != nullptr);
        REQUIRE(returnValueNode->getNodeType() == ASTNode::Types::BOOLEAN_LITERAL);
        REQUIRE(returnValueNode->getLiteralValue() == "true");
    }
}
