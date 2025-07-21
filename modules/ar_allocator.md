# Allocator Module

## Overview

The allocator module (`ar_allocator`) provides type-safe memory allocation functions for Zig modules in the AgeRun codebase. It wraps the AgeRun heap tracking macros with Zig-friendly interfaces that provide compile-time type safety and automatic pointer casting.

**Implementation Note**: This module is implemented purely in Zig and is only used by other Zig modules. It is not exposed as a C API.

## Purpose

This module serves as the bridge between Zig code and AgeRun's memory tracking system, providing:

1. **Type Safety**: Compile-time type checking for allocations
2. **Automatic Casting**: Eliminates manual `@ptrCast` and `@alignCast` calls
3. **Consistent Interface**: Unified API for all memory operations
4. **Heap Tracking Integration**: Maintains compatibility with AgeRun's memory leak detection

## Key Functions

### create
```zig
pub inline fn create(comptime T: type, desc: [*c]const u8) ?*T
```
Allocates memory for a single instance of type T.
- Returns a properly typed pointer or null on failure
- Integrates with AR__HEAP__MALLOC for tracking

### alloc
```zig
pub inline fn alloc(comptime T: type, n: usize, desc: [*c]const u8) ?[*]T
```
Allocates zeroed memory for an array of n items of type T.
- Returns a pointer to the array or null on failure
- Uses AR__HEAP__CALLOC for zero-initialization

### dupe
```zig
pub inline fn dupe(str: anytype, desc: [*c]const u8) ?[*:0]u8
```
Duplicates a string with proper null termination.
- Accepts various string pointer types
- Returns a new string or null on failure
- Uses AR__HEAP__STRDUP for tracking

### free
```zig
pub inline fn free(ptr: anytype) void
```
Type-safe deallocation that accepts any pointer type.
- Handles optional pointers, arrays, and single pointers
- Automatically extracts the correct pointer for AR__HEAP__FREE

## Usage Examples

### Creating a Struct
```zig
const MyStruct = struct {
    value: i32,
    name: ?[*:0]u8,
};

const instance = ar_allocator.create(MyStruct, "my_struct") orelse return null;
defer ar_allocator.free(instance);
```

### Allocating an Array
```zig
const items = ar_allocator.alloc(*c.void, 10, "item_array") orelse return null;
defer ar_allocator.free(items);
```

### Duplicating a String
```zig
const original = "Hello, World!";
const copy = ar_allocator.dupe(original, "string_copy") orelse return null;
defer ar_allocator.free(copy);
```

## Design Principles

1. **Zero-Cost Abstraction**: All functions are inline, adding no runtime overhead
2. **Compile-Time Safety**: Type information is preserved and checked at compile time
3. **Flexible Interface**: Accepts various pointer types through generic programming
4. **Heap Tracking**: All allocations are tracked for memory leak detection

## Dependencies

- `ar_heap`: Provides the underlying memory tracking macros

## Related Patterns

- [Zig Memory Allocation with ar_allocator](../kb/zig-memory-allocation-with-ar-allocator.md)
- [Zig Integration Comprehensive](../kb/zig-integration-comprehensive.md)