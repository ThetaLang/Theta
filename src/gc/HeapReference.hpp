#pragma once

#include "DataTypeId.hpp"

namespace Theta {
  struct HeapReference {
    DataTypeId typeId;
    int size;
    int address;
    int previousAddress;

    HeapReference(int addr, int bytes, DataTypeId type) : address(addr), size(bytes), typeId(type) {}
  };
}
