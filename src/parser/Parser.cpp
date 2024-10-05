#include <vector>
#include <algorithm>
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
      shared_ptr<SourceNode> sourceNode = make_shared<SourceNode>();

      while (match(Token::KEYWORD, Lexemes::LINK)) {
        links.push_back(parseLink(sourceNode));
      }

      sourceNode->setLinks(links);
      sourceNode->setValue(parseCapsule(sourceNode));

      return sourceNode;
    }

    shared_ptr<ASTNode> parseLink(shared_ptr<ASTNode> parent) {
      match(Token::IDENTIFIER);
      shared_ptr<LinkNode> linkNode = Theta::Compiler::getInstance().getIfExistsParsedLinkAST(currentToken.getLexeme());

      if (linkNode) return linkNode;

      linkNode = make_shared<LinkNode>(currentToken.getLexeme(), parent);

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

    shared_ptr<ASTNode> parseCapsule(shared_ptr<ASTNode> parent) {
      if (match(Token::KEYWORD, Lexemes::CAPSULE)) {
        match(Token::IDENTIFIER);

        shared_ptr<ASTNode> capsule = make_shared<CapsuleNode>(currentToken.getLexeme(), parent);
        capsule->setValue(parseBlock(capsule));

        return capsule;
      }

      return parseAssignment(parent);
    }

    shared_ptr<ASTNode> parseReturn(shared_ptr<ASTNode> parent) {
      if (match(Token::KEYWORD, Lexemes::RETURN)) {
        shared_ptr<ASTNode> ret = make_shared<ReturnNode>(parent);
        ret->setValue(parseAssignment(ret));

        return ret;
      }

      return parseStructDefinition(parent);
    }

    shared_ptr<ASTNode> parseStructDefinition(shared_ptr<ASTNode> parent) {
      if (match(Token::KEYWORD, Lexemes::STRUCT)) {
        match(Token::IDENTIFIER);

        shared_ptr<StructDefinitionNode> str = make_shared<StructDefinitionNode>(currentToken.getLexeme(), parent);

        if (!match(Token::BRACE_OPEN)) {
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

        while (!match(Token::BRACE_CLOSE)) {
          match(Token::IDENTIFIER);
          shared_ptr<ASTNode> el = parseIdentifier(str);

          if (el == nullptr) break;

          items.push_back(el);
        }

        str->setElements(items);

        return str;
      }

      return parseAssignment(parent);
    }

    shared_ptr<ASTNode> parseAssignment(shared_ptr<ASTNode> parent) {
      shared_ptr<ASTNode> expr = parseExpression(parent);

      if (match(Token::ASSIGNMENT)) {
        shared_ptr<ASTNode> left = expr;

        expr = make_shared<AssignmentNode>(parent);

        left->setParent(expr);

        expr->setLeft(left);
        expr->setRight(parseFunctionDeclaration(expr));
      }

      return expr;
    }

    shared_ptr<ASTNode> parseBlock(shared_ptr<ASTNode> parent) {
      if (match(Token::BRACE_OPEN)) {
        vector<shared_ptr<ASTNode>> blockExpr;
        shared_ptr<BlockNode> block = make_shared<BlockNode>(parent);

        while (!match(Token::BRACE_CLOSE)) {
          shared_ptr<ASTNode> expr = parseReturn(block);

          if (expr == nullptr) break;

          blockExpr.push_back(expr);
        }

        block->setElements(blockExpr);

        return block;
      }

      return parseFunctionDeclaration(parent);
    }

    shared_ptr<ASTNode> parseFunctionDeclaration(shared_ptr<ASTNode> parent) {
      shared_ptr<ASTNode> expr = parseAssignment(parent);

      if (match(Token::FUNC_DECLARATION)) {
        shared_ptr<FunctionDeclarationNode> func_def = make_shared<FunctionDeclarationNode>(parent);

        if (expr && expr->getNodeType() != ASTNode::AST_NODE_LIST) {
          shared_ptr<ASTNodeList> parameters = make_shared<ASTNodeList>(func_def);
          expr->setParent(parameters);

          parameters->setElements({ expr });

          expr = parameters;
        } else if (!expr) {
          expr = make_shared<ASTNodeList>(func_def);
        }

        shared_ptr<ASTNodeList> params = dynamic_pointer_cast<ASTNodeList>(expr); 
        for (auto param : params->getElements()) {
          param->setParent(params);
        } 

        func_def->setParameters(params);

        shared_ptr<ASTNode> definitionBlock = parseBlock(func_def);

        // In the case of shorthand single-line function bodies, we still want to wrap them in a block within the ast
        // for scoping reasons
        if (definitionBlock->getNodeType() != ASTNode::BLOCK) {
          shared_ptr<BlockNode> block = make_shared<BlockNode>(func_def);
          definitionBlock->setParent(block);

          block->setElements({ definitionBlock });

          definitionBlock = block;
        }

        func_def->setDefinition(definitionBlock);

        expr = func_def;
      }

      return expr;
    }

    shared_ptr<ASTNode> parseExpression(shared_ptr<ASTNode> parent) {
      return parseStructDeclaration(parent);
    }

    shared_ptr<ASTNode> parseStructDeclaration(shared_ptr<ASTNode> parent) {
      if (match(Token::AT)) {
        match(Token::IDENTIFIER);

        shared_ptr<StructDeclarationNode> str = make_shared<StructDeclarationNode>(currentToken.getLexeme(), parent);

        match(Token::BRACE_OPEN);

        str->setValue(parseDict(str));

        return str;
      }

      return parseEnum(parent);
    }

    shared_ptr<ASTNode> parseEnum(shared_ptr<ASTNode> parent) {
      if (match(Token::KEYWORD, Lexemes::ENUM)) {
        match(Token::IDENTIFIER);

        shared_ptr<EnumNode> root = make_shared<EnumNode>(parent);
        root->setIdentifier(parseIdentifier(root));

        if (!match(Token::BRACE_OPEN)) {
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

        while (!match(Token::BRACE_CLOSE)) {
          if (!match(Token::COLON)) {
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

          shared_ptr<ASTNode> node = parseSymbol(root);

          if (!node) break;

          enumVals.push_back(node);
        }

        root->setElements(enumVals);

        return root;
      }

      return parseControlFlow(parent);
    }

    shared_ptr<ASTNode> parseControlFlow(shared_ptr<ASTNode> parent) {
      if (match(Token::KEYWORD, Lexemes::IF)) {
        shared_ptr<ControlFlowNode> cfNode = make_shared<ControlFlowNode>(parent);

        shared_ptr<ASTNode> cnd = parseExpression(cfNode);
        shared_ptr<ASTNode> expr = parseBlock(cfNode);

        vector<pair<shared_ptr<ASTNode>, shared_ptr<ASTNode>>> conditionExpressionPairs = {
          make_pair(cnd, expr)
        };

        while (match(Token::KEYWORD, Lexemes::ELSE) && match(Token::KEYWORD, Lexemes::IF)) {
          cnd = parseExpression(cfNode);
          expr = parseBlock(cfNode);
          conditionExpressionPairs.push_back(make_pair(cnd, expr));
        }

        // If we just matched an else but no if afterwards. This way it only matches one else block per control flow
        if (currentToken.getType() == Token::KEYWORD && currentToken.getLexeme() == Lexemes::ELSE) {
          conditionExpressionPairs.push_back(make_pair(nullptr, parseBlock(cfNode)));
        }

        cfNode->setConditionExpressionPairs(conditionExpressionPairs);

        return cfNode;
      }

      return parsePipeline(parent);
    }

    shared_ptr<ASTNode> parsePipeline(shared_ptr<ASTNode> parent) {
      shared_ptr<ASTNode> expr = parseBooleanComparison(parent);

      while (match(Token::OPERATOR, Lexemes::PIPE)) {
        expr = parseBooleanComparison(parent, expr);
      }

      return expr;
    }

    shared_ptr<ASTNode> parseBooleanComparison(shared_ptr<ASTNode> parent, shared_ptr<ASTNode> passedLeftArg = nullptr) {
      shared_ptr<ASTNode> expr = parseEquality(parent, passedLeftArg);

      while (match(Token::OPERATOR, Lexemes::OR) || match(Token::OPERATOR, Lexemes::AND)) {
        shared_ptr<ASTNode> left = expr;

        expr = make_shared<BinaryOperationNode>(currentToken.getLexeme(), parent);
        left->setParent(expr);

        expr->setLeft(left);
        expr->setRight(parseExpression(expr));
      }

      return expr;
    }

    shared_ptr<ASTNode> parseEquality(shared_ptr<ASTNode>parent, shared_ptr<ASTNode> passedLeftArg = nullptr) {
      shared_ptr<ASTNode> expr = parseComparison(parent, passedLeftArg);

      while (match(Token::OPERATOR, Lexemes::EQUALITY) || match(Token::OPERATOR, Lexemes::INEQUALITY)) {
        shared_ptr<ASTNode> left = expr;

        expr = make_shared<BinaryOperationNode>(currentToken.getLexeme(), parent);
        left->setParent(expr);
    
        expr->setLeft(left);
        expr->setRight(parseComparison(expr));
      }

      return expr;
    }

    shared_ptr<ASTNode> parseComparison(shared_ptr<ASTNode> parent, shared_ptr<ASTNode> passedLeftArg = nullptr) {
      shared_ptr<ASTNode> expr = parseTerm(parent, passedLeftArg);

      while (
        match(Token::OPERATOR, Lexemes::GT) ||
        match(Token::OPERATOR, Lexemes::GTEQ) ||
        match(Token::OPERATOR, Lexemes::LT) ||
        match(Token::OPERATOR, Lexemes::LTEQ)
      ) {
        shared_ptr<ASTNode> left = expr;

        expr = make_shared<BinaryOperationNode>(currentToken.getLexeme(), parent);
        left->setParent(expr);

        expr->setLeft(left);
        expr->setRight(parseTerm(expr));
      }

      return expr;
    }

    shared_ptr<ASTNode> parseTerm(shared_ptr<ASTNode> parent, shared_ptr<ASTNode> passedLeftArg = nullptr) {
      shared_ptr<ASTNode> expr = parseFactor(parent, passedLeftArg);

      while (match(Token::OPERATOR, Lexemes::MINUS) || match(Token::OPERATOR, Lexemes::PLUS)) {
        shared_ptr<ASTNode> left = expr;

        expr = make_shared<BinaryOperationNode>(currentToken.getLexeme(), parent);
        left->setParent(expr);

        expr->setLeft(left);
        expr->setRight(parseFactor(expr));
      }

      return expr;
    }

    shared_ptr<ASTNode> parseFactor(shared_ptr<ASTNode> parent, shared_ptr<ASTNode> passedLeftArg = nullptr) {
      shared_ptr<ASTNode> expr = parseExponent(parent, passedLeftArg);

      while (
        match(Token::OPERATOR, Lexemes::DIVISION) ||
        match(Token::OPERATOR, Lexemes::TIMES) ||
        match(Token::OPERATOR, Lexemes::MODULO)
      ) {
        shared_ptr<ASTNode> left = expr;

        expr = make_shared<BinaryOperationNode>(currentToken.getLexeme(), parent);
        left->setParent(expr);

        expr->setLeft(left);
        expr->setRight(parseExponent(expr));
      }

      return expr;
    }

    shared_ptr<ASTNode> parseExponent(shared_ptr<ASTNode>parent, shared_ptr<ASTNode> passedLeftArg = nullptr) {
      shared_ptr<ASTNode> expr = parseUnary(parent, passedLeftArg);

      while (match(Token::OPERATOR, Lexemes::EXPONENT)) {
        shared_ptr<ASTNode> left = expr;

        expr = make_shared<BinaryOperationNode>(currentToken.getLexeme(), parent);
        left->setParent(expr);

        expr->setLeft(left);
        expr->setRight(parseUnary(expr));
      }

      return expr;
    }

    shared_ptr<ASTNode> parseUnary(shared_ptr<ASTNode> parent, shared_ptr<ASTNode> passedLeftArg = nullptr) {
      // Unary cant have a left arg, so if we get one passed in we can skip straight to primary
      if (!passedLeftArg && (match(Token::OPERATOR, Lexemes::NOT) || match(Token::OPERATOR, Lexemes::MINUS))) {
        shared_ptr<ASTNode> un = make_shared<UnaryOperationNode>(currentToken.getLexeme(), parent);
        un->setValue(parseUnary(un, passedLeftArg));

        return un;
      }

      return parsePrimary(parent, passedLeftArg);
    }

    shared_ptr<ASTNode> parsePrimary(shared_ptr<ASTNode> parent, shared_ptr<ASTNode> passedLeftArg = nullptr) {
      if (match(Token::IDENTIFIER)) {
        return parseFunctionInvocation(parent, passedLeftArg);
      }

      if (passedLeftArg) return passedLeftArg;

      if (match(Token::BOOLEAN) || match(Token::NUMBER) || match(Token::STRING)) {
        map<Token::Types, ASTNode::Types> tokenTypeToAstTypeMap = {
          { Token::NUMBER, ASTNode::NUMBER_LITERAL },
          { Token::BOOLEAN, ASTNode::BOOLEAN_LITERAL },
          { Token::STRING, ASTNode::STRING_LITERAL }
        };

        auto it = tokenTypeToAstTypeMap.find(currentToken.getType());

        string value = currentToken.getLexeme();

        // Pulls the string out of quotation marks
        if (currentToken.getType() == Token::STRING) {
          value = value.substr(1, value.length() - 2);
        }

        return make_shared<LiteralNode>(it->second, value, parent);
      }

      if (match(Token::COLON)) {
        return parseSymbol(parent);
      }

      if (match(Token::BRACKET_OPEN)) {
        return parseList(parent);
      }

      if (match(Token::BRACE_OPEN)) {
        return parseDict(parent);
      }

      if (match(Token::PAREN_OPEN)) {
        return parseExpressionList(parent);
      }

      return nullptr;
    }

    shared_ptr<ASTNode> parseExpressionList(shared_ptr<ASTNode> parent, bool forceList = false) {
      shared_ptr<ASTNode> expr = parseFunctionDeclaration(parent);

      if (check(Token::COMMA) || !expr || forceList) {
        shared_ptr<ASTNodeList> nodeList = make_shared<ASTNodeList>(parent);
        vector<shared_ptr<ASTNode>> expressions;

        if (expr) {
          expr->setParent(nodeList);
          expressions.push_back(expr);
        }

        while (match(Token::COMMA)) {
          expressions.push_back(parseFunctionDeclaration(nodeList));
        }

        nodeList->setElements(expressions);

        expr = nodeList;
      }

      match(Token::PAREN_CLOSE);

      return expr;
    }

    shared_ptr<ASTNode> parseDict(shared_ptr<ASTNode> parent) {
      pair<string, shared_ptr<ASTNode>> p = parseKvPair(parent);
      shared_ptr<ASTNode> expr = p.second;

      if (p.first == "kv" && expr && expr->getNodeType() == ASTNode::TUPLE) {
        vector<shared_ptr<ASTNode>> el;

        if (expr->getLeft()) el.push_back(expr);

        while (match(Token::COMMA)) {
          el.push_back(parseKvPair(parent).second);
        }

        expr = make_shared<DictionaryNode>(parent);

        for (auto e : el) {
          e->setParent(expr);
        }

        dynamic_pointer_cast<DictionaryNode>(expr)->setElements(el);

        match(Token::BRACE_CLOSE);
      }

      return expr;
    }

    pair<string, shared_ptr<ASTNode>> parseKvPair(shared_ptr<ASTNode> parent) {
      // Because both flows of this function return a tuple, we need a type flag to indicate whether
      // we generated the tuple with the intention of it being a kvPair or not. Otherwise it would
      // be ambiguous and we would accidentally convert dicts with a single key-value pair into a tuple
      string type = "tuple";
      shared_ptr<ASTNode> expr = parseTuple(parent);

      if (match(Token::COLON)) {
        type = "kv";
        shared_ptr<ASTNode> left = expr;

        if (left->getNodeType() == ASTNode::IDENTIFIER) {
          left = make_shared<SymbolNode>(dynamic_pointer_cast<IdentifierNode>(left)->getIdentifier(), expr);
        }

        expr = make_shared<TupleNode>(parent);
        left->setParent(expr);
    
        expr->setLeft(left);
        expr->setRight(parseExpression(expr));
      } else if (expr == nullptr) {
        // parseTuplen will return a nullptr if it just immediately encounters a BRACE_CLOSE. We can treat this
        // as a dict since a valid tuple must have 2 values in it.
        type = "kv";
        expr = make_shared<TupleNode>(parent);
      }

      return make_pair(type, expr);
    }

    shared_ptr<ASTNode> parseTuple(shared_ptr<ASTNode> parent) {
      shared_ptr<ASTNode> expr;

      if (match(Token::BRACE_CLOSE)) return nullptr;

      try {
        expr = parseExpression(parent);
      } catch (ParseError e) {
        if (e.getErrorParseType() == "symbol") remainingTokens->pop_front();
      }

      if (match(Token::COMMA)) {
        shared_ptr<ASTNode> first = expr;

        expr = make_shared<TupleNode>(parent);
        first->setParent(expr);
        expr->setLeft(first);

        try {
          expr->setRight(parseExpression(expr));
        } catch (ParseError e) {
          if (e.getErrorParseType() == "symbol") remainingTokens->pop_front();
        }

        if (!match(Token::BRACE_CLOSE)) {
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

    shared_ptr<ASTNode> parseList(shared_ptr<ASTNode> parent) {
      shared_ptr<ListNode> listNode = make_shared<ListNode>(parent);
      vector<shared_ptr<ASTNode>> el;

      if (!match(Token::BRACKET_CLOSE)) {
        el.push_back(parseExpression(listNode));

        while(match(Token::COMMA)) {
          el.push_back(parseExpression(listNode));
        }

        listNode->setElements(el);

        match(Token::BRACKET_CLOSE);
      }

      return listNode;
    }

    shared_ptr<ASTNode> parseFunctionInvocation(shared_ptr<ASTNode> parent, shared_ptr<ASTNode> passedLeftArg = nullptr) {
      shared_ptr<ASTNode> expr = parseIdentifier(parent);

      if (match(Token::PAREN_OPEN)) {
        shared_ptr<FunctionInvocationNode> funcInvNode = make_shared<FunctionInvocationNode>(parent);
        expr->setParent(funcInvNode);
        funcInvNode->setIdentifier(expr);
        shared_ptr<ASTNodeList> arguments = dynamic_pointer_cast<ASTNodeList>(parseExpressionList(funcInvNode, true));

        // This is used for pipeline operators pointing to function invocations. It takes the passed
        // left arg and sets it as the first argument to the function call
        if (passedLeftArg) {
          arguments->getElements().insert(arguments->getElements().begin(), passedLeftArg);
        }

        funcInvNode->setParameters(arguments);

        expr = funcInvNode;
      }

      return expr;
    }

    shared_ptr<ASTNode> parseIdentifier(shared_ptr<ASTNode> parent) {
      validateIdentifier(currentToken);

      shared_ptr<ASTNode> ident = make_shared<IdentifierNode>(currentToken.getLexeme(), parent);

      if (match(Token::OPERATOR, Lexemes::LT)) {
        ident->setValue(parseType(ident));

        match(Token::OPERATOR, Lexemes::GT);
      }

      return ident;
    }

    shared_ptr<ASTNode> parseType(shared_ptr<ASTNode> parent) {
      match(Token::IDENTIFIER);

      string typeName = currentToken.getLexeme();
      shared_ptr<ASTNode> typ = make_shared<TypeDeclarationNode>(typeName, parent);

      if (match(Token::OPERATOR, Lexemes::LT)) {
        shared_ptr<TypeDeclarationNode> typeDecl = dynamic_pointer_cast<TypeDeclarationNode>(typ);
        vector<shared_ptr<ASTNode>> types = { parseType(typeDecl) };

        while (match(Token::COMMA)) {
          types.push_back(parseType(typeDecl));
        }

        if (types.size() > 1) {
          typeDecl->setElements(types);
        } else {
          typeDecl->setValue(types.at(0));
        }

        match(Token::OPERATOR, Lexemes::GT);
      }

      return typ;
    }

    shared_ptr<ASTNode> parseSymbol(shared_ptr<ASTNode> parent) {
      if (match(Token::IDENTIFIER) || match(Token::NUMBER)) {
        if (currentToken.getType() == Token::IDENTIFIER) validateIdentifier(currentToken);

        return make_shared<SymbolNode>(currentToken.getLexeme(), parent);
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
