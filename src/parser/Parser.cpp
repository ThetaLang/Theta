#include <vector>
#include <deque>
#include <string>
#include <map>
#include <memory>
#include "../lexer/Token.hpp"
#include "exceptions/CompilationError.hpp"
#include "exceptions/ParseError.hpp"
#include "ast/AssignmentNode.hpp"
#include "ast/ControlFlowNode.hpp"
#include "ast/EnumNode.hpp"
#include "ast/FunctionInvocationNode.hpp"
#include "ast/ReturnNode.hpp"
#include "ast/StructDeclarationNode.hpp"
#include "ast/StructDefinitionNode.hpp"
#include "ast/UnaryOperationNode.hpp"
#include "ast/BinaryOperationNode.hpp"
#include "ast/LiteralNode.hpp"
#include "ast/IdentifierNode.hpp"
#include "ast/ASTNode.hpp"
#include "ast/TypeDeclarationNode.hpp"
#include "ast/ListNode.hpp"
#include "ast/CapsuleNode.hpp"
#include "ast/FunctionDeclarationNode.hpp"
#include "ast/SourceNode.hpp"
#include "ast/LinkNode.hpp"
#include "ast/SymbolNode.hpp"
#include "ast/DictionaryNode.hpp"
#include "ast/BlockNode.hpp"
#include "ast/TupleNode.hpp"
#include "ast/ASTNodeList.hpp"
#include "../compiler/Compiler.hpp"
#include "../lexer/Lexemes.hpp"
#include "../compiler/DataTypes.hpp"

using namespace std;

namespace Theta {
    class Parser {
        public:
            shared_ptr<ASTNode> parse(deque<Token> &tokens, string &src, string file, shared_ptr<map<string, string>> filesByCapsuleName) {
                source = src;
                fileName = file;
                remainingTokens = &tokens;
                filesByCapsule = filesByCapsuleName;

                shared_ptr<ASTNode> parsedSource = parseSource();

                // Throw parse errors for any remaining tokens after we've finished our parser run
                for (int i = 0; i < tokens.size(); i++) {
                    Theta::Compiler::getInstance().addException(
                        make_shared<Theta::CompilationError>(
                            "ParseError",
                            "Unparsed token " + tokens[i].getLexeme(),
                            tokens[i],
                            source,
                            fileName
                        )
                    );
                }

                return parsedSource;
            }

        private:
            string source;
            string fileName;
            deque<Token> *remainingTokens;

            shared_ptr<map<string, string>> filesByCapsule;
            Token currentToken;

            shared_ptr<ASTNode> parseSource() {
                vector<shared_ptr<ASTNode>> links;

                while (match(Token::Types::KEYWORD, Lexemes::LINK)) {
                    links.push_back(parseLink());
                }

                shared_ptr<SourceNode> sourceNode = make_shared<SourceNode>();
                sourceNode->setLinks(links);
                sourceNode->setValue(parseCapsule());

                return sourceNode;
            }

            shared_ptr<ASTNode> parseLink() {
                match(Token::Types::IDENTIFIER);
                shared_ptr<LinkNode> linkNode = Theta::Compiler::getInstance().getIfExistsParsedLinkAST(currentToken.getLexeme());

                if (linkNode) return linkNode;

                linkNode = make_shared<LinkNode>(currentToken.getLexeme());

                auto fileContainingLinkedCapsule = filesByCapsule->find(currentToken.getLexeme());

                if (fileContainingLinkedCapsule == filesByCapsule->end()) {
                    Theta::Compiler::getInstance().addException(
                        make_shared<Theta::CompilationError>(
                            "LinkageError",
                            "Could not find capsule " + currentToken.getLexeme() + " referenced",
                            currentToken,
                            source,
                            fileName
                        )
                    );
                } else {
                    shared_ptr<ASTNode> linkedAST = Theta::Compiler::getInstance().buildAST(fileContainingLinkedCapsule->second);

                    linkNode->setValue(linkedAST);
                }

                Theta::Compiler::getInstance().addParsedLinkAST(currentToken.getLexeme(), linkNode);

                return linkNode;
            }

            shared_ptr<ASTNode> parseCapsule() {
                if (match(Token::Types::KEYWORD, Lexemes::CAPSULE)) {
                    match(Token::Types::IDENTIFIER);

                    shared_ptr<ASTNode> capsule = make_shared<CapsuleNode>(currentToken.getLexeme());
                    capsule->setValue(parseBlock());

                    return capsule;
                }

                return parseAssignment();
            }

            shared_ptr<ASTNode> parseReturn() {
                if (match(Token::Types::KEYWORD, Lexemes::RETURN)) {
                    shared_ptr<ASTNode> ret = make_shared<ReturnNode>();
                    ret->setValue(parseAssignment());

                    return ret;
                }

                return parseStructDefinition();
            }

            shared_ptr<ASTNode> parseStructDefinition() {
                if (match(Token::Types::KEYWORD, Lexemes::STRUCT)) {
                    match(Token::Types::IDENTIFIER);

                    shared_ptr<StructDefinitionNode> str = make_shared<StructDefinitionNode>(currentToken.getLexeme());

                    if (!match(Token::Types::BRACE_OPEN)) {
                        Theta::Compiler::getInstance().addException(
                            make_shared<Theta::CompilationError>(
                                "SyntaxError",
                                "Expected open brace during struct definition",
                                currentToken,
                                source,
                                fileName
                            )
                        );
                    }

                    vector<shared_ptr<ASTNode>> items;

                    while (!match(Token::Types::BRACE_CLOSE)) {
                        match(Token::Types::IDENTIFIER);
                        shared_ptr<ASTNode> el = parseIdentifier();

                        if (el == nullptr) break;

                        items.push_back(el);
                    }

                    str->setElements(items);

                    return str;
                }

                return parseAssignment();
            }

            shared_ptr<ASTNode> parseAssignment() {
                shared_ptr<ASTNode> expr = parseExpression();

                if (match(Token::Types::ASSIGNMENT)) {
                    shared_ptr<ASTNode> left = expr;

                    expr = make_shared<AssignmentNode>();
                    expr->setLeft(left);
                    expr->setRight(parseFunctionDeclaration());
                }

                return expr;
            }

            shared_ptr<ASTNode> parseBlock() {
                if (match(Token::Types::BRACE_OPEN)) {
                    vector<shared_ptr<ASTNode>> blockExpr;

                    while (!match(Token::Types::BRACE_CLOSE)) {
                        shared_ptr<ASTNode> expr = parseReturn();

                        if (expr == nullptr) break;

                        blockExpr.push_back(expr);
                    }

                    shared_ptr<BlockNode> block = make_shared<BlockNode>();
                    block->setElements(blockExpr);

                    return block;
                }

                return parseFunctionDeclaration();
            }

            shared_ptr<ASTNode> parseFunctionDeclaration() {
                shared_ptr<ASTNode> expr = parseAssignment();

                if (match(Token::Types::FUNC_DECLARATION)) {
                    shared_ptr<FunctionDeclarationNode> func_def = make_shared<FunctionDeclarationNode>();

                    if (expr && expr->getNodeType() != ASTNode::Types::AST_NODE_LIST) {
                        shared_ptr<ASTNodeList> parameters = make_shared<ASTNodeList>();
                        parameters->setElements({ expr });

                        expr = parameters;
                    } else if (!expr) {
                        expr = make_shared<ASTNodeList>();
                    }

                    func_def->setParameters(dynamic_pointer_cast<ASTNodeList>(expr));

                    shared_ptr<ASTNode> definitionBlock = parseBlock();

                    // In the case of shorthand single-line function bodies, we still want to wrap them in a block within the ast
                    // for scoping reasons
                    if (definitionBlock->getNodeType() != ASTNode::BLOCK) {
                        shared_ptr<BlockNode> block = make_shared<BlockNode>();

                        block->setElements({ definitionBlock });

                        definitionBlock = block;
                    }

                    func_def->setDefinition(definitionBlock);

                    expr = func_def;
                }

                return expr;
            }

            shared_ptr<ASTNode> parseExpression() {
                return parseStructDeclaration();
            }

            shared_ptr<ASTNode> parseStructDeclaration() {
                if (match(Token::Types::AT)) {
                    match(Token::Types::IDENTIFIER);

                    shared_ptr<StructDeclarationNode> str = make_shared<StructDeclarationNode>(currentToken.getLexeme());

                    str->setValue(parseDict());

                    return str;
                }

                return parseEnum();
            }

            shared_ptr<ASTNode> parseEnum() {
                if (match(Token::Types::KEYWORD, Lexemes::ENUM)) {
                    match(Token::Types::IDENTIFIER);

                    shared_ptr<EnumNode> root = make_shared<EnumNode>();
                    root->setIdentifier(parseIdentifier());

                    if (!match(Token::Types::BRACE_OPEN)) {
                        Theta::Compiler::getInstance().addException(
                            make_shared<Theta::CompilationError>(
                                "SyntaxError",
                                "Expected opening brace during enum declaration",
                                currentToken,
                                source,
                                fileName
                            )
                        );

                        return root;
                    }

                    vector<shared_ptr<ASTNode>> enumVals;

                    while (!match(Token::Types::BRACE_CLOSE)) {
                        if (!match(Token::Types::COLON)) {
                            Theta::Compiler::getInstance().addException(
                                make_shared<Theta::CompilationError>(
                                    "SyntaxError",
                                    "Enum must only contain symbols",
                                    remainingTokens->front(),
                                    source,
                                    fileName
                                )
                            );

                            remainingTokens->pop_front();

                            continue;
                        }

                        shared_ptr<ASTNode> node = parseSymbol();

                        if (!node) break;

                        enumVals.push_back(node);
                    }

                    root->setElements(enumVals);

                    return root;
                }

                return parseControlFlow();
            }

            shared_ptr<ASTNode> parseControlFlow() {
                if (match(Token::Types::KEYWORD, Lexemes::IF)) {
                    shared_ptr<ControlFlowNode> cfNode = make_shared<ControlFlowNode>();
                    vector<pair<shared_ptr<ASTNode>, shared_ptr<ASTNode>>> conditionExpressionPairs = {
                        make_pair(parseExpression(), parseBlock())
                    };

                    while (match(Token::Types::KEYWORD, Lexemes::ELSE) && match(Token::Types::KEYWORD, Lexemes::IF)) {
                        conditionExpressionPairs.push_back(make_pair(parseExpression(), parseBlock()));
                    }

                    // If we just matched an else but no if afterwards. This way it only matches one else block per control flow
                    if (currentToken.getType() == Token::Types::KEYWORD && currentToken.getLexeme() == Lexemes::ELSE) {
                        conditionExpressionPairs.push_back(make_pair(nullptr, parseBlock()));
                    }

                    cfNode->setConditionExpressionPairs(conditionExpressionPairs);

                    return cfNode;
                }

                return parsePipeline();
            }

            shared_ptr<ASTNode> parsePipeline() {
                shared_ptr<ASTNode> expr = parseBooleanComparison();

                while (match(Token::Types::OPERATOR, Lexemes::PIPE)) {
                    shared_ptr<ASTNode> left = expr;

                    expr = make_shared<BinaryOperationNode>(currentToken.getLexeme());
                    expr->setLeft(left);
                    expr->setRight(parseBooleanComparison());
                }

                return expr;
            }

            shared_ptr<ASTNode> parseBooleanComparison() {
                shared_ptr<ASTNode> expr = parseEquality();

                while (match(Token::Types::OPERATOR, Lexemes::OR) || match(Token::Types::OPERATOR, Lexemes::AND)) {
                    shared_ptr<ASTNode> left = expr;

                    expr = make_shared<BinaryOperationNode>(currentToken.getLexeme());
                    expr->setLeft(left);
                    expr->setRight(parseExpression());
                }

                return expr;
            }

            shared_ptr<ASTNode> parseEquality() {
                shared_ptr<ASTNode> expr = parseComparison();

                while (match(Token::Types::OPERATOR, Lexemes::EQUALITY) || match(Token::Types::OPERATOR, Lexemes::INEQUALITY)) {
                    shared_ptr<ASTNode> left = expr;

                    expr = make_shared<BinaryOperationNode>(currentToken.getLexeme());
                    expr->setLeft(left);
                    expr->setRight(parseComparison());
                }

                return expr;
            }

            shared_ptr<ASTNode> parseComparison() {
                shared_ptr<ASTNode> expr = parseTerm();

                while (
                    match(Token::Types::OPERATOR, Lexemes::GT) ||
                    match(Token::Types::OPERATOR, Lexemes::GTEQ) ||
                    match(Token::Types::OPERATOR, Lexemes::LT) ||
                    match(Token::Types::OPERATOR, Lexemes::LTEQ)
                ) {
                    shared_ptr<ASTNode> left = expr;

                    expr = make_shared<BinaryOperationNode>(currentToken.getLexeme());
                    expr->setLeft(left);
                    expr->setRight(parseTerm());
                }

                return expr;
            }

            shared_ptr<ASTNode> parseTerm() {
                shared_ptr<ASTNode> expr = parseFactor();

                while (match(Token::Types::OPERATOR, Lexemes::MINUS) || match(Token::Types::OPERATOR, Lexemes::PLUS)) {
                    shared_ptr<ASTNode> left = expr;

                    expr = make_shared<BinaryOperationNode>(currentToken.getLexeme());
                    expr->setLeft(left);
                    expr->setRight(parseFactor());
                }

                return expr;
            }

            shared_ptr<ASTNode> parseFactor() {
                shared_ptr<ASTNode> expr = parseExponent();

                while (
                    match(Token::Types::OPERATOR, Lexemes::DIVISION) ||
                    match(Token::Types::OPERATOR, Lexemes::TIMES) ||
                    match(Token::Types::OPERATOR, Lexemes::MODULO)
                ) {
                    shared_ptr<ASTNode> left = expr;

                    expr = make_shared<BinaryOperationNode>(currentToken.getLexeme());
                    expr->setLeft(left);
                    expr->setRight(parseExponent());
                }

                return expr;
            }

            shared_ptr<ASTNode> parseExponent() {
                shared_ptr<ASTNode> expr = parseUnary();

                while (match(Token::Types::OPERATOR, Lexemes::EXPONENT)) {
                    shared_ptr<ASTNode> left = expr;

                    expr = make_shared<BinaryOperationNode>(currentToken.getLexeme());
                    expr->setLeft(left);
                    expr->setRight(parseUnary());
                }

                return expr;
            }

            shared_ptr<ASTNode> parseUnary() {
                if (match(Token::Types::OPERATOR, Lexemes::NOT) || match(Token::Types::OPERATOR, Lexemes::MINUS)) {
                    shared_ptr<ASTNode> un = make_shared<UnaryOperationNode>(currentToken.getLexeme());
                    un->setValue(parseUnary());

                    return un;
                }

                return parsePrimary();
            }

            shared_ptr<ASTNode> parsePrimary() {
                if (match(Token::Types::BOOLEAN) || match(Token::Types::NUMBER) || match(Token::Types::STRING)) {
                    map<Token::Types, ASTNode::Types> tokenTypeToAstTypeMap = {
                        { Token::Types::NUMBER, ASTNode::Types::NUMBER_LITERAL },
                        { Token::Types::BOOLEAN, ASTNode::Types::BOOLEAN_LITERAL },
                        { Token::Types::STRING, ASTNode::Types::STRING_LITERAL }
                    };

                    auto it = tokenTypeToAstTypeMap.find(currentToken.getType());

                    string value = currentToken.getLexeme();

                    // Pulls the string out of quotation marks
                    if (currentToken.getType() == Token::Types::STRING) {
                        value = value.substr(1, value.length() - 2);
                    }

                    return make_shared<LiteralNode>(it->second, value);
                }

                if (match(Token::Types::IDENTIFIER)) {
                    return parseFunctionInvocation();
                }

                if (match(Token::Types::COLON)) {
                    return parseSymbol();
                }

                if (match(Token::Types::BRACKET_OPEN)) {
                    return parseList();
                }

                if (match(Token::Types::BRACE_OPEN)) {
                    return parseDict();
                }

                if (match(Token::Types::PAREN_OPEN)) {
                    return parseExpressionList();
                }

                return nullptr;
            }

            shared_ptr<ASTNode> parseExpressionList(bool forceList = false) {
                shared_ptr<ASTNode> expr = parseFunctionDeclaration();

                if (check(Token::Types::COMMA) || !expr || forceList) {
                    shared_ptr<ASTNodeList> nodeList = make_shared<ASTNodeList>();
                    vector<shared_ptr<ASTNode>> expressions;

                    if (expr) expressions.push_back(expr);

                    while (match(Token::Types::COMMA)) {
                        expressions.push_back(parseFunctionDeclaration());
                    }

                    nodeList->setElements(expressions);

                    expr = nodeList;
                }

                match(Token::Types::PAREN_CLOSE);

                return expr;
            }

            shared_ptr<ASTNode> parseDict() {
                pair<string, shared_ptr<ASTNode>> p = parseKvPair();
                shared_ptr<ASTNode> expr = p.second;

                if (p.first == "kv" && expr && expr->getNodeType() == ASTNode::Types::TUPLE) {
                   vector<shared_ptr<ASTNode>> el;
                    el.push_back(expr);

                    while (match(Token::Types::COMMA)) {
                        el.push_back(parseKvPair().second);
                    }

                    expr = make_shared<DictionaryNode>();
                    dynamic_pointer_cast<DictionaryNode>(expr)->setElements(el);

                    match(Token::Types::BRACE_CLOSE);
                }

                return expr;
            }

            pair<string, shared_ptr<ASTNode>> parseKvPair() {
                // Because both flows of this function return a tuple, we need a type flag to indicate whether
                // we generated the tuple with the intention of it being a kvPair or not. Otherwise it would
                // be ambiguous and we would accidentally convert dicts with a single key-value pair into a tuple
                string type = "tuple";
                shared_ptr<ASTNode> expr = parseTuple();

                if (match(Token::Types::COLON)) {
                    type = "kv";
                    shared_ptr<ASTNode> left = expr;

                    if (left->getNodeType() == ASTNode::Types::IDENTIFIER) {
                        left = make_shared<SymbolNode>(dynamic_pointer_cast<IdentifierNode>(left)->getIdentifier());
                    }

                    expr = make_shared<TupleNode>();
                    expr->setLeft(left);
                    expr->setRight(parseExpression());
                }

                return make_pair(type, expr);
            }

            shared_ptr<ASTNode> parseTuple() {
                shared_ptr<ASTNode> expr;

                try {
                    expr = parseExpression();
                } catch (ParseError e) {
                    if (e.getErrorParseType() == "symbol") remainingTokens->pop_front();
                }

                if (match(Token::Types::COMMA)) {
                    shared_ptr<ASTNode> first = expr;

                    expr = make_shared<TupleNode>();
                    expr->setLeft(first);

                    try {
                        expr->setRight(parseExpression());
                    } catch (ParseError e) {
                        if (e.getErrorParseType() == "symbol") remainingTokens->pop_front();
                    }


                    if (!match(Token::Types::BRACE_CLOSE)) {
                        Theta::Compiler::getInstance().addException(
                            make_shared<Theta::CompilationError>(
                                "SyntaxError",
                                "Expected closing brace after tuple definition",
                                remainingTokens->front(),
                                source,
                                fileName
                            )
                        );
                    }
                }

                return expr;
            }

            shared_ptr<ASTNode> parseList() {
                shared_ptr<ListNode> listNode = make_shared<ListNode>();
                vector<shared_ptr<ASTNode>> el;

                if (!match(Token::Types::BRACKET_CLOSE)) {
                    el.push_back(parseExpression());

                    while(match(Token::Types::COMMA)) {
                        el.push_back(parseExpression());
                    }

                    listNode->setElements(el);

                    match(Token::Types::BRACKET_CLOSE);
                }

                return listNode;
            }

            shared_ptr<ASTNode> parseFunctionInvocation() {
                shared_ptr<ASTNode> expr = parseIdentifier();

                if (match(Token::Types::PAREN_OPEN)) {
                    shared_ptr<FunctionInvocationNode> funcInvNode = make_shared<FunctionInvocationNode>();
                    funcInvNode->setIdentifier(expr);
                    funcInvNode->setParameters(dynamic_pointer_cast<ASTNodeList>(parseExpressionList(true)));

                    expr = funcInvNode;
                }

                return expr;
            }

            shared_ptr<ASTNode> parseIdentifier() {
                validateIdentifier(currentToken);

                shared_ptr<ASTNode> ident = make_shared<IdentifierNode>(currentToken.getLexeme());

                if (match(Token::Types::OPERATOR, Lexemes::LT)) {
                    ident->setValue(parseType());

                    match(Token::Types::OPERATOR, Lexemes::GT);
                }

                return ident;
            }

            shared_ptr<ASTNode> parseType() {
                match(Token::Types::IDENTIFIER);

                string typeName = currentToken.getLexeme();
                shared_ptr<ASTNode> typ = make_shared<TypeDeclarationNode>(typeName);

                if (match(Token::Types::OPERATOR, Lexemes::LT)) {
                    shared_ptr<ASTNode> l = parseType();

                    if (typeName == DataTypes::VARIADIC) {
                        shared_ptr<TypeDeclarationNode> variadic = dynamic_pointer_cast<TypeDeclarationNode>(typ);
                        vector<shared_ptr<ASTNode>> types;
                        types.push_back(l);

                        while (match(Token::Types::COMMA)) {
                            types.push_back(parseType());
                        }

                        variadic->setElements(types);
                    } else if (match(Token::Types::COMMA)) {
                        typ->setLeft(l);
                        typ->setRight(parseType());
                    } else {
                        typ->setValue(l);
                    }

                    match(Token::Types::OPERATOR, Lexemes::GT);
                }

                return typ;
            }

            shared_ptr<ASTNode> parseSymbol() {
                if (match(Token::Types::IDENTIFIER) || match(Token::Types::NUMBER)) {
                    if (currentToken.getType() == Token::Types::IDENTIFIER) validateIdentifier(currentToken);

                    return make_shared<SymbolNode>(currentToken.getLexeme());
                }

                Theta::Compiler::getInstance().addException(
                    make_shared<Theta::CompilationError>(
                        "SyntaxError",
                        "Expected identifier as part of symbol declaration",
                        remainingTokens->front(),
                        source,
                        fileName
                    )
                );

                throw ParseError("symbol");

                return nullptr;
            }

            bool match(Token::Types type, string lexeme = "") {
                if (check(type, lexeme)) {
                    currentToken = remainingTokens->front();
                    remainingTokens->pop_front();
                    return true;
                }

                return false;
            }

            bool check(Token::Types type, string lexeme = "") {
                return remainingTokens->size() != 0 &&
                    remainingTokens->front().getType() == type &&
                    (lexeme != "" ? remainingTokens->front().getLexeme() == lexeme : true);
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
                string disallowedIdentifierChars = "!@#$%^&*()-=+/<>{}[]|?,`~";

                for (int i = 0; i < token.getLexeme().length(); i++) {
                    char identChar = tolower(token.getLexeme()[i]);

                    bool isDisallowedChar = find(disallowedIdentifierChars.begin(), disallowedIdentifierChars.end(), identChar) != disallowedIdentifierChars.end();
                    bool isStartsWithDigit = i == 0 && isdigit(identChar);

                    if (isStartsWithDigit || isDisallowedChar) {
                        Theta::Compiler::getInstance().addException(
                            make_shared<Theta::CompilationError>(
                                "SyntaxError",
                                "Invalid identifier \"" + token.getLexeme() + "\"",
                                token,
                                source,
                                fileName
                            )
                        );
                    }
                }
            }
    };
}
