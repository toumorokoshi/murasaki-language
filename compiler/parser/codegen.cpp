#include "nodes.hpp"
#include "exceptions.hpp"
#include <iostream>

#define debug(s);
// #define debug(s) std::cout << s << std::endl;

using namespace VM;
using namespace lexer;

namespace parser {

  GType* evaluateType(std::string typeName) {
    if (typeName == "Int") { return getInt32Type(); }
    else if (typeName == "Bool") { return getBoolType(); }
    else if (typeName == "String") { return getStringType(); }
    else if (typeName == "None") { return getNoneType(); }

    throw ParserException("Cannot find class " + typeName);
  }

  GInstruction* generateRoot(VM::GScope* scope, PBlock* block) {
    auto instructions = block->generate(scope);
    instructions->push_back(GInstruction { END, NULL });
    return &(*instructions)[0];
  }

  GInstructionVector* PBlock::generate(VM::GScope* scope) {
    auto instructions = new GInstructionVector;
    for (auto statement : statements) {
      statement->generateStatement(scope, *instructions);
    }
    return instructions;
  }

  GObject* PCall::generateExpression(VM::GScope* scope,
                                     GInstructionVector& instructions) {
    if (name == "print") {
      instructions.push_back(GInstruction {
          PRINT,
            new GObject*[1] {
            arguments[0]->generateExpression(scope, instructions)
              }});
      return getNoneObject();
    }
    return getNoneObject();
  }

  void PDeclare::generateStatement(VM::GScope* scope,
                                   GInstructionVector& instructions) {
    auto value = expression->generateExpression(scope, instructions);
    auto newValue = new GObject { value->type, {0} };
    scope->locals[name] = newValue;
    instructions.push_back(GInstruction {
        SET, new GObject*[2] { value, newValue }
      });
  }

  GObject* intToFloat(GObject* integer, GInstructionVector& instructions) {
    auto castResult = new GObject { getFloatType(), {0}};
    instructions.push_back(GInstruction {
        GOPCODE::INT_TO_FLOAT, new GObject*[2] { integer, castResult }
      });
    return castResult;
  }

  GObject* PBinaryOperation::generateExpression(VM::GScope* scope,
                                                GInstructionVector& instructions) {
    auto lhsObject = lhs->generateExpression(scope, instructions);
    auto rhsObject = rhs->generateExpression(scope, instructions);

    // cast as necessary
    if (lhsObject->type == getFloatType() && rhsObject->type == getInt32Type()) {
      rhsObject = intToFloat(rhsObject, instructions);
    }
    if (lhsObject->type == getInt32Type() && rhsObject->type == getFloatType()) {
      lhsObject = intToFloat(lhsObject, instructions);
    }

    if (lhsObject->type != rhsObject->type) {
      throw ParserException("type mismatch during binary operation!");
    }

    GObject* resultObject;
    GOPCODE opCode;

    switch (op) {
    case L::LESS_THAN:
      resultObject = new GObject { getBoolType(), { 0 }};
      opCode = GOPCODE::LESS_THAN;
      break;

    case L::PLUS: {
      if (lhsObject->type == getFloatType()) {
        resultObject = new GObject { getFloatType(), { 0 }};
        opCode = GOPCODE::ADD_FLOAT;
        break;

      } else if (lhsObject->type == getInt32Type()) {
        resultObject = new GObject { getInt32Type(), { 0 }};
        opCode = GOPCODE::ADD_INT;
        break;

      }

    }

    default:
      throw ParserException("binary op not implemented");
    }

    instructions.push_back(GInstruction { opCode, new GObject*[3] {
          lhsObject, rhsObject, resultObject
        }});
    return resultObject;
  }

  void PIncrement::generateStatement(VM::GScope* scope,
                                     GInstructionVector& instructions) {
    auto toIncrement = identifier->generateExpression(scope, instructions);
    if (toIncrement->type != getInt32Type()) {
      throw ParserException("only supporting int32 for increment ATM");
    }

    auto incrementer = expression->generateExpression(scope, instructions);
    if (incrementer->type != getInt32Type()) {
      throw ParserException("only supporting int32 for increment ATM");
    }

    instructions.push_back(GInstruction{
        GOPCODE::ADD_INT, new GObject*[3] {
          toIncrement, incrementer, toIncrement
        }
      });
  }

  GObject* PIdentifier::generateExpression(VM::GScope* scope,
                                           GInstructionVector& instructions) {
    auto object = scope->getObject(name);

    if (object == NULL) {
      throw ParserException("Object " + name + " is not defined in this scope!");
    }
    return object;
  }

  void PForLoop::generateStatement(VM::GScope* scope,
                                   GInstructionVector& instructions) {
    initializer->generateStatement(scope, instructions);
    auto statements = body->generate(scope);
    auto forLoopStart = instructions.size();
    instructions.reserve(instructions.size()
                         + distance(statements->begin(), statements->end()));
    instructions.insert(instructions.end(),
                        statements->begin(),
                        statements->end());
    incrementer->generateStatement(scope, instructions);
    auto conditionObject = condition->generateExpression(scope, instructions);
    instructions.push_back(GInstruction {
        GOPCODE::BRANCH, new GObject*[3] {
          conditionObject,
            new GObject { getInt32Type(), { (int) forLoopStart - ((int) instructions.size()) }},
            new GObject { getInt32Type(), { 1 }}
        }
      });
  }

  void PIfElse::generateStatement(VM::GScope* scope,
                                  GInstructionVector& instructions) {
    auto conditionObject = condition->generateExpression(scope, instructions);
    auto trueJump = new GObject { getInt32Type(), {1}};
    auto falseJump = new GObject { getInt32Type(), {0}};
    instructions.push_back(GInstruction {
        GOPCODE::BRANCH, new GObject*[3] { conditionObject, trueJump, falseJump }
      });
    auto startInstruction = (int) instructions.size();

    GScope trueScope(scope);
    auto trueInstructions = trueBlock->generate(&trueScope);

    instructions.reserve(instructions.size()
                         + distance(trueInstructions->begin(),
                                    trueInstructions->end()));
    instructions.insert(instructions.end(),
                        trueInstructions->begin(),
                        trueInstructions->end());

    auto trueToFinish = new GObject { getInt32Type(), {1}};
    auto endOfTBlock = instructions.size();
    instructions.push_back(GInstruction { GOPCODE::GO, new GObject*[1] { trueToFinish } });

    falseJump->value.asInt32 = (int) instructions.size() - startInstruction + 1;
    GScope falseScope(scope);
    auto falseInstructions = falseBlock->generate(&falseScope);

    instructions.reserve(instructions.size()
                         + distance(falseInstructions->begin(),
                                    falseInstructions->end()));
    instructions.insert(instructions.end(),
                        falseInstructions->begin(),
                        falseInstructions->end());

    trueToFinish->value.asInt32 = (int) instructions.size() - endOfTBlock;
  }

  GObject* PArray::generateExpression(VM::GScope* scope,
                                      GInstructionVector& instructions) {
    auto array = new GObject*[elements.size()];
    auto type = getNoneType();
    for (int i = 0; i < elements.size(); i++) {
      array[i] = elements[i]->generateExpression(scope, instructions);
      type = array[i]->type;
    }
    auto arrayObject = new GObject { getArrayType(type), { 0 } };
    arrayObject->value.asArray = new GArray {
      array, (int) elements.size()
    };
    return arrayObject;
  }

  GObject* PArrayAccess::generateExpression(VM::GScope* scope,
                                            GInstructionVector& instructions) {
    auto valueObject = value->generateExpression(scope, instructions);
    auto indexObject = index->generateExpression(scope, instructions);
    auto objectRegister = new GObject { valueObject->type->subTypes, {0} };
    if (indexObject->type->classifier != INT32) {
      throw ParserException("index on array is not an int");
    }
    instructions.push_back(GInstruction {
        GOPCODE::ACCESS_ELEMENT, new GObject*[3] {
          valueObject, indexObject, objectRegister
        }
      });
    return objectRegister;
  }

  // generates the instructions to parse the array
  void parseArrayIterator(std::string varName, GObject* array, PBlock* body,
                          GScope* scope, GInstructionVector& instructions) {
    GScope forScope(scope);
    auto iteratorIndex = new GObject { getInt32Type(), {0}};
    auto iteratorObject = new GObject { array->type->subTypes, {0}};
    auto zero = new GObject { getInt32Type(), { 0 }};
    auto one = new GObject { getInt32Type(), { 1 }};
    auto arraySize = new GObject { getInt32Type(), { 0 }};
    // initialize statement
    forScope.locals[varName] = iteratorObject;
    instructions.push_back(GInstruction { GOPCODE::LENGTH, new GObject*[2] { array, arraySize }});
    instructions.push_back(GInstruction { GOPCODE::SET, new GObject*[2] { zero, iteratorIndex }});

    auto forLoopStart = instructions.size();
    instructions.push_back(GInstruction { GOPCODE::ACCESS_ELEMENT, new GObject*[3] {
          array, iteratorIndex, iteratorObject
        }});
    auto statements = body->generate(&forScope);
    instructions.reserve(instructions.size()
                         + distance(statements->begin(), statements->end()));
    instructions.insert(instructions.end(), statements->begin(), statements->end());
    auto conditionObject = new GObject { getBoolType(), { 0 }};
    instructions.push_back(GInstruction {
        GOPCODE::ADD_INT, new GObject*[3] { iteratorIndex, one, iteratorIndex }
      });
    instructions.push_back(GInstruction {
        GOPCODE::LESS_THAN, new GObject*[3] { iteratorIndex, arraySize, conditionObject }
      });
    instructions.push_back(GInstruction {
        GOPCODE::BRANCH, new GObject*[3] {
          conditionObject,
            new GObject { getInt32Type(), { (int) forLoopStart - (int) instructions.size() }},
            new GObject { getInt32Type(), { 1 }}
        }
      });
  }

  void PForeachLoop::generateStatement(VM::GScope* scope,
                                       GInstructionVector& instructions) {
    auto iterableValue = iterableExpression->generateExpression(scope, instructions);
    // if the value is an array, we iterate through the array first
    if (iterableValue->type->classifier == ARRAY) {
      parseArrayIterator(variableName, iterableValue, block,
                         scope, instructions);
    } else {
      throw ParserException("foreach loop not yet implemented!");
    }
  }




  /* VMClass* evaluateType(std::string typeName) {
    if (typeName == "Int") {
      return getVMIntClass();
    } else if (typeName == "Bool") {
      return getVMBoolClass();
    } else if (typeName == "String") {
      return getVMStringClass();
    } else if (typeName == "None") {
      return getNoneType();
    }
    throw ParserException("Cannot find class " + typeName);
  }

  VMBlock* PBlock::generate(VMScope* scope) {
    auto block = new VMBlock();
    for (auto statement : statements) {
      block->statements.push_back(statement->generateStatement(scope));
    }
    return block;
  }

  VMStatement* PDeclare::generateStatement(VMScope* scope) {
    if (scope->getObjectType(name)) {
      throw ParserException("Cannot redeclare variable " + name);
    }

    auto vmExpression = expression->generateExpression(scope);

    scope->localTypes[name] = expression->getType(scope);
    return new VMDeclare(name, vmExpression);
  }

  VMStatement* PAssign::generateStatement(VMScope* scope) {
    auto vmExpression = expression->generateExpression(scope);

    if (auto pIdentifier = dynamic_cast<PIdentifier*>(identifier)) {
      if (!scope->getObjectType(pIdentifier->name)) {
        throw ParserException("Cannot assign undeclared variable " + pIdentifier->name);
      }

      if (!expression->getType(scope)->matches(scope->localTypes[pIdentifier->name])) {
        throw ParserException("type mismatch in assignment!");
      }

      return new VMAssign(pIdentifier->name, vmExpression);
    } else if (auto pArrayAccess = dynamic_cast<PArrayAccess*>(identifier)) {
      return new VMCallMethod(pArrayAccess->value->generateExpression(scope),
                              "__set",
                              *new std::vector<VMExpression*> {
                                pArrayAccess->index->generateExpression(scope),
                                  vmExpression
                                  });

    }
    throw ParserException("Cannot assign value!");
  }

  VMExpression* PBinaryOperation::generateExpression(VM::VMScope* scope) {
    if (!lhs->getType(scope)->matches(rhs->getType(scope))) {
      throw ParserException("Type mismatch for binary operation!");
    }

    std::string methodName = "";
    switch(op) {

    case PLUS: methodName =  "__add"; break;
    case MINUS: methodName = "__sub"; break;
    case MUL: methodName =   "__mul"; break;
    case DIV: methodName =   "__div"; break;

    default:
      throw ParserException("Cannot find operator!");
    }

    auto arguments = new std::vector<VMExpression*>;
    arguments->push_back(lhs->generateExpression(scope));
    arguments->push_back(rhs->generateExpression(scope));
    return new VMCall(methodName, *arguments);
  }

  VMStatement* PFunctionDeclaration::generateStatement(VMScope* scope) {
    if (scope->localTypes.find(name) != scope->localTypes.end()) {
      throw ParserException("Cannot redeclare " + name);
    }

    VMScope functionScope(scope);
    for (auto argument : arguments) {
      functionScope.localTypes[argument->first] = evaluateType(argument->second);
    }

    auto vmBody = body->generate(&functionScope);

    scope->localTypes[name] = getVMFunctionClass();
    auto function = new VMFunctionDeclaration(name,
                                              evaluateType(returnType),
                                              arguments,
                                              vmBody);
    scope->locals[name] = function;
    return function;
  }

  VMStatement* PReturn::generateStatement(VMScope* scope) {
    return new VMReturn(expression->generateExpression(scope));
  }

  VMClass* PCall::getType(VM::VMScope* scope) {
    auto function = dynamic_cast<VMFunction*>(scope->getObject(name));
    return function->getType();
  }

  VMExpression* PCall::generateExpression(VMScope* scope) {
    if (!scope->getObjectType(name)) {
      throw ParserException("function does not exist in scope: " + name);
    }

    auto scopeType = scope->getObjectType(name);

    if (!getVMFunctionClass()->matches(scopeType)) {
      throw ParserException(name + " is not a function");
    }

    auto function = dynamic_cast<VMFunction*>(scope->getObject(name));


    std::vector<VMClass*> argumentTypes;
    auto argumentExpressions = new std::vector<VMExpression*>;
    for (auto argument : arguments) {
      argumentTypes.push_back(argument->getType(scope));
      argumentExpressions->push_back(argument->generateExpression(scope));
    }

    function->validateTypes(argumentTypes);

    return new VMCall(name, *argumentExpressions);
  }

  VMExpression* PArrayAccess::generateExpression(VM::VMScope* scope) {
    return new VMCallMethod(value->generateExpression(scope),
                            "__get",
                            *new std::vector<VMExpression*> { index->generateExpression(scope) });
  }

  VMExpression* PMethodCall::generateExpression(VM::VMScope* scope) {
    auto vmValue = currentValue->generateExpression(scope);
    auto vmArguments = new std::vector<VMExpression*>;
    for (auto argument : arguments) {
      vmArguments->push_back(argument->generateExpression(scope));
    }
    return new VMCallMethod(vmValue, methodName, *vmArguments);
    } */
}
