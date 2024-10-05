#pragma once

#include <sstream>
#include <vector>
#include <string>
#include "Pointer.hpp"

using namespace std;

namespace Theta {
  class WasmClosure {
  public:
    WasmClosure(Pointer<PointerType::Function> ptr, int initialArity) : fnPointer(ptr), arity(initialArity) {
      argPointers.resize(arity);
    }

    WasmClosure(
      Pointer<PointerType::Function> ptr,
      int initialArity,
      vector<Pointer<PointerType::Data>> args
    ) : fnPointer(ptr), arity(initialArity) {
      argPointers.resize(arity);

      for (int i = 0; i < args.size(); i++) {
        if (args.at(i).getAddress() != -1) {
          argPointers[arity - 1] = args.at(i);

          arity--;
        }
      }
    }

    void setAddress(int closureMemAddress) {
      pointer = Pointer<PointerType::Closure>(closureMemAddress);
    }

    Pointer<PointerType::Closure> getPointer() { return pointer; }

    Pointer<PointerType::Function> getFunctionPointer() { return fnPointer; }

    int getArity() { return arity; }

    vector<Pointer<PointerType::Data>> getArgPointers() { return argPointers; }

    void addArgs(vector<Pointer<PointerType::Data>> argPtrs) {
      for (auto argPtr : argPtrs) {
        argPointers[arity - 1] = argPtr;
        arity--;
      }
    }

    string toJSON() {
      ostringstream oss;

      oss << "{";
      oss << "\"ptr\": \"" << to_string(fnPointer.getAddress()) << "\"";
      oss << ", \"arity\": " << to_string(arity);
      oss << ", \"argPointers\": [";

      for (int i = 0; i < argPointers.size(); i++) {
        if (i > 0) oss << ", ";

        oss << to_string(argPointers[i].getAddress());
      }

      oss << "] ";
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
    Pointer<PointerType::Closure> pointer;
    Pointer<PointerType::Function> fnPointer;
    int arity;
    vector<Pointer<PointerType::Data>> argPointers;
  };
}
