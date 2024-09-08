#define Catch Catch_Wasmer // Both wasmer and catch2 have an identifier "Catch". This fixes the naming collision
#include <wasmer.h>
#undef Catch
#define CATCH_CONFIG_MAIN
#include "catch2/catch_amalgamated.hpp"
#include "../src/lexer/Lexer.cpp"
#include "../src/parser/Parser.cpp"
#include "../src/compiler/Compiler.hpp"
#include "../src/compiler/TypeChecker.hpp"
#include "../src/compiler/CodeGen.hpp"
#include <cstdlib>
#include "binaryen-c.h"

using namespace std;
using namespace Theta;

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

    wasm_instance_t* setup(string source) {
        Compiler::getInstance().clearExceptions();

        // IMPORTANT: This disables binaryen from outputting colors along with its print output.
        // If we don't disable that, wasmer will try to parse the escape sequences as part of the
        // module and will fail
        BinaryenSetColorsEnabled(false);

        lexer.lex(source);

        shared_ptr<ASTNode> parsedAST = parser.parse(
            lexer.tokens,
            source,
            "fakeFile.th",
            filesByCapsuleName
        );

        Compiler::getInstance().optimizeAST(parsedAST, true);
        typeChecker.checkAST(parsedAST);

        BinaryenModuleRef module = codeGen.generateWasmFromAST(parsedAST);

        vector<char> buffer(4096);
        size_t written = BinaryenModuleWrite(module, buffer.data(), buffer.size());
        
        wasm_byte_vec_t wasm;
        wasm_byte_vec_new(&wasm, written, buffer.data());

        wasm_engine_t *engine = wasm_engine_new();
        wasm_store_t *store = wasm_store_new(engine);

        wasm_module_t *wasmerModule = wasm_module_new(store, &wasm);

        if (!wasmerModule) {
            cout << "Error compiling module in wasmer" << endl;
            exit(1);
        }

        wasm_byte_vec_delete(&wasm);

        wasm_extern_vec_t importObject = WASM_EMPTY_VEC;
        wasm_instance_t *instance = wasm_instance_new(store, wasmerModule, &importObject, NULL);

        if (!instance) {
            cout << "Error instantiating module" << endl;
            exit(1);
        }

        return instance;
    } 
};

TEST_CASE_METHOD(CodeGenTest, "CodeGen") {
    SECTION("Can codegen multiplication") {
        wasm_instance_t *instance = setup(R"(
            capsule Test {
                main<Function<Number>> = () -> 10 + 5
            }
        )");

        wasm_extern_vec_t exports;
        wasm_instance_exports(instance, &exports);

        REQUIRE(exports.size == 2);

        wasm_func_t *mainFunc = wasm_extern_as_func(exports.data[1]);

        wasm_val_t args_val[0] = {};
        wasm_val_t results_val[1] = { WASM_INIT_VAL };
        wasm_val_vec_t args = WASM_ARRAY_VEC(args_val);
        wasm_val_vec_t results = WASM_ARRAY_VEC(results_val);

        wasm_func_call(mainFunc, &args, &results);

        REQUIRE(results_val[0].of.i64 == 15);
    }

    SECTION("Can codegen addition") {
        wasm_instance_t *instance = setup(R"(
            capsule Test {
                main<Function<Number>> = () -> 9 + 27
            }
        )");

        wasm_extern_vec_t exports;
        wasm_instance_exports(instance, &exports);

        REQUIRE(exports.size == 2);

        wasm_func_t *mainFunc = wasm_extern_as_func(exports.data[1]);

        wasm_val_t args_val[0] = {};
        wasm_val_t results_val[1] = { WASM_INIT_VAL };
        wasm_val_vec_t args = WASM_ARRAY_VEC(args_val);
        wasm_val_vec_t results = WASM_ARRAY_VEC(results_val);

        wasm_func_call(mainFunc, &args, &results);

        REQUIRE(results_val[0].of.i64 == 36);
    }

    SECTION("Can codegen division") {
        wasm_instance_t *instance = setup(R"(
            capsule Test {
                main<Function<Number>> = () -> 10 / 2
            }
        )");

        wasm_extern_vec_t exports;
        wasm_instance_exports(instance, &exports);

        REQUIRE(exports.size == 2);

        wasm_func_t *mainFunc = wasm_extern_as_func(exports.data[1]);

        wasm_val_t args_val[0] = {};
        wasm_val_t results_val[1] = { WASM_INIT_VAL };
        wasm_val_vec_t args = WASM_ARRAY_VEC(args_val);
        wasm_val_vec_t results = WASM_ARRAY_VEC(results_val);

        wasm_func_call(mainFunc, &args, &results);

        REQUIRE(results_val[0].of.i64 == 5);
    }

    SECTION("Can codegen subtraction") {
        wasm_instance_t *instance = setup(R"(
            capsule Test {
                main<Function<Number>> = () -> 47 - 10
            }
        )");

        wasm_extern_vec_t exports;
        wasm_instance_exports(instance, &exports);

        REQUIRE(exports.size == 2);

        wasm_func_t *mainFunc = wasm_extern_as_func(exports.data[1]);

        wasm_val_t args_val[0] = {};
        wasm_val_t results_val[1] = { WASM_INIT_VAL };
        wasm_val_vec_t args = WASM_ARRAY_VEC(args_val);
        wasm_val_vec_t results = WASM_ARRAY_VEC(results_val);

        wasm_func_call(mainFunc, &args, &results);

        REQUIRE(results_val[0].of.i64 == 37);
    }

    SECTION("Correctly codegens negative numbers") {
        wasm_instance_t *instance = setup(R"(
            capsule Test {
                main<Function<Number>> = () -> -10 + 20
            }
        )");

        wasm_extern_vec_t exports;
        wasm_instance_exports(instance, &exports);

        REQUIRE(exports.size == 2);

        wasm_func_t *mainFunc = wasm_extern_as_func(exports.data[1]);

        wasm_val_t args_val[0] = {};
        wasm_val_t results_val[1] = { WASM_INIT_VAL };
        wasm_val_vec_t args = WASM_ARRAY_VEC(args_val);
        wasm_val_vec_t results = WASM_ARRAY_VEC(results_val);

        wasm_func_call(mainFunc, &args, &results);

        REQUIRE(results_val[0].of.i64 == 10);
    }

    SECTION("Negative multiplication outputs correct result") {
        wasm_instance_t *instance = setup(R"(
            capsule Test {
                main<Function<Number>> = () -> 5 * -7
            }
        )");

        wasm_extern_vec_t exports;
        wasm_instance_exports(instance, &exports);

        REQUIRE(exports.size == 2);

        wasm_func_t *mainFunc = wasm_extern_as_func(exports.data[1]);

        wasm_val_t args_val[0] = {};
        wasm_val_t results_val[1] = { WASM_INIT_VAL };
        wasm_val_vec_t args = WASM_ARRAY_VEC(args_val);
        wasm_val_vec_t results = WASM_ARRAY_VEC(results_val);

        wasm_func_call(mainFunc, &args, &results);

        REQUIRE(results_val[0].of.i64 == -35);
    }


    SECTION("Negative division outputs correct result") {
        wasm_instance_t *instance = setup(R"(
            capsule Test {
                main<Function<Number>> = () -> -90 / 30
            }
        )");

        wasm_extern_vec_t exports;
        wasm_instance_exports(instance, &exports);

        REQUIRE(exports.size == 2);

        wasm_func_t *mainFunc = wasm_extern_as_func(exports.data[1]);

        wasm_val_t args_val[0] = {};
        wasm_val_t results_val[1] = { WASM_INIT_VAL };
        wasm_val_vec_t args = WASM_ARRAY_VEC(args_val);
        wasm_val_vec_t results = WASM_ARRAY_VEC(results_val);

        wasm_func_call(mainFunc, &args, &results);

        REQUIRE(results_val[0].of.i64 == -3);
    }

    SECTION("More complex arithmetic outputs correct retult") {
        wasm_instance_t *instance = setup(R"(
            capsule Test {
                main<Function<Number>> = () -> 10 * (5 - 1) + (8 / (23 - 5))
            }
        )");

        wasm_extern_vec_t exports;
        wasm_instance_exports(instance, &exports);

        REQUIRE(exports.size == 2);

        wasm_func_t *mainFunc = wasm_extern_as_func(exports.data[1]);

        wasm_val_t args_val[0] = {};
        wasm_val_t results_val[1] = { WASM_INIT_VAL };
        wasm_val_vec_t args = WASM_ARRAY_VEC(args_val);
        wasm_val_vec_t results = WASM_ARRAY_VEC(results_val);

        wasm_func_call(mainFunc, &args, &results);

        // Note: this is 40 because all numbers are currently being treated as integers instead of
        // being typecast to floats. Once we fix this, it'll be 40.44
        REQUIRE(results_val[0].of.i64 == 40);
    }

    SECTION("Can codegen conditionals") {
         wasm_instance_t *instance = setup(R"(
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

        wasm_extern_vec_t exports;
        wasm_instance_exports(instance, &exports);

        REQUIRE(exports.size == 2);

        wasm_func_t *mainFunc = wasm_extern_as_func(exports.data[1]);

        wasm_val_t args_val[0] = {};
        wasm_val_t results_val[1] = { WASM_INIT_VAL };
        wasm_val_vec_t args = WASM_ARRAY_VEC(args_val);
        wasm_val_vec_t results = WASM_ARRAY_VEC(results_val);

        wasm_func_call(mainFunc, &args, &results);

        REQUIRE(results_val[0].of.i64 == 4);
    }

    SECTION("Can codegen early returns") {
        wasm_instance_t *instance = setup(R"(
            capsule Test {
                main<Function<Number>> = () -> {
                    if (1 == 1) {
                        return 10
                    }

                    return 5
                }
            }
        )");

        wasm_extern_vec_t exports;
        wasm_instance_exports(instance, &exports);

        REQUIRE(exports.size == 2);

        wasm_func_t *mainFunc = wasm_extern_as_func(exports.data[1]);

        wasm_val_t args_val[0] = {};
        wasm_val_t results_val[1] = { WASM_INIT_VAL };
        wasm_val_vec_t args = WASM_ARRAY_VEC(args_val);
        wasm_val_vec_t results = WASM_ARRAY_VEC(results_val);

        wasm_func_call(mainFunc, &args, &results);

        REQUIRE(results_val[0].of.i64 == 10);
    }

    SECTION("Can codegen with capsule variables") {
        wasm_instance_t *instance = setup(R"(
            capsule Test {
                count<Number> = 11
                                          
                main<Function<Number>> = () -> {
                    return count + 1
                }
            }
        )");

        wasm_extern_vec_t exports;
        wasm_instance_exports(instance, &exports);

        REQUIRE(exports.size == 2);

        wasm_func_t *mainFunc = wasm_extern_as_func(exports.data[1]);

        wasm_val_t args_val[0] = {};
        wasm_val_t results_val[1] = { WASM_INIT_VAL };
        wasm_val_vec_t args = WASM_ARRAY_VEC(args_val);
        wasm_val_vec_t results = WASM_ARRAY_VEC(results_val);

        wasm_func_call(mainFunc, &args, &results);

        REQUIRE(results_val[0].of.i64 == 12);
    }

    SECTION("Can codegen with local variables") {
         wasm_instance_t *instance = setup(R"(
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

        wasm_extern_vec_t exports;
        wasm_instance_exports(instance, &exports);

        REQUIRE(exports.size == 2);

        wasm_func_t *mainFunc = wasm_extern_as_func(exports.data[1]);

        wasm_val_t args_val[0] = {};
        wasm_val_t results_val[1] = { WASM_INIT_VAL };
        wasm_val_vec_t args = WASM_ARRAY_VEC(args_val);
        wasm_val_vec_t results = WASM_ARRAY_VEC(results_val);

        wasm_func_call(mainFunc, &args, &results);

        REQUIRE(results_val[0].of.i64 == 2);
    }

    SECTION("Can call capsule functions") {
         wasm_instance_t *instance = setup(R"(
            capsule Test {
                main<Function<Number>> = () -> double(5)

                double<Function<Number, Number>> = (x<Number>) -> x * 2
            }
        )");

        wasm_extern_vec_t exports;
        wasm_instance_exports(instance, &exports);

        REQUIRE(exports.size == 3);

        wasm_func_t *mainFunc = wasm_extern_as_func(exports.data[1]);

        wasm_val_t args_val[0] = {};
        wasm_val_t results_val[1] = { WASM_INIT_VAL };
        wasm_val_vec_t args = WASM_ARRAY_VEC(args_val);
        wasm_val_vec_t results = WASM_ARRAY_VEC(results_val);

        wasm_func_call(mainFunc, &args, &results);

        REQUIRE(results_val[0].of.i64 == 10);
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
         wasm_instance_t *instance = setup(R"(
            capsule Test {
                main<Function<Number>> = () -> {
                    multiplyBy10<Function<Number, Number>> = curriedMultiply(10) 
                
                    return multiplyBy10(50)
                }

                curriedMultiply<Function<Number, Function<Number, Number>>> = (x<Number>) -> (y<Number>) -> x * y
            }
        )");

        wasm_extern_vec_t exports;
        wasm_instance_exports(instance, &exports);

        REQUIRE(exports.size == 3);

        wasm_func_t *mainFunc = wasm_extern_as_func(exports.data[1]);

        wasm_val_t args_val[0] = {};
        wasm_val_t results_val[1] = { WASM_INIT_VAL };
        wasm_val_vec_t args = WASM_ARRAY_VEC(args_val);
        wasm_val_vec_t results = WASM_ARRAY_VEC(results_val);

        wasm_func_call(mainFunc, &args, &results);

        REQUIRE(results_val[0].of.i64 == 500);
    }

    SECTION("Can call functions that have an internal function as part of its result") {
         wasm_instance_t *instance = setup(R"(
            capsule Test {
                main<Function<Number>> = () -> add1000(5)

                add1000<Function<Number, Number>> = (x<Number>) -> {
                    add<Function<Number, Number>> = (y<Number>) -> x + y

                    result<Number> = add(1000)

                    return result
                }
            }
        )");

        wasm_extern_vec_t exports;
        wasm_instance_exports(instance, &exports);

        REQUIRE(exports.size == 3);

        wasm_func_t *mainFunc = wasm_extern_as_func(exports.data[1]);

        wasm_val_t args_val[0] = {};
        wasm_val_t results_val[1] = { WASM_INIT_VAL };
        wasm_val_vec_t args = WASM_ARRAY_VEC(args_val);
        wasm_val_vec_t results = WASM_ARRAY_VEC(results_val);

        wasm_func_call(mainFunc, &args, &results);

        REQUIRE(results_val[0].of.i64 == 1005);
    }

    // TODO: Return types of assignments are not being typechecked correctly
    SECTION("Correctly return value if an assignment is the last expression in a block") {
         wasm_instance_t *instance = setup(R"(
            capsule Test {
                main<Function<Boolean>> = () -> {
                    x<Boolean> = false
                }
            }
        )");

        wasm_extern_vec_t exports;
        wasm_instance_exports(instance, &exports);

        REQUIRE(exports.size == 3);

        wasm_func_t *mainFunc = wasm_extern_as_func(exports.data[1]);

        wasm_val_t args_val[0] = {};
        wasm_val_t results_val[1] = { WASM_INIT_VAL };
        wasm_val_vec_t args = WASM_ARRAY_VEC(args_val);
        wasm_val_vec_t results = WASM_ARRAY_VEC(results_val);

        wasm_func_call(mainFunc, &args, &results);

        REQUIRE(results_val[0].of.i32 == 0);
    }
}

