#include "src/cli/CLI.hpp"

using namespace std;

int main(int argc, char* argv[]) {
    Theta::CLI::parseCommand(argc, argv);

    return 0;
}
