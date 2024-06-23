#include <cstddef>
#include <vector>
#include <deque>
#include <string>
#include <map>
#include <fstream>
#include <iostream>
#include <memory>
#include <filesystem>
#include "ThetaCompiler.hpp"
#include "../lexer/ThetaLexer.cpp"
#include "../parser/ThetaParser.cpp"
#include "../parser/ast/ASTNode.hpp"

using namespace std;

ThetaCompiler& ThetaCompiler::getInstance() {
    static ThetaCompiler instance;
    return instance;
}

void ThetaCompiler::compile(string entrypoint, string outputFile, bool emitTokens, bool emitAST) {
    isEmitTokens = emitTokens;
    isEmitAST = emitAST;

    shared_ptr<ASTNode> programAST = buildAST(entrypoint);

    for (int i = 0; i < encounteredExceptions.size(); i++) {
        encounteredExceptions[i].display();
    }
}

shared_ptr<ASTNode> ThetaCompiler::compileDirect(string source) {
    shared_ptr<ASTNode> ast = buildAST(source, "ith");

    for (int i = 0; i < encounteredExceptions.size(); i++) {
        encounteredExceptions[i].display();
    }

    return ast;
}

shared_ptr<ASTNode> ThetaCompiler::buildAST(string file) {
    std::ifstream t(file);
    std::stringstream buffer;
    buffer << t.rdbuf();

    string fileSource = buffer.str();

    return buildAST(fileSource, file);
}

shared_ptr<ASTNode> ThetaCompiler::buildAST(string source, string fileName) {
    ThetaLexer lexer;
    lexer.lex(source);

    if (isEmitTokens) {
        cout << "Lexed Tokens for \"" + fileName + "\":" << endl;
        for (int i = 0; i < lexer.tokens.size(); i++) {
            cout << lexer.tokens[i].toJSON() << endl;
        }
        cout << endl;
    }

    ThetaParser parser;
    shared_ptr<ASTNode> parsedAST = parser.parse(lexer.tokens, source, fileName, filesByCapsuleName);

    if (parsedAST && isEmitAST) {
        cout << "Generated AST for \"" + fileName + "\":" << endl;
        cout << parsedAST->toJSON() << endl;
        cout << endl;
    } else if (!parsedAST) {
        cout << "Could not parse AST for file " + fileName << endl;
    }

    return parsedAST;
}

void ThetaCompiler::addException(ThetaCompilationError e) {
    encounteredExceptions.push_back(e);
}

vector<ThetaCompilationError> ThetaCompiler::getEncounteredExceptions() {
    return encounteredExceptions;
}

void ThetaCompiler::clearExceptions() {
    encounteredExceptions.clear();
}

shared_ptr<LinkNode> ThetaCompiler::getIfExistsParsedLinkAST(string capsuleName) {
    auto it = parsedLinkASTs.find(capsuleName);

    if (it != parsedLinkASTs.end()) return it->second;

    return nullptr;
}

void ThetaCompiler::addParsedLinkAST(string capsuleName, shared_ptr<LinkNode> linkNode) {
    parsedLinkASTs.insert(make_pair(capsuleName, linkNode));
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
