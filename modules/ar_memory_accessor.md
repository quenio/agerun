# ar_memory_accessor Module

## Overview

The `ar_memory_accessor` module provides utilities for accessing memory paths in the AgeRun system. It centralizes the logic for checking if a path is a memory path and extracting the key portion after the "memory." prefix.

This module eliminates code duplication across the 9 instruction evaluators that need to work with memory paths.

## Implementation

- **Language**: Zig with C-compatible API
- **Header**: `ar_memory_accessor.h`
- **Implementation**: `ar_memory_accessor.zig`
- **Dependencies**: `ar_path`

## API Reference

### Functions

#### ar_memory_accessor__get_key

```c
const char* ar_memory_accessor__get_key(const char *ref_path);
```

Gets the memory key from a path if it's a memory path.

**Parameters:**
- `ref_path`: The path to check

**Returns:**
- BORROW: The key portion after "memory.", or NULL if not a memory path
- The returned pointer points into the original string and should not be freed

**Behavior:**
- Returns NULL if the input is NULL
- Returns NULL if the path doesn't start with exactly "memory" as the first segment
- Returns NULL if the path has fewer than 2 segments
- For "memory.user.name", returns "user.name"
- For "memory.x", returns "x"
- For "memory", returns NULL (no key)
- For "memoryfoo.x", returns NULL (first segment isn't exactly "memory")

## Design Decisions

1. **Strict Validation**: The module explicitly checks that the first segment equals "memory" rather than just checking if the string starts with "memory". This prevents false positives for paths like "memoryfoo.x".

2. **Segment Count Check**: Requires at least 2 segments to ensure there's a valid key after "memory.".

3. **Zero-Cost Abstraction**: Implemented in Zig for optimal performance while maintaining C compatibility.

4. **No Memory Allocation**: Returns a pointer into the original string, avoiding memory allocation overhead.

## Usage Example

```c
#include "ar_memory_accessor.h"

// In an evaluator
const char *path = "memory.user.name";
const char *key = ar_memory_accessor__get_key(path);
if (key) {
    // key now points to "user.name"
    // Use the key to access memory map
}
```

## Testing

The module includes comprehensive tests in `ar_memory_accessor_tests.c` that verify:
- NULL input handling
- Non-memory path rejection
- Simple and nested memory paths
- Edge cases (empty string, "memory" only, trailing dot)
- Paths that start with "memory" but aren't memory paths

## Module Dependencies

- Depends on: `ar_path`
- Depended on by: All 9 instruction evaluators
  - `ar_assignment_instruction_evaluator`
  - `ar_send_instruction_evaluator`
  - `ar_condition_instruction_evaluator`
  - `ar_parse_instruction_evaluator`
  - `ar_build_instruction_evaluator`
  - `ar_method_instruction_evaluator`
  - `ar_agent_instruction_evaluator`
  - `ar_destroy_agent_instruction_evaluator`
  - `ar_destroy_method_instruction_evaluator`