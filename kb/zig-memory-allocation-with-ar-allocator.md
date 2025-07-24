# Zig Memory Allocation with ar_allocator

The ar_allocator module provides type-safe memory allocation for all Zig modules in AgeRun, eliminating the need for direct heap macro usage and improving code clarity.

## Overview

All Zig modules should use ar_allocator instead of direct AR__HEAP__ macros. This provides:
- Type safety without manual casting
- Cleaner code without verbose `@ptrCast(@alignCast(...))` patterns
- Consistent ownership semantics with `own_` prefix convention
- Automatic integration with AgeRun's heap tracking system

## Usage

### Import the Module

```zig
const ar_allocator = @import("ar_allocator.zig");
```

### Available Functions

The ar_allocator module follows Zig's standard allocator naming conventions:

- `create(T, desc)` - Allocate a single instance of type T
- `alloc(T, n, desc)` - Allocate an array of n items of type T (zeroed)
- `dupe(str, desc)` - Duplicate a string (handles various pointer types)
- `free(ptr)` - Free any pointer type
- `realloc(T, ptr, n, desc)` - Reallocate memory for n items of type T

### Examples

#### Creating a Single Instance

```zig
const own_node = ar_allocator.create(ar_expression_ast_t, "Expression AST node");
if (own_node == null) {
    return null;
}

// Direct field access - no casting needed
own_node.?.node_type = c.AR_EXPRESSION_AST_TYPE__LITERAL_INT;
own_node.?.value = 42;

// Return with simple cast
return @ptrCast(own_node);
```

#### Allocating an Array

```zig
// Allocate array of string pointers
own_args = ar_allocator.alloc(?[*:0]u8, arg_count, "function arguments");
if (own_args == null) {
    return null;
}
```

#### Duplicating Strings

```zig
// ar_allocator.dupe handles various string pointer types automatically
const own_name = ar_allocator.dupe(ref_name, "module name");
if (own_name == null) {
    ar_allocator.free(own_module);
    return null;
}
```

#### Cleanup

```zig
// Free accepts any pointer type
ar_allocator.free(own_string);
ar_allocator.free(own_array);
ar_allocator.free(own_struct);
```

## Error Handling Patterns

### During Construction

When allocations fail during object construction, clean up partial allocations:

```zig
const own_ast = ar_allocator.create(ar_method_ast_t, "method AST");
if (own_ast == null) {
    return null;
}

own_ast.?.instructions = c.ar_list__create();
if (own_ast.?.instructions == null) {
    ar_allocator.free(own_ast);  // Clean up partial allocation
    return null;
}
```

### Complex Cleanup

For objects with multiple allocations, consider using the destroy function:

```zig
if (allocation_fails) {
    // For simple cases, direct cleanup
    ar_allocator.free(own_partial_data);
    ar_allocator.free(own_node);
    
    // For complex cases, use destroy if object is partially valid
    ar_method_ast__destroy(@ptrCast(own_ast));
}
```

## Migration from Direct Heap Macros

### Before (Direct Macros)

```zig
// Verbose and error-prone
const own_node: ?*anyopaque = c.AR__HEAP__MALLOC(@sizeOf(ar_expression_ast_t), "node");
if (own_node == null) return null;

const ref_node: *ar_expression_ast_t = @ptrCast(@alignCast(own_node));
ref_node.node_type = c.AR_EXPRESSION_AST_TYPE__LITERAL_INT;

// String duplication requires casting
const own_str: ?[*:0]u8 = @ptrCast(c.AR__HEAP__STRDUP(
    @as([*c]const u8, @ptrCast(ref_input)), "description"
));
```

### After (ar_allocator)

```zig
// Clean and type-safe
const own_node = ar_allocator.create(ar_expression_ast_t, "node");
if (own_node == null) return null;

own_node.?.node_type = c.AR_EXPRESSION_AST_TYPE__LITERAL_INT;

// String duplication is straightforward
const own_str = ar_allocator.dupe(ref_input, "description");
```

## Integration with Heap Tracking

The ar_allocator module automatically integrates with AgeRun's heap tracking system:

1. In debug builds (`-DDEBUG -D__ZIG__`), all allocations are tracked
2. Memory leak reports show allocation descriptions
3. The heap tracking system works consistently between C and Zig modules

## Best Practices

1. **Always use ar_allocator** for Zig module allocations
2. **NEVER use ar_heap__free** - always use ar_allocator.free() exclusively
3. **Remove ar_heap.h imports** from Zig modules using ar_allocator
4. **Use descriptive allocation names** for better debugging
5. **Follow ownership conventions** - prefix allocated resources with `own_`
6. **Clean up on error paths** to prevent memory leaks
7. **Prefer destroy functions** for complex object cleanup

## Common Pitfalls

### Mixing Allocation Methods

Never mix ar_allocator with direct heap macros or functions in the same module:

```zig
// WRONG - Inconsistent allocation methods
const own_ast = ar_allocator.create(ar_instruction_ast_t, "instruction AST");
own_ast.?.own_args = @ptrCast(c.AR__HEAP__MALLOC(size, "args"));  // Don't mix!

// WRONG - Using heap function for cleanup
const own_items = c.ar_list__items(ref_list);
defer c.ar_heap__free(own_items);  // BAD: Never use ar_heap__free

// CORRECT - Use ar_allocator consistently
const own_ast = ar_allocator.create(ar_instruction_ast_t, "instruction AST");
own_ast.?.own_args = ar_allocator.alloc(?[*:0]u8, arg_count, "args");

// CORRECT - Use ar_allocator for cleanup
const own_items = c.ar_list__items(ref_list);
defer ar_allocator.free(own_items);  // Always use ar_allocator
```

### Forgetting Ownership Transfer

Remember that ar_allocator functions transfer ownership:

```zig
// WRONG - Using ref_ prefix for owned memory
const ref_ast = ar_allocator.create(ar_method_ast_t, "method AST");  // This transfers ownership!

// CORRECT - Use own_ prefix
const own_ast = ar_allocator.create(ar_method_ast_t, "method AST");
```

## See Also

- [C to Zig Module Migration Guide](c-to-zig-module-migration.md)
- [Zig-C Memory Tracking Consistency](zig-c-memory-tracking-consistency.md)
- [Ownership Naming Conventions](ownership-naming-conventions.md)
- [Frank Communication Principle](frank-communication-principle.md)