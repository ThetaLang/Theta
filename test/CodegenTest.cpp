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

class WasmExecutionContext {
public:
    wasm::Val result;                 // Updated to use wasm::Val from libwee8
    vector<string> exportNames;       // List of export names from the Wasm module

    // Constructor to initialize result and exportNames
    WasmExecutionContext(wasm::Val result, vector<string> exportNames)
        : result(result), exportNames(exportNames) {}

    // Check if the result is a number
    bool isNumber() {
        return result.kind() == wasm::I32 || result.kind() == wasm::I64 || 
               result.kind() == wasm::F32 || result.kind() == wasm::F64;
    }

    // Check if the result is a BigInt (not supported directly in libwee8, using I64 instead)
    bool isBigInt() {
        return result.kind() == wasm::I64;
    }

    // Check if the result is a string (WebAssembly doesn't natively return strings, so this is more hypothetical)
    bool isString() {
        // Strings are not natively supported in Wasm exports, this would be hypothetical
        return false;
    }

    // Retrieve the result as a number (for I32, I64, F32, F64 types)
    double getNumberValue() {
        if (result.kind() == wasm::I32) {
            return result.i32();
        } else if (result.kind() == wasm::I64) {
            return static_cast<double>(result.i64());  // Cast I64 to double
        } else if (result.kind() == wasm::F32) {
            return result.f32();
        } else if (result.kind() == wasm::F64) {
            return result.f64();
        }
        throw runtime_error("Result is not a number");
    }

    // Retrieve the result as a BigInt (int64_t)
    int64_t getBigIntValue() {
        if (isBigInt()) {
            return result.i64();
        }
        throw runtime_error("Result is not a bigint");
    }

    // Retrieve the result as a string (not applicable for native Wasm exports)
    string getStringValue() {
        throw runtime_error("Result is not a string");
    }
};

class CodeGenTest {
public:
    Lexer lexer;
    Parser parser;
    TypeChecker typeChecker;
    CodeGen codeGen;
    shared_ptr<map<string, string>> filesByCapsuleName;
    wasm::own<wasm::Engine> engine;
    wasm::own<wasm::Store> store;

    CodeGenTest() {
        filesByCapsuleName = Compiler::getInstance().filesByCapsuleName;
        
        // Initialize libwee8 engine and store
        engine = wasm::Engine::make();
        store = wasm::Store::make(engine.get());
    }

    WasmExecutionContext setup(string source, string functionName = "main0") {
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

        if (!isTypeValid) {
            cerr << "Typechecking failed" << endl;
            return WasmExecutionContext(nullptr, {}, nullptr);
        }

        BinaryenModuleRef module = codeGen.generateWasmFromAST(parsedAST);

        // Serialize the WebAssembly module
        vector<char> buffer(4096);
        size_t written = BinaryenModuleWrite(module, buffer.data(), buffer.size());

        // Load binary into libwee8
        auto binary = wasm::vec<byte_t>::make_uninitialized(written);
        memcpy(binary.get(), buffer.data(), written);

        // Compile the WebAssembly module
        auto wasmModule = wasm::Module::make(store.get(), binary);
        if (!wasmModule) {
            cerr << "> Error compiling module!" << endl;
            return WasmExecutionContext(nullptr, {}, nullptr);
        }

        // Instantiate the module
        auto instance = wasm::Instance::make(store.get(), wasmModule.get(), nullptr);
        if (!instance) {
            cerr << "> Error instantiating module!" << endl;
            return WasmExecutionContext(nullptr, {}, nullptr);
        }

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

        if (!func) {
            cerr << "Exported function not found" << endl;
            return WasmExecutionContext(nullptr, exportNames, nullptr);
        }

        // Call the function with no arguments
        wasm::Val args[0];  // No arguments for this test
        wasm::Val results[1];  // A single result (the number returned by the function)
        auto trap = func->call(args, results);

        if (trap) {
            cerr << "> Error calling function!" << endl;
            return WasmExecutionContext(nullptr, exportNames, nullptr);
        }

        // Return the result and the exports
        auto result_value = results[0].i32();  // Assuming it's a 32-bit integer result
        return WasmExecutionContext(result_value, exportNames, store.get());
    }
};

TEST_CASE_METHOD(CodeGenTest, "CodeGen") {
    SECTION("Can codegen multiplication") {
        WasmExecutionContext result = setup(R"(
            capsule Test {
                main<Function<Number>> = () -> 10 * 5
            }
        )");

        REQUIRE(result.exportNames.size() == 2);
        REQUIRE(result.isBigInt());
        REQUIRE(result.getBigIntValue() == 50);
    }

    SECTION("Can codegen addition") {
        WasmExecutionContext result = setup(R"(
            capsule Test {
                main<Function<Number>> = () -> 9 + 27
            }
        )");

        REQUIRE(result.exportNames.size() == 2);
        REQUIRE(result.isBigInt());
        REQUIRE(result.getBigIntValue() == 36);
    }

    SECTION("Can codegen division") {
        WasmExecutionContext result = setup(R"(
            capsule Test {
                main<Function<Number>> = () -> 10 / 2
            }
        )");

        REQUIRE(result.exportNames.size() == 2);
        REQUIRE(result.isBigInt());
        REQUIRE(result.getBigIntValue() == 5);
    }

    SECTION("Can codegen subtraction") {
        WasmExecutionContext result = setup(R"(
            capsule Test {
                main<Function<Number>> = () -> 47 - 10
            }
        )");

        REQUIRE(result.exportNames.size() == 2);
        REQUIRE(result.isBigInt());
        REQUIRE(result.getBigIntValue() == 37);
    }

    SECTION("Correctly codegens negative numbers") {
        WasmExecutionContext result = setup(R"(
            capsule Test {
                main<Function<Number>> = () -> -10 + 20
            }
        )");

        REQUIRE(result.exportNames.size() == 2);
        REQUIRE(result.isBigInt());
        REQUIRE(result.getBigIntValue() == 10);
    }

    SECTION("Negative multiplication outputs correct result") {
        WasmExecutionContext result = setup(R"(
            capsule Test {
                main<Function<Number>> = () -> 5 * -7
            }
        )");

        REQUIRE(result.exportNames.size() == 2);
        REQUIRE(result.isBigInt());
        REQUIRE(result.getBigIntValue() == -35);
    }

    SECTION("Negative division outputs correct result") {
        WasmExecutionContext result = setup(R"(
            capsule Test {
                main<Function<Number>> = () -> -90 / 30
            }
        )");

        REQUIRE(result.exportNames.size() == 2);
        REQUIRE(result.isBigInt());
        REQUIRE(result.getBigIntValue() == -3);
    }

    SECTION("More complex arithmetic outputs correct result") {
        WasmExecutionContext result = setup(R"(
            capsule Test {
                main<Function<Number>> = () -> 10 * (5 - 1) + (8 / (23 - 5))
            }
        )");

        REQUIRE(result.exportNames.size() == 2);
        REQUIRE(result.isBigInt());
        // Note: this is 40 because all numbers are currently being treated as integers instead of
        // being typecast to floats. Once we fix this, it'll be 40.44
        REQUIRE(result.getBigIntValue() == 40);
    }

    SECTION("Can codegen conditionals") {
         WasmExecutionContext result = setup(R"(
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

        REQUIRE(result.exportNames.size() == 2);
        REQUIRE(result.isBigInt());
        REQUIRE(result.getBigIntValue() == 4);
    }

    SECTION("Can codegen early returns") {
        WasmExecutionContext result = setup(R"(
            capsule Test {
                main<Function<Number>> = () -> {
                    if (1 == 1) {
                        return 10
                    }

                    return 5
                }
            }
        )");

        REQUIRE(result.exportNames.size() == 2);
        REQUIRE(result.isBigInt());
        REQUIRE(result.getBigIntValue() == 10);
    }

    SECTION("Can codegen with capsule variables") {
        WasmExecutionContext result = setup(R"(
            capsule Test {
                count<Number> = 11
                                          
                main<Function<Number>> = () -> {
                    return count + 1
                }
            }
        )");

        REQUIRE(result.exportNames.size() == 2);
        REQUIRE(result.isBigInt());
        REQUIRE(result.getBigIntValue() == 12);
    }

    SECTION("Can codegen with local variables") {
         WasmExecutionContext result = setup(R"(
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

        REQUIRE(result.exportNames.size() == 2);
        REQUIRE(result.isBigInt());
        REQUIRE(result.getBigIntValue() == 2);
    }

    SECTION("Can call capsule functions") {
         WasmExecutionContext result = setup(R"(
            capsule Test {
                main<Function<Number>> = () -> double(5)

                double<Function<Number, Number>> = (x<Number>) -> x * 2
            }
        )");

        REQUIRE(result.exportNames.size() == 3);
        REQUIRE(result.isBigInt());
        REQUIRE(result.getBigIntValue() == 10);
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
         WasmExecutionContext result = setup(R"(
            capsule Test {
                main<Function<Number>> = () -> {
                    multiplyBy10<Function<Number, Number>> = curriedMultiply(10) 
                
                    return multiplyBy10(50)
                }

                curriedMultiply<Function<Number, Function<Number, Number>>> = (x<Number>) -> (y<Number>) -> x * y
            }
        )");

        REQUIRE(result.exportNames.size() == 3);
        REQUIRE(result.isBigInt());
        REQUIRE(result.getBigIntValue() == 500);
    }

    SECTION("Can call functions that have an internal function as part of its result") {
         WasmExecutionContext result = setup(R"(
            capsule Test {
                main<Function<Number>> = () -> add1000(5)

                add1000<Function<Number, Number>> = (x<Number>) -> {
                    add<Function<Number, Number>> = (y<Number>) -> x + y

                    result<Number> = add(1000)

                    return result
                }
            }
        )");

        REQUIRE(result.exportNames.size() == 3);
        REQUIRE(result.isBigInt());
        REQUIRE(result.getBigIntValue() == 1005);
    }

    SECTION("Correctly return value if an assignment is the last expression in a block") {
         WasmExecutionContext result = setup(R"(
            capsule Test {
                main<Function<Boolean>> = () -> {
                    x<Boolean> = false
                }
            }
        )");

        REQUIRE(result.exportNames.size() == 2);
        REQUIRE(result.isNumber());
        REQUIRE(result.getNumberValue() == 0);
    }

    SECTION("Can call recursive functions correctly") {
         WasmExecutionContext result = setup(R"(
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

        REQUIRE(result.exportNames.size() == 3);
        REQUIRE(result.isBigInt());
        REQUIRE(result.getBigIntValue() == 55);
    }
}

