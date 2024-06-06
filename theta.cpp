#include <iostream>
#include <fstream>
#include <sstream>
#include "src/lexer/lexer.cpp"

using namespace std;

int main() {
    ThetaLexer lexer;

    std::ifstream t("fixtures/test.th");
    std::stringstream buffer;
    buffer << t.rdbuf();

    cout << "WAT";

    vector<Token> tokens = lexer.lex(buffer.str());

    cout << "\n========== LEXED TOKENS ==========\n";
    for (int i = 0; i < tokens.size(); i++) {
        cout << "{ type: " + tokens[i].getType() + ", text: " + tokens[i].getText() + ", location: line " + to_string(tokens[i].getStartLocation()[0]) + ", column " + to_string(tokens[i].getStartLocation()[1]) + "}\n";
    }

    return 0;
}