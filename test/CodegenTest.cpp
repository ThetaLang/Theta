#define CATCH_CONFIG_MAIN
#include "catch2/catch_amalgamated.hpp"
#include "../src/lexer/Lexer.cpp"
#include "../src/parser/Parser.cpp"
#include "../src/compiler/Compiler.hpp"
#include "../src/compiler/TypeChecker.hpp"
#include "../src/compiler/CodeGen.hpp"
#include "runtime/Runtime.hpp"
#include "binaryen-c.h"
#include "wasm.hh"
#include <v8.h>
#include <string>
#include <vector>

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

    ExecutionContext setup(string source, string functionName = "main0") {
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

        vector<char> buffer = Compiler::writeModuleToBuffer(module);

        return Runtime::getInstance().execute(buffer, functionName);
    }
};

TEST_CASE_METHOD(CodeGenTest, "CodeGen") {
    SECTION("Can codegen multiplication") {
        ExecutionContext context = setup(R"(
            capsule Test {
                main<Function<Number>> = () -> 10 * 5
            }
        )");

        REQUIRE(context.exportNames.size() >= 2);
        REQUIRE(context.result.kind() == wasm::I64);
        REQUIRE(context.result.i64() == 50);
    }

    SECTION("Can codegen addition") {
        ExecutionContext context = setup(R"(
            capsule Test {
                main<Function<Number>> = () -> 9 + 27
            }
        )");

        REQUIRE(context.exportNames.size() >= 2);
        REQUIRE(context.result.kind() == wasm::I64);
        REQUIRE(context.result.i64() == 36);
    }

    SECTION("Can codegen division") {
        ExecutionContext context = setup(R"(
            capsule Test {
                main<Function<Number>> = () -> 10 / 2
            }
        )");

        REQUIRE(context.exportNames.size() >= 2);
        REQUIRE(context.result.kind() == wasm::I64);
        REQUIRE(context.result.i64() == 5);
    }

    SECTION("Can codegen subtraction") {
        ExecutionContext context = setup(R"(
            capsule Test {
                main<Function<Number>> = () -> 47 - 10
            }
        )");

        REQUIRE(context.exportNames.size() >= 2);
        REQUIRE(context.result.kind() == wasm::I64);
        REQUIRE(context.result.i64() == 37);
    }

    // TODO: Fix this test
    // 
    //SECTION("Can codegen string concatenation") {
    //    ExecutionContext context = setup(R"(
    //        capsule Test {
    //            main<Function<String>> = () -> 'Hello, ' + 'world!'
    //        }
    //    )");

    //    REQUIRE(context.exportNames.size() == 2);
    //    REQUIRE(context.result.kind() == wasm::ANYREF);

    //    // Cast the wasm::Ref to a v8::Value.
    //    wasm::Ref* ref = context.result.ref();
    //    REQUIRE(ref != nullptr);  // Ensure it's a valid reference.

    //    // Convert the wasm::Ref to a v8::Value (V8 object).
    //    v8::Isolate* isolate = v8::Isolate::GetCurrent();
    //    v8::HandleScope handle_scope(isolate);
    //    v8::Local<v8::Value> v8_value = *reinterpret_cast<v8::Local<v8::Value>*>(ref);

    //    // Check if the value is a string.
    //    REQUIRE(v8_value->IsString());  // Make sure the result is a string.

    //    // Convert the v8::String to std::string.
    //    v8::String::Utf8Value utf8_string(isolate, v8_value);
    //    std::string result_string(*utf8_string);

    //    // Check if the result matches the expected value.
    //    REQUIRE(result_string == "Hello, world!");
    //}

    SECTION("Correctly codegens negative numbers") {
        ExecutionContext context = setup(R"(
            capsule Test {
                main<Function<Number>> = () -> -10 + 20
            }
        )");

        REQUIRE(context.exportNames.size() >= 2);
        REQUIRE(context.result.kind() == wasm::I64);
        REQUIRE(context.result.i64() == 10);
    }

    SECTION("Negative multiplication outputs correct result") {
        ExecutionContext context = setup(R"(
            capsule Test {
                main<Function<Number>> = () -> 5 * -7
            }
        )");

        REQUIRE(context.exportNames.size() >= 2);
        REQUIRE(context.result.kind() == wasm::I64);
        REQUIRE(context.result.i64() == -35);
    }

    SECTION("Negative division outputs correct result") {
        ExecutionContext context = setup(R"(
            capsule Test {
                main<Function<Number>> = () -> -90 / 30
            }
        )");

        REQUIRE(context.exportNames.size() >= 2);
        REQUIRE(context.result.kind() == wasm::I64);
        REQUIRE(context.result.i64() == -3);
    }

    SECTION("More complex arithmetic outputs correct result") {
        ExecutionContext context = setup(R"(
            capsule Test {
                main<Function<Number>> = () -> 10 * (5 - 1) + (8 / (23 - 5))
            }
        )");

        REQUIRE(context.exportNames.size() >= 2);
        REQUIRE(context.result.kind() == wasm::I64);
        // Note: this is 40 because all numbers are currently being treated as integers instead of
        // being typecast to floats. Once we fix this, it'll be 40.44
        REQUIRE(context.result.i64() == 40);
    }

    SECTION("Can codegen conditionals") {
         ExecutionContext context = setup(R"(
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

        REQUIRE(context.exportNames.size() >= 2);
        REQUIRE(context.result.kind() == wasm::I64);
        REQUIRE(context.result.i64() == 4);
    }

    SECTION("Can codegen early returns") {
        ExecutionContext context = setup(R"(
            capsule Test {
                main<Function<Number>> = () -> {
                    if (1 == 1) {
                        return 10
                    }

                    return 5
                }
            }
        )");

        REQUIRE(context.exportNames.size() >= 2);
        REQUIRE(context.result.kind() == wasm::I64);
        REQUIRE(context.result.i64() == 10);
    }

    SECTION("Can codegen with capsule variables") {
        ExecutionContext context = setup(R"(
            capsule Test {
                count<Number> = 11
                                          
                main<Function<Number>> = () -> {
                    return count + 1
                }
            }
        )");

        REQUIRE(context.exportNames.size() >= 2);
        REQUIRE(context.result.kind() == wasm::I64);
        REQUIRE(context.result.i64() == 12);
    }

    SECTION("Can codegen with local variables") {
         ExecutionContext context = setup(R"(
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

        REQUIRE(context.exportNames.size() >= 2);
        REQUIRE(context.result.kind() == wasm::I64);
        REQUIRE(context.result.i64() == 2);
    }

    SECTION("Can call capsule functions") {
         ExecutionContext context = setup(R"(
            capsule Test {
                main<Function<Number>> = () -> double(5)

                double<Function<Number, Number>> = (x<Number>) -> x * 2
            }
        )");

        REQUIRE(context.exportNames.size() >= 3);
        REQUIRE(context.result.kind() == wasm::I64);
        REQUIRE(context.result.i64() == 10);
    }

// TODO: Fix this test case. This is failing because of the unary comparison
// to i64.eqz that the !isOdd is doing. 
//
//    SECTION("Can call capsule functions that reference other functions") {
//        ExecutionContext context = setup(R"(
//            capsule Test {
//                main<Function<Boolean>> = () -> isEven(5)
//
//                isEven<Function<Number, Boolean>> = (x<Number>) -> !isOdd(x)
//
//                isOdd<Function<Number, Boolean>> = (x<Number>) -> x % 3 == 0 
//            }
//        )");
//
//        REQUIRE(context.exportNames.size() == 3);
//        REQUIRE(context.result.kind() == wasm::I32);
//        REQUIRE(context.result.i32() == 0);
//    }
    
    SECTION("Can call functions that are curried") {
         ExecutionContext context = setup(R"(
            capsule Test {
                main<Function<Number>> = () -> {
                    multiplyBy10<Function<Number, Number>> = curriedMultiply(10) 
                
                    return multiplyBy10(50)
                }

                curriedMultiply<Function<Number, Function<Number, Number>>> = (x<Number>) -> (y<Number>) -> x * y
            }
        )");

        REQUIRE(context.exportNames.size() >= 3);
        REQUIRE(context.result.kind() == wasm::I64);
        REQUIRE(context.result.i64() == 500);
    }

    SECTION("Can call functions that have an internal function as part of its result") {
         ExecutionContext context = setup(R"(
            capsule Test {
                main<Function<Number>> = () -> add1000(5)

                add1000<Function<Number, Number>> = (x<Number>) -> {
                    add<Function<Number, Number>> = (y<Number>) -> x + y

                    result<Number> = add(1000)

                    return result
                }
            }
        )");

        REQUIRE(context.exportNames.size() >= 3);
        REQUIRE(context.result.kind() == wasm::I64);
        REQUIRE(context.result.i64() == 1005);
    }

    SECTION("Correctly return value if an assignment is the last expression in a block") {
         ExecutionContext context = setup(R"(
            capsule Test {
                main<Function<Boolean>> = () -> {
                    x<Boolean> = false
                }
            }
        )");

        REQUIRE(context.exportNames.size() >= 2);
        REQUIRE(context.result.kind() == wasm::I32);
        REQUIRE(context.result.i32() == 0);
    }

    SECTION("Can call recursive functions correctly") {
         ExecutionContext context = setup(R"(
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

        REQUIRE(context.exportNames.size() >= 3);
        REQUIRE(context.result.kind() == wasm::I64);
        REQUIRE(context.result.i64() == 55);
    }
}

