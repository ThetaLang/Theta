#include <vector>
#include <deque>
#include <string>
#include <map>
#include <fstream> 
#include <iostream>
#include <memory>
#include <filesystem>
#include "compiler.hpp"
#include "../lexer/lexer.cpp"
#include "../parser/parser.cpp"
#include "../parser/ast/ast_node.hpp"

using namespace std;

ThetaCompiler& ThetaCompiler::getInstance() {
    static ThetaCompiler instance;
    return instance;
}

void ThetaCompiler::compile(string entrypoint) {
    buildAST(entrypoint);
}

shared_ptr<ASTNode> ThetaCompiler::buildAST(string file) {
    ThetaLexer lexer;

    std::ifstream t(file);
    std::stringstream buffer;
    buffer << t.rdbuf();

    string fileSource = buffer.str();

    lexer.lex(fileSource);

    cout << "\n========== LEXED TOKENS ==========\n";
    for (int i = 0; i < lexer.tokens.size(); i++) {
        cout << lexer.tokens[i].toJSON() << "\n";
    }
    cout << "====================================\n";
    
    ThetaParser parser;
    return parser.parse(lexer.tokens, fileSource, file, filesByCapsuleName);
}

void ThetaCompiler::discoverCapsules() {
    for (const auto& entry : std::__fs::filesystem::recursive_directory_iterator(".")) {
        if (entry.is_regular_file() && entry.path().extension() == ".th") {
            string capsuleName = findCapsuleName(entry.path().string());
            
            filesByCapsuleName->insert(make_pair(capsuleName, entry.path().string()));
        }
    }
}

string ThetaCompiler::findCapsuleName(string file) {
    std::ifstream t(file);
    std::stringstream buffer;

    buffer << t.rdbuf();

    string source = buffer.str();

    int i = 0;
    bool capsuleNameFound = false;
    string capsuleName;

    // Iterate over the source but stop once we find a capsule
    while (i + 6 < source.length() && !capsuleNameFound) {
        if (source[i] == 'c' && source[i + 1] == 'a' && source[i + 2] == 'p' && source[i + 3] == 's' && source[i + 4] == 'u' && source[i + 5] == 'l' && source[i + 6] == 'e') {
            capsuleNameFound = true;
            i += 7;
        }

        i++;
    }

    if (capsuleNameFound) {
        while (source[i] != '\n' && !isspace(source[i]) && source[i] != '{') {
            capsuleName.push_back(source[i]);

            i++;
        }
    }

    return capsuleName;
}