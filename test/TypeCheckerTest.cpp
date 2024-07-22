#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch2/catch_amalgamated.hpp"
#include "../src/lexer/Lexer.cpp"
#include "../src/parser/Parser.cpp"
#include "../src/compiler/Compiler.hpp"
#include "../src/compiler/TypeChecker.hpp"

using namespace std;
using namespace Theta;

TEST_CASE("TypeChecker") {
    Theta::Lexer lexer;
    Theta::Parser parser;
    TypeChecker typeChecker;

    shared_ptr<map<string, string>> filesByCapsuleName = Theta::Compiler::getInstance().filesByCapsuleName;

    // --------- PRIMITIVES ----------
    SECTION("Can typecheck Binary operations with numbers") {
        string source = "5 * 12.23";
        lexer.lex(source);

        shared_ptr<ASTNode> parsedAST = dynamic_pointer_cast<SourceNode>(
            parser.parse(lexer.tokens, source, "fakeFile.th", filesByCapsuleName)
        );

        Compiler::getInstance().optimizeAST(parsedAST);

        bool isValid = typeChecker.checkAST(parsedAST);

        REQUIRE(isValid);
    }
}
