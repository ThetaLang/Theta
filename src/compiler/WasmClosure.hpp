#pragma once

#include <vector>

using namespace std;

namespace Theta {
    class WasmClosure {
    public:
        WasmClosure(int tableIndex, int initialArity) {
            idx = tableIndex;
            arity = initialArity;

            argPointers.resize(arity);
        }

        int getFunctionIndex() { return idx; }

        int getArity() { return arity; }

        vector<int> getArgPointers() { return argPointers; }

        void addArgs(vector<int> argPtrs) {
            for (int argPtr : argPtrs) {
                argPointers[arity - 1] = argPtr;
                arity--;
            }
        }

        static WasmClosure clone(WasmClosure toClone) {
            return WasmClosure(
                toClone.getFunctionIndex(),
                toClone.arity
            );
        }


    private:
        int idx;
        int arity;
        vector<int> argPointers;
    };
}
