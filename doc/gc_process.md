# Copying Garbage Collector for Theta (WebAssembly Target)

## Overview

This document outlines the design and implementation of a **copying garbage collector** for the Theta programming language, which compiles to WebAssembly. The primary challenge addressed in this design is that WebAssembly's internal stack is opaque to the developer, which complicates the process of tracking and updating heap references stored on the stack during garbage collection. To overcome this, a **shadow stack** is maintained to track references on the WebAssembly stack.

## Key Concepts

### WebAssembly Stack
WebAssembly does not expose its internal stack to the developer, making it impossible to directly update stack-allocated pointers during garbage collection. However, by maintaining a parallel data structure (the shadow stack), the state of the WebAssembly stack can be tracked externally.

### Shadow Stack
The **shadow stack** is an in-memory representation that mirrors the WebAssembly stack, containing all pointers to heap-allocated objects. The shadow stack is synchronized with the WebAssembly stack during normal execution, allowing the garbage collector to operate without needing direct access to the WebAssembly stack.

## GC Process: Stack-Swapping Approach

The copying garbage collector operates by swapping out the WebAssembly stack for the shadow stack during garbage collection, and then "rehydrating" the WebAssembly stack with updated heap references after the collection is complete.

### Stages of the GC Process

### 1. Pre-Garbage Collection: Stack Synchronization
- During normal execution, the shadow stack is kept in **parity** with the WebAssembly stack.
- All function calls and local variables that reference heap-allocated objects are stored both on the WebAssembly stack and the shadow stack.

### 2. GC Initiation: Stack Clearing
- When garbage collection begins, the WebAssembly stack is **cleared**. This is safe because the shadow stack already holds all relevant references.
- At this point, the shadow stack serves as the **primary** reference for heap-allocated objects.

### 3. During Garbage Collection: Object Relocation
- The garbage collector moves live objects from one region of memory (the "from-space") to another (the "to-space").
- As objects are moved, their new addresses are updated in the **shadow stack**.
- The WebAssembly stack remains empty during this phase, relying entirely on the shadow stack to track heap references.

### 4. Post-Garbage Collection: Stack Rehydration
- Once the garbage collection process is complete and all live objects have been relocated, the WebAssembly stack is **rehydrated** using the updated shadow stack.
- Each entry in the shadow stack is pushed back onto the WebAssembly stack, with any updated pointers reflecting new object locations in memory.
- After rehydration, the WebAssembly stack resumes normal execution with updated references, and the shadow stack continues to track references in parallel
