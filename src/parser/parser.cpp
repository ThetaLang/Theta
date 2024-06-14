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
#include "../util/tokens.hpp"

using namespace std;

/**
 * @brief The ThetaParser class implements a recursive descent parser that processes a sequence
 * of tokens to construct an Abstract Syntax Tree (AST). It follows a top-down parsing
 * strategy where it expands the leftmost nonterminal nodes first, leveraging lookahead
 * to make parsing decisions based on the current and subsequent tokens.
 * 
 * The parsing process involves consuming tokens from a deque and recursively constructing
 * nodes of various types (such as identifiers, literals, functions, etc.) based on the
 * grammar rules of the Theta language.
 */
class ThetaParser {
    public:
        /**
         * @brief Parses the sequence of tokens to construct the Abstract Syntax Tree (AST).
         * 
         * This method initiates the parsing process, utilizing a recursive descent approach
         * to build the AST by consuming tokens from the deque in a leftmost derivation order.
         * It begins by parsing out linked capsules (imports), compiling them into AST nodes,
         * and then proceeds to compile the current list of tokens into the main AST structure.
         * 
         * @param tokens A deque of tokens to be parsed.
         * @param src The source code as a string.
         * @param file The name of the current file being parsed.
         * @param filesByCapsuleName Shared pointer to a map storing capsule names mapped to file paths.
         * @return A shared pointer to the root node of the constructed AST.
         */
        shared_ptr<ASTNode> parse(deque<Token> &tokens, string &src, string file, shared_ptr<map<string, string>> filesByCapsuleName) {
            source = src;
            fileName = file;
            remainingTokens = &tokens;
            filesByCapsule = filesByCapsuleName;

            shared_ptr<SourceNode> rootASTNode = make_shared<SourceNode>();
            vector<shared_ptr<ASTNode>> linkNodes;

            // Parse out file imports
            while (remainingTokens->front().getType() == Tokens::KEYWORD && remainingTokens->front().getText() != Symbols::CAPSULE) {
                remainingTokens->pop_front();
                string linkCapsuleName = remainingTokens->front().getText();
                shared_ptr<LinkNode> parsedLinkAST;

                auto it = parsedLinkASTs.find(linkCapsuleName);

                if (it != parsedLinkASTs.end()) {
                    cout << "FOUND PREPARED AST, SKIPPING PARSING OF LINKED CAPSULE: " + linkCapsuleName << endl;

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

        /**
         * @brief Consumes the next token from the remaining tokens deque.
         * 
         * This method retrieves and removes the front token from the deque of remaining tokens,
         * determines its type, and initiates parsing of different node types based on the token's
         * type and the context of the following tokens.
         * 
         * @return A shared pointer to the ASTNode representing the parsed construct.
         */
        shared_ptr<ASTNode> consume() {
            if (remainingTokens->size() <= 0) return nullptr;

            Token currentToken = remainingTokens->front();
            remainingTokens->pop_front();

            Token nextToken = remainingTokens->front();

            if (currentToken.getType() == Tokens::KEYWORD && currentToken.getText() == Symbols::CAPSULE) {
                return parseCapsule(nextToken);
            } else if (
                currentToken.getType() == Tokens::IDENTIFIER && 
                nextToken.getType() == Tokens::ANGLE_BRACKET_OPEN && 
                remainingTokens->at(1).getType() == Tokens::IDENTIFIER &&
                (remainingTokens->at(2).getType() == Tokens::ANGLE_BRACKET_CLOSE || remainingTokens->at(2).getType() == Tokens::ANGLE_BRACKET_OPEN)
            ) {
                shared_ptr<ASTNode> identNode = parseIdentifierWithType(currentToken, nextToken);

                if (remainingTokens->front().getType() == Tokens::ASSIGNMENT) {
                    return parseAssignment(identNode);
                } else if (remainingTokens->front().getType() == Tokens::COMMA || remainingTokens->front().getType() == Tokens::FUNC_DECLARATION) {
                    return parseFuncDeclaration(identNode);
                } else {
                    return identNode;
                }
            } else if (
                nextToken.getType() == Tokens::OPERATOR && (
                    currentToken.getType() == Tokens::IDENTIFIER ||
                    currentToken.getType() == Tokens::STRING ||
                    currentToken.getType() == Tokens::NUMBER
                )
            ) {
                return parseBinaryOperation(currentToken, nextToken);
            } else if (currentToken.getType() == Tokens::IDENTIFIER && nextToken.getType() == Tokens::BRACKET_OPEN) {
                return parseKeyedAccess(parseIdentifier(currentToken));
            } else if (currentToken.getType() == Tokens::BRACKET_OPEN) {
                shared_ptr<ASTNode> listDefinition = parseListDefinition(currentToken, nextToken);
                
                if (remainingTokens->front().getType() == Tokens::BRACKET_OPEN) {
                    return parseKeyedAccess(listDefinition);
                } else {
                    return listDefinition;
                }
            } else if (
                currentToken.getType() == Tokens::STRING ||
                currentToken.getType() == Tokens::NUMBER ||
                currentToken.getType() == Tokens::BOOLEAN
            ) {
                return parseLiteral(currentToken);
            } else if (currentToken.getType() == Tokens::IDENTIFIER) {
                return parseIdentifier(currentToken);
            }

            return nullptr;
        }

        /**
         * @brief Parses a capsule definition based on the next token in the deque.
         * 
         * This method handles the parsing of a capsule definition node, including its name
         * and nested definitions, by consuming tokens until the closing brace '}' is encountered.
         * 
         * @param nextToken The token following the capsule keyword.
         * @return A shared pointer to the CapsuleNode representing the parsed capsule definition.
         */
        shared_ptr<ASTNode> parseCapsule(Token nextToken) {
            remainingTokens->pop_front();
            remainingTokens->pop_front(); // Pops the {

            shared_ptr<CapsuleNode> capsuleNode = make_shared<CapsuleNode>(nextToken.getText());

            vector<shared_ptr<ASTNode>> definitionNodes;

            while (remainingTokens->front().getType() != Tokens::BRACE_CLOSE) {
                definitionNodes.push_back(consume());
            }

            capsuleNode->setDefinitions(definitionNodes);

            return capsuleNode;
        }

        /**
         * @brief Parses an assignment node, handling the assignment operator '=' and its operands.
         * 
         * This method constructs an AssignmentNode by parsing the left-hand side (identifier) and
         * the right-hand side (expression) of the assignment.
         * 
         * @param identifier The identifier node on the left-hand side of the assignment.
         * @return A shared pointer to the AssignmentNode representing the parsed assignment.
         */
        shared_ptr<ASTNode> parseAssignment(shared_ptr<ASTNode> identifier) {
            shared_ptr<ASTNode> assignmentNode = make_shared<AssignmentNode>();
            assignmentNode->setLeft(identifier);

            // Pop the =
            remainingTokens->pop_front();

            assignmentNode->setRight(consume());

            return assignmentNode;
        }

        /**
         * @brief Parses a function declaration node, including its parameters and definition.
         * 
         * This method handles the parsing of a function declaration, including its parameters,
         * arrow '->' separator, and the function body (either a single expression or a block of
         * multiple expressions enclosed in braces '{ }').
         * 
         * @param param The parameter node representing the function's input parameter.
         * @return A shared pointer to the FunctionDeclarationNode representing the parsed function declaration.
         */
        shared_ptr<ASTNode> parseFuncDeclaration(shared_ptr<ASTNode> param) {
            shared_ptr<FunctionDeclarationNode> funcNode = make_shared<FunctionDeclarationNode>();

            vector<shared_ptr<ASTNode>> paramNodes;
            vector<shared_ptr<ASTNode>> definitionNodes;

            paramNodes.push_back(param);

            while (remainingTokens->front().getType() != Tokens::FUNC_DECLARATION) {
                Token curr = remainingTokens->front();
                remainingTokens->pop_front();

                paramNodes.push_back(parseIdentifierWithType(curr, remainingTokens->front()));
            }

            funcNode->setParameters(paramNodes);

            remainingTokens->pop_front(); // Pops the ->

            // If the user inputted a curly brace, then we want to continue reading this function definition until we hit another curly brace.
            // Otherwise, we know the function will end at the next expression
            bool hasMultipleExpressions = remainingTokens->front().getType() == Tokens::BRACE_OPEN;
            
            if (hasMultipleExpressions) {
                remainingTokens->pop_front(); // Pops the {

                while (remainingTokens->front().getType() != Tokens::BRACE_CLOSE) {
                    definitionNodes.push_back(consume());
                }

                remainingTokens->pop_front(); // Pops the }
            } else {
                definitionNodes.push_back(consume());
            }

            funcNode->setDefinition(definitionNodes);

            return funcNode;
        }

        /**
         * @brief Parses an identifier with an associated type declaration.
         * 
         * This method handles the parsing of an identifier followed by a type declaration,
         * which is enclosed in angle brackets '< >'. It constructs an IdentifierNode with
         * an additional TypeDeclarationNode representing the specified type.
         * 
         * @param currentToken The token representing the identifier.
         * @param nextToken The token following the identifier, expected to be '<' indicating type declaration.
         * @return A shared pointer to the IdentifierNode with associated type declaration.
         */
        shared_ptr<ASTNode> parseIdentifierWithType(Token currentToken, Token nextToken) {
            deque<Token> typeDeclarationTokens;
            typeDeclarationTokens.push_back(nextToken);
            remainingTokens->pop_front(); // Pops the <

            int typeDeclarationDepth = 1;
            while (typeDeclarationDepth > 0) {
                if (remainingTokens->front().getType() == Tokens::ANGLE_BRACKET_OPEN) {
                    typeDeclarationDepth++;
                } else if (remainingTokens->front().getType() == Tokens::ANGLE_BRACKET_CLOSE){
                    typeDeclarationDepth--;
                }

                typeDeclarationTokens.push_back(remainingTokens->front());
                remainingTokens->pop_front();
            }

            shared_ptr<IdentifierNode> identNode = dynamic_pointer_cast<IdentifierNode>(parseIdentifier(currentToken));
            identNode->setValue(parseNestedTypeDeclaration(typeDeclarationTokens));

            return identNode;
        }
        
        // TODO: This can probably be done recursively instead of requiring the
        // building of a separate deque first
        /**
         * @brief Parses a nested type declaration within angle brackets '< >'.
         * 
         * This method recursively parses nested type declarations within angle brackets,
         * constructing a TypeDeclarationNode hierarchy based on the tokens provided.
         * 
         * @param typeTokens A deque of tokens representing the type declaration.
         * @return A shared pointer to the root of the TypeDeclarationNode hierarchy.
         */
        shared_ptr<ASTNode> parseNestedTypeDeclaration(deque<Token> &typeTokens) {
            typeTokens.pop_front(); // Pops the <

            shared_ptr<ASTNode> node = make_shared<TypeDeclarationNode>(typeTokens.front().getText());

            typeTokens.pop_front(); // Pops the type identifier

            if (typeTokens.front().getType() == Tokens::ANGLE_BRACKET_OPEN) {
                shared_ptr<ASTNode> typeChild = parseNestedTypeDeclaration(typeTokens);

                node->setValue(typeChild);
            }

            return node;
        }

        /**
         * @brief Parses a list definition node, handling the elements enclosed in square brackets '[ ]'.
         * 
         * This method constructs a ListDefinitionNode by parsing the elements inside square brackets,
         * separated by commas ','.
         * 
         * @param currentToken The token representing the opening '[' of the list.
         * @param nextToken The token following the opening '['.
         * @return A shared pointer to the ListDefinitionNode representing the parsed list definition.
         */
        shared_ptr<ASTNode> parseListDefinition(Token currentToken, Token nextToken) {
            shared_ptr<ListDefinitionNode> listNode = make_shared<ListDefinitionNode>();

            vector<shared_ptr<ASTNode>> listElementNodes;

            while (remainingTokens->front().getType() != Tokens::BRACKET_CLOSE) {
                listElementNodes.push_back(consume());

                // Commas should be skipped
                if (remainingTokens->front().getType() == Tokens::COMMA) {
                    remainingTokens->pop_front();
                }
            }

            listNode->setElements(listElementNodes);

            remainingTokens->pop_front();  // Pops the ]

            return listNode;
        }

        /**
         * @brief Parses a keyed access node, handling the indexing operation using square brackets '[ ]'.
         * 
         * This method constructs a KeyedAccessNode by parsing the expression on the left-hand side
         * and the index expression inside square brackets on the right-hand side.
         * 
         * @param left The left-hand side expression node before '['.
         * @return A shared pointer to the KeyedAccessNode representing the parsed keyed access operation.
         */
        shared_ptr<ASTNode> parseKeyedAccess(shared_ptr<ASTNode> left) {
            remainingTokens->pop_front(); // Pop the [

            shared_ptr<ASTNode> keyedAccessNode = make_shared<KeyedAccessNode>();
            keyedAccessNode->setLeft(left);
            keyedAccessNode->setRight(consume());

            remainingTokens->pop_front(); // Pop the ]

            return keyedAccessNode;
        }

        /**
         * @brief Parses a binary operation node, handling binary operators between two expressions.
         * 
         * This method constructs a BinaryOperationNode by parsing the left-hand side and the operator
         * with the right-hand side expression.
         * 
         * @param currentToken The token representing the left-hand side expression.
         * @param nextToken The token representing the binary operator.
         * @return A shared pointer to the BinaryOperationNode representing the parsed binary operation.
         */
        shared_ptr<ASTNode> parseBinaryOperation(Token currentToken, Token nextToken) {
            remainingTokens->pop_front();

            shared_ptr<ASTNode> node = make_shared<BinaryOperationNode>(nextToken.getText());

            node->setLeft(currentToken.getType() == Tokens::IDENTIFIER
                ? parseIdentifier(currentToken)
                : parseLiteral(currentToken)
            );
            
            node->setRight(consume());

            return node;
        }

        /**
         * @brief Parses an identifier node, validating the identifier's syntax and constructing the node.
         * 
         * This method validates the syntax of the identifier token and constructs an IdentifierNode
         * representing the identifier.
         * 
         * @param currentToken The token representing the identifier.
         * @return A shared pointer to the IdentifierNode representing the parsed identifier.
         */
        shared_ptr<ASTNode> parseIdentifier(Token currentToken) {
            try {
                validateIdentifier(currentToken);

                return make_shared<IdentifierNode>(currentToken.getText());
            } catch (SyntaxError &e) {
                ExceptionFormatter::displayFormattedError("SyntaxError", e, source, fileName, currentToken);

                exit(0);
            }
        }

        /**
         * @brief Parses a literal node, constructing a LiteralNode based on the token's type and value.
         * 
         * This method constructs a LiteralNode representing a literal value (string, number, boolean)
         * based on the token's type and text.
         * 
         * @param currentToken The token representing the literal value.
         * @return A shared pointer to the LiteralNode representing the parsed literal.
         */
        shared_ptr<ASTNode> parseLiteral(Token currentToken) {
            return make_shared<LiteralNode>(currentToken.getType(), currentToken.getText());
        }

        /**
         * @brief Parses a linked capsule node, resolving the linked capsule's AST and constructing a LinkNode.
         * 
         * This method handles parsing of a linked capsule reference, resolving the AST of the linked
         * capsule by calling ThetaCompiler's buildAST method and constructing a LinkNode with the linked AST.
         * 
         * @return A shared pointer to the LinkNode representing the parsed linked capsule.
         */
        shared_ptr<ASTNode> parseLink() {
            Token currentToken = remainingTokens->front();
            remainingTokens->pop_front();

            shared_ptr<LinkNode> linkNode = make_shared<LinkNode>(currentToken.getText());

            auto fileContainingLinkedCapsule = filesByCapsule->find(currentToken.getText());

            if (fileContainingLinkedCapsule == filesByCapsule->end()) {
                LinkageError e = LinkageError(
                    "Could not find capsule " + currentToken.getText() + " referenced",
                    currentToken.getStartLocation()
                );

                ExceptionFormatter::displayFormattedError("LinkageError", e, source, fileName, currentToken);
                
                // TODO: Make an errorCount var that gets incremented each time we hit an error, and then exit after parsing,
                // instead of exiting right away. This will let us show more useful errors to the user at a time
                // exit(0); 
            } else {
                shared_ptr<ASTNode> linkedAST = ThetaCompiler::getInstance().buildAST(fileContainingLinkedCapsule->second);

                linkNode->setValue(linkedAST);
            }

            return linkNode;
        }

        /**
         * @brief Validates an identifier token to ensure it conforms to language syntax rules.
         * 
         * This method checks each character in the identifier's text to ensure it does not
         * contain disallowed characters or start with a digit. If an invalid character or 
         * format is detected, a SyntaxError exception is thrown with details about the error.
         * 
         * @param token The token representing the identifier to be validated.
         * @throws SyntaxError If the identifier contains disallowed characters or starts with a digit.
         */
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
};