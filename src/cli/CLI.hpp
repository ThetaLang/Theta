#pragma once

#include <string>

using namespace std;

namespace Theta {
    class CLI {
    public:
        static void parseCommand(int argc, char* argv[]);

        static string makeLink(string url, string text = "");

        static void printLanguageVersion();

    private:
        static void printUsageInstructions();

        static bool validateOption(string option);
    };
}
