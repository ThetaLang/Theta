#include <stdint.h>
#include <emscripten.h>
#include "ShadowStack.hpp"
#include "emscripten/em_macros.h"
#include "HeapReference.hpp"
// The start of the heap as determined by LLVM. This is also where
// any malloc or memory used by the following code will be allocated
extern "C" int __heap_base;

// Theta initially uses 128kb of memory, and can grow it as needed.
int THETA_MEMORY_REGION_INITIAL_SIZE = 1024 * 128; 
int THETA_MEMORY_REGION_BASE;
int THETA_HEAP_BASE;
int THETA_GC_INITIALIZED = false;
int newSpaceBoundary;
int allocationPointer;

// The current GC epoch
int epoch = 1;

extern "C" void print(int32_t);

extern "C" {

  EMSCRIPTEN_KEEPALIVE
  int __Theta_Lang_getAllocationPointer() {
    return allocationPointer;  
  }

  // It's very important that this function gets called before anything else! If it doesn't
  // memory weirdness will ensue and things wont work as expected 
  EMSCRIPTEN_KEEPALIVE
  void __Theta_Lang_initializeGC() {
    // We allocate 1 MB of space after the Clang heap to avoid collisions
    THETA_MEMORY_REGION_BASE = __heap_base + 1024 * 1024;

    // Our first 8kb are reserved for the temporary offloading region,
    // therefore the beginning of our usable heap should be placed after it
    THETA_HEAP_BASE = THETA_MEMORY_REGION_BASE + 8 * 1024;
    
    // New space will start with the left half of the Theta memory region
    newSpaceBoundary = THETA_HEAP_BASE + THETA_MEMORY_REGION_INITIAL_SIZE / 2;

    allocationPointer = THETA_HEAP_BASE;
  }

  EMSCRIPTEN_KEEPALIVE
  void __Theta_Lang_populateClosure(int32_t closureMemAddress, int32_t paramAddress) {
    int32_t *arityAddr = reinterpret_cast<int32_t*>(closureMemAddress + 4);

    int32_t arity = *arityAddr;

    *arityAddr -= 1;
    
    int32_t *argAddress = arityAddr + arity; // Pointer arithmetic dictates that this will increase the address by arity * 4

    *argAddress = paramAddress;
  }

  void __Theta_Lang_gcBoundary() {
  }

  // Called every time we execute a function, at the beginning of the function
  EMSCRIPTEN_KEEPALIVE
  void __Theta_Lang_pushFrame() {
    if (!THETA_GC_INITIALIZED) __Theta_Lang_initializeGC();

    __Theta_Lang_gcBoundary();

    ShadowStack::getInstance().pushFrame(epoch);
  }

  EMSCRIPTEN_KEEPALIVE
  void __Theta_Lang_popFrame(int32_t returnedReferenceAddress) {
    std::optional<HeapReference> returnedReference;
  
    // Find the returned reference by address (if any was returned)
    if (returnedReferenceAddress != -1) {
      for (const HeapReference &reference : ShadowStack::getInstance().currentFrame().references) {
        if (reference.address == returnedReferenceAddress) {
          returnedReference = std::optional<HeapReference>(reference);
          break;
        }
      }
    }

    ShadowStack::getInstance().popFrame();

    if (returnedReference) {
      ShadowStack::getInstance().pushReference(returnedReference.value());
    }

    __Theta_Lang_gcBoundary();
  }

  EMSCRIPTEN_KEEPALIVE
  int32_t __Theta_Lang_allocateMem(int32_t byteSize, int32_t dataTypeId) {
    int provisionedAddress = allocationPointer;

    ShadowStack::getInstance().pushReference(
      provisionedAddress,
      byteSize,
      static_cast<DataTypeId>(dataTypeId)
    );
  
    allocationPointer += byteSize;

    return provisionedAddress;
  }

  EMSCRIPTEN_KEEPALIVE
  void* loadFromMemory(int address) {
    return (void*)(THETA_HEAP_BASE + address);
  }

  EMSCRIPTEN_KEEPALIVE
  int32_t loadI32(int address) {
    return *((int32_t*)loadFromMemory(address));
  }

  EMSCRIPTEN_KEEPALIVE
  int64_t loadI64(int address) {
    return *((int64_t*)loadFromMemory(address));
  }

  EMSCRIPTEN_KEEPALIVE
  void storeI32(int address, int32_t value) {
    int32_t *memoryLocation = reinterpret_cast<int32_t*>(THETA_HEAP_BASE + address);

    *memoryLocation = value;
  }

  EMSCRIPTEN_KEEPALIVE
  void storeI64(int address, int64_t value) {
    int64_t *memoryLocation = reinterpret_cast<int64_t*>(THETA_HEAP_BASE + address);

    *memoryLocation = value;
  }
}
