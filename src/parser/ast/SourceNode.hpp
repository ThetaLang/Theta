#pragma once

#include <string>
#include <sstream>
#include "ASTNode.hpp"

using namespace std;

namespace Theta {
  class SourceNode : public ASTNode {
  public:
    vector<shared_ptr<ASTNode>> links;

    SourceNode() : ASTNode(ASTNode::SOURCE, nullptr) {};

    void setLinks(vector<shared_ptr<ASTNode>> ln) { links = ln; }

    vector<shared_ptr<ASTNode>> getLinks() { return links; }

    string toJSON() const override {
      ostringstream oss;

      oss << "{";
      oss << "\"type\": \"" << getNodeTypePretty() << "\"";
      oss << ", \"links\": [";

      for (int i = 0; i < links.size(); i++) {
        if (i > 0) {
          oss << ", ";
        }

        oss << (links[i] ? links[i]->toJSON() : "null");
      }

      oss << "] ";
      oss << ", \"value\": " << (value ? value->toJSON() : "null");
      oss << "}";

      return oss.str();
      }
  };
}
