// The start of the heap as determined by LLVM. This is also where
// any malloc or memory used by the following code will be allocated
extern "C" int __heap_base;

// We allocate 1 MB of space after the Clang heap to avoid collisions
int THETA_MEMORY_REGION_BASE = __heap_base + 1024 * 1024;

// Our first 8kb are reserved for the temporary offloading region,
// therefore the beginning of our usable heap should be placed after it
int THETA_HEAP_BASE = THETA_MEMORY_REGION_BASE + 8 * 1024;
int allocationPointer = THETA_HEAP_BASE;

extern "C" int initializeMem() {
  int memAddr = allocationPointer;
  allocationPointer++;
  return memAddr;
}
