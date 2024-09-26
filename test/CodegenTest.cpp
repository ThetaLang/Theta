#define CATCH_CONFIG_MAIN
#include "catch2/catch_amalgamated.hpp"
#include "../src/lexer/Lexer.cpp"
#include "../src/parser/Parser.cpp"
#include "../src/compiler/Compiler.hpp"
#include "../src/compiler/TypeChecker.hpp"
#include "../src/compiler/CodeGen.hpp"
#include <cstdlib>
#include "binaryen-c.h"
#include "wasm.hh"
#include <string>
#include <vector>

using namespace std;
using namespace Theta;

class WasmEngine {
public:
    wasm::own<wasm::Engine> engine;
    wasm::own<wasm::Store> store;

    // Constructor initializes engine and store only once
    WasmEngine() {
        engine = wasm::Engine::make();
        store = wasm::Store::make(engine.get());
    }

    // Provide access to the store
    wasm::Store* getStore() {
        return store.get();
    }

    // Provide access to the engine (if needed)
    wasm::Engine* getEngine() {
        return engine.get();
    }
};

// Create a singleton instance of WasmEngine
WasmEngine& getWasmEngine() {
    static WasmEngine wasmEngine;
    return wasmEngine;
}

class WasmExecutionContext {
public:
    wasm::Val result;
    vector<string> exportNames;

    WasmExecutionContext(wasm::Val result, vector<string> exportNames) : result(std::move(result)), exportNames(exportNames) {}
};

class CodeGenTest {
public:
    Lexer lexer;
    Parser parser;
    TypeChecker typeChecker;
    CodeGen codeGen;
    shared_ptr<map<string, string>> filesByCapsuleName;

    CodeGenTest() {
        filesByCapsuleName = Compiler::getInstance().filesByCapsuleName;
    }

    WasmExecutionContext setup(string source, string functionName = "main0") {
        cout << "in setup!" << endl;
        Compiler::getInstance().clearExceptions();

        BinaryenSetColorsEnabled(false);
        lexer.lex(source);

        shared_ptr<ASTNode> parsedAST = parser.parse(
            lexer.tokens,
            source,
            "fakeFile.th",
            filesByCapsuleName
        );

        Compiler::getInstance().optimizeAST(parsedAST, true);
        bool isTypeValid = typeChecker.checkAST(parsedAST);

        for (int i = 0; i < Compiler::getInstance().getEncounteredExceptions().size(); i++) {
            Compiler::getInstance().getEncounteredExceptions()[i]->display();
        }

        if (!isTypeValid) FAIL("Typechecking failed");

        BinaryenModuleRef module = codeGen.generateWasmFromAST(parsedAST);

        vector<char> buffer(4096);
        size_t written = BinaryenModuleWrite(module, buffer.data(), buffer.size());

        auto binary = wasm::vec<byte_t>::make_uninitialized(written);
        memcpy(binary.get(), buffer.data(), written);

        // Use the shared WasmEngine's store
        auto wasmModule = wasm::Module::make(getWasmEngine().getStore(), binary);
        if (!wasmModule) FAIL("Error compiling module");

        // Can pass imports in place of nullptr if needed
        auto instance = wasm::Instance::make(getWasmEngine().getStore(), wasmModule.get(), nullptr);
        if (!instance) FAIL("Error instantiating module");

        // Extract exports
        auto exports = instance->exports();
        std::vector<std::string> exportNames;
        for (size_t i = 0; i < exports.size(); ++i) {
            auto exportName = exports[i]->kind() == wasm::EXTERN_FUNC ? "function" : "unknown";
            exportNames.push_back(exportName);  // Add more specific names if available
        }

        // Check if the requested function exists in the exports
        wasm::Func* func = nullptr;
        for (size_t i = 0; i < exports.size(); ++i) {
            if (exports[i]->kind() == wasm::EXTERN_FUNC) {
                // We assume the function we're interested in is the first one
                func = exports[i]->func();
                break;
            }
        }

        if (!func) FAIL("Exported function not found");

        // Call the function with no arguments
        wasm::Val args[0];  // No arguments for this test
        wasm::Val results[1];  // A single result (the number returned by the function)
        auto trap = func->call(args, results);

        if (trap) FAIL("Error calling function");

        cout << "Made wasm execution!" << endl;

        return WasmExecutionContext(std::move(results[0]), exportNames);
    }
};

TEST_CASE_METHOD(CodeGenTest, "CodeGen") {
    SECTION("Can codegen multiplication") {
        WasmExecutionContext context = setup(R"(
            capsule Test {
                main<Function<Number>> = () -> 10 * 5
            }
        )");

        REQUIRE(context.exportNames.size() == 2);
        REQUIRE(context.result.kind() == wasm::I64);
        REQUIRE(context.result.i64() == 50);
    }

    SECTION("Can codegen addition") {
        WasmExecutionContext context = setup(R"(
            capsule Test {
                main<Function<Number>> = () -> 9 + 27
            }
        )");

        REQUIRE(context.exportNames.size() == 2);
        REQUIRE(context.result.kind() == wasm::I64);
        REQUIRE(context.result.i64() == 36);
    }

    SECTION("Can codegen division") {
        WasmExecutionContext context = setup(R"(
            capsule Test {
                main<Function<Number>> = () -> 10 / 2
            }
        )");

        REQUIRE(context.exportNames.size() == 2);
        REQUIRE(context.result.kind() == wasm::I64);
        REQUIRE(context.result.i64() == 5);
    }

    SECTION("Can codegen subtraction") {
        WasmExecutionContext context = setup(R"(
            capsule Test {
                main<Function<Number>> = () -> 47 - 10
            }
        )");

        REQUIRE(context.exportNames.size() == 2);
        REQUIRE(context.result.kind() == wasm::I64);
        REQUIRE(context.result.i64() == 37);
    }

    SECTION("Correctly codegens negative numbers") {
        WasmExecutionContext context = setup(R"(
            capsule Test {
                main<Function<Number>> = () -> -10 + 20
            }
        )");

        REQUIRE(context.exportNames.size() == 2);
        REQUIRE(context.result.kind() == wasm::I64);
        REQUIRE(context.result.i64() == 10);
    }

    SECTION("Negative multiplication outputs correct result") {
        WasmExecutionContext context = setup(R"(
            capsule Test {
                main<Function<Number>> = () -> 5 * -7
            }
        )");

        REQUIRE(context.exportNames.size() == 2);
        REQUIRE(context.result.kind() == wasm::I64);
        REQUIRE(context.result.i64() == -35);
    }

    SECTION("Negative division outputs correct result") {
        WasmExecutionContext context = setup(R"(
            capsule Test {
                main<Function<Number>> = () -> -90 / 30
            }
        )");

        REQUIRE(context.exportNames.size() == 2);
        REQUIRE(context.result.kind() == wasm::I64);
        REQUIRE(context.result.i64() == -3);
    }

    SECTION("More complex arithmetic outputs correct result") {
        WasmExecutionContext context = setup(R"(
            capsule Test {
                main<Function<Number>> = () -> 10 * (5 - 1) + (8 / (23 - 5))
            }
        )");

        REQUIRE(context.exportNames.size() == 2);
        REQUIRE(context.result.kind() == wasm::I64);
        // Note: this is 40 because all numbers are currently being treated as integers instead of
        // being typecast to floats. Once we fix this, it'll be 40.44
        REQUIRE(context.result.i64() == 40);
    }

    SECTION("Can codegen conditionals") {
         WasmExecutionContext context = setup(R"(
            capsule Test {
                main<Function<Number>> = () -> {
                    if (1 == 1) {
                        return 4
                    } else {
                        return 3
                    }
                }
            }
        )");

        REQUIRE(context.exportNames.size() == 2);
        REQUIRE(context.result.kind() == wasm::I64);
        REQUIRE(context.result.i64() == 4);
    }

    SECTION("Can codegen early returns") {
        WasmExecutionContext context = setup(R"(
            capsule Test {
                main<Function<Number>> = () -> {
                    if (1 == 1) {
                        return 10
                    }

                    return 5
                }
            }
        )");

        REQUIRE(context.exportNames.size() == 2);
        REQUIRE(context.result.kind() == wasm::I64);
        REQUIRE(context.result.i64() == 10);
    }

    SECTION("Can codegen with capsule variables") {
        WasmExecutionContext context = setup(R"(
            capsule Test {
                count<Number> = 11
                                          
                main<Function<Number>> = () -> {
                    return count + 1
                }
            }
        )");

        REQUIRE(context.exportNames.size() == 2);
        REQUIRE(context.result.kind() == wasm::I64);
        REQUIRE(context.result.i64() == 12);
    }

    SECTION("Can codegen with local variables") {
         WasmExecutionContext context = setup(R"(
            capsule Test {
                main<Function<Number>> = () -> {
                    x<Number> = 43

                    if (x == 12) {
                        return 10
                    }
                    
                    return 2
                }
            }
        )");

        REQUIRE(context.exportNames.size() == 2);
        REQUIRE(context.result.kind() == wasm::I64);
        REQUIRE(context.result.i64() == 2);
    }

    SECTION("Can call capsule functions") {
         WasmExecutionContext context = setup(R"(
            capsule Test {
                main<Function<Number>> = () -> double(5)

                double<Function<Number, Number>> = (x<Number>) -> x * 2
            }
        )");

        REQUIRE(context.exportNames.size() == 3);
        REQUIRE(context.result.kind() == wasm::I64);
        REQUIRE(context.result.i64() == 10);
    }

// TODO: Fix this test case. This is failing because of the unary comparison
// to i64.eqz that the !isOdd is doing. 
//
//    SECTION("Can call capsule functions that reference other functions") {
//        wasm_instance_t *instance = setup(R"(
//            capsule Test {
//                main<Function<Boolean>> = () -> isEven(5)
//
//                isEven<Function<Number, Boolean>> = (x<Number>) -> !isOdd(x)
//
//                isOdd<Function<Number, Boolean>> = (x<Number>) -> x % 3 == 0 
//            }
//        )");
//
//        wasm_extern_vec_t exports;
//        wasm_instance_exports(instance, &exports);
//
//        REQUIRE(exports.size == 3);
//
//        wasm_func_t *mainFunc = wasm_extern_as_func(exports.data[1]);
//
//        wasm_val_t args_val[0] = {};
//        wasm_val_t results_val[1] = { WASM_INIT_VAL };
//        wasm_val_vec_t args = WASM_ARRAY_VEC(args_val);
//        wasm_val_vec_t results = WASM_ARRAY_VEC(results_val);
//
//        wasm_func_call(mainFunc, &args, &results);
//
//        REQUIRE(results_val[0].of.i32 == 0);
//    }
    
    SECTION("Can call functions that are curried") {
         WasmExecutionContext context = setup(R"(
            capsule Test {
                main<Function<Number>> = () -> {
                    multiplyBy10<Function<Number, Number>> = curriedMultiply(10) 
                
                    return multiplyBy10(50)
                }

                curriedMultiply<Function<Number, Function<Number, Number>>> = (x<Number>) -> (y<Number>) -> x * y
            }
        )");

        REQUIRE(context.exportNames.size() == 3);
        REQUIRE(context.result.kind() == wasm::I64);
        REQUIRE(context.result.i64() == 500);
    }

    SECTION("Can call functions that have an internal function as part of its result") {
         WasmExecutionContext context = setup(R"(
            capsule Test {
                main<Function<Number>> = () -> add1000(5)

                add1000<Function<Number, Number>> = (x<Number>) -> {
                    add<Function<Number, Number>> = (y<Number>) -> x + y

                    result<Number> = add(1000)

                    return result
                }
            }
        )");

        REQUIRE(context.exportNames.size() == 3);
        REQUIRE(context.result.kind() == wasm::I64);
        REQUIRE(context.result.i64() == 1005);
    }

    SECTION("Correctly return value if an assignment is the last expression in a block") {
         WasmExecutionContext context = setup(R"(
            capsule Test {
                main<Function<Boolean>> = () -> {
                    x<Boolean> = false
                }
            }
        )");

        REQUIRE(context.exportNames.size() == 2);
        REQUIRE(context.result.kind() == wasm::I32);
        REQUIRE(context.result.i32() == 0);
    }

    SECTION("Can call recursive functions correctly") {
         WasmExecutionContext context = setup(R"(
            capsule Test {
                main<Function<Number>> = () -> {
                    fibonacci(10)
                }

                fibonacci<Function<Number, Number>> = (n<Number>) -> {
                    if (n <= 1) {
                        return n
                    }

                    fibonacci(n - 1) + fibonacci(n - 2)
                }
            }
        )");

        REQUIRE(context.exportNames.size() == 3);
        REQUIRE(context.result.kind() == wasm::I64);
        REQUIRE(context.result.i64() == 55);
    }
}

