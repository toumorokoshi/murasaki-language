#include <map>
#include <string>
#include <vector>
#include "../lexer/tokens.hpp"
#include "../vm/vm.hpp"
#include "yaml-cpp/yaml.h"
#include <string.h>
#include <iostream>

#ifndef PARSER_NODES_HPP
#define PARSER_NODES_HPP

namespace parser {

  typedef std::vector<VM::GInstruction> GInstructionVector;
  class FunctionBuilder;

  class PNode {
  public:
    virtual YAML::Node* toYaml() = 0;
    virtual ~PNode() {};
  };

  /* Statements */

  class PBlock;

  class PStatement : public PNode {
  public:
    virtual void generateStatement(VM::GScope*, GInstructionVector&) = 0;
  };

  typedef std::vector<PStatement*> PStatements;

  class PExpression : public PStatement {
  public:

    virtual void generateStatement(VM::GScope* s, GInstructionVector& i) {
      generateExpression(s, i);
    }

    virtual VM::GObject* generateExpression(VM::GScope*, GInstructionVector&) = 0;
    virtual VM::GType* getType(VM::GScope*) = 0;
  };

  typedef std::vector<PExpression*> PExpressions;

  class PAssign : public PStatement {
  public:
    PExpression* identifier;
    PExpression* expression;

    virtual YAML::Node* toYaml();
    virtual void generateStatement(VM::GScope*, GInstructionVector&) {}

    PAssign(PExpression* _identifier,
            PExpression* _expression) :
      identifier(_identifier), expression(_expression) {}
  };

  class PIncrement : public PStatement {
  public:
    PExpression* identifier;
    PExpression* expression;

    virtual YAML::Node* toYaml();
    virtual void generateStatement(VM::GScope*, GInstructionVector&);

    PIncrement(PExpression* _identifier, PExpression* _expression) :
      identifier(_identifier), expression(_expression) {}
  };

  class PDecrement: public PStatement {
  public:
    PExpression* identifier;
    PExpression* expression;

    virtual YAML::Node* toYaml();
    virtual void generateStatement(VM::GScope*, GInstructionVector&) {}

    PDecrement(PExpression* _identifier, PExpression* _expression) :
      identifier(_identifier), expression(_expression) {}
  };

  class PDeclare : public PStatement {
  public:
    std::string name;
    PExpression* expression;

    virtual YAML::Node* toYaml();
    virtual void generateStatement(VM::GScope*, GInstructionVector&);

    PDeclare(std::string _name,
             PExpression* _expression) :
      name(_name), expression(_expression) {}
  };

  class PForeachLoop : public PStatement {
  public:
    std::string variableName;
    PExpression* iterableExpression;
    PBlock* block;

    virtual YAML::Node* toYaml();
    virtual void generateStatement(VM::GScope*, GInstructionVector&) {}

    PForeachLoop(std::string _variableName,
             PExpression* _iterableExpression,
             PBlock* _block) :
      variableName(_variableName),
      iterableExpression(_iterableExpression),
      block(_block) {}
  };

  class PForLoop : public PStatement {
  public:
    PStatement* initializer;
    PExpression* condition;
    PStatement* incrementer;
    PBlock* body;

    virtual YAML::Node* toYaml();
    virtual void generateStatement(VM::GScope*, GInstructionVector&);

    PForLoop(PStatement* _initializer,
             PExpression* _condition,
             PStatement* _incrementer,
             PBlock* _body) :
      initializer(_initializer), condition(_condition),
      incrementer(_incrementer), body(_body) {}
  };

  typedef std::pair<std::string, std::string> PArgumentDefinition;
  typedef std::vector<PArgumentDefinition*> PArgumentList;

  class PFunctionDeclaration : public PStatement {
  public:
    std::string returnType;
    std::string name;
    PArgumentList& arguments;
    PBlock* body;

    virtual YAML::Node* toYaml();
    virtual void generateStatement(VM::GScope*, GInstructionVector&) {}

    PFunctionDeclaration(std::string _returnType,
                         std::string _name,
                         PArgumentList& _arguments,
                         PBlock* _body) :
      returnType(_returnType), name(_name),
      arguments(_arguments), body(_body) {}
  };

  class PIfElse : public PStatement {
  public:
    PExpression* condition;
    PBlock* trueBlock;
    PBlock* falseBlock;

    virtual YAML::Node* toYaml();
    virtual void generateStatement(VM::GScope*, GInstructionVector&) {}

    PIfElse(PExpression* _condition,
            PBlock* _trueBlock,
            PBlock* _falseBlock):
      condition(_condition), trueBlock(_trueBlock),
      falseBlock(_falseBlock) {}
  };

  class PReturn : public PStatement {
  public:
    PExpression* expression;

    virtual YAML::Node* toYaml();
    virtual void generateStatement(VM::GScope*, GInstructionVector&) {}

    PReturn(PExpression* _expression) :
      expression(_expression) {}
  };

  /* expressions */

  class PConstantBool : public PExpression {
  public:
    bool value;

    virtual YAML::Node* toYaml();
    virtual VM::GType* getType(VM::GScope*) { return VM::getBoolType(); }
    virtual VM::GObject* generateExpression(VM::GScope* scope, GInstructionVector& instructions) {
      auto target = scope->frame->allocateObject(VM::getBoolType());
      instructions.push_back(VM::GInstruction {
          VM::LOAD_CONSTANT_BOOL, new VM::GOPARG[2] {
            { target->registerNum }, VM::GOPARG { .asBool = value }
          }});
      return target;
    }

    PConstantBool(bool _value) : value(_value) {}
  };

  class PConstantInt : public PExpression {
  public:
    int value;

    virtual YAML::Node* toYaml();
    virtual VM::GType* getType(VM::GScope*) { return VM::getInt32Type(); }
    virtual VM::GObject* generateExpression(VM::GScope* s, GInstructionVector& i) {
      auto target = s->frame->allocateObject(VM::getInt32Type());
      i.push_back(VM::GInstruction {
          VM::LOAD_CONSTANT_INT, new VM::GOPARG[2] {
            { target->registerNum }, { value }
          }});
      return target;
    }

    PConstantInt(int _value) : value(_value) {}
  };

  class PConstantFloat : public PExpression {
  public:
    double value;

    virtual YAML::Node* toYaml();
    virtual VM::GType* getType(VM::GScope*) { return VM::getFloatType(); }

    virtual VM::GObject* generateExpression(VM::GScope* s, GInstructionVector& instructions) {
      auto target = s->frame->allocateObject(VM::getFloatType());
        instructions.push_back(VM::GInstruction {
            VM::LOAD_CONSTANT_FLOAT, new VM::GOPARG[2] {
              { target->registerNum }, VM::GOPARG{ .asFloat = value }
            }});
        return target;
    }

    PConstantFloat(double _value) : value(_value) {}
  };

  class PConstantString : public PExpression {
  public:
    std::string value;

    virtual YAML::Node* toYaml();
    virtual VM::GType* getType(VM::GScope*) { return VM::getStringType(); }
    virtual VM::GObject* generateExpression(VM::GScope* s, GInstructionVector& i) {
      auto target = s->frame->allocateObject(VM::getFloatType());
      i.push_back(VM::GInstruction {
          VM::LOAD_CONSTANT_STRING, new VM::GOPARG[2] {
            { target->registerNum }, VM::GOPARG { .asString = value.c_str() }
          }});
      return target;
    }

    PConstantString(std::string _value) : value(_value) {};
  };

  class PIdentifier : public PExpression {
  public:
    std::string name;

    virtual YAML::Node* toYaml();
    virtual VM::GType* getType(VM::GScope* scope) {
      auto object = scope->getObject(name);
      return object != NULL ? object->type : VM::getNoneType();
    }

    virtual VM::GObject* generateExpression(VM::GScope*, GInstructionVector&);

    PIdentifier(std::string _name) : name(_name) {}
  };

  class PArray : public PExpression {
  public:
    std::vector<PExpression*>& elements;
    virtual YAML::Node* toYaml();
    virtual VM::GType* getType(VM::GScope*) { return VM::getNoneType(); }
    virtual VM::GObject* generateExpression(VM::GScope*, GInstructionVector&) { return NULL; }

    PArray(std::vector<PExpression*>& _elements) :
      elements(_elements) {}
  };

  class PCall : public PExpression {
  public:
    std::string name;
    PExpressions& arguments;

    virtual YAML::Node* toYaml();
    virtual VM::GType* getType(VM::GScope*) { return VM::getNoneType(); }
    virtual VM::GObject* generateExpression(VM::GScope*, GInstructionVector&);

    PCall(std::string _name,
          PExpressions& _arguments) :
      name(_name), arguments(_arguments) {}
  };

  class PArrayAccess : public PExpression {
  public:
    PExpression* value;
    PExpression* index;

    virtual YAML::Node* toYaml();
    virtual VM::GType* getType(VM::GScope*) { return VM::getNoneType(); };
    virtual VM::GObject* generateExpression(VM::GScope*, GInstructionVector&) { return NULL; }

    PArrayAccess(PExpression* _value,
                 PExpression* _index) :
      value(_value), index(_index) {}
  };

  class PMethodCall : public PExpression {
  public:
    PExpression* currentValue;
    std::string methodName;
    PExpressions& arguments;

    virtual YAML::Node* toYaml();
    virtual VM::GType* getType(VM::GScope*) { return NULL; }
    virtual VM::GObject* generateExpression(VM::GScope*, GInstructionVector&) { return NULL; }

    PMethodCall(PExpression* _currentValue,
                std::string _methodName,
                PExpressions& _arguments) :
      currentValue(_currentValue),
      methodName(_methodName),
      arguments(_arguments) {}
  };

  class PBinaryOperation : public PExpression {
  public:
    PExpression* lhs;
    lexer::L op;
    PExpression* rhs;

    virtual YAML::Node* toYaml();
    virtual VM::GType* getType(VM::GScope* s) { return lhs->getType(s); }
    virtual VM::GObject* generateExpression(VM::GScope*, GInstructionVector&);

    PBinaryOperation(PExpression* _lhs,
                     lexer::L _op,
                     PExpression* _rhs) :
      lhs(_lhs), op(_op), rhs(_rhs) {}
  };

  /* block */

  class PBlock : public PNode {
  public:
    PStatements statements;
    virtual YAML::Node* toYaml();
    GInstructionVector& generate(VM::GScope*);
  };

  // we'll stick it here for now, move it somewhere else later
  VM::GInstruction* generateRoot(VM::GScope*, PBlock*);
}

#endif
