#pragma once

namespace Theta {
    class WasmClosure {
    public:
        WasmClosure(int tableIndex, int arity) {
            idx = tableIndex;
            arity = arity;
            argPointers = new int[arity];
        }

        int getFunctionIndex() { return idx; }

        int getArity() { return arity; }

        int* getArgPointers() { return argPointers; }

        void addArg(int argPtr) {
            argPointers[arity] = argPtr;
            currentArgs++;
        }


    private:
        int idx;
        int arity;
        int* argPointers;
        int currentArgs = 0;
    };
}
