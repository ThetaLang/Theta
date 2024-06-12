#include <vector>
#include <deque>
#include <string>
#include <map>
#include <iostream>
#include <memory>
#include <filesystem>
#include "../lexer/token.hpp"
#include "../util/exceptions.hpp"
#include "ast/assignment_node.hpp"
#include "ast/binary_operation_node.hpp"
#include "ast/literal_node.hpp"
#include "ast/identifier_node.hpp"
#include "ast/ast_node.hpp"
#include "ast/type_declaration_node.hpp"
#include "ast/list_definition_node.hpp"
#include "ast/capsule_node.hpp"
#include "ast/function_declaration_node.hpp"
#include "ast/keyed_access_node.hpp"
#include "ast/source_node.hpp"
#include "ast/link_node.hpp"
#include "../compiler/compiler.hpp"

using namespace std;

// Top-down parsing based on lookahead, expanding leftmost nonterminal nodes first

class ThetaParser {
    public:
        shared_ptr<ASTNode> parse(deque<Token> &tokens, string &src, string file, shared_ptr<map<string, string>> filesByCapsuleName) {
            source = src;
            fileName = file;
            remainingTokens = &tokens;
            filesByCapsule = filesByCapsuleName;

            shared_ptr<SourceNode> rootASTNode = make_shared<SourceNode>();
            vector<shared_ptr<ASTNode>> linkNodes;

            // Parse out file imports
            while (remainingTokens->front().getType() == "keyword" && remainingTokens->front().getText() != "capsule") {
                string linkCapsuleName = remainingTokens->front().getText();
                shared_ptr<LinkNode> parsedLinkAST;

                auto it = parsedLinkASTs.find(linkCapsuleName);

                if (it != parsedLinkASTs.end()) {
                    cout << "FOUND PREPARED AST, SKIPPING PARSING OF LINKED CAPSULE" << endl;

                    parsedLinkAST = it->second;
                } else {
                    parsedLinkAST = dynamic_pointer_cast<LinkNode>(parseLink());
                    parsedLinkASTs.insert(make_pair(linkCapsuleName, parsedLinkAST));
                }

                linkNodes.push_back(parsedLinkAST);
                
            }

            rootASTNode->setLinks(linkNodes);
            rootASTNode->setValue(consume());

            if (rootASTNode) {
                cout << rootASTNode->toJSON() << endl;
            } else {
                cout << "NO ROOT NODE" << endl;
            }

            return rootASTNode;
        }

    private:
        string source;
        string fileName;
        deque<Token> *remainingTokens;
        map<string, shared_ptr<LinkNode>> parsedLinkASTs;
        shared_ptr<map<string, string>> filesByCapsule;

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

            if (currentToken.getType() == "keyword" && currentToken.getText() == "capsule") {
                return parseCapsule(nextToken);
            } else if (
                currentToken.getType() == "identifier" && 
                nextToken.getType() == "angle_bracket_open" && 
                remainingTokens->at(1).getType() == "identifier" &&
                (remainingTokens->at(2).getType() == "angle_bracket_close" || remainingTokens->at(2).getType() == "angle_bracket_open")
            ) {
                shared_ptr<ASTNode> identNode = parseIdentifierWithType(currentToken, nextToken);

                if (remainingTokens->front().getType() == "assignment") {
                    return parseAssignment(identNode);
                } else if (remainingTokens->front().getType() == "comma" || remainingTokens->front().getType() == "func_declaration") {
                    return parseFuncDeclaration(identNode);
                } else {
                    return identNode;
                }
            } else if ((currentToken.getType() == "identifier" || currentToken.getType() == "string" || currentToken.getType() == "number") && nextToken.getType() == "operator") {
                return parseBinaryOperation(currentToken, nextToken);
            } else if (currentToken.getType() == "identifier" && nextToken.getType() == "bracket_open") {
                return parseKeyedAccess(parseIdentifier(currentToken));
            } else if (currentToken.getType() == "bracket_open") {
                shared_ptr<ASTNode> listDefinition = parseListDefinition(currentToken, nextToken);
                
                if (remainingTokens->front().getType() == "bracket_open") {
                    return parseKeyedAccess(listDefinition);
                } else {
                    return listDefinition;
                }
            } else if (currentToken.getType() == "string" || currentToken.getType() == "number" || currentToken.getType() == "boolean") {
                return parseLiteral(currentToken);
            } else if (currentToken.getType() == "identifier") {
                return parseIdentifier(currentToken);
            }

            return nullptr;
        }

        shared_ptr<ASTNode> parseCapsule(Token nextToken) {
            remainingTokens->pop_front();
            remainingTokens->pop_front(); // Pops the {

            shared_ptr<CapsuleNode> capsuleNode = make_shared<CapsuleNode>(nextToken.getText());

            vector<shared_ptr<ASTNode>> definitionNodes;

            while (remainingTokens->front().getType() != "brace_close") {
                definitionNodes.push_back(consume());
            }

            capsuleNode->setDefinitions(definitionNodes);

            return capsuleNode;
        }

        shared_ptr<ASTNode> parseAssignment(shared_ptr<ASTNode> identifier) {
            shared_ptr<ASTNode> assignmentNode = make_shared<AssignmentNode>();
            assignmentNode->setLeft(identifier);

            // Pop the =
            remainingTokens->pop_front();

            assignmentNode->setRight(consume());

            return assignmentNode;
        }

        shared_ptr<ASTNode> parseFuncDeclaration(shared_ptr<ASTNode> param) {
            shared_ptr<FunctionDeclarationNode> funcNode = make_shared<FunctionDeclarationNode>();

            vector<shared_ptr<ASTNode>> paramNodes;
            vector<shared_ptr<ASTNode>> definitionNodes;

            paramNodes.push_back(param);

            while (remainingTokens->front().getType() != "func_declaration") {
                Token curr = remainingTokens->front();
                remainingTokens->pop_front();

                paramNodes.push_back(parseIdentifierWithType(curr, remainingTokens->front()));
            }

            funcNode->setParameters(paramNodes);

            remainingTokens->pop_front(); // Pops the ->

            // If the user inputted a curly brace, then we want to continue reading this function definition until we hit another curly brace.
            // Otherwise, we know the function will end at the next expression
            bool hasMultipleExpressions = remainingTokens->front().getType() == "brace_open";
            
            if (hasMultipleExpressions) {
                remainingTokens->pop_front(); // Pops the {

                while (remainingTokens->front().getType() != "brace_close") {
                    definitionNodes.push_back(consume());
                }

                remainingTokens->pop_front(); // Pops the }
            } else {
                definitionNodes.push_back(consume());
            }

            funcNode->setDefinition(definitionNodes);

            return funcNode;
        }

        shared_ptr<ASTNode> parseIdentifierWithType(Token currentToken, Token nextToken) {
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

            shared_ptr<IdentifierNode> identNode = dynamic_pointer_cast<IdentifierNode>(parseIdentifier(currentToken));
            identNode->setValue(parseNestedTypeDeclaration(typeDeclarationTokens));

            return identNode;
        }

        shared_ptr<ASTNode> parseNestedTypeDeclaration(deque<Token> &typeTokens) {
            typeTokens.pop_front(); // Pops the <

            shared_ptr<ASTNode> node = make_shared<TypeDeclarationNode>(typeTokens.front().getText());

            typeTokens.pop_front(); // Pops the type identifier

            if (typeTokens.front().getType() == "angle_bracket_open") {
                shared_ptr<ASTNode> typeChild = parseNestedTypeDeclaration(typeTokens);

                node->setValue(typeChild);
            }

            return node;
        }

        shared_ptr<ASTNode> parseListDefinition(Token currentToken, Token nextToken) {
            shared_ptr<ListDefinitionNode> listNode = make_shared<ListDefinitionNode>();

            vector<shared_ptr<ASTNode>> listElementNodes;

            while (remainingTokens->front().getType() != "bracket_close") {
                listElementNodes.push_back(consume());

                // Commas should be skipped
                if (remainingTokens->front().getType() == "comma") {
                    remainingTokens->pop_front();
                }
            }

            listNode->setElements(listElementNodes);

            remainingTokens->pop_front();  // Pops the ]

            return listNode;
        }

        shared_ptr<ASTNode> parseKeyedAccess(shared_ptr<ASTNode> left) {
            remainingTokens->pop_front(); // Pop the [

            shared_ptr<ASTNode> keyedAccessNode = make_shared<KeyedAccessNode>();
            keyedAccessNode->setLeft(left);
            keyedAccessNode->setRight(consume());

            remainingTokens->pop_front(); // Pop the ]

            return keyedAccessNode;
        }

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
            try {
                validateIdentifier(currentToken);

                return make_shared<IdentifierNode>(currentToken.getText());
            } catch (SyntaxError &e) {
                ExceptionFormatter::displayFormattedError("SyntaxError", e, source, fileName, currentToken);

                exit(0);
            }
        }

        shared_ptr<ASTNode> parseLiteral(Token currentToken) {
            return make_shared<LiteralNode>(currentToken.getType(), currentToken.getText());
        }

        shared_ptr<ASTNode> parseLink() {
            Token currentToken = remainingTokens->front();
            remainingTokens->pop_front();

            Token nextToken = remainingTokens->front();
            remainingTokens->pop_front();

            shared_ptr<LinkNode> linkNode = make_shared<LinkNode>(nextToken.getText());

            auto fileContainingLinkedCapsule = filesByCapsule->find(nextToken.getText());

            if (fileContainingLinkedCapsule == filesByCapsule->end()) {
                cout << "ParseError: Could not find capsule " + nextToken.getText() + " referenced in file " + fileName << endl;
                exit(0); 
            }

            shared_ptr<ASTNode> linkedAST = ThetaCompiler::getInstance().buildAST(fileContainingLinkedCapsule->second);

            linkNode->setValue(linkedAST);

            return linkNode;
        }
};