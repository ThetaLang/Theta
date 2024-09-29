#pragma once

#include <memory>
#include "../parser/ast/ASTNode.hpp"
#include "../parser/ast/BinaryOperationNode.hpp"
#include "../parser/ast/UnaryOperationNode.hpp"
#include "../parser/ast/LiteralNode.hpp"
#include "../parser/ast/SourceNode.hpp"
#include "compiler/SymbolTableStack.hpp"
#include "compiler/WasmClosure.hpp"
#include "parser/ast/ASTNodeList.hpp"
#include "parser/ast/AssignmentNode.hpp"
#include "parser/ast/CapsuleNode.hpp"
#include "parser/ast/FunctionDeclarationNode.hpp"
#include "parser/ast/IdentifierNode.hpp"
#include "parser/ast/ReturnNode.hpp"
#include "parser/ast/TypeDeclarationNode.hpp"
#include "parser/ast/FunctionInvocationNode.hpp"
#include "parser/ast/ControlFlowNode.hpp"
#include "compiler/FunctionMetaData.hpp"
#include <binaryen-c.h>
#include <set>
#include <unordered_map>
#include <functional>
using namespace std;

namespace Theta {
    class CodeGen {
        public:
            BinaryenModuleRef generateWasmFromAST(shared_ptr<ASTNode> ast);
            BinaryenExpressionRef generate(shared_ptr<ASTNode> node, BinaryenModuleRef &module);
            void generateCapsule(shared_ptr<CapsuleNode> node, BinaryenModuleRef &module);
            BinaryenExpressionRef generateAssignment(shared_ptr<AssignmentNode> node, BinaryenModuleRef &module);
            BinaryenExpressionRef generateBlock(shared_ptr<ASTNodeList> node, BinaryenModuleRef &module);
            BinaryenExpressionRef generateReturn(shared_ptr<ReturnNode> node, BinaryenModuleRef &module);
            void generateFunctionDeclaration(
                string identifier, 
                shared_ptr<FunctionDeclarationNode> node,
                BinaryenModuleRef &module,
                bool addToExports = false
            );
            BinaryenExpressionRef generateClosureFunctionDeclaration(
                shared_ptr<FunctionDeclarationNode> node,
                BinaryenModuleRef &module,
                std::function<BinaryenExpressionRef(const BinaryenExpressionRef&)> returnValueFormatter = [](const BinaryenExpressionRef &addrExpr) { return addrExpr; },
                optional<pair<string, int>> assignmentIdentifierPair = nullopt
            );
            BinaryenExpressionRef generateFunctionInvocation(shared_ptr<FunctionInvocationNode> node, BinaryenModuleRef &module);
            BinaryenExpressionRef generateControlFlow(shared_ptr<ControlFlowNode> controlFlowNode, BinaryenModuleRef &module);
            BinaryenExpressionRef generateIdentifier(shared_ptr<IdentifierNode> node, BinaryenModuleRef &module);
            BinaryenExpressionRef generateBinaryOperation(shared_ptr<BinaryOperationNode> node, BinaryenModuleRef &module);
            BinaryenExpressionRef generateUnaryOperation(shared_ptr<UnaryOperationNode> node, BinaryenModuleRef &module);
            BinaryenExpressionRef generateNumberLiteral(shared_ptr<LiteralNode> node, BinaryenModuleRef &module);
            BinaryenExpressionRef generateStringLiteral(shared_ptr<LiteralNode> node, BinaryenModuleRef &module);
            BinaryenExpressionRef generateBooleanLiteral(shared_ptr<LiteralNode> node, BinaryenModuleRef &module);
            BinaryenExpressionRef generateExponentOperation(shared_ptr<BinaryOperationNode> node, BinaryenModuleRef &module);
            void generateSource(shared_ptr<SourceNode> node, BinaryenModuleRef &module);

            shared_ptr<FunctionDeclarationNode> liftLambda(
                shared_ptr<FunctionDeclarationNode> node,
                BinaryenModuleRef &module
            );

        private:
            SymbolTableStack<shared_ptr<ASTNode>> scope;          
            SymbolTableStack<string> scopeReferences;
            string FN_TABLE_NAME = "ThetaFunctionRefs";
            string STRINGREF_TABLE = "ThetaStringRefs";
            string MEMORY_NAME = "0";
            int memoryOffset = 0;
            int stringRefOffset = 1;
            unordered_map<string, WasmClosure> functionNameToClosureTemplateMap;
            string LOCAL_IDX_SCOPE_KEY = "ThetaLang.internal.localIdxCounter";

            BinaryenModuleRef initializeWasmModule();

            BinaryenExpressionRef generateStringBinaryOperation(
                string op,
                BinaryenExpressionRef left,
                BinaryenExpressionRef right,
                BinaryenModuleRef &modul
            );

            vector<Pointer<PointerType::Data>> generateFunctionInvocationArgMemoryInsertions(
                shared_ptr<FunctionInvocationNode> funcInvNode,
                vector<BinaryenExpressionRef> &expressions,
                BinaryenModuleRef &module,
                string refIdentifier = ""
            );

            BinaryenExpressionRef generateCallIndirectForNewClosure(
                shared_ptr<FunctionInvocationNode> funcInvNode,
                shared_ptr<ASTNode> ref,
                string refIdentifier,
                BinaryenModuleRef &module
            );

            BinaryenExpressionRef generateCallIndirectForExistingClosure(
                shared_ptr<FunctionInvocationNode> funcInvNode,
                shared_ptr<ASTNode> ref,
                string refIdentifier,
                BinaryenModuleRef &module
            );
        
            static BinaryenOp getBinaryenOpFromBinOpNode(shared_ptr<BinaryOperationNode> node);
            static BinaryenType getBinaryenTypeFromTypeDeclaration(shared_ptr<TypeDeclarationNode> node);
            static BinaryenType getBinaryenStorageTypeFromTypeDeclaration(shared_ptr<TypeDeclarationNode> node);

            template<typename Node>
            static FunctionMetaData getFunctionMetaData(shared_ptr<Node> node);

            static FunctionMetaData getFunctionMetaDataFromTypeDeclaration(shared_ptr<TypeDeclarationNode> type);

            static FunctionMetaData getDerivedFunctionMetaData(
                shared_ptr<FunctionInvocationNode> inv,
                shared_ptr<FunctionInvocationNode> ref
            );

            void hoistCapsuleElements(vector<shared_ptr<ASTNode>> ielements);
            void bindIdentifierToScope(shared_ptr<ASTNode> ast);
            void registerModuleFunctions(BinaryenModuleRef &module);

            bool checkIsLastInBlock(shared_ptr<ASTNode> node);

            pair<WasmClosure, vector<BinaryenExpressionRef>> generateAndStoreClosure(
                string qualifiedReferenceFunctionName,
                shared_ptr<FunctionDeclarationNode> simplifiedReference,
                shared_ptr<FunctionDeclarationNode> originalReference,
                BinaryenModuleRef &module
            );

            vector<BinaryenExpressionRef> generateClosureMemoryStore(WasmClosure &closure, BinaryenModuleRef &module);

            void collectClosureScope(
                shared_ptr<ASTNode> node,
                set<string> &identifiersToFind,
                vector<shared_ptr<ASTNode>> &parameters,
                vector<shared_ptr<ASTNode>> &bodyExpression
            );

            string generateFunctionHash(shared_ptr<FunctionDeclarationNode> function);

            int getByteSizeForType(shared_ptr<TypeDeclarationNode> type);
            int getByteSizeForType(BinaryenType type);

            BinaryenModuleRef importCoreLangWasm();

            string resolveAbsolutePath(string relativePath);
    };
}
