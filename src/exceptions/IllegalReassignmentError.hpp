#pragma once

#include <string>
#include <iostream>
#include "Error.hpp"

using namespace std;

namespace Theta {
    class IllegalReassignmentError : public Error {
    public:
        IllegalReassignmentError(string ident) : identifier(ident) {}

        string identifier;

        void display() override {
            cout << "  \033[1;31mIllegalReassignmentError\033[0m: '" +
                identifier +
                "' can not be reassigned once it has been defined" << endl;
        }
    };
}
