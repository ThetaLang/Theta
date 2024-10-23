#pragma once

#include "RuntimeTypeId.hpp"

namespace Theta {
  struct HeapReference {
    RuntimeTypeId typeId;
    int size;
    int address;
    int previousAddress;

    HeapReference(int addr, int bytes, RuntimeTypeId type) : address(addr), size(bytes), typeId(type) {}
  };
}
