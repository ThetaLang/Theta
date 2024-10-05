#pragma once

#include "wasm.hh"
#include <stdexcept>
#include <vector>

using namespace std;

namespace Theta {
  class ExecutionContext {
  public:
    wasm::Val result;
    vector<string> exportNames;

    ExecutionContext(wasm::Val result, vector<string> exportNames) : result(std::move(result)), exportNames(exportNames) {}

    string stringifiedResult() {
      if (result.kind() == wasm::I64) return to_string(result.i64());
      if (result.kind() == wasm::I32) return result.i32() == 1 ? "true" : "false";
      
      throw runtime_error("Could not parse result string");
    }
  };
}
