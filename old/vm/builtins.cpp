#include "./class.hpp"
#include "./builtins.hpp"
#include "./exceptions.hpp"
#include "./interface.hpp"
#include "./arithmetic.hpp"
#include "basic_types/int.hpp"
#include "builtins/file.hpp"
#include "function.hpp"
#include <iostream>

namespace VM {

  /** print() **/
  VMObject* vm_print(std::vector<VMObject*>& args) {
    auto string = (VMString*) (*args.begin())->call("toString",
                                                    *new std::vector<VMObject*>());
    std::cout << string->value << std::endl;
    return NULL;
  }

  VMFunction& _getVMPrint() {
    auto argumentTypes = new std::vector<VMClass*>();
    argumentTypes->push_back(getVMIStringable());

    return *new VMRawFunctionWrapper(getNoneType(), *argumentTypes, (VMRawFunction) &vm_print);
  }


  VMScope& getBuiltinScope() {
    static VMScope _BUILTIN_SCOPE;
    static bool _initialized = false;
    if (!_initialized) {
      _BUILTIN_SCOPE.locals["__add"] = &_getVMAdd();
      _BUILTIN_SCOPE.locals["__sub"] = &_getVMSub();
      _BUILTIN_SCOPE.locals["__mul"] = &_getVMMul();
      _BUILTIN_SCOPE.locals["__div"] = &_getVMDiv();
      _BUILTIN_SCOPE.locals["print"] = &_getVMPrint();
      _BUILTIN_SCOPE.localTypes["print"] = getVMFunctionClass();
      // built in classes
      _BUILTIN_SCOPE.locals["IntRange"] = getVMIntRangeClass()->constructor;
      _BUILTIN_SCOPE.localTypes["IntRange"] = getVMFunctionClass();
      _BUILTIN_SCOPE.locals["Int"] = _getVMIntConstructor();
      _BUILTIN_SCOPE.locals["stdin"] = _getVMStdin();
      _BUILTIN_SCOPE.locals["stdin"] = _getVMStdin();
      _initialized = true;
    }
    return _BUILTIN_SCOPE;
  }
}