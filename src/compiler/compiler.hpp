#pragma once

#include <vector>
#include <deque>
#include <string>
#include <map>
#include <fstream> 
#include <iostream>
#include <memory>
#include <filesystem>
#include "../parser/ast/ast_node.hpp"

using namespace std;

class ThetaCompiler {
    public:
        void compile(string entrypoint);

        shared_ptr<ASTNode> buildAST(string entrypoint);
        
        static ThetaCompiler& getInstance();

    private:
        ThetaCompiler() {
            filesByCapsuleName = make_shared<map<string, string>>();
            discoverCapsules();
        }

        ThetaCompiler(const ThetaCompiler&) = delete;
        ThetaCompiler& operator=(const ThetaCompiler&) = delete;

        shared_ptr<map<string, string>> filesByCapsuleName;

        void discoverCapsules();

        string findCapsuleName(string file);
};