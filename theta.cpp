#include <iostream>
#include <fstream>
#include <sstream>
#include "src/lexer/lexer.cpp"
#include "src/parser/parser.cpp"

using namespace std;

int main(int argc, char* argv[]) {
    ThetaLexer lexer;

    // For now, always expect the first argument to be a .th file that needs lexing
    string fileName = argv[1];

    std::ifstream t(fileName);
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
    parser.parse(lexer.tokens, fileSource, fileName);

    return 0;
}