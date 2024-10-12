#pragma once

namespace Theta {
  struct HeapReference {
    int address;
    int previousAddress;

    HeapReference(int addr) : address(addr) {}
    
  };
}
