#pragma once

#include "wasm.hh"
#include <vector>

using namespace std;

namespace Theta {
  class ExecutionContext {
  public:
      wasm::Val result;
      vector<string> exportNames;

      ExecutionContext(wasm::Val result, vector<string> exportNames) : result(std::move(result)), exportNames(exportNames) {}
  };
}
