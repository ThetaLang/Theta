#include <vector>
#include <deque>
#include <string>
#include <iostream>
#include <memory>
#include "../lexer/token.hpp"
#include "../util/exceptions.hpp"
#include "ast/assignment_node.hpp"
#include "ast/binary_operation_node.hpp"
#include "ast/literal_node.hpp"
#include "ast/identifier_node.hpp"
#include "ast/ast_node.hpp"
#include "ast/type_declaration_node.hpp"

using namespace std;

// Top-down parsing based on lookahead, expanding leftmost nonterminal nodes first

class ThetaParser {
    public:
        void parse(deque<Token> &tokens, string source, string fileName) {
            remainingTokens = &tokens;

            for (int i = 0; i <= tokens.size(); i++) {
                if (tokens[i].getType() == "identifier") {
                    try {
                        validateIdentifier(tokens[i]);
                    } catch (SyntaxError &e) {
                        ExceptionFormatter::displayFormattedError("SyntaxError", e, source, fileName, tokens[i]);
                    }
                }
            }

            shared_ptr<ASTNode> rootASTNode = consume();

            if (rootASTNode) {
                cout << rootASTNode->toJSON() << endl;
            } else {
                cout << "NO ROOT NODE" << endl;
            }
        }

    private:
        deque<Token> *remainingTokens;

        void validateIdentifier(Token token) {
            string disallowedIdentifierChars = "!@#$%^&*()-=+/<>{}[]|?.,`~";
            
            for (int i = 0; i < token.getText().length(); i++) {
                char identChar = tolower(token.getText()[i]);

                bool isDisallowedChar = find(disallowedIdentifierChars.begin(), disallowedIdentifierChars.end(), identChar) != disallowedIdentifierChars.end();
                bool isStartsWithDigit = i == 0 && isdigit(identChar);

                if (isStartsWithDigit || isDisallowedChar) {
                    throw SyntaxError(
                        "Invalid identifier \"" + token.getText() + "\"",
                        token.getStartLocation()
                    );
                }
            }
        }

        // Consumes next token from our remaining tokens
        shared_ptr<ASTNode> consume() {
            if (remainingTokens->size() <= 0) return nullptr;

            Token currentToken = remainingTokens->front();
            remainingTokens->pop_front();

            Token nextToken = remainingTokens->front();

            if (
                currentToken.getType() == "identifier" && 
                nextToken.getType() == "angle_bracket_open" && 
                remainingTokens->at(1).getType() == "identifier" &&
                (remainingTokens->at(2).getType() == "angle_bracket_close" || remainingTokens->at(2).getType() == "angle_bracket_open")
            ) {
                deque<Token> typeDeclarationTokens;
                typeDeclarationTokens.push_back(nextToken);
                remainingTokens->pop_front(); // Pops the <

                int typeDeclarationDepth = 1;
                while (typeDeclarationDepth > 0) {
                    if (remainingTokens->front().getType() == "angle_bracket_open") {
                        typeDeclarationDepth++;
                    } else if (remainingTokens->front().getType() == "angle_bracket_close"){
                        typeDeclarationDepth--;
                    }

                    typeDeclarationTokens.push_back(remainingTokens->front());
                    remainingTokens->pop_front();
                }

                shared_ptr<ASTNode> typeNode = parseNestedTypeDeclaration(typeDeclarationTokens);

                return parseAssignment(currentToken, remainingTokens->front(), typeNode);
            } else if (
                currentToken.getType() == "angle_bracket_open" &&
                nextToken.getType() == "identifier" &&
                remainingTokens->at(1).getType() == "angle_bracket_close"
            ) {
                return parseTypeDeclaration(currentToken, nextToken);
            } else if (
                currentToken.getType() == "angle_bracket_open" &&
                nextToken.getType() == "identifier" &&
                remainingTokens->at(1).getType() == "angle_bracket_open"
            ) {
                return parseTypeDeclaration(currentToken, nextToken);
            } else if ((currentToken.getType() == "identifier" || currentToken.getType() == "string" || currentToken.getType() == "number") && nextToken.getType() == "operator") {
                return parseBinaryOperation(currentToken, nextToken);
            } else if (currentToken.getType() == "string" || currentToken.getType() == "number" || currentToken.getType() == "boolean") {
                return parseLiteral(currentToken);
            }

            return nullptr;
        }

        shared_ptr<ASTNode> parseAssignment(Token currentToken, Token nextToken, shared_ptr<ASTNode> typeNode) {
            shared_ptr<ASTNode> assignmentNode = make_shared<AssignmentNode>();

            shared_ptr<IdentifierNode> identNode = make_shared<IdentifierNode>(currentToken.getText());
            identNode->setType(typeNode);

            assignmentNode->setLeft(identNode);

            // Pop the =
            remainingTokens->pop_front();

            cout << remainingTokens->front().toJSON() + "IS IT" << "\n";

            assignmentNode->setRight(consume());

            return assignmentNode;
        }

        shared_ptr<ASTNode> parseNestedTypeDeclaration(deque<Token> &typeTokens) {
            // TODO: Recursively parse types from <identifier<identifier<...>>>
            typeTokens.pop_front(); // Pops the <

            shared_ptr<ASTNode> node = make_shared<TypeDeclarationNode>(typeTokens.front().getText());

            typeTokens.pop_front(); // Pops the type identifier

            if (typeTokens.front().getType() == "angle_bracket_open") {
                shared_ptr<ASTNode> typeChild = parseNestedTypeDeclaration(typeTokens);

                node->setValue(typeChild);
            }

            return node;
        }

        // shared_ptr<ASTNode> parseNestedTypeDeclaration(deque<Token> &typeTokens, shared_ptr<ASTNode> &parentNode) {
        //     // TODO: Recursively parse types from <identifier<identifier<...>>>
        // }

        shared_ptr<ASTNode> parseBinaryOperation(Token currentToken, Token nextToken) {
            remainingTokens->pop_front();

            shared_ptr<ASTNode> node = make_shared<BinaryOperationNode>(nextToken.getText());

            node->setLeft(currentToken.getType() == "identifier"
                ? parseIdentifier(currentToken)
                : parseLiteral(currentToken)
            );
            
            node->setRight(consume());

            return node;
        }

        shared_ptr<ASTNode> parseTypeDeclaration(Token currentToken, Token nextToken) {
            remainingTokens->pop_front();

            shared_ptr<ASTNode> node = make_shared<TypeDeclarationNode>(nextToken.getText());

            if (remainingTokens->front().getType() == "angle_bracket_open") {
                node->setValue(consume());
            }

            return node;
        }

        shared_ptr<ASTNode> parseIdentifier(Token currentToken) {
            return make_shared<IdentifierNode>(currentToken.getText());
        }

        shared_ptr<ASTNode> parseLiteral(Token currentToken) {
            return make_shared<LiteralNode>(currentToken.getType(), currentToken.getText());
        }
};