This document presents the design and implementation of a **copying garbage collector** for the Theta programming language. The garbage collector uses several key mechanisms to manage memory efficiently within WebAssembly’s constraints:
- A **shadow stack** is used to track heap references across function calls, providing visibility where the WebAssembly stack is opaque.
- A pre-allocated **8 KB offloading region** temporarily stores non-heap values during garbage collection (GC).
- A **GC Epoch** system is employed to prevent redundant updates to function stack frames by ensuring each frame is updated only once per GC run.

This approach balances performance, correctness, and WebAssembly's unique environment constraints. GC is triggered when the heap memory is half-full and runs at the next available function boundary to avoid interrupting mid-function execution.

## Key Concepts

### Shadow Stack

The **shadow stack** is a critical data structure used to track **heap references** across function calls. In WebAssembly, the stack is not directly accessible, making it difficult to inspect stack frames for heap references during GC. The shadow stack solves this problem by maintaining a parallel stack where **only heap references** are stored. Non-heap values, such as local variables or primitive data types, are not tracked in the shadow stack.

**Main Responsibilities of the Shadow Stack**:
- **Track heap references**: Each time a function allocates or uses a heap object, the reference (pointer) to that object is pushed onto the shadow stack.
- **Ensure correct updates**: When objects are moved during GC, the shadow stack ensures heap references are updated with their new addresses.
- **Handle function calls/returns**: Heap references from a function’s stack frame are pushed onto the shadow stack when the function is entered, and popped when the function returns.

**Why We Don’t Track Non-Heap Values**:
- **Efficiency**: The WebAssembly stack handles non-heap values (such as integers, floats, and intermediate calculation results) efficiently. By excluding these values from the shadow stack, we reduce the memory and performance overhead of tracking stack frames.

### Temporary Offloading Region

The **temporary offloading region** is a pre-allocated **8 KB** block of memory located at the **beginning of memory**. Its purpose is to handle **non-heap values** during garbage collection.

**Key Use Cases**:
1. **Offloading non-heap values**: When garbage collection is triggered, non-heap values in the WebAssembly stack (e.g., local variables, function arguments) are temporarily moved to the offloading region. This allows the WebAssembly stack to be cleared for garbage collection.
2. **Rehydrating the WebAssembly stack**: Once GC completes, non-heap values stored in the offloading region are restored back onto the WebAssembly stack.

**Why We Use a Fixed 8 KB Region**:
- **Worst-case scenario**: We pre-allocate a region large enough to handle the maximum expected stack frame size for any function. This avoids dynamic resizing during GC, simplifying memory management.
- **Predictable performance**: The fixed region ensures that we avoid costly memory allocation operations during GC, which could otherwise introduce latency.

### GC Epoch System

The **GC Epoch** system ensures that stack frames are updated **exactly once per garbage collection cycle**, preventing redundant updates as functions return through the call stack. 

**Key Responsibilities of the GC Epoch System**:
- **Track the state of each stack frame relative to GC runs**: Every time GC is triggered, the system increments a global **GC Epoch counter**.
- **Per-frame tracking**: Each function stack frame is assigned a **GC Epoch value**. This value indicates when the stack frame was last updated relative to the global GC Epoch.
- **Efficient updates**: When a function returns, the stack frame is checked to see if its **GC Epoch** is **older** than the current global GC Epoch. If it is older, it means the frame hasn’t been updated since the last GC run, and its heap references need to be updated. If the frame’s epoch matches the current epoch, no updates are needed.

**How the GC Epoch System Works**:
1. **GC is triggered**: The global GC Epoch counter is incremented (e.g., from 1 to 2).
2. **Heap relocation**: Objects are moved, and the shadow stack is updated with the new addresses.
3. **Checking frames**: As functions return, each function’s stack frame is checked against the current GC Epoch. If its epoch is outdated, the frame is updated.

This system minimizes redundant updates when heap references are relocated multiple times during multiple GC cycles.

### Heap Object Relocation

The **copying garbage collector** moves live heap objects from one memory region (the **from-space**) to another (the **to-space**). This process requires that all heap references (i.e., pointers) are updated to reflect the new addresses of the moved objects. The shadow stack plays a critical role in ensuring that all references are correctly updated.

**Steps in the Relocation Process**:
1. **Identify live objects**: During GC, all live objects in the heap are identified by traversing the shadow stack.
2. **Move objects**: Live objects are moved from from-space to to-space.
3. **Update references**: The shadow stack is updated to point to the new addresses of the relocated objects.
4. **Stack frame updates**: When functions return, their stack frames are checked for outdated references, and if necessary, they are updated with the new heap addresses.

### Triggering Garbage Collection

Garbage collection is triggered when **memory usage reaches 50%** of the allocated heap. However, to avoid disrupting function execution, GC does not run immediately. Instead, it is deferred to the **next function boundary** (either function entry or exit).

**Why Trigger at Function Boundaries?**
- **Minimized disruption**: By delaying garbage collection until a function boundary, we ensure that GC doesn’t interrupt ongoing operations or calculations within a function. This approach makes the system less prone to performance spikes and ensures smoother execution.

**Triggering Conditions**:
- **Memory usage reaches 50%**: When the allocated heap fills to half its size, a GC flag is set, indicating that the next function boundary should trigger garbage collection.

### Updating and Rehydrating the WebAssembly Stack

Once garbage collection completes, the WebAssembly stack is **rehydrated** with the updated heap references from the shadow stack and the non-heap values from the offloading region.

**Rehydration Process**:
1. **Heap references**: Updated heap references are restored to the WebAssembly stack from the shadow stack.
2. **Non-heap values**: Non-heap values stored in the offloading region are restored to their original locations on the WebAssembly stack.
3. **Resume execution**: After rehydration, execution resumes with the correct stack frame state.

## Detailed Example: Function Call Flow with GC

### Example Flow with Function A, B, and C

#### 1. Initial Function Calls:
- **Function A** allocates memory and stores a pointer to a heap object in its stack frame.
- **Function B** is called by A, and it also allocates memory. Heap references are tracked in both **A** and **B**’s stack frames, which are pushed onto the shadow stack.

#### 2. Function C is Called:
- **Function B** calls **Function C**, which also allocates memory.
- The heap reference from **Function C** is pushed to the shadow stack.

#### 3. Garbage Collection is Triggered:
- **Memory usage reaches 50%**: A flag is set to trigger GC, but it waits until the next function boundary.
- **Function C** completes, and GC begins at the function exit boundary.
- The global **GC Epoch** is incremented from **1 to 2**.

#### 4. Heap Relocation:
- Live heap objects are moved from **from-space** to **to-space**.
- The shadow stack is updated with the new heap addresses.

#### 5. Function C Returns to B:
- **Function B**’s GC Epoch is **1** (outdated). Since the global GC Epoch is now **2**, **Function B**’s stack frame is updated with the new heap references, and its epoch is updated to **2**.

#### 6. Function B Returns to A:
- **Function A**’s GC Epoch is also **1**. Its frame is updated, and the epoch is set to **2**.

#### 7. Rehydrating the WebAssembly Stack:
- Heap references from the shadow stack are restored to the WebAssembly stack.
- Non-heap values from the offloading region are restored to the WebAssembly stack.
- Execution continues from **Function A** with the correct stack frame state.
