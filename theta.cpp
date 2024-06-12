#include "src/compiler/compiler.hpp"

using namespace std;

int main(int argc, char* argv[]) {
    // For now, always expect the first argument to be a .th file that needs lexing
    ThetaCompiler::getInstance().compile(argv[1]);

    return 0;
}