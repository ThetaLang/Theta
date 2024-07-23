#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch2/catch_amalgamated.hpp"
#include "../src/lexer/Lexer.cpp"
#include "../src/parser/Parser.cpp"
#include "../src/compiler/Compiler.hpp"
#include "../src/compiler/TypeChecker.hpp"

using namespace std;
using namespace Theta;

class TypeCheckerTest {
public:
    Lexer lexer;
    Parser parser;
    TypeChecker typeChecker;
    shared_ptr<map<string, string>> filesByCapsuleName;

    TypeCheckerTest() {
        filesByCapsuleName = Compiler::getInstance().filesByCapsuleName;
    }

    shared_ptr<ASTNode> setup(string source) {
        Compiler::getInstance().clearExceptions();

        lexer.lex(source);

        shared_ptr<ASTNode> parsedAST = parser.parse(
            lexer.tokens,
            source,
            "fakeFile.th",
            filesByCapsuleName
        );

        Compiler::getInstance().optimizeAST(parsedAST);

        return parsedAST;
    }
};

TEST_CASE_METHOD(TypeCheckerTest, "TypeChecker") {
    SECTION("Can typecheck variable assignments correctly") {
        shared_ptr<ASTNode> ast = setup(R"(
            capsule Test {
                x<String> = 'hello, world!'
            }
        )");

        bool isValid = typeChecker.checkAST(ast);

        REQUIRE(isValid);
        REQUIRE(Compiler::getInstance().getEncounteredExceptions().size() == 0);
    }

    SECTION("Throws error when variable assignment doesnt pass typechecking") {
        shared_ptr<ASTNode> ast = setup(R"(
            capsule Test {
                x<String> = 5
            }
        )");

        bool isValid = typeChecker.checkAST(ast);

        REQUIRE(!isValid);
        REQUIRE(Compiler::getInstance().getEncounteredExceptions().size() == 1);
    }

    SECTION("Can typecheck Binary operations with numbers") {
        shared_ptr<ASTNode> ast = setup("5 * 12.23");

        bool isValid = typeChecker.checkAST(ast);

        REQUIRE(isValid);
    }
}
