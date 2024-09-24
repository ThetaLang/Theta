#include "v8-typed-array.h"
#include "v8-wasm.h"
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
#include <v8.h>
#include <libplatform/libplatform.h>

#include "v8-context.h"
#include "v8-initialization.h"
#include "v8-isolate.h"
#include "v8-local-handle.h"
#include "v8-primitive.h"
#include "v8-script.h"

using namespace std;
using namespace Theta;

struct V8GlobalSetup {
  unique_ptr<v8::Platform> platform;

  V8GlobalSetup() {
    string pwd = Compiler::resolveAbsolutePath("");
    v8::V8::InitializeICUDefaultLocation(pwd.c_str());
    v8::V8::InitializeExternalStartupData(pwd.c_str());

    platform = v8::platform::NewDefaultPlatform();
    v8::V8::InitializePlatform(platform.get());
    v8::V8::Initialize();
  }

  ~V8GlobalSetup() {
    v8::V8::Dispose();
    v8::V8::DisposePlatform();
  }
};

V8GlobalSetup v8GlobalSetup;

#include <v8.h>
#include <string>
#include <vector>

class WasmExecutionContext {
public:
    v8::Local<v8::Value> result;
    std::vector<std::string> exportNames;
    v8::Isolate* isolate;

    WasmExecutionContext(v8::Local<v8::Value> result, std::vector<std::string> exportNames, v8::Isolate* isolate)
        : result(result), exportNames(exportNames), isolate(isolate) {}

    // Method to check if the result is a number
    bool isNumber() {
        return result->IsNumber();
    }

    // Method to check if the result is a bigint
    bool isBigInt() {
        return result->IsBigInt();
    }

    // Method to check if the result is a string
    bool isString() {
        return result->IsString();
    }

    // Method to convert the result to a number, if applicable
    double getNumberValue() {
        if (isNumber()) {
            return result->NumberValue(isolate->GetCurrentContext()).ToChecked();
        }
        throw std::runtime_error("Result is not a number");
    }

    // Method to convert the result to a bigint, if applicable
    int64_t getBigIntValue() {
        if (isBigInt()) {
            return result->ToBigInt(isolate->GetCurrentContext()).ToLocalChecked()->Int64Value();
        }
        throw std::runtime_error("Result is not a bigint");
    }

    // Method to get the result as a string, if applicable
    std::string getStringValue() {
        if (isString()) {
            v8::String::Utf8Value utf8(isolate, result);
            return *utf8;
        }
        throw std::runtime_error("Result is not a string");
    }
};

class CodeGenTest {
public:
    Lexer lexer;
    Parser parser;
    TypeChecker typeChecker;
    CodeGen codeGen;
    shared_ptr<map<string, string>> filesByCapsuleName;
    v8::Isolate *isolate;

    CodeGenTest() {
        filesByCapsuleName = Compiler::getInstance().filesByCapsuleName;
        
        v8::Isolate::CreateParams create_params;
        create_params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
        isolate = v8::Isolate::New(create_params); 

        delete create_params.array_buffer_allocator;
    }

    ~CodeGenTest() {
      isolate->Dispose();
    }

    WasmExecutionContext setup(string source, string functionName = "main0") {
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
        bool isTypeValid = typeChecker.checkAST(parsedAST);

        for (int i = 0; i < Compiler::getInstance().getEncounteredExceptions().size(); i++) {
          Compiler::getInstance().getEncounteredExceptions()[i]->display();
        }

        if (!isTypeValid) {
          cerr << "Typechecking failed" << endl;
          return WasmExecutionContext(v8::Undefined(isolate), {}, isolate);
        }

        BinaryenModuleRef module = codeGen.generateWasmFromAST(parsedAST);

        vector<char> buffer(4096);
        size_t written = BinaryenModuleWrite(module, buffer.data(), buffer.size());

        v8::Isolate::Scope isolate_scope(isolate);
        v8::HandleScope handle_scope(isolate);
        
        v8::Local<v8::Context> context = v8::Context::New(isolate);
        v8::Context::Scope context_scope(context);

        // Create an ArrayBuffer to hold the Wasm bytes
        std::shared_ptr<v8::BackingStore> backing_store = v8::ArrayBuffer::NewBackingStore(
            buffer.data(), buffer.size(), v8::BackingStore::EmptyDeleter, nullptr);

        v8::Local<v8::ArrayBuffer> array_buffer = v8::ArrayBuffer::New(isolate, std::move(backing_store));
        v8::Local<v8::Uint8Array> wasm_bytes = v8::Uint8Array::New(array_buffer, 0, written);

        // JavaScript source to create and instantiate WebAssembly Module from bytes
        const char csource[] = R"(
            let module = new WebAssembly.Module(bytes);
            let instance = new WebAssembly.Instance(module);
            instance.exports;
        )";

        // Create the JavaScript source code
        v8::Local<v8::String> jsSource = v8::String::NewFromUtf8Literal(isolate, csource);

        // Inject the bytes as "bytes" into the global context
        context->Global()->Set(context, v8::String::NewFromUtf8Literal(isolate, "bytes"), wasm_bytes).FromMaybe(false);

        v8::TryCatch try_catch(isolate);

        // Compile the source
        v8::Local<v8::Script> script;
        if (!v8::Script::Compile(context, jsSource).ToLocal(&script)) {
            v8::String::Utf8Value error(isolate, try_catch.Exception());
            cerr << "Script Compile Error: " << *error << endl;
            return WasmExecutionContext(v8::Undefined(isolate), {}, isolate);
        }

        // Run the script to get the exports object
        v8::Local<v8::Value> exports_value;
        if (!script->Run(context).ToLocal(&exports_value)) {
            v8::String::Utf8Value error(isolate, try_catch.Exception());
            cerr << "Script Run Error: " << *error << endl;
            return WasmExecutionContext(v8::Undefined(isolate), {}, isolate);
        }

        v8::Local<v8::Object> exports = exports_value.As<v8::Object>();

        // Get the export names
        std::vector<std::string> exportNames;
        v8::Local<v8::Array> property_names = exports->GetPropertyNames(context).ToLocalChecked();
        for (uint32_t i = 0; i < property_names->Length(); i++) {
            v8::Local<v8::Value> name = property_names->Get(context, i).ToLocalChecked();
            v8::String::Utf8Value utf8(isolate, name);
            exportNames.push_back(*utf8);
        }

        // Check if the requested function exists in the exports object
        v8::Local<v8::String> func_name = v8::String::NewFromUtf8(isolate, functionName.c_str()).ToLocalChecked();
        if (!exports->Has(context, func_name).FromMaybe(false)) {
            cerr << "Exported function not found" << endl;
            return WasmExecutionContext(v8::Undefined(isolate), exportNames, isolate);
        }

        // Get the exported function
        v8::Local<v8::Value> func_value = exports->Get(context, func_name).ToLocalChecked();

        // Cast to a V8 function and call it
        v8::Local<v8::Function> func = func_value.As<v8::Function>();
        v8::Local<v8::Value> args[] = {};

        v8::Local<v8::Value> result;
        if (!func->Call(context, exports, 0, args).ToLocal(&result)) {
            v8::String::Utf8Value error(isolate, try_catch.Exception());
            cerr << "Function call error: " << *error << endl;
            return WasmExecutionContext(v8::Undefined(isolate), exportNames, isolate);
        }

        // Return the result and the export names
        return WasmExecutionContext(result, exportNames, isolate);
    }
};

TEST_CASE_METHOD(CodeGenTest, "CodeGen") {
    SECTION("Can codegen multiplication") {
       WasmExecutionContext result = setup(R"(
            capsule Test {
                main<Function<Number>> = () -> 10 + 5
            }
        )");

        REQUIRE(result.exportNames.size() == 2);


        REQUIRE(result.isBigInt());
        REQUIRE(result.getBigIntValue() == 15);
    }

//    SECTION("Can codegen addition") {
//        wasm_instance_t *instance = setup(R"(
//            capsule Test {
//                main<Function<Number>> = () -> 9 + 27
//            }
//        )");
//
//        wasm_extern_vec_t exports;
//        wasm_instance_exports(instance, &exports);
//
//        REQUIRE(exports.size == 2);
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
//        REQUIRE(results_val[0].of.i64 == 36);
//    }
//
//    SECTION("Can codegen division") {
//        wasm_instance_t *instance = setup(R"(
//            capsule Test {
//                main<Function<Number>> = () -> 10 / 2
//            }
//        )");
//
//        wasm_extern_vec_t exports;
//        wasm_instance_exports(instance, &exports);
//
//        REQUIRE(exports.size == 2);
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
//        REQUIRE(results_val[0].of.i64 == 5);
//    }
//
//    SECTION("Can codegen subtraction") {
//        wasm_instance_t *instance = setup(R"(
//            capsule Test {
//                main<Function<Number>> = () -> 47 - 10
//            }
//        )");
//
//        wasm_extern_vec_t exports;
//        wasm_instance_exports(instance, &exports);
//
//        REQUIRE(exports.size == 2);
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
//        REQUIRE(results_val[0].of.i64 == 37);
//    }
//
//    SECTION("Correctly codegens negative numbers") {
//        wasm_instance_t *instance = setup(R"(
//            capsule Test {
//                main<Function<Number>> = () -> -10 + 20
//            }
//        )");
//
//        wasm_extern_vec_t exports;
//        wasm_instance_exports(instance, &exports);
//
//        REQUIRE(exports.size == 2);
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
//        REQUIRE(results_val[0].of.i64 == 10);
//    }
//
//    SECTION("Negative multiplication outputs correct result") {
//        wasm_instance_t *instance = setup(R"(
//            capsule Test {
//                main<Function<Number>> = () -> 5 * -7
//            }
//        )");
//
//        wasm_extern_vec_t exports;
//        wasm_instance_exports(instance, &exports);
//
//        REQUIRE(exports.size == 2);
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
//        REQUIRE(results_val[0].of.i64 == -35);
//    }
//
//
//    SECTION("Negative division outputs correct result") {
//        wasm_instance_t *instance = setup(R"(
//            capsule Test {
//                main<Function<Number>> = () -> -90 / 30
//            }
//        )");
//
//        wasm_extern_vec_t exports;
//        wasm_instance_exports(instance, &exports);
//
//        REQUIRE(exports.size == 2);
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
//        REQUIRE(results_val[0].of.i64 == -3);
//    }
//
//    SECTION("More complex arithmetic outputs correct retult") {
//        wasm_instance_t *instance = setup(R"(
//            capsule Test {
//                main<Function<Number>> = () -> 10 * (5 - 1) + (8 / (23 - 5))
//            }
//        )");
//
//        wasm_extern_vec_t exports;
//        wasm_instance_exports(instance, &exports);
//
//        REQUIRE(exports.size == 2);
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
//        // Note: this is 40 because all numbers are currently being treated as integers instead of
//        // being typecast to floats. Once we fix this, it'll be 40.44
//        REQUIRE(results_val[0].of.i64 == 40);
//    }
//
//    SECTION("Can codegen conditionals") {
//         wasm_instance_t *instance = setup(R"(
//            capsule Test {
//                main<Function<Number>> = () -> {
//                    if (1 == 1) {
//                        return 4
//                    } else {
//                        return 3
//                    }
//                }
//            }
//        )");
//
//        wasm_extern_vec_t exports;
//        wasm_instance_exports(instance, &exports);
//
//        REQUIRE(exports.size == 2);
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
//        REQUIRE(results_val[0].of.i64 == 4);
//    }
//
//    SECTION("Can codegen early returns") {
//        wasm_instance_t *instance = setup(R"(
//            capsule Test {
//                main<Function<Number>> = () -> {
//                    if (1 == 1) {
//                        return 10
//                    }
//
//                    return 5
//                }
//            }
//        )");
//
//        wasm_extern_vec_t exports;
//        wasm_instance_exports(instance, &exports);
//
//        REQUIRE(exports.size == 2);
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
//        REQUIRE(results_val[0].of.i64 == 10);
//    }
//
//    SECTION("Can codegen with capsule variables") {
//        wasm_instance_t *instance = setup(R"(
//            capsule Test {
//                count<Number> = 11
//                                          
//                main<Function<Number>> = () -> {
//                    return count + 1
//                }
//            }
//        )");
//
//        wasm_extern_vec_t exports;
//        wasm_instance_exports(instance, &exports);
//
//        REQUIRE(exports.size == 2);
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
//        REQUIRE(results_val[0].of.i64 == 12);
//    }
//
//    SECTION("Can codegen with local variables") {
//         wasm_instance_t *instance = setup(R"(
//            capsule Test {
//                main<Function<Number>> = () -> {
//                    x<Number> = 43
//
//                    if (x == 12) {
//                        return 10
//                    }
//                    
//                    return 2
//                }
//            }
//        )");
//
//        wasm_extern_vec_t exports;
//        wasm_instance_exports(instance, &exports);
//
//        REQUIRE(exports.size == 2);
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
//        REQUIRE(results_val[0].of.i64 == 2);
//    }
//
//    SECTION("Can call capsule functions") {
//         wasm_instance_t *instance = setup(R"(
//            capsule Test {
//                main<Function<Number>> = () -> double(5)
//
//                double<Function<Number, Number>> = (x<Number>) -> x * 2
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
//        REQUIRE(results_val[0].of.i64 == 10);
//    }
//
//// TODO: Fix this test case. This is failing because of the unary comparison
//// to i64.eqz that the !isOdd is doing. 
////
////    SECTION("Can call capsule functions that reference other functions") {
////        wasm_instance_t *instance = setup(R"(
////            capsule Test {
////                main<Function<Boolean>> = () -> isEven(5)
////
////                isEven<Function<Number, Boolean>> = (x<Number>) -> !isOdd(x)
////
////                isOdd<Function<Number, Boolean>> = (x<Number>) -> x % 3 == 0 
////            }
////        )");
////
////        wasm_extern_vec_t exports;
////        wasm_instance_exports(instance, &exports);
////
////        REQUIRE(exports.size == 3);
////
////        wasm_func_t *mainFunc = wasm_extern_as_func(exports.data[1]);
////
////        wasm_val_t args_val[0] = {};
////        wasm_val_t results_val[1] = { WASM_INIT_VAL };
////        wasm_val_vec_t args = WASM_ARRAY_VEC(args_val);
////        wasm_val_vec_t results = WASM_ARRAY_VEC(results_val);
////
////        wasm_func_call(mainFunc, &args, &results);
////
////        REQUIRE(results_val[0].of.i32 == 0);
////    }
//    
//    SECTION("Can call functions that are curried") {
//         wasm_instance_t *instance = setup(R"(
//            capsule Test {
//                main<Function<Number>> = () -> {
//                    multiplyBy10<Function<Number, Number>> = curriedMultiply(10) 
//                
//                    return multiplyBy10(50)
//                }
//
//                curriedMultiply<Function<Number, Function<Number, Number>>> = (x<Number>) -> (y<Number>) -> x * y
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
//        REQUIRE(results_val[0].of.i64 == 500);
//    }
//
//    SECTION("Can call functions that have an internal function as part of its result") {
//         wasm_instance_t *instance = setup(R"(
//            capsule Test {
//                main<Function<Number>> = () -> add1000(5)
//
//                add1000<Function<Number, Number>> = (x<Number>) -> {
//                    add<Function<Number, Number>> = (y<Number>) -> x + y
//
//                    result<Number> = add(1000)
//
//                    return result
//                }
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
//        REQUIRE(results_val[0].of.i64 == 1005);
//    }
//
//    // TODO: Return types of assignments are not being typechecked correctly
//    SECTION("Correctly return value if an assignment is the last expression in a block") {
//         wasm_instance_t *instance = setup(R"(
//            capsule Test {
//                main<Function<Boolean>> = () -> {
//                    x<Boolean> = false
//                }
//            }
//        )");
//
//        wasm_extern_vec_t exports;
//        wasm_instance_exports(instance, &exports);
//
//        REQUIRE(exports.size == 2);
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
//
//    SECTION("Can call recursive functions correctly") {
//         wasm_instance_t *instance = setup(R"(
//            capsule Test {
//                main<Function<Number>> = () -> {
//                    fibonacci(10)
//                }
//
//                fibonacci<Function<Number, Number>> = (n<Number>) -> {
//                    if (n <= 1) {
//                        return n
//                    }
//
//                    fibonacci(n - 1) + fibonacci(n - 2)
//                }
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
//        REQUIRE(results_val[0].of.i64 == 55);
//    }
}

