
# Copying Garbage Collector for Theta (WebAssembly Target)

## Overview

This document outlines an optimized design and implementation of a **copying garbage collector** for the Theta programming language, which compiles to WebAssembly. Due to the opaque nature of WebAssembly's internal stack, we use a **shadow stack** to track heap references across function frames. This ensures that heap references are updated correctly during garbage collection (GC), even when multiple stack frames are involved. However, to reduce the overhead associated with this approach, we implement several optimizations.

## Key Concepts

### WebAssembly Stack
The WebAssembly stack is managed by the WebAssembly runtime, and developers have no direct control over it. Each function call creates a new stack frame, and function frames are isolated, meaning functions can only access their own stack frame. This poses challenges for managing heap references during GC.

### Shadow Stack
The **shadow stack** is an in-memory data structure that mirrors the WebAssembly stack in terms of heap references. Each function that allocates or uses a heap reference pushes that reference onto the shadow stack, ensuring that the garbage collector can access all heap references across frames, even when WebAssembly stack frames are not directly accessible.

## Optimized GC Process

### 1. Pre-Garbage Collection: Shadow Stack Tracking
- During the execution of the program, heap references from each function are pushed onto both the **WebAssembly stack** (as local variables) and the **shadow stack** (for global tracking).
- The shadow stack only tracks **heap references**, minimizing the number of push/pop operations.
- **Escape analysis** is applied to determine if an object escapes its current function. Non-escaping objects are handled within their local frame and do not need to be pushed to the shadow stack.

### 2. GC Triggered During Execution
- **Garbage collection** is triggered selectively at safe points, such as function boundaries (entry or exit) or based on memory usage thresholds.
- When GC is triggered, the shadow stack is used to identify and update heap references across all stack frames, regardless of whether the references are from the current or previous function calls.

### 3. Heap Object Relocation
- During GC, heap objects are moved from one memory region (the "from-space") to another (the "to-space"), and heap references in the shadow stack are updated with new memory addresses.
- Since the shadow stack contains all live heap references from every function frame, it ensures that all references, even those from previous frames, are updated correctly.

### 4. Post-GC: Rehydrating the WebAssembly Stack
- After garbage collection, if a function's WebAssembly stack frame contains outdated heap references, those references are updated based on the shadow stack.
- **Lazy updates** are applied, meaning that WebAssembly stack frames are only updated when necessary (i.e., when heap references have changed).
- The updated heap references from the shadow stack are pushed back onto the WebAssembly stack when the function returns or resumes execution.

### Example Flow

#### Function A allocates memory:
1. Function **A** allocates an object on the heap and stores the pointer in its WebAssembly stack frame:
   ```wasm
   (local $heap_ref i32)   ;; Local variable in A's frame, a pointer to a heap object
   ```
2. The pointer is also pushed onto the **shadow stack**:
   ```wasm
   (global.set $shadow_stack[i32])  ;; Shadow stack holds the reference
   ```

#### Function A calls function B:
1. Function **A** calls function **B**, which creates a new stack frame:
   - Function **B** pushes its own heap references onto both the WebAssembly and shadow stacks.

#### GC is triggered in function B:
1. Garbage collection is triggered during **function B**'s execution.
2. The garbage collector uses the **shadow stack** to update heap references for both **function A** and **function B**.
3. Heap objects are moved, and the **shadow stack** is updated with new memory addresses for all live objects.

#### Function B returns:
1. When **function B** returns, **function A** resumes execution.
2. If **function A**'s WebAssembly stack frame contains outdated heap references (due to GC), those references are updated using the **shadow stack**.

### Optimizations

#### 1. Escape Analysis
- **Escape analysis** minimizes shadow stack operations by identifying objects that do not escape their current function. These objects are handled within the local stack frame without being pushed to the shadow stack.

#### 2. Lazy Stack Updates
- WebAssembly stack frames are updated only when necessary (i.e., when heap objects have been moved during GC). This reduces the frequency of updates and improves performance.

#### 3. Selective GC Triggering
- GC is triggered at function boundaries or based on memory usage thresholds, reducing unnecessary garbage collection cycles.

## Trade-offs

### Pros
- **Correctness**: The shadow stack ensures that all heap references across all function frames are updated correctly during garbage collection.
- **Optimized performance**: By applying escape analysis, lazy updates, and selective GC triggering, the overhead associated with maintaining the shadow stack and updating the WebAssembly stack is minimized.

### Cons
- **Overhead**: There is still some overhead associated with pushing/popping heap references onto the shadow stack and updating the WebAssembly stack.
- **Complexity**: The implementation requires careful management of the shadow stack and synchronization with the WebAssembly stack, which adds complexity to the system.
