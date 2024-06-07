#include <iostream>
#include <fstream>
#include <sstream>
#include "src/lexer/lexer.cpp"
#include "src/parser/parser.cpp"

using namespace std;

int main(int argc, char* argv[]) {
    ThetaLexer lexer;

    // For now, always expect the first argument to be a .th file that needs lexing
    std::ifstream t(argv[1]);
    std::stringstream buffer;
    buffer << t.rdbuf();

    string fileSource = buffer.str();

    vector<Token> tokens = lexer.lex(fileSource);

    cout << "\n========== LEXED TOKENS ==========\n";
    for (int i = 0; i < tokens.size(); i++) {
        cout << "{ type: " + tokens[i].getType() + ", text: " + tokens[i].getText() + ", location: " + tokens[i].getStartLocationString() + " }\n";
    }
    cout << "====================================\n";
    
    ThetaParser parser;
    parser.parse(tokens, fileSource);

    return 0;
}