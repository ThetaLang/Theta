#include "Compiler.hpp"
#include "../lexer/Lexer.cpp"
#include "../parser/Parser.cpp"
#include "compiler/TypeChecker.hpp"

using namespace std;

namespace Theta {
    Compiler& Compiler::getInstance() {
        static Compiler instance;
        return instance;
    }

    void Compiler::compile(string entrypoint, string outputFile, bool emitTokens, bool emitAST, bool emitWAT) {
        isEmitTokens = emitTokens;
        isEmitAST = emitAST;
        isEmitWAT = emitWAT;

        shared_ptr<ASTNode> programAST = buildAST(entrypoint);

        if (!optimizeAST(programAST)) return;

        outputAST(programAST, entrypoint);
    
        TypeChecker typeChecker;
        bool isTypeValid = typeChecker.checkAST(programAST);

        for (int i = 0; i < encounteredExceptions.size(); i++) {
            encounteredExceptions[i]->display();
        }

        if (!isTypeValid) return;

        BinaryenModuleRef module = CodeGen::generateWasmFromAST(programAST);

        if (isEmitWAT) {
            cout << "Generated WAT for \"" + entrypoint + "\":" << endl;
            BinaryenModulePrint(module);
        }

        writeModuleToFile(module, outputFile);
    }

    shared_ptr<ASTNode> Compiler::compileDirect(string source) {
        shared_ptr<ASTNode> ast = buildAST(source, "ith");

        if (!optimizeAST(ast)) return nullptr;
        
        outputAST(ast, "ith");

        TypeChecker typeChecker;
        bool isTypeValid = typeChecker.checkAST(ast);

        for (int i = 0; i < encounteredExceptions.size(); i++) {
            encounteredExceptions[i]->display();
        }

        if (!isTypeValid) return ast;

        BinaryenModuleRef module = CodeGen::generateWasmFromAST(ast);

        cout << "-> " + ast->toJSON() << endl;
        cout << "-> ";
        BinaryenModulePrint(module);
        cout << endl;

        return ast;
    }

    shared_ptr<ASTNode> Compiler::buildAST(string file) {
        std::ifstream t(file);
        std::stringstream buffer;
        buffer << t.rdbuf();

        string fileSource = buffer.str();

        return buildAST(fileSource, file);
    }

    shared_ptr<ASTNode> Compiler::buildAST(string source, string fileName) {
        Theta::Lexer lexer;
        lexer.lex(source);

        if (isEmitTokens) {
            cout << "Lexed Tokens for \"" + fileName + "\":" << endl;
            for (int i = 0; i < lexer.tokens.size(); i++) {
                cout << lexer.tokens[i].toJSON() << endl;
            }
            cout << endl;
        }

        Theta::Parser parser;
        shared_ptr<Theta::ASTNode> parsedAST = parser.parse(lexer.tokens, source, fileName, filesByCapsuleName);

        return parsedAST;
    }

    void Compiler::addException(shared_ptr<Theta::Error> e) {
        encounteredExceptions.push_back(e);
    }

    vector<shared_ptr<Theta::Error>> Compiler::getEncounteredExceptions() {
        return encounteredExceptions;
    }

    void Compiler::clearExceptions() {
        encounteredExceptions.clear();
    }

    shared_ptr<Theta::LinkNode> Compiler::getIfExistsParsedLinkAST(string capsuleName) {
        auto it = parsedLinkASTs.find(capsuleName);

        if (it != parsedLinkASTs.end()) return it->second;

        return nullptr;
    }

    void Compiler::addParsedLinkAST(string capsuleName, shared_ptr<Theta::LinkNode> linkNode) {
        parsedLinkASTs.insert(make_pair(capsuleName, linkNode));
    }

    void Compiler::discoverCapsules() {
        for (const auto& entry : std::__fs::filesystem::recursive_directory_iterator(".")) {
            if (entry.is_regular_file() && entry.path().extension() == ".th") {
                string capsuleName = findCapsuleName(entry.path().string());

                filesByCapsuleName->insert(make_pair(capsuleName, entry.path().string()));
            }
        }
    }

    string Compiler::findCapsuleName(string file) {
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

    bool Compiler::optimizeAST(shared_ptr<ASTNode> &ast, bool silenceErrors) {
        for (auto &pass : optimizationPasses) {
            pass->optimize(ast);

            if (encounteredExceptions.size() > 0) {
                if (!silenceErrors) {
                    for (int i = 0; i < encounteredExceptions.size(); i++) {
                        encounteredExceptions[i]->display();
                    }
                }

                pass->cleanup();
                return false;
            }

            pass->cleanup();
        }

        return true;
    }

    void Compiler::writeModuleToFile(BinaryenModuleRef &module, string fileName) {
        // TODO: This isnt the right way to do this. This will only allow 4k to be written.
        // Figure out a better way to decide on size
        vector<char> buffer(4096);

        size_t written = BinaryenModuleWrite(module, buffer.data(), buffer.size());

        ofstream outFile(fileName, std::ios::binary);
        if (!outFile) {
            throw std::runtime_error("Failed to open file for writing: " + fileName);
        }

        outFile.write(buffer.data(), written);
        outFile.close();

        if (!outFile.good()) {
            throw std::runtime_error("Failed to write module to file: " + fileName);
        }

        cout << "Compilation successful. Output: " + fileName << endl;
    }

    void Compiler::outputAST(shared_ptr<ASTNode> ast, string fileName) {
        if (ast && isEmitAST) {
            cout << "Generated AST for \"" + fileName + "\":" << endl;
            cout << ast->toJSON() << endl;
            cout << endl;
        } else if (!ast) {
            cout << "Could not parse AST for file " + fileName << endl;
        }
    }
}
