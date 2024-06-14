#pragma once

#include <string>
#include <sstream>
#include "ast_node.hpp"
#include "../../util/tokens.hpp"

using namespace std;

class LiteralNode : public ASTNode {
    public:
        string nodeType;
        Tokens type;
        string literalValue;

        LiteralNode(Tokens typ, string val) : type(typ), literalValue(val){
            string formattedType = tokenTypeToString(typ);

            for (int i = 1; i < formattedType.length(); ++i) {
                formattedType[i] = tolower(formattedType[i]);
            }

            nodeType = formattedType + "Literal";
        };

        string getNodeType() const override { return nodeType; }

        string getLiteralValue() { return literalValue; }

        string toJSON() const override {
            ostringstream oss;
            
            oss << "{";
            oss << "\"type\": \"" << nodeType << "\", ";
            oss << "\"value\": \"" << literalValue << "\" ";
            oss << "}";

            return oss.str();
        }
};