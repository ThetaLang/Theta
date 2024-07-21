#pragma once

using namespace std;

namespace Theta {
    class Error : public exception {
        public:
            virtual void display() = 0;
    };
}

