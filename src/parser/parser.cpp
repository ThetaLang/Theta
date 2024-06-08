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
#include "ast/ast_node.hpp"

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

            if (currentToken.getType() == "identifier"){
                return parseIdentifier(currentToken);
            } else if (currentToken.getType() == "string") {
                return parseExpr(currentToken);
            }

            return nullptr;
        }

        shared_ptr<ASTNode> parseIdentifier(Token currentToken) {
            Token nextToken = remainingTokens->front();
            remainingTokens->pop_front();

            if (nextToken.getType() == "type" && remainingTokens->front().getType() == "operator" && remainingTokens->front().getText() == "=") {
                // Pop the assignment token;
                remainingTokens->pop_front();

                shared_ptr<ASTNode> node = make_shared<AssignmentNode>(currentToken.getText(), nextToken.getText());

                node->setValue(consume());
                return node;
            }

            return nullptr;
        }

        shared_ptr<ASTNode> parseExpr(Token currentToken) {
            Token nextToken = remainingTokens->front();
            remainingTokens->pop_front();

            if (currentToken.getType() == "string" && nextToken.getType() == "operator") {
                shared_ptr<ASTNode> node = make_shared<BinaryOperationNode>(currentToken.getText());
                
                node->setValue("left", parseLiteral(currentToken));
                node->setValue("right", parseLiteral(remainingTokens->front()));
                remainingTokens->pop_front();

                return node;
            }

            return nullptr;
        }

        shared_ptr<ASTNode> parseLiteral(Token currentToken) {
            if (currentToken.getType() == "string") {
                return make_shared<LiteralNode>(currentToken.getType(), currentToken.getText());
            }

            return nullptr;
        }
};