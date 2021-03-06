#include "type.hpp"
#include <functional>
#include <map>

#ifndef VM2_VALUE_HPP
#define VM2_VALUE_HPP

namespace VM {

  union GValue;
  struct GArray;
  struct GObject;
  struct GEnvironmentInstance;
  struct GFunctionInstance;
  typedef GValue* Builtin(GValue*);

  typedef GValue  RawPrimitiveMethod(GValue, GValue*);

  typedef struct {
    GType* returnType;
    RawPrimitiveMethod* rawMethod;
  } PrimitiveMethod;

  typedef std::map<std::string, PrimitiveMethod> PrimitiveMethodMap;

  typedef union GValue {
    int asInt32;
    bool asBool;
    char asChar;
    double asFloat;
    void* asNone;
    GArray* asArray;
    GArray* asTuple;
    GEnvironmentInstance* asModule;
    GEnvironmentInstance* asInstance;
    GFunctionInstance* asFunction;
    Builtin* asBuiltin;
    GType* asType;
    FILE* asFile;
  } GValue;

  typedef struct GArray {
    GValue* elements;
    int size;
  } GArray;

  typedef struct GObject {
    GType* type;
    GValue value;
  } GObject;

  enum GIndexType {
    LOCAL,
    GLOBAL,
    OBJECT_PROPERTY,
  };

  struct GIndex;

  typedef struct GIndex {
    GIndexType indexType;
    GIndex* objectIndex;
    int registerNum;
    GType* type;
  } GIndex;

  GValue* getNoneObject();

  std::string getValueDebugInfo(GValue v);
}

#endif
