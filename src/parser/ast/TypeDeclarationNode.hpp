#pragma once

#include <memory>
#include <string>
#include <sstream>
#include "ASTNode.hpp"
#include "ASTNodeList.hpp"

using namespace std;

namespace Theta {
    class TypeDeclarationNode : public ASTNodeList {
        public:
            string type;

            TypeDeclarationNode(string typ) : type(typ), ASTNodeList(ASTNode::Types::TYPE_DECLARATION) {};

            string getType() { return type; }

            string toString(bool bare = false) {
                string typeString;

                if (!bare) typeString += "<";

                typeString += type;

                if (value) {
                    typeString += "<" + dynamic_pointer_cast<TypeDeclarationNode>(value)->toString(true) + ">";
                } else if (left) {
                    typeString += "<" + dynamic_pointer_cast<TypeDeclarationNode>(left)->toString(true) + ", ";
                    typeString += dynamic_pointer_cast<TypeDeclarationNode>(right)->toString(true) + ">";
                } else if (elements.size() > 0) {
                    typeString += "<";

                    for (int i = 0; i < elements.size(); i++) {
                        if (i > 0) typeString += ", ";

                        typeString += dynamic_pointer_cast<TypeDeclarationNode>(elements.at(i))->toString(true);
                    }

                    typeString += ">";
                }

                if (!bare) typeString += ">";

                return typeString;
            }

            string toJSON() const override {
                ostringstream oss;

                oss << "{";
                oss << "\"type\": \"" << getNodeTypePretty() << "\"";
                oss << ",\"declaredType\": \"" << type << "\"";

                if (left) {
                    oss << ",\"left\": " << left->toJSON();
                    oss << ",\"right\": " << (right ? right->toJSON() : "null");
                } else if (elements.size() > 0) {
                    oss << ",\"elements\": [";

                    for (int i = 0; i < elements.size(); i++) {
                        if (i > 0) {
                            oss << ", ";
                        }

                        oss << elements.at(i)->toJSON();
                    }

                    oss << "]";
                } else {
                    oss << ",\"value\": " << (value ? value->toJSON() : "null");
                }

                oss << "}";

                return oss.str();
            }
    };
}
