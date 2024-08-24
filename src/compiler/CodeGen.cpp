#include <iostream>
#include <libgen.h>
#include <limits.h>
#include <unistd.h>
#include <iterator>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include "asmjs/shared-constants.h"
#include "binaryen-c.h"
#include "compiler/Compiler.hpp"
#include "compiler/TypeChecker.hpp"
#include "compiler/WasmClosure.hpp"
#include "lexer/Lexemes.hpp"
#include "StandardLibrary.hpp"
#include "CodeGen.hpp"
#include "DataTypes.hpp"
#include "parser/ast/ASTNodeList.hpp"
#include "parser/ast/AssignmentNode.hpp"
#include "parser/ast/BlockNode.hpp"
#include "parser/ast/FunctionDeclarationNode.hpp"
#include "parser/ast/IdentifierNode.hpp"
#include "parser/ast/TypeDeclarationNode.hpp"
#include "cli/CLI.cpp"

#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

namespace Theta {
    BinaryenModuleRef CodeGen::generateWasmFromAST(shared_ptr<ASTNode> ast) {
        BinaryenModuleRef module = initializeWasmModule();        

        generate(ast, module);

        registerModuleFunctions(module);
    
        // Automatically adds drops to unused stack values
        BinaryenModuleAutoDrop(module);

        return module;
    }

    BinaryenModuleRef CodeGen::initializeWasmModule() {
        BinaryenModuleRef module = importCoreLangWasm();

        BinaryenModuleSetFeatures(module, BinaryenFeatureStrings());
        BinaryenSetMemory(
            module,
            1, // IMPORTANT: Memory size is dictated in pages, NOT bytes, where each page is 64k
            10,
            "memory", // TODO: We don't actually want to export this -- just for now
            NULL,
            NULL,
            NULL,
            NULL,
            NULL,
            0,
            false,
            false,
            MEMORY_NAME.c_str()
        );
    
        StandardLibrary::registerFunctions(module);

        return module;
    }

    BinaryenExpressionRef CodeGen::generate(shared_ptr<ASTNode> node, BinaryenModuleRef &module) {
        if (node->hasOwnScope()) {
            scope.enterScope();
            scopeReferences.enterScope();
        }

        if (node->getNodeType() == ASTNode::SOURCE) {
            generateSource(dynamic_pointer_cast<SourceNode>(node), module);
        } else if (node->getNodeType() == ASTNode::CAPSULE) {
            return generateCapsule(dynamic_pointer_cast<CapsuleNode>(node), module);
        } else if (node->getNodeType() == ASTNode::ASSIGNMENT) {
            return generateAssignment(dynamic_pointer_cast<AssignmentNode>(node), module);
        } else if (node->getNodeType() == ASTNode::BLOCK) {
            return generateBlock(dynamic_pointer_cast<ASTNodeList>(node), module);
        } else if (node->getNodeType() == ASTNode::RETURN) {
            return generateReturn(dynamic_pointer_cast<ReturnNode>(node), module); 
        } else if (node->getNodeType() == ASTNode::FUNCTION_DECLARATION) {
            // The only time we should get here is if we have a function defined inside a function,
            // because the normal function declaration flow goes through the generateAssignment flow
            simplifyNestedFunctionDeclaration(dynamic_pointer_cast<FunctionDeclarationNode>(node), module);
        } else if (node->getNodeType() == ASTNode::FUNCTION_INVOCATION) {
            return generateFunctionInvocation(dynamic_pointer_cast<FunctionInvocationNode>(node), module);
        } else if (node->getNodeType() == ASTNode::CONTROL_FLOW) {
            return generateControlFlow(dynamic_pointer_cast<ControlFlowNode>(node), module);
        } else if (node->getNodeType() == ASTNode::IDENTIFIER) {
            return generateIdentifier(dynamic_pointer_cast<IdentifierNode>(node), module);
        } else if (node->getNodeType() == ASTNode::BINARY_OPERATION) {
            return generateBinaryOperation(dynamic_pointer_cast<BinaryOperationNode>(node), module);
        } else if (node->getNodeType() == ASTNode::UNARY_OPERATION) {
            return generateUnaryOperation(dynamic_pointer_cast<UnaryOperationNode>(node), module);
        } else if (node->getNodeType() == ASTNode::NUMBER_LITERAL) {
            return generateNumberLiteral(dynamic_pointer_cast<LiteralNode>(node), module);
        } else if (node->getNodeType() == ASTNode::STRING_LITERAL) {
            return generateStringLiteral(dynamic_pointer_cast<LiteralNode>(node), module);
        } else if (node->getNodeType() == ASTNode::BOOLEAN_LITERAL) {
            return generateBooleanLiteral(dynamic_pointer_cast<LiteralNode>(node), module);
        }

        if (node->hasOwnScope()) {
            scope.exitScope();
            scopeReferences.exitScope();
        }

        return nullptr;
    }

    BinaryenExpressionRef CodeGen::generateCapsule(shared_ptr<CapsuleNode> capsuleNode, BinaryenModuleRef &module) {
        vector<shared_ptr<ASTNode>> capsuleElements = dynamic_pointer_cast<ASTNodeList>(capsuleNode->getValue())->getElements();

        hoistCapsuleElements(capsuleElements);

        for (auto elem : capsuleElements) {
            string elemType = dynamic_pointer_cast<TypeDeclarationNode>(elem->getResolvedType())->getType();
            if (elem->getNodeType() == ASTNode::ASSIGNMENT) {
                string identifier = dynamic_pointer_cast<IdentifierNode>(elem->getLeft())->getIdentifier();

                if (elemType == DataTypes::FUNCTION) {
                    generateFunctionDeclaration(
                        identifier,
                        dynamic_pointer_cast<FunctionDeclarationNode>(elem->getRight()),
                        module,
                        true
                    );
                } else {
                    shared_ptr<ASTNode> assignmentRhs = elem->getRight();
                    assignmentRhs->setMappedBinaryenIndex(-1); //Index of -1 means its a global
                    scope.insert(identifier, assignmentRhs);

                    BinaryenGlobalSet(
                        module,
                        identifier.c_str(),
                        generate(assignmentRhs, module)
                    );
                }
            }
        }
    }

    BinaryenExpressionRef CodeGen::generateAssignment(shared_ptr<AssignmentNode> assignmentNode, BinaryenModuleRef &module) {
        string assignmentIdentifier = dynamic_pointer_cast<IdentifierNode>(assignmentNode->getLeft())->getIdentifier();

        // Using a space in scope for an idx counter so we dont have to have a whole separate stack just to keep track of the current
        // local idx
        shared_ptr<LiteralNode> currentIdentIdx = dynamic_pointer_cast<LiteralNode>(scope.lookup(LOCAL_IDX_SCOPE_KEY).value());
        int idxOfAssignment = stoi(currentIdentIdx->getLiteralValue());

        currentIdentIdx->setLiteralValue(to_string(idxOfAssignment + 1));
        scope.insert(LOCAL_IDX_SCOPE_KEY, currentIdentIdx);

        // Function declarations dont get generated generically like the rest of the AST elements, they are not part of the "generate" method,
        // because they behave differently depending on where the function was declared. A function declared at the top level of capsule will
        // be hoisted and will have no inherent scope bound to it. 
        //
        // A function declared within another function body OR within any other structure will be turned into a closure that contains the scope
        // of anything outside of that function.
        if (assignmentNode->getRight()->getNodeType() != ASTNode::FUNCTION_DECLARATION) {
            shared_ptr<ASTNode> assignmentRhs = assignmentNode->getRight();
            assignmentRhs->setMappedBinaryenIndex(idxOfAssignment);
        
            string identName = assignmentIdentifier;
            if (assignmentNode->getRight()->getNodeType() == ASTNode::FUNCTION_INVOCATION) {
                identName = Compiler::getQualifiedFunctionIdentifier(identName, assignmentNode->getRight()->getResolvedType());
            }

            scope.insert(identName, assignmentRhs);

            return BinaryenLocalSet(
                module,
                idxOfAssignment,
                generate(assignmentRhs, module)
            );
        }

        shared_ptr<FunctionDeclarationNode> originalDeclaration = dynamic_pointer_cast<FunctionDeclarationNode>(assignmentNode->getRight());

        shared_ptr<FunctionDeclarationNode> simplifiedDeclaration = simplifyNestedFunctionDeclaration(
            originalDeclaration,
            module
        );

        // Generating a unique hash for this function is necessary because it will be stored on the module globally,
        // so we need to make sure there are no naming collisions
        string simplifiedDeclarationHash = generateFunctionHash(simplifiedDeclaration);

        generateFunctionDeclaration(
            simplifiedDeclarationHash,
            simplifiedDeclaration,
            module
        );

        string globalQualifiedFunctionName = Compiler::getQualifiedFunctionIdentifier(
            simplifiedDeclarationHash,
            simplifiedDeclaration
        );

        pair<WasmClosure, vector<BinaryenExpressionRef>> storage = generateAndStoreClosure(
            globalQualifiedFunctionName,
            simplifiedDeclaration,
            originalDeclaration,
            module
        );

        simplifiedDeclaration->setMappedBinaryenIndex(idxOfAssignment);

        string localQualifiedFunctionName = Compiler::getQualifiedFunctionIdentifier(
            assignmentIdentifier,
            originalDeclaration
        );

        // Assign it in scope to the lhs identifier so we can always look it up later when it is referenced. This
        // way the caller does not need to know the global function name in order to call it
        scope.insert(globalQualifiedFunctionName, simplifiedDeclaration);
        scopeReferences.insert(localQualifiedFunctionName, globalQualifiedFunctionName);

        // Also insert the assignment identifier into scope referenecs so that if we want to return a reference to the function
        // using the identifier, we can do that. This will overwrite any previous scope references with that identifier, so only
        // the most recent identifier of a given name can be returned as a reference 
        scopeReferences.insert(assignmentIdentifier, globalQualifiedFunctionName);

        vector<BinaryenExpressionRef> expressions = storage.second;

        // Returns a reference to the closure memory address
        expressions.push_back(
            BinaryenLocalSet(
                module,
                idxOfAssignment,
                BinaryenConst(
                    module,
                    BinaryenLiteralInt32(storage.first.getPointer().getAddress())
                )
            )
        );

        BinaryenExpressionRef* blockExpressions = new BinaryenExpressionRef[expressions.size()];
        for (int i = 0; i < expressions.size(); i++) {
            blockExpressions[i] = expressions.at(i);
        }

        return BinaryenBlock(
            module,
            NULL,
            blockExpressions,
            expressions.size(),
            BinaryenTypeInt32()
        );
    }

    pair<WasmClosure, vector<BinaryenExpressionRef>> CodeGen::generateAndStoreClosure(
        string qualifiedReferenceFunctionName,
        shared_ptr<FunctionDeclarationNode> simplifiedReference,
        shared_ptr<FunctionDeclarationNode> originalReference,
        BinaryenModuleRef &module
    ) {
        Pointer referencePtr = functionNameToClosureTemplateMap.find(qualifiedReferenceFunctionName)->second.getFunctionPointer();
        set<string> originalParameters;

        for (auto param : originalReference->getParameters()->getElements()) {
            originalParameters.insert(dynamic_pointer_cast<IdentifierNode>(param)->getIdentifier());
        }

        vector<BinaryenExpressionRef> expressions;
        vector<Pointer<PointerType::Data>> argPointers; 

        // Store the args into memory
        for (auto param : simplifiedReference->getParameters()->getElements()) {
            string paramName = dynamic_pointer_cast<IdentifierNode>(param)->getIdentifier();

            if (originalParameters.find(paramName) != originalParameters.end()) continue;

            shared_ptr<ASTNode> paramValue = scope.lookup(paramName).value();
            shared_ptr<TypeDeclarationNode> paramType = dynamic_pointer_cast<TypeDeclarationNode>(param->getValue());

            int byteSize = getByteSizeForType(paramType);
            expressions.push_back(
                BinaryenStore(
                    module,
                    byteSize,
                    0,
                    0,
                    BinaryenConst(module, BinaryenLiteralInt32(memoryOffset)),
                    generate(paramValue, module),
                    getBinaryenTypeFromTypeDeclaration(paramType),
                    MEMORY_NAME.c_str()
                )
            );

            argPointers.push_back(Pointer<PointerType::Data>(memoryOffset));
            
            // TODO: change to only increment memoryOffset after the loop finishes
            memoryOffset += byteSize;
        }

        WasmClosure closure = WasmClosure(
            referencePtr,
            simplifiedReference->getParameters()->getElements().size(),
            argPointers
        );

        // Store a closure pointing to the args that were stored in memory
        vector<BinaryenExpressionRef> storageExpressions = generateClosureMemoryStore(closure, module);

        copy(storageExpressions.begin(), storageExpressions.end(), back_inserter(expressions));
    
        return make_pair(closure, expressions);
    }

    // Transforms nested function declarations and generates an anonymous function in the function table
    shared_ptr<FunctionDeclarationNode> CodeGen::simplifyNestedFunctionDeclaration(
        shared_ptr<FunctionDeclarationNode> fnDeclNode,
        BinaryenModuleRef &module
    ) {
        // Capture the outer scope
        set<string> requiredScopeIdentifiers;
        set<string> paramIdentifiers;

        for (auto param : fnDeclNode->getParameters()->getElements()) {
            paramIdentifiers.insert(dynamic_pointer_cast<IdentifierNode>(param)->getIdentifier());
        }

        vector<shared_ptr<ASTNode>> identifiersInBody = Compiler::findAllInTree(fnDeclNode->getDefinition(), ASTNode::IDENTIFIER);
    
        for (auto ident : identifiersInBody) {
            string identifierName = dynamic_pointer_cast<IdentifierNode>(ident)->getIdentifier();

            // Only add identifiers that are not present in the function params
            if (paramIdentifiers.find(identifierName) != paramIdentifiers.end()) continue;

            // If an identifier is globally available we dont need to include it either
            shared_ptr<ASTNode> inScope = scope.lookup(identifierName).value();
            if (inScope->getMappedBinaryenIndex() == -1) continue;

            requiredScopeIdentifiers.insert(identifierName);
        }

        vector<shared_ptr<ASTNode>> simplifiedDeclarationParameters;
        vector<shared_ptr<ASTNode>> simplifiedDeclarationExpressions;

        collectClosureScope(fnDeclNode, requiredScopeIdentifiers, simplifiedDeclarationParameters, simplifiedDeclarationExpressions);

        // The collectClosureScope function will collect the parameters in reverse order, in order to preserve the order
        // in which the params are passed throughout the ancestry path, so we need to reverse it back here to get the
        // correct order
        reverse(simplifiedDeclarationParameters.begin(), simplifiedDeclarationParameters.end());
        reverse(simplifiedDeclarationExpressions.begin(), simplifiedDeclarationExpressions.end());

        // Add the most immediate-level function's parameters to the end
        simplifiedDeclarationParameters.insert(
            simplifiedDeclarationParameters.end(),
            fnDeclNode->getParameters()->getElements().begin(),
            fnDeclNode->getParameters()->getElements().end()
        );

        vector<shared_ptr<ASTNode>> originalFnExpressions =
            dynamic_pointer_cast<ASTNodeList>(fnDeclNode->getDefinition())->getElements();

        simplifiedDeclarationExpressions.insert(
            simplifiedDeclarationExpressions.end(),
            originalFnExpressions.begin(),
            originalFnExpressions.end()
        );
    
        shared_ptr<FunctionDeclarationNode> simplifiedDeclaration = make_shared<FunctionDeclarationNode>(nullptr);
        simplifiedDeclaration->setResolvedType(Compiler::deepCopyTypeDeclaration(
            dynamic_pointer_cast<TypeDeclarationNode>(fnDeclNode->getResolvedType()), 
            simplifiedDeclaration
        ));

        shared_ptr<ASTNodeList> parametersNode = make_shared<ASTNodeList>(simplifiedDeclaration);
        parametersNode->setElements(simplifiedDeclarationParameters);
        simplifiedDeclaration->setParameters(parametersNode);

        shared_ptr<BlockNode> simplifiedDeclarationBody = make_shared<BlockNode>(simplifiedDeclaration);
        simplifiedDeclarationBody->setElements(simplifiedDeclarationExpressions);
        simplifiedDeclaration->setDefinition(simplifiedDeclarationBody);
    
        // If we've traversed the tree for parameters and we still have some missing identifiers, they must be defined in bodies
        if (requiredScopeIdentifiers.size() > 0) {
            cerr << "\033[1;31mFATAL ERROR: Could not locate necessary closure identifiers!\033[0m" << endl;
            cerr << "   Missed identifiers: ";
            for (int i = 0; i < requiredScopeIdentifiers.size(); i++) {
                if (i > 0) cerr << ", ";
                cerr << i;
            }
            cerr << endl << "This error is not caused by your code, but rather an issue with the compiler itself. Please report an issue at " << CLI::makeLink("https://github.com/alexdovzhanyn/ThetaLang/issues");

            exit(1);
        }
 
        return simplifiedDeclaration;
    }
    
    void CodeGen::collectClosureScope(
        shared_ptr<ASTNode> node, 
        set<string> &identifiersToFind,
        vector<shared_ptr<ASTNode>> &parameters,
        vector<shared_ptr<ASTNode>> &bodyExpressions
    ) {
        if (identifiersToFind.size() == 0 || node->getParent()->getNodeType() == ASTNode::CAPSULE) return;

        if (node->getParent()->getNodeType() == ASTNode::BLOCK) {
            vector<shared_ptr<ASTNode>> parentExpressions = dynamic_pointer_cast<ASTNodeList>(node->getParent())->getElements();

            // Go through the parent expressions backwards so that we can collect dependencies and resolve them in one pass
            // in case this expression relies on one before it
            for (int i = parentExpressions.size() - 1; i >= 0; i--) {
                shared_ptr<ASTNode> expr = parentExpressions.at(i);

                if (expr->getNodeType() != ASTNode::ASSIGNMENT) continue;

                string identifier = dynamic_pointer_cast<IdentifierNode>(expr->getLeft())->getIdentifier();
                
                auto identExpr = identifiersToFind.find(identifier);

                if (identExpr == identifiersToFind.end()) continue;
            
                bodyExpressions.push_back(expr);
                identifiersToFind.erase(identifier);

                // This expression we just found might depend on other identifiers, in which case we need to copy those over too
                vector<shared_ptr<ASTNode>> dependentIdentifiers = Compiler::findAllInTree(expr->getRight(), ASTNode::IDENTIFIER);
                for (auto ident : dependentIdentifiers) {
                    identifiersToFind.insert(dynamic_pointer_cast<IdentifierNode>(ident)->getIdentifier());
                }
            }
        } else if (node->getParent()->getNodeType() == ASTNode::FUNCTION_DECLARATION) {
            shared_ptr<FunctionDeclarationNode> parent = dynamic_pointer_cast<FunctionDeclarationNode>(node->getParent());

            // Go through the parameters backwards so we can preserve their order if we ascend further into the ancestors. This 
            // will get reversed at the end
            for (int i = parent->getParameters()->getElements().size() - 1; i >= 0; i--) {
                shared_ptr<IdentifierNode> ident = dynamic_pointer_cast<IdentifierNode>(parent->getParameters()->getElements().at(i));

                auto identNeeded = identifiersToFind.find(ident->getIdentifier());

                if (identNeeded == identifiersToFind.end()) continue;

                parameters.push_back(ident);
                identifiersToFind.erase(ident->getIdentifier());
            }
        }

        collectClosureScope(node->getParent(), identifiersToFind, parameters, bodyExpressions);
    }

    BinaryenExpressionRef CodeGen::generateFunctionDeclaration(
        string identifier,
        shared_ptr<FunctionDeclarationNode> fnDeclNode,
        BinaryenModuleRef &module,
        bool addToExports
    ) {
        scope.enterScope();
        scopeReferences.enterScope();
        BinaryenType parameterType = BinaryenTypeNone();
        int totalParams = fnDeclNode->getParameters()->getElements().size();

        scope.insert(LOCAL_IDX_SCOPE_KEY, make_shared<LiteralNode>(ASTNode::NUMBER_LITERAL, to_string(totalParams), nullptr));

        if (totalParams > 0) {
            BinaryenType* types = new BinaryenType[totalParams];

            for (int i = 0; i < totalParams; i++) {
                shared_ptr<IdentifierNode> identNode = dynamic_pointer_cast<IdentifierNode>(fnDeclNode->getParameters()->getElements().at(i));

                identNode->setMappedBinaryenIndex(i);

                scope.insert(identNode->getIdentifier(), identNode);
                types[i] = getBinaryenTypeFromTypeDeclaration(

                    dynamic_pointer_cast<TypeDeclarationNode>(fnDeclNode->getParameters()->getElements().at(i)->getValue())
                );
            }

            parameterType = BinaryenTypeCreate(types, totalParams);
        }

        vector<shared_ptr<ASTNode>> localVariables = Compiler::findAllInTree(fnDeclNode->getDefinition(), ASTNode::ASSIGNMENT);

        BinaryenType* localVariableTypes = new BinaryenType[localVariables.size()];
        for (int i = 0; i < localVariables.size(); i++) {
            localVariableTypes[i] = getBinaryenTypeFromTypeDeclaration(
                dynamic_pointer_cast<TypeDeclarationNode>(localVariables.at(i)->getResolvedType())
            );
        }

        string functionName = Compiler::getQualifiedFunctionIdentifier(
            identifier,
            dynamic_pointer_cast<ASTNode>(fnDeclNode)
        );

        BinaryenFunctionRef fn = BinaryenAddFunction(
            module,
            functionName.c_str(),
            parameterType,
            getBinaryenTypeFromTypeDeclaration(TypeChecker::getFunctionReturnType(fnDeclNode)),
            localVariableTypes,
            localVariables.size(),
            generate(fnDeclNode->getDefinition(), module)
        );

        // Only add to the closure template map if its not already in there. It may have been added during hoisting
        if (functionNameToClosureTemplateMap.find(functionName) == functionNameToClosureTemplateMap.end()) {
            functionNameToClosureTemplateMap.insert(make_pair(
                functionName,
                WasmClosure(
                    Pointer<PointerType::Function>(functionNameToClosureTemplateMap.size()),
                    totalParams
                )
            ));
        }

        if (addToExports) {
            BinaryenAddFunctionExport(module, functionName.c_str(), functionName.c_str());
        }

        scope.exitScope();
        scopeReferences.exitScope();
    }

    BinaryenExpressionRef CodeGen::generateBlock(shared_ptr<ASTNodeList> blockNode, BinaryenModuleRef &module) {
        BinaryenExpressionRef* blockExpressions = new BinaryenExpressionRef[blockNode->getElements().size()];

        for (int i = 0; i < blockNode->getElements().size(); i++) {
            blockExpressions[i] = generate(blockNode->getElements().at(i), module);
        }

        return BinaryenBlock(
            module,
            NULL,
            blockExpressions,
            blockNode->getElements().size(),
            BinaryenTypeNone()
        );
    }

    BinaryenExpressionRef CodeGen::generateReturn(shared_ptr<ReturnNode> returnNode, BinaryenModuleRef &module) {
        return BinaryenReturn(module, generate(returnNode->getValue(), module));
    }

    BinaryenExpressionRef CodeGen::generateFunctionInvocation(shared_ptr<FunctionInvocationNode> funcInvNode, BinaryenModuleRef &module) {
        BinaryenExpressionRef* arguments = new BinaryenExpressionRef[funcInvNode->getParameters()->getElements().size()];

        string funcName = Compiler::getQualifiedFunctionIdentifier(
            dynamic_pointer_cast<IdentifierNode>(funcInvNode->getIdentifier())->getIdentifier(),
            funcInvNode
        );
    
        for (int i = 0; i < funcInvNode->getParameters()->getElements().size(); i++) {
            arguments[i] = generate(funcInvNode->getParameters()->getElements().at(i), module);
        }

        string scopeLookupIdentifier = funcName;

        auto localReference = scopeReferences.lookup(funcName); 
        if (localReference.has_value()) {
           scopeLookupIdentifier = localReference.value();
        }

        auto foundLocalReference = scope.lookup(scopeLookupIdentifier);

        if (foundLocalReference.has_value()) {
            return generateIndirectInvocation(
                funcInvNode,
                foundLocalReference.value(),
                module,
                scopeLookupIdentifier
            );
        }

        // TODO: Check if this needs to be an indirect call, and generate that instead of a normal call. Thats why the current compile
        // is failing

        cout << "AAAAAAAHHH I SHOULD NEVER GET HERE" << endl;

        return BinaryenCall(
            module,
            funcName.c_str(),
            arguments,
            funcInvNode->getParameters()->getElements().size(),
            getBinaryenTypeFromTypeDeclaration(dynamic_pointer_cast<TypeDeclarationNode>(funcInvNode->getResolvedType()))
        );
    }

    // TODO: This needs to be refactored
    BinaryenExpressionRef CodeGen::generateIndirectInvocation(
        shared_ptr<FunctionInvocationNode> funcInvNode,
        shared_ptr<ASTNode> reference,
        BinaryenModuleRef &module,
        string passedRefIdentifier
    ) {
        string funcInvIdentifier = dynamic_pointer_cast<IdentifierNode>(funcInvNode->getIdentifier())->getIdentifier();

        if (reference->getNodeType() == ASTNode::FUNCTION_DECLARATION) {
            shared_ptr<FunctionDeclarationNode> ref = dynamic_pointer_cast<FunctionDeclarationNode>(reference);
    
            string refIdentifier = passedRefIdentifier;
            if (refIdentifier == "") {
                refIdentifier = Compiler::getQualifiedFunctionIdentifier(funcInvIdentifier, funcInvNode);
            } 
                
            WasmClosure closureTemplate = functionNameToClosureTemplateMap.find(refIdentifier)->second;
            
            vector<BinaryenExpressionRef> expressions;

            string funcInvName = Compiler::getQualifiedFunctionIdentifier(funcInvIdentifier, funcInvNode);

            // If the calculated name isn't the same as the refIdentifier, we know
            // this is a reference to a function and must have a closure already
            // in memory
            if (funcInvName != refIdentifier) {
                for (shared_ptr<ASTNode> arg : funcInvNode->getParameters()->getElements()) {
                    shared_ptr<TypeDeclarationNode> argType = dynamic_pointer_cast<TypeDeclarationNode>(arg->getResolvedType());
                
                    int argByteSize = getByteSizeForType(argType);

                    expressions.push_back(
                        BinaryenStore(
                            module,
                            argByteSize,
                            0,
                            0,
                            BinaryenConst(module, BinaryenLiteralInt32(memoryOffset)),
                            generate(arg, module),
                            getBinaryenTypeFromTypeDeclaration(argType),
                            MEMORY_NAME.c_str()
                        )
                    );

                    expressions.push_back(
                        BinaryenCall(
                            module,
                            "Theta.Function.populateClosure",
                            (BinaryenExpressionRef[]){
                                BinaryenLocalGet(
                                    module,
                                    scope.lookup(refIdentifier).value()->getMappedBinaryenIndex(),
                                    BinaryenTypeInt32()
                                ),
                                BinaryenConst(
                                    module,
                                    BinaryenLiteralInt32(memoryOffset)
                                )
                            },
                            2,
                            BinaryenTypeNone()
                        )
                    );

                    memoryOffset += argByteSize;
                }

                BinaryenFunctionRef functionToExecute = BinaryenGetFunction(module, refIdentifier.c_str());
                BinaryenType functionReturnType = BinaryenFunctionGetResults(functionToExecute);
                BinaryenType functionParamType = BinaryenFunctionGetParams(functionToExecute);
                int functionArity = BinaryenTypeArity(functionParamType);

                BinaryenType* types;
                BinaryenTypeExpand(functionParamType, types);
                BinaryenExpressionRef* loadArgsExpressions = new BinaryenExpressionRef[functionArity];

                for (int i = 0; i < functionArity; i++) {
                    loadArgsExpressions[i] = BinaryenLoad(
                        module,
                        getByteSizeForType(types[i]),
                        false, // TODO: support negative values
                        0,
                        0,
                        types[i],
                        BinaryenLoad( // Loads the arg pointer
                            module,
                            4,
                            false, 
                            8 + i * 4,
                            0,
                            BinaryenTypeInt32(),
                            BinaryenLocalGet( //  The local thats storing the pointer to the closure
                                module,
                                scope.lookup(refIdentifier).value()->getMappedBinaryenIndex(),
                                BinaryenTypeInt32()
                            ),
                            MEMORY_NAME.c_str()
                        ),
                        MEMORY_NAME.c_str()
                    );
                }

                // If arity hits 0, we can call_indirect
                expressions.push_back(
                    BinaryenIf(
                        module,
                        BinaryenUnary( // Check if the arity is equal to 0
                            module,
                            BinaryenEqZInt32(),
                            BinaryenLoad(
                                module,
                                4, // Add 4 to the closure pointer address to get the arity address
                                false,
                                4,
                                0,
                                BinaryenTypeInt32(),
                                BinaryenLocalGet( //  The local thats storing the pointer to the function we want to call
                                    module,
                                    scope.lookup(refIdentifier).value()->getMappedBinaryenIndex(),
                                    BinaryenTypeInt32()
                                ),
                                MEMORY_NAME.c_str()
                            )
                        ),
                        BinaryenCallIndirect( // If the above check is true, execute_indirect
                            module, 
                            FN_TABLE_NAME.c_str(), 
                            BinaryenLoad(
                                module,
                                4,
                                false,
                                0,
                                0,
                                BinaryenTypeInt32(),
                                BinaryenLocalGet(
                                    module,
                                    scope.lookup(refIdentifier).value()->getMappedBinaryenIndex(),
                                    BinaryenTypeInt32()
                                ),
                                MEMORY_NAME.c_str()
                            ),
                            loadArgsExpressions,
                            BinaryenTypeArity(functionParamType),
                            functionParamType,
                            functionReturnType
                        ),   
                        BinaryenConst(module, BinaryenLiteralInt64(-1))
                    )
                );

                BinaryenExpressionRef* blockExpressions = new BinaryenExpressionRef[expressions.size()];
                for (int i = 0; i < expressions.size(); i++) {
                    blockExpressions[i] = expressions.at(i);
                }
            
                return BinaryenBlock(module, NULL, blockExpressions, expressions.size(), functionReturnType);
            }

            WasmClosure closure = WasmClosure::clone(closureTemplate);

            vector<Pointer<PointerType::Data>> paramMemPointers;

            // TODO: This can be improved by checking if the arity will be 0 before adding anything to memory
            // That way, we save a bunch of store and load calls, and can just skip to the call_indirect
            for (auto arg : funcInvNode->getParameters()->getElements()) {
                int byteSize = getByteSizeForType(dynamic_pointer_cast<TypeDeclarationNode>(arg->getResolvedType()));
                int memLocation = memoryOffset;

                paramMemPointers.push_back(Pointer<PointerType::Data>(memLocation));

                memoryOffset += byteSize;

                // Store each passed argument into memory
                expressions.push_back(
                    BinaryenStore(
                        module,
                        byteSize,
                        0,
                        0,
                        BinaryenConst(module, BinaryenLiteralInt32(memLocation)),
                        generate(arg, module),
                        getBinaryenTypeFromTypeDeclaration(dynamic_pointer_cast<TypeDeclarationNode>(arg->getResolvedType())),
                        MEMORY_NAME.c_str()
                    )
                );
            }

            closure.addArgs(paramMemPointers);

            vector<BinaryenExpressionRef> storageExpressions = generateClosureMemoryStore(closure, module);

            copy(storageExpressions.begin(), storageExpressions.end(), back_inserter(expressions));

            // If we're at 0 arity we can go ahead and execute the function call
            if (closure.getArity() == 0) {
                BinaryenExpressionRef* operands = new BinaryenExpressionRef[closure.getArgPointers().size()];
            
                for (int i = 0; i < closure.getArgPointers().size(); i++) {
                    shared_ptr<ASTNode> arg = funcInvNode->getParameters()->getElements().at(i);

                    operands[i] = BinaryenLoad(
                        module,
                        getByteSizeForType(dynamic_pointer_cast<TypeDeclarationNode>(arg->getResolvedType())),
                        false, // TODO: Support signed values!
                        0,
                        0,
                        getBinaryenTypeFromTypeDeclaration(dynamic_pointer_cast<TypeDeclarationNode>(arg->getResolvedType())), // TODO: fix the hardcoded stuff here
                        BinaryenConst(module, BinaryenLiteralInt32(closure.getArgPointers().at(i).getAddress())),
                        MEMORY_NAME.c_str()
                    );
                }

                pair<BinaryenType, BinaryenType> fnTypes = getBinaryenTypeForFunctionDeclaration(ref);

                expressions.push_back(
                    BinaryenCallIndirect(
                        module,
                        FN_TABLE_NAME.c_str(),
                        BinaryenConst(module, BinaryenLiteralInt32(closure.getFunctionPointer().getAddress())),
                        operands,
                        closure.getArgPointers().size(),
                        fnTypes.first,
                        fnTypes.second
                    )
                );
            } else {
                expressions.push_back(BinaryenConst(module, BinaryenLiteralInt32(closure.getPointer().getAddress())));
            }

            BinaryenExpressionRef* blockExpressions = new BinaryenExpressionRef[expressions.size()];
            for (int i = 0; i < expressions.size(); i++) {
                blockExpressions[i] = expressions.at(i);
            }
        
            return BinaryenBlock(module, NULL, blockExpressions, expressions.size(), BinaryenTypeInt64());
        }

        shared_ptr<FunctionInvocationNode> ref = dynamic_pointer_cast<FunctionInvocationNode>(reference);    
        string refIdentifier = passedRefIdentifier;

        if (refIdentifier == "") {
            refIdentifier = dynamic_pointer_cast<IdentifierNode>(ref->getIdentifier())->getIdentifier();
        }

        cout << "refidentifier is " << refIdentifier << endl;

        vector<BinaryenExpressionRef> expressions;
        vector<int> paramMemPointers;

        string qualifiedInvName = Compiler::getQualifiedFunctionIdentifier(funcInvIdentifier, funcInvNode);

        auto inScope = scope.lookup(qualifiedInvName);

        for (auto arg : funcInvNode->getParameters()->getElements()) {
            int byteSize = getByteSizeForType(dynamic_pointer_cast<TypeDeclarationNode>(arg->getResolvedType()));
            int memLocation = memoryOffset;

            paramMemPointers.push_back(memLocation);
            
            // TODO: instead of incrementing memoryoffset each time, lets change to only increment after the loop, and then
            // for each generated store operation just use the offset field instead of 0
            memoryOffset += byteSize;

            expressions.push_back(
                BinaryenStore(
                    module,
                    byteSize,
                    0,
                    0,
                    BinaryenConst(module, BinaryenLiteralInt32(memLocation)),
                    generate(arg, module),
                    getBinaryenTypeFromTypeDeclaration(dynamic_pointer_cast<TypeDeclarationNode>(arg->getResolvedType())),
                    MEMORY_NAME.c_str()
                )
            );
        }

        BinaryenExpressionPrint(
        BinaryenLoad(
            module,
            4,
            false,
            0,
            0,
            BinaryenTypeInt32(),
            BinaryenLocalGet( // TODO: need to check if is -1 and globalget instead
                module,
                inScope.value()->getMappedBinaryenIndex(),
                BinaryenTypeInt32()
            ),
            MEMORY_NAME.c_str()
        )
        );

        //functionNameToClosureTemplateMap


        cout << "teehee" << endl;
    }

    BinaryenExpressionRef CodeGen::generateControlFlow(shared_ptr<ControlFlowNode> controlFlowNode, BinaryenModuleRef &module) {
        controlFlowNode->getConditionExpressionPairs();

        BinaryenExpressionRef expr = NULL;

        // WASM doesnt support else-if structured natively, so we merge the else-ifs into nested else blocks that have ifs
        // inside of them. So the following:
        // if (x == 1) {
        // } else if (x == 2) {
        // } else if (x == 3) {
        // } else {
        // }
        //
        // becomes this:
        // if (x == 1) {
        // } else {
        //   if (x == 2) {
        //   } else {
        //     if (x == 3) {
        //     } else {
        //     }
        //   }
        // }
        for (int i = controlFlowNode->getConditionExpressionPairs().size() - 1; i >= 0; i--) {
            pair<shared_ptr<ASTNode>, shared_ptr<ASTNode>> cndExprPair = controlFlowNode->getConditionExpressionPairs().at(i);

            // Handle the else case
            if (cndExprPair.first == nullptr) {
                expr = generate(cndExprPair.second, module);
                continue;
            }

            expr = BinaryenIf(
                module,
                generate(cndExprPair.first, module),
                generate(cndExprPair.second, module),
                expr
            );
        }

        return expr;
    }

    BinaryenExpressionRef CodeGen::generateIdentifier(shared_ptr<IdentifierNode> identNode, BinaryenModuleRef &module) {
        string identName = identNode->getIdentifier();
        cout << "generating " << identName << endl;
        optional<string> scopeRef = scopeReferences.lookup(identName);

        if (scopeRef) {
            cout << identName << " references " << scopeRef.value() << endl;
            identName = scopeRef.value();
        }

        shared_ptr<ASTNode> identInScope = scope.lookup(identName).value();

        // The ident in this case may refer to a parameter to a function, which may not have a resolvedType
        shared_ptr<TypeDeclarationNode> type = dynamic_pointer_cast<TypeDeclarationNode>(
            identInScope->getResolvedType() 
                ? identInScope->getResolvedType()
                : identInScope->getValue()
        );

        cout << "resolvedtype of " << identName << " is " << type->toJSON() << endl;

        if (identInScope->getMappedBinaryenIndex() == -1) {
            return BinaryenGlobalGet(
                module,
                identName.c_str(),
                getBinaryenTypeFromTypeDeclaration(type)
            );
        }

        return BinaryenLocalGet(
            module,
            identInScope->getMappedBinaryenIndex(),
            getBinaryenTypeFromTypeDeclaration(type)
        );
    }

    BinaryenExpressionRef CodeGen::generateBinaryOperation(shared_ptr<BinaryOperationNode> binOpNode, BinaryenModuleRef &module) {
        if (binOpNode->getOperator() == Lexemes::EXPONENT) {
            return generateExponentOperation(binOpNode, module);
        }

        BinaryenExpressionRef binaryenLeft = generate(binOpNode->getLeft(), module);
        BinaryenExpressionRef binaryenRight = generate(binOpNode->getRight(), module);

        if (!binaryenLeft || !binaryenRight) {
            throw runtime_error("Invalid operand types for binary operation");
        }

        if (dynamic_pointer_cast<TypeDeclarationNode>(binOpNode->getLeft()->getResolvedType())->getType() == DataTypes::STRING) {
            return generateStringBinaryOperation(binOpNode->getOperator(), binaryenLeft, binaryenRight, module);
        } 

        BinaryenOp op = getBinaryenOpFromBinOpNode(binOpNode);

        return BinaryenBinary(
            module,
            op,
            binaryenLeft,
            binaryenRight
        );
    }

    BinaryenExpressionRef CodeGen::generateStringBinaryOperation(string op, BinaryenExpressionRef left, BinaryenExpressionRef right, BinaryenModuleRef &module) {
        if (op == Lexemes::PLUS) {
            return BinaryenStringConcat(module, left, right);
        } else if (op == Lexemes::INEQUALITY) {
            // Binaryen supports string equality checks but not inequality checks, so we need to wrap an equality check
            // in a unary NOT to achieve the same effect
            return BinaryenUnary(
                module,
                BinaryenEqZInt32(),
                BinaryenStringEq(module, BinaryenStringEqEqual(), left, right)
            );
        }

        return BinaryenStringEq(module, BinaryenStringEqEqual(), left, right);
    }

    BinaryenExpressionRef CodeGen::generateUnaryOperation(shared_ptr<UnaryOperationNode> unaryOpNode, BinaryenModuleRef &module) {
        BinaryenExpressionRef binaryenVal = generate(unaryOpNode->getValue(), module);

        if (!binaryenVal) {
            throw runtime_error("Invalid operand type for unary operation");
        }

        if (unaryOpNode->getOperator() == Lexemes::NOT) {
            return BinaryenUnary(module, BinaryenEqZInt64(), binaryenVal);
        }

        // Must be a negative. Multiply by negative 1
        return BinaryenBinary(
            module,
            BinaryenMulInt64(),
            binaryenVal,
            BinaryenConst(module, BinaryenLiteralInt64(-1))
        );
    }

    BinaryenExpressionRef CodeGen::generateNumberLiteral(shared_ptr<LiteralNode> literalNode, BinaryenModuleRef &module) {
        return BinaryenConst(
            module,
            BinaryenLiteralInt64(stoi(literalNode->getLiteralValue()))
        );
    }

    BinaryenExpressionRef CodeGen::generateStringLiteral(shared_ptr<LiteralNode> literalNode, BinaryenModuleRef &module) {
        return BinaryenStringConst(module, literalNode->getLiteralValue().c_str());
    }

    BinaryenExpressionRef CodeGen::generateBooleanLiteral(shared_ptr<LiteralNode> literalNode, BinaryenModuleRef &module) {
        return BinaryenConst(
            module,
            BinaryenLiteralInt32(literalNode->getLiteralValue() == "true" ? 1 : 0)
        );
    }

    BinaryenExpressionRef CodeGen::generateExponentOperation(shared_ptr<BinaryOperationNode> binOpNode, BinaryenModuleRef &module) {
        BinaryenExpressionRef binaryenLeft = generate(binOpNode->getLeft(), module);
        BinaryenExpressionRef binaryenRight = generate(binOpNode->getRight(), module);

        if (!binaryenLeft || !binaryenRight) {
            throw runtime_error("Invalid operand types for binary operation");
        }

        return BinaryenCall(
            module,
            "Theta.Math.pow",
            (BinaryenExpressionRef[]){ binaryenLeft, binaryenRight },
            2,
            BinaryenTypeInt64()
        );
    }

    void CodeGen::generateSource(shared_ptr<SourceNode> sourceNode, BinaryenModuleRef &module) {
        if (sourceNode->getValue()->getNodeType() != ASTNode::CAPSULE) {
            BinaryenExpressionRef body = generate(sourceNode->getValue(), module);

            if (!body) {
                throw runtime_error("Invalid body type for source node");
            }

            shared_ptr<TypeDeclarationNode> returnType = dynamic_pointer_cast<TypeDeclarationNode>(sourceNode->getValue()->getResolvedType());

            BinaryenFunctionRef mainFn = BinaryenAddFunction(
                module,
                "main",
                BinaryenTypeNone(),
                getBinaryenTypeFromTypeDeclaration(returnType),
                NULL,
                0,
                body
            );

            BinaryenAddFunctionExport(module, "main", "main");
        } else {
            generate(sourceNode->getValue(), module);
        }
    }

    vector<BinaryenExpressionRef> CodeGen::generateClosureMemoryStore(WasmClosure &closure, BinaryenModuleRef &module) {
        // At least 4 bytes for the fn_idx and 4 bytes for the arity. Then 4 bytes for each parameter the closure takes.
        // We also multiply the remaining arity, since not all parameters may have been applied to the function
        int totalMemSize = 8 + (closure.getArgPointers().size() * 4) + (closure.getArity() * 4);
        int memLocation = memoryOffset;

        memoryOffset += totalMemSize;

        vector<int> closureDataSegments = { closure.getFunctionPointer().getAddress(), closure.getArity() };
        for (int i = 0; i < closure.getArgPointers().size(); i++) {
            closureDataSegments.push_back(closure.getArgPointers().at(i).getAddress());
        }

        vector<BinaryenExpressionRef> expressions;

        for (int i = 0; i < closureDataSegments.size(); i++) {
            // Don't store uninitialized pointers
            if (closureDataSegments.at(i) == -1) continue;

            expressions.push_back(
                BinaryenStore(
                    module,
                    4,
                    i * 4,
                    0,
                    BinaryenConst(module, BinaryenLiteralInt32(memLocation)),
                    BinaryenConst(module, BinaryenLiteralInt32(closureDataSegments.at(i))),
                    BinaryenTypeInt32(),
                    MEMORY_NAME.c_str()
                )
            );
        }

        closure.setAddress(memLocation);

        return expressions;
    }

    BinaryenOp CodeGen::getBinaryenOpFromBinOpNode(shared_ptr<BinaryOperationNode> binOpNode) {
        string op = binOpNode->getOperator();

        if (op == Lexemes::PLUS) return BinaryenAddInt64();
        if (op == Lexemes::MINUS) return BinaryenSubInt64();
        if (op == Lexemes::DIVISION) return BinaryenDivSInt64();
        if (op == Lexemes::TIMES) return BinaryenMulInt64();
        if (op == Lexemes::MODULO) return BinaryenRemSInt64();

        string dataType = dynamic_pointer_cast<TypeDeclarationNode>(binOpNode->getLeft()->getResolvedType())->getType();

        if (op == Lexemes::EQUALITY && dataType == DataTypes::NUMBER) return BinaryenEqInt64();
        if (op == Lexemes::EQUALITY && dataType == DataTypes::BOOLEAN) return BinaryenEqInt32();
        if (op == Lexemes::INEQUALITY && dataType == DataTypes::NUMBER) return BinaryenNeInt64();
        if (op == Lexemes::INEQUALITY && dataType == DataTypes::BOOLEAN) return BinaryenEqInt32();
        if (op == Lexemes::LT && dataType == DataTypes::NUMBER) return BinaryenLtSInt64();
        if (op == Lexemes::GT && dataType == DataTypes::NUMBER) return BinaryenGtSInt64();
    }

    BinaryenType CodeGen::getBinaryenTypeFromTypeDeclaration(shared_ptr<TypeDeclarationNode> typeDeclaration) {
        if (typeDeclaration->getType() == DataTypes::NUMBER) return BinaryenTypeInt64();
        if (typeDeclaration->getType() == DataTypes::STRING) return BinaryenTypeStringref();
        if (typeDeclaration->getType() == DataTypes::BOOLEAN) return BinaryenTypeInt32();

        // Function references are returned as i32 pointers to a closure in the function table
        if (typeDeclaration->getType() == DataTypes::FUNCTION) return BinaryenTypeInt32();
    }

    pair<BinaryenType, BinaryenType> CodeGen::getBinaryenTypeForFunctionDeclaration(shared_ptr<FunctionDeclarationNode> function) {
        int totalParams = function->getParameters()->getElements().size();

        BinaryenType* types = new BinaryenType[totalParams];
        for (int i = 0; i < totalParams; i++) {
            types[i] = getBinaryenTypeFromTypeDeclaration(dynamic_pointer_cast<TypeDeclarationNode>(function->getParameters()->getElements().at(i)->getValue()));
        }
    
        BinaryenType paramType = BinaryenTypeCreate(types, totalParams);

        BinaryenType returnType = getBinaryenTypeFromTypeDeclaration(
            dynamic_pointer_cast<TypeDeclarationNode>(TypeChecker::getFunctionReturnType(function))
        );

        return make_pair(paramType, returnType);
    }

    void CodeGen::hoistCapsuleElements(vector<shared_ptr<ASTNode>> elements) {
        scope.enterScope();
        scopeReferences.enterScope();

        for (auto ast : elements) bindIdentifierToScope(ast);
    }

    void CodeGen::bindIdentifierToScope(shared_ptr<ASTNode> ast) {
        string identifier = dynamic_pointer_cast<IdentifierNode>(ast->getLeft())->getIdentifier();

        if (ast->getRight()->getNodeType() == ASTNode::FUNCTION_DECLARATION) {
            identifier = Compiler::getQualifiedFunctionIdentifier(identifier, ast->getRight());

            int totalParams = dynamic_pointer_cast<FunctionDeclarationNode>(ast->getRight())->getParameters()->getElements().size();

            functionNameToClosureTemplateMap.insert(make_pair(
                identifier,
                WasmClosure(functionNameToClosureTemplateMap.size(), totalParams)
            ));
        } 

        scope.insert(identifier, ast->getRight());
    }

    void CodeGen::registerModuleFunctions(BinaryenModuleRef &module) {
        BinaryenAddTable(
            module,
            FN_TABLE_NAME.c_str(),
            functionNameToClosureTemplateMap.size(),
            functionNameToClosureTemplateMap.size(),
            BinaryenTypeFuncref()
        );

        const char** fnNames = new const char*[functionNameToClosureTemplateMap.size()];

        for (auto& [fnName, fnRef] : functionNameToClosureTemplateMap) {
            fnNames[fnRef.getFunctionPointer().getAddress()] = fnName.c_str();
        }

        BinaryenAddActiveElementSegment(
            module,
            FN_TABLE_NAME.c_str(),
            "0",
            fnNames,
            functionNameToClosureTemplateMap.size(),
            BinaryenConst(module, BinaryenLiteralInt32(0))
        );
    }

    int CodeGen::getByteSizeForType(shared_ptr<TypeDeclarationNode> type) {
        if (type->getType() == DataTypes::NUMBER) return 8;
        if (type->getType() == DataTypes::BOOLEAN) return 4;
        // TODO: Figure out if this holds true. According to
        // https://github.com/WebAssembly/stringref/blob/main/proposals/stringref/Overview.md#the-stringref-facility
        // stringrefs are either i32 or i64
        if (type->getType() == DataTypes::STRING) return 4; 

        cout << "Not implemented for type: " << type->getType() << endl;
        throw new runtime_error("Not implemented");
    }

    int CodeGen::getByteSizeForType(BinaryenType type) {
        if (type == BinaryenTypeInt32()) return 4;
        if (type == BinaryenTypeInt64()) return 8;
        if (type == BinaryenTypeStringref()) return 4;

        cout << "Not implemented for type: " << to_string(type) << endl;
        throw new runtime_error("Not implemented");
    }

    BinaryenModuleRef CodeGen::importCoreLangWasm() {
        ifstream file(resolveAbsolutePath("wasm/ThetaLangCore.wat"), ios::binary);
        if (!file.is_open()) {
            cerr << "Failed to open the file." << endl;
            return nullptr;
        }

        vector<char> buffer(istreambuf_iterator<char>(file), {});

        if (buffer.empty()) {
            cerr << "Failed to read the file or the file is empty." << endl;
            return nullptr;
        }

        // Add a null terminator at the end
        buffer.push_back('\0');

        BinaryenModuleRef module = BinaryenModuleParse(buffer.data());

        file.close();

        return module;
    }

    string CodeGen::resolveAbsolutePath(string relativePath) {
        char path[PATH_MAX];

        #ifdef __APPLE__
            uint32_t size = sizeof(path);
            if (_NSGetExecutablePath(path, &size) != 0) {
                cerr << "Buffer too small; should be resized to " << size << " bytes\n" << endl;
                return "";
            }
        #else
            ssize_t count = readlink("/proc/self/exe", path, PATH_MAX);
            if (count <= 0) {
                cerr << "Failed to read the path of the executable." << endl;
                return "";
            }
            path[count] = '\0'; // Ensure null termination
        #endif

        char realPath[PATH_MAX];
        if (realpath(path, realPath) == NULL) {
            cerr << "Error resolving symlink for " << path << endl;
            return "";
        }
            
        string exePath = string(realPath);
        if (exePath.empty()) return "";

        char *pathCStr = strdup(exePath.c_str());
        string dirPath = dirname(pathCStr); // Use dirname to get the directory part
        free(pathCStr); // Free the duplicated string

        return dirPath + "/" + relativePath;
    }

    string CodeGen::generateFunctionHash(shared_ptr<FunctionDeclarationNode> function) {
        hash<string> hasher;

        size_t hashed = hasher(function->toJSON());

        ostringstream stream;

        stream << hex << nouppercase << setw(sizeof(size_t) * 2) << setfill('0');

        stream << hashed;

        return stream.str();
    }
}
