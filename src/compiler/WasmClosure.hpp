#pragma once

#include <sstream>
#include <vector>
#include <string>
#include "Pointer.hpp"

using namespace std;

namespace Theta {
  class WasmClosure {
  public:
    WasmClosure(Pointer<PointerType::Function> ptr, int initialArity) : fnPointer(ptr), arity(initialArity) {}

    WasmClosure(
      Pointer<PointerType::Function> ptr,
      int initialArity,
      int totalArgs
    ) : fnPointer(ptr), arity(initialArity - totalArgs), argCount(totalArgs) {}

    Pointer<PointerType::Function> getFunctionPointer() { return fnPointer; }

    int getArity() { return arity; }

    int getArgCount() { return argCount; }

    void addArgs(int argsToAdd) {
      argCount += argsToAdd;
      arity -= argsToAdd;
    }

    int getTotalStorageSize() {
      // At least 4 bytes for the fn_idx and 4 bytes for the arity. Then 4 bytes for each parameter the closure takes.
      // We also multiply the remaining arity, since not all parameters may have been applied to the function
      return 8 + (argCount * 4) + (arity * 4);
    }

    string toJSON() {
      ostringstream oss;

      oss << "{";
      oss << "\"ptr\": \"" << to_string(fnPointer.getAddress()) << "\"";
      oss << ", \"arity\": " << to_string(arity);
      oss << ", \"argCount\": " << to_string(argCount);
      oss << "}";

      return oss.str();
    }

    static WasmClosure clone(WasmClosure toClone) {
      return WasmClosure(
        toClone.getFunctionPointer(),
        toClone.arity
      );
    }

  private:
    Pointer<PointerType::Function> fnPointer;
    int arity;
    int argCount;
  };
}
