#pragma once

#include <vector>
#include <deque>
#include <string>
#include <map>
#include <fstream>
#include <iostream>
#include <memory>
#include <filesystem>
#include "../parser/ast/ASTNode.hpp"
#include <binaryen-c.h>

using namespace std;

namespace Theta {
    class CodeGen {
        public:
            static void generate();
    };
}
