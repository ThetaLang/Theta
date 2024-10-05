#pragma once

#include "binaryen-c.h"

namespace Theta {
  class FunctionMetaData {
  public:
    FunctionMetaData(
      int totalParams,
      BinaryenType* paramTypes,
      BinaryenType resultType
    ) : arity(totalParams), params(paramTypes), returnType(resultType) {
      paramType = BinaryenTypeCreate(params, arity);
    };

    int getArity() { return arity; }

    BinaryenType* getParams() { return params; }

    BinaryenType getParamType() { return paramType; }
    
    BinaryenType getReturnType() { return returnType; }

  private:
    int arity;
    BinaryenType* params;
    BinaryenType paramType;
    BinaryenType returnType;
  };
}
