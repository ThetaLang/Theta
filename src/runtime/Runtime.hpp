#pragma once

#include "wasm.hh"
#include <vector>
#include <stdexcept>
#include "runtime/ExecutionContext.hpp"
#include <iostream>
#include <v8.h>

using namespace std;

namespace Theta {
  class Runtime {
  public:
    wasm::own<wasm::Engine> engine;
    wasm::own<wasm::Store> store;

    static Runtime& getInstance() {
      static Runtime instance;
      return instance;
    }

    wasm::Store* getStore() { return store.get(); }

    wasm::Engine* getEngine() { return engine.get(); }
  
    ExecutionContext execute(vector<char> wasmBinary, string functionName) {
      auto binary = wasm::vec<byte_t>::make_uninitialized(wasmBinary.size());
      memcpy(binary.get(), wasmBinary.data(), wasmBinary.size());

      auto wasmModule = wasm::Module::make(store.get(), binary);
      if (!wasmModule) throw runtime_error("Error compiling module");

      // Can pass imports in place of nullptr if needed
      auto instance = wasm::Instance::make(store.get(), wasmModule.get(), nullptr);
      if (!instance) throw runtime_error("Error instantiating module");


      // Extract exports
      wasm::ownvec<wasm::ExportType> exportTypes = wasmModule->exports();
      auto instanceExports = instance->exports();
      vector<string> exportNames;
      wasm::Func* func = nullptr;
      for (size_t i = 0; i < exportTypes.size(); i++) {
        wasm::ExportType *exportType = exportTypes[i].get();

        string exportName(exportType->name().get(), exportType->name().size());
      
        // Save the idx of the function so we can call it later
        if (exportType->type()->kind() == wasm::EXTERN_FUNC && exportName == functionName) {
          func = instanceExports[i]->func();
        }
      
        exportNames.push_back(exportName);
      }

      if (!func) throw runtime_error("Exported function not found");

      // Call the function with no arguments
      wasm::Val args[0];  // No arguments for this test
      wasm::Val results[1];  // A single result (the number returned by the function)
      auto trap = func->call(args, results);


      if (trap) throw runtime_error("Error calling function");

      return ExecutionContext(std::move(results[0]), exportNames);
    }
  
  private:
    Runtime() {
      v8::V8::SetFlagsFromString("--experimental-wasm-stringref");
      engine = wasm::Engine::make();
      store = wasm::Store::make(engine.get());
    } 

    Runtime(const Runtime&) = delete;
    Runtime& operator=(const Runtime&) = delete;
  };
}

