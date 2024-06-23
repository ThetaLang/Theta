#include "src/cli/CLI.cpp"

using namespace std;

int main(int argc, char* argv[]) {
    ThetaCLI::parseCommand(argc, argv);

    return 0;
}
