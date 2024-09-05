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

        Compiler::getInstance().optimizeAST(parsedAST, true);

        return parsedAST;
    }
};

TEST_CASE_METHOD(TypeCheckerTest, "TypeChecker") {
    SECTION("Can typecheck string assignments correctly") {
        shared_ptr<ASTNode> ast = setup(R"(
            capsule Test {
                x<String> = 'hello, world!'
            }
        )");

        bool isValid = typeChecker.checkAST(ast);

        REQUIRE(isValid);
        REQUIRE(Compiler::getInstance().getEncounteredExceptions().size() == 0);
    }

    SECTION("Can typecheck whole number assignments correctly") {
        shared_ptr<ASTNode> ast = setup(R"(
            capsule Test {
                x<Number> = 5
            }
        )");

        bool isValid = typeChecker.checkAST(ast);

        REQUIRE(isValid);
        REQUIRE(Compiler::getInstance().getEncounteredExceptions().size() == 0);
    }

    SECTION("Can typecheck decimal number assignments correctly") {
        shared_ptr<ASTNode> ast = setup(R"(
            capsule Test {
                x<Number> = 123.2
            }
        )");

        bool isValid = typeChecker.checkAST(ast);

        REQUIRE(isValid);
        REQUIRE(Compiler::getInstance().getEncounteredExceptions().size() == 0);
    }

    SECTION("Can typecheck boolean assignments correctly") {
        shared_ptr<ASTNode> ast = setup(R"(
            capsule Test {
                x<Boolean> = true
            }
        )");

        bool isValid = typeChecker.checkAST(ast);

        REQUIRE(isValid);
        REQUIRE(Compiler::getInstance().getEncounteredExceptions().size() == 0);
    }

    SECTION("Can typecheck list assignments correctly") {
        shared_ptr<ASTNode> ast = setup(R"(
            capsule Test {
                x<List<String>> = ['a', 'b', 'c']
            }
        )");

        bool isValid = typeChecker.checkAST(ast);

        REQUIRE(isValid);
        REQUIRE(Compiler::getInstance().getEncounteredExceptions().size() == 0);
    }
    
    SECTION("Can typecheck empty list assignments correctly") {
        shared_ptr<ASTNode> ast = setup(R"(
            capsule Test {
                x<List<Number>> = []
            }
        )");

        bool isValid = typeChecker.checkAST(ast);

        REQUIRE(isValid);
        REQUIRE(Compiler::getInstance().getEncounteredExceptions().size() == 0);
    }

    SECTION("Throws if list is not homogenous") {
        shared_ptr<ASTNode> ast = setup(R"(
            capsule Test {
                x<List<Boolean>> = [true, false, 'false']
            }
        )");

        bool isValid = typeChecker.checkAST(ast);

        REQUIRE(!isValid);
        REQUIRE(Compiler::getInstance().getEncounteredExceptions().size() == 1);
    }

    SECTION("Can typecheck dictionary assignments correctly") {
        shared_ptr<ASTNode> ast = setup(R"(
            capsule Test {
                x<Dict<String>> = { a: 'a', b: 'b', c: 'c' }
            }
        )");

        bool isValid = typeChecker.checkAST(ast);

        REQUIRE(isValid);
        REQUIRE(Compiler::getInstance().getEncounteredExceptions().size() == 0);
    }

    SECTION("Can typecheck empty dictionary assignments correctly") {
        shared_ptr<ASTNode> ast = setup(R"(
            capsule Test {
                x<Dict<Number>> = {}
            }
        )");

        bool isValid = typeChecker.checkAST(ast);

        REQUIRE(isValid);
        REQUIRE(Compiler::getInstance().getEncounteredExceptions().size() == 0);
    }

    SECTION("Throws if dictionary keys are not the same type as declared") {
        shared_ptr<ASTNode> ast = setup(R"(
            capsule Test {
                x<Dict<String>> = { a: '1', b: 2 }
            }
        )");

        bool isValid = typeChecker.checkAST(ast);

        REQUIRE(!isValid);
        REQUIRE(Compiler::getInstance().getEncounteredExceptions().size() == 1);
    }

    SECTION("Can typecheck symbol assignments correctly") {
        shared_ptr<ASTNode> ast = setup(R"(
            capsule Test {
                x<Symbol> = :meow
            }
        )");

        bool isValid = typeChecker.checkAST(ast);

        REQUIRE(isValid);
        REQUIRE(Compiler::getInstance().getEncounteredExceptions().size() == 0);
    }

    SECTION("Can typecheck tuple assignments correctly") {
        shared_ptr<ASTNode> ast = setup(R"(
            capsule Test {
                x<Tuple<Symbol, String>> = { :ok, 'success' }
            }
        )");

        bool isValid = typeChecker.checkAST(ast);

        REQUIRE(isValid);
        REQUIRE(Compiler::getInstance().getEncounteredExceptions().size() == 0);
    }

    SECTION("Throws if tuple doesnt match type spec") {
        shared_ptr<ASTNode> ast = setup(R"(
            capsule Test {
                x<Tuple<Boolean, Number>> = { 0, true }
            }
        )");

        bool isValid = typeChecker.checkAST(ast);

        REQUIRE(!isValid);
        REQUIRE(Compiler::getInstance().getEncounteredExceptions().size() == 1);
    }

    SECTION("Can typecheck function assignments correctly") {
        shared_ptr<ASTNode> ast = setup(R"(
            capsule Test {
                x<Function<Number>> = () -> 5
            }
        )");

        bool isValid = typeChecker.checkAST(ast);

        REQUIRE(isValid);
        REQUIRE(Compiler::getInstance().getEncounteredExceptions().size() == 0);
    }

    SECTION("Can typecheck function assignents with parameters correctly") {
        shared_ptr<ASTNode> ast = setup(R"(
            capsule Test {
                x<Function<String, String>> = (a<String>) -> a
            }
        )");

        bool isValid = typeChecker.checkAST(ast);

        REQUIRE(isValid);
        REQUIRE(Compiler::getInstance().getEncounteredExceptions().size() == 0);
    }
    
    SECTION("Can typecheck variadic function assignments correctly") {
        shared_ptr<ASTNode> ast = setup(R"(
            capsule Test {
                x<Function<Variadic<String, Boolean>>> = () -> {
                    return 'hello'

                    false
                }
            }
        )");

        bool isValid = typeChecker.checkAST(ast);
        
        REQUIRE(isValid);
        REQUIRE(Compiler::getInstance().getEncounteredExceptions().size() == 0);
    }

    SECTION("Throws when variadic function body is not assigned to variadic identifier") {
        shared_ptr<ASTNode> ast = setup(R"(
            capsule Test {
                x<Function<Number>> = () -> {
                    return 5

                    ['a', 'b', 'c']
                }
            }
        )");

        bool isValid = typeChecker.checkAST(ast);

        REQUIRE(!isValid);
        REQUIRE(Compiler::getInstance().getEncounteredExceptions().size() == 1);
    }

    SECTION("Can typecheck curried functions correctly") {
        shared_ptr<ASTNode> ast = setup(R"(
            capsule Test {
                x<Function<Number, Function<Number, Number>>> = (a<Number>) -> (b<Number>) -> a + b
            }
        )");

        bool isValid = typeChecker.checkAST(ast);

        REQUIRE(isValid);
        REQUIRE(Compiler::getInstance().getEncounteredExceptions().size() == 0);
    } 

    SECTION("Can typecheck enum definition and usage correctly") {
        shared_ptr<ASTNode> ast = setup(R"(
            capsule Test {
                enum Severity {
                    :LOW
                    :MEDIUM
                    :HIGH
                }

                severityIdent<Function<Severity, Severity>> = (sev<Severity>) -> {
                    sev
                }
            }
        )");

        bool isValid = typeChecker.checkAST(ast);

        REQUIRE(isValid);
        REQUIRE(Compiler::getInstance().getEncounteredExceptions().size() == 0);
    }

    SECTION("Can typecheck control flow correctly") {
        shared_ptr<ASTNode> ast = setup(R"(
            capsule Test {
                isEven<Function<Number, Boolean>> = (num<Number>) -> {
                    if (num % 2 == 0) {
                        return true
                    } else if (num % 3 == 0) {
                        return false
                    } else {
                        return false
                    }
                }
            }
        )");

        bool isValid = typeChecker.checkAST(ast);

        REQUIRE(isValid);
        REQUIRE(Compiler::getInstance().getEncounteredExceptions().size() == 0);
    }

    SECTION("Throws if control flow condition is not a boolean") {
        shared_ptr<ASTNode> ast = setup(R"(
            capsule Test {
                isLongString<Function<String, Boolean>> = (str<String>) -> {
                    if (str) {
                        return true
                    }
                }
            }
        )");

        bool isValid = typeChecker.checkAST(ast);

        REQUIRE(!isValid);
        REQUIRE(Compiler::getInstance().getEncounteredExceptions().size() == 1);
    }

    SECTION("Can typecheck recursive functions correctly") {
        shared_ptr<ASTNode> ast = setup(R"(
            capsule Test {
                addUntilTen<Function<Number, Number>> = (sum<Number>) -> {
                    if (sum == 10) {
                        return sum
                    }

                    addUntilTen(sum + 1)
                }
            }
        )");

        bool isValid = typeChecker.checkAST(ast);

        REQUIRE(isValid);
        REQUIRE(Compiler::getInstance().getEncounteredExceptions().size() == 0);
    }

    SECTION("Throws if function arguments dont match parameter types") {
        shared_ptr<ASTNode> ast = setup(R"(
            capsule Test {
                add<Function<Number>> = (x<Number>, y<Number>) -> x + y

                doTheThing<Function<Number>> = () -> add(1, '4')
            }
        )");

        bool isValid = typeChecker.checkAST(ast);

        REQUIRE(!isValid);
        REQUIRE(Compiler::getInstance().getEncounteredExceptions().size() == 1);
    }

    SECTION("Throws if function is called with too many arguments") {
        shared_ptr<ASTNode> ast = setup(R"(
            capsule Test {
                add<Function<Number>> = (x<Number>, y<Number>) -> x + y

                doTheThing<Function<Number>> = () -> add(1, 2, 4)
            }
        )");

        bool isValid = typeChecker.checkAST(ast);

        REQUIRE(!isValid);
        REQUIRE(Compiler::getInstance().getEncounteredExceptions().size() == 1);
    }

    SECTION("Throws if function is called with too few arguments") {
        shared_ptr<ASTNode> ast = setup(R"(
            capsule Test {
                add<Function<Number>> = (x<Number>, y<Number>) -> x + y

                doTheThing<Function<Number>> = () -> add(1)
            }
        )");

        bool isValid = typeChecker.checkAST(ast);

        REQUIRE(!isValid);
        REQUIRE(Compiler::getInstance().getEncounteredExceptions().size() == 1);
    }

    SECTION("Can typecheck function overloads correctly") {
        shared_ptr<ASTNode> ast = setup(R"(
            capsule Test {
                add<Function<Number, Number>> = (x<Number>) -> x
                add<Function<Number, Number, Number>> = (x<Number>, y<Number>) -> x + y
                add<Function<Number, String, Number>> = (x<Number>, y<String>) -> x

                doTheThing<Function<Number>> = () -> {
                    add(1)
                    add(1, 2)
                    add(5, 'a')
                }
            }
        )");

        bool isValid = typeChecker.checkAST(ast);

        REQUIRE(isValid);
        REQUIRE(Compiler::getInstance().getEncounteredExceptions().size() == 0);
    }

    SECTION("Throws if trying to access a variable before defining it") {
        shared_ptr<ASTNode> ast = setup(R"(
            capsule Test {
                invalidFunc<Function<Number>> = () -> {
                    x + 1

                    x<Number> = 0
                }
            }
        )");

        bool isValid = typeChecker.checkAST(ast);

        REQUIRE(!isValid);
        REQUIRE(Compiler::getInstance().getEncounteredExceptions().size() == 1);
    }

    SECTION("Throws if trying to reassign a literal variable") {
        shared_ptr<ASTNode> ast = setup(R"(
            capsule Test {
                badReassignment<Function<Number>> = () -> {
                    x<Number> = 0

                    x<Number> = x + 1
                }
            }
        )");

        // It wouldnt even get to checkAST in this case, because the optimizer pass should
        // add a compiler exception
        REQUIRE(Compiler::getInstance().getEncounteredExceptions().size() == 1);
    }

    SECTION("Throws if trying to reassign a complex variable") {
        shared_ptr<ASTNode> ast = setup(R"(
            capsule Test {
                badReassignment<Function<Number>> = () -> {
                    x<List<Number>> = []

                    x<List<Number>> = [1, 2, 3]
                }
            }
        )");


        // It wouldnt even get to checkAST in this case, because the optimizer pass should
        // add a compiler exception
        REQUIRE(Compiler::getInstance().getEncounteredExceptions().size() == 1);
    }

    SECTION("Can typecheck capsule hoisted elements correctly") {
        shared_ptr<ASTNode> ast = setup(R"(
            capsule Test {
                myList<List<Number>> = [x, y, z]

                x<Number> = 5

                z<Number> = y

                y<Number> = x
            }
        )");

        bool isValid = typeChecker.checkAST(ast);

        REQUIRE(isValid);
        REQUIRE(Compiler::getInstance().getEncounteredExceptions().size() == 0);
    }

    SECTION("Can typecheck struct definition and declaration correctly") {
        shared_ptr<ASTNode> ast = setup(R"(
            capsule Test {
                struct Point {
                    x<Number>
                    y<Number>
                }

                addPoints<Function<Point, Point, Point>> = (p1<Point>, p2<Point>) -> {
                    @Point {
                        x: 4,
                        y: 2
                    }
                }
            }
        )");

        bool isValid = typeChecker.checkAST(ast);

        REQUIRE(isValid);
        REQUIRE(Compiler::getInstance().getEncounteredExceptions().size() == 0);
    }

    SECTION("Throws if struct declaration is missing fields from definition") {
        shared_ptr<ASTNode> ast = setup(R"(
            capsule Test {
                struct Point {
                    x<Number>
                    y<Number>
                }

                addPoints<Function<Point, Point, Point>> = (p1<Point>, p2<Point>) -> {
                    @Point {
                        x: 4
                    }
                }
            }
        )");

        bool isValid = typeChecker.checkAST(ast);

        REQUIRE(!isValid);
        REQUIRE(Compiler::getInstance().getEncounteredExceptions().size() == 1);
    }


    SECTION("Throws if struct declaration contains fields that are not in definition") {
        shared_ptr<ASTNode> ast = setup(R"(
            capsule Test {
                struct Point {
                    x<Number>
                    y<Number>
                }

                addPoints<Function<Point>> = (p1<Point>, p2<Point>) -> {
                    @Point {
                        x: 4,
                        y: 3,
                        z: 1
                    }
                }
            }
        )");

        bool isValid = typeChecker.checkAST(ast);

        REQUIRE(!isValid);
        REQUIRE(Compiler::getInstance().getEncounteredExceptions().size() == 1);
    }

    SECTION("Throws if struct declaration field types dont match") {
        shared_ptr<ASTNode> ast = setup(R"(
            capsule Test {
                struct Point {
                    x<Number>
                    y<Number>
                }

                addPoints<Function<Point>> = (p1<Point>, p2<Point>) -> {
                    @Point {
                        x: 4,
                        y: '9'
                    }
                }
            }
        )");

        bool isValid = typeChecker.checkAST(ast);

        REQUIRE(!isValid);
        REQUIRE(Compiler::getInstance().getEncounteredExceptions().size() == 1);
    }

    SECTION("Can typecheck references to outer-scoped variables from within inner scope") {
        shared_ptr<ASTNode> ast = setup(R"(
            capsule Test {
                x<Number> = 5

                add3ToX<Function<Number>> = () -> {
                    x + 3
                }
            }
        )");

        bool isValid = typeChecker.checkAST(ast);

        REQUIRE(isValid);
        REQUIRE(Compiler::getInstance().getEncounteredExceptions().size() == 0);
    }

    SECTION("Curried functions retain scope of parent functions") {
        shared_ptr<ASTNode> ast = setup(R"(
            capsule Test {
                z<Number> = 5

                add<Function<Number, Function<Number, Number>>> = (x<Number>) -> (y<Number>) -> x + y + z
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
