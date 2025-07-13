# C to Zig Module Migration Guide

## Overview

This guide provides a systematic approach for migrating AgeRun modules from C to Zig implementation while maintaining full compatibility and following AgeRun's design principles.

## Real-World Migration Example: ar_expression_ast

The migration of `ar_expression_ast` from C to Zig revealed critical patterns and pitfalls that inform this guide. This module handles abstract syntax trees for expressions with complex memory management requirements.

## Key Principle: C API Compatibility

**The fundamental requirement**: Zig implementations must be drop-in replacements for C implementations. This means:

- **No changes to header files** - Keep the original .h file unchanged
- **No changes to calling code** - Other modules continue working without modification
- **Binary compatible** - The Zig .o file replaces the C .o file seamlessly
- **Behavior identical** - Same inputs produce same outputs, including error cases
- **All parameters nullable** - C can pass NULL to any pointer parameter
- **Export all public functions** - Use `export fn` for C linkage

## When to Migrate

Consider migrating a C module to Zig when:

- **Performance-critical components** requiring zero-cost abstractions
- **Cross-platform modules** where C portability becomes complex
- **New modules** that would benefit from compile-time safety guarantees
- **Components requiring precise memory layout control**
- **Pure computation modules** (parsers, validators) - often need no heap

## Pre-Migration Assessment

### 1. Dependency Audit

Before migration, audit the module's dependencies:

```bash
# Check all dependencies
grep -n "#include.*ar_" module.h module.c

# Remove unused includes to simplify migration
# Verify each include is actually needed
```

### 2. Identify Migration Challenges

- **Variadic functions**: Must remain in C (platform va_list incompatibility)
- **C macros returning void/anyopaque**: Cannot be used directly in Zig (e.g., AR_ASSERT_*)
- **Complex pointer manipulations**: May require rethinking in Zig
- **Platform-specific code**: Should be eliminated first

## Migration Process

### Phase 1: Preparation

1. **Run full test suite** to establish baseline behavior
2. **Document current API** and all function signatures
3. **Identify public vs private functions**
4. **Check for circular dependencies** that need stack allocation fixes
5. **Create backup** via git commit

### Phase 2: Create Zig Implementation

1. **Create the .zig file** with same module name:
   ```bash
   # For ar_expression_ast.c -> ar_expression_ast.zig  # EXAMPLE: Replace with your module
   touch modules/ar_expression_ast.zig  # EXAMPLE: Replace with your module
   ```

2. **Set up basic structure**:
   ```zig
   const std = @import("std");
   const c = @cImport({
       @cInclude("ar_heap.h");
       @cInclude("ar_data.h");
       // Keep related headers in same @cImport block
   });

   // Import other Zig modules
   const ar_assert = @import("ar_assert.zig");
   ```

3. **Define type mappings**:
   - `c_int` → `c_int`
   - `char*` → `?[*:0]u8`
   - `c_uchar` → `u8`
   - String literals → `@as([*c]const u8, "str")`

### Phase 3: Function Migration

1. **Use consistent naming**:
   - Public functions: `ar_<module>__<function>` (same as C)
   - Static functions: `_<name>` with snake_case
   - Export all public functions with C ABI
   - **NULL as parameter convention**: Zig doesn't have NULL keyword, use `null`

### C API Compatibility Requirements

**CRITICAL**: The Zig implementation must be a drop-in replacement for the C implementation. This means:

1. **Function Signatures Must Match Exactly**:
   ```zig
   // C declaration:
   // ar_expression_ast_t* ar_expression_ast__create_literal_int(int value);
   
   // Zig implementation:
   export fn ar_expression_ast__create_literal_int(value: c_int) ?*c.ar_expression_ast_t {
       // Implementation
   }
   ```

2. **All Parameters Must Be Nullable**:
   ```zig
   // WRONG - not C API compatible
   export fn ar_expression_ast__destroy(own_node: *c.ar_expression_ast_t) void {
   
   // CORRECT - matches C's ability to pass NULL
   export fn ar_expression_ast__destroy(own_node: ?*c.ar_expression_ast_t) void {
       if (own_node == null) return;
   ```

3. **Return Types Must Match**:
   - C functions returning pointers → Zig returns optional pointers (`?*T`)
   - C functions returning values → Zig returns same type
   - Default values must match C implementation

4. **Struct Layout Compatibility**:
   - Opaque types remain opaque (no struct definition in header)
   - Internal struct layout can differ as long as external behavior matches
   - Field order matters for non-opaque structs

5. **Const Correctness**:
   ```zig
   // C: const char* ar_expression_ast__get_string_value(const ar_expression_ast_t *node);
   // Zig:
   export fn ar_expression_ast__get_string_value(ref_node: ?*const c.ar_expression_ast_t) ?[*:0]const u8 {
   ```

6. **Array/Pointer Parameters**:
   ```zig
   // For C: char** path, size_t path_count
   export fn ar_expression_ast__create_memory_access(
       ref_base: ?[*:0]const u8,
       ref_path: ?[*]const ?[*:0]const u8,  // Array of nullable strings
       path_count: usize
   ) ?*c.ar_expression_ast_t {
   ```

2. **Handle debug detection**:
   ```zig
   // Replace #ifdef DEBUG with:
   if (builtin.mode == .Debug or builtin.mode == .ReleaseSafe) {
       // Debug code
   }
   ```

3. **Parameter Validation Pattern**:
   ```zig
   export fn ar_expression_ast__create_memory_access(
       ref_base: ?[*:0]const u8,
       ref_path: ?[*]const ?[*:0]const u8,
       path_count: usize
   ) ?*c.ar_expression_ast_t {
       if (ref_base == null) {
           return null;
       }
       // Continue with implementation
   }
   ```

3. **Memory management alignment**:
   - Use AgeRun's heap tracking via C imports
   - Follow ownership naming conventions (own_, mut_, ref_)
   - Maintain same ownership semantics as C version
   - **Use C types directly**: Declare variables with C types (e.g., `?*c.ar_expression_ast_t`) instead of custom Zig types to eliminate unnecessary `@ptrCast` operations
   - **CRITICAL**: Match C implementation exactly for ownership transfer functions
   - **Naming convention enforcement**: Apply own_/mut_/ref_ prefixes to ALL variables (including locals)

4. **Error handling**:
   - Use Zig's explicit error unions
   - Complement AgeRun's error propagation pattern
   - Set errors at source, propagate via get_error()

### Phase 4: Platform Compatibility

1. **Handle platform differences**:
   ```zig
   // macOS: stderr/stdout as functions
   const stderr = c.stderr();
   
   // Create errno helper functions
   fn getErrno() c_int {
       return c.__error().*; // macOS
   }
   ```

2. **Circular dependency resolution**:
   - Use stack allocation to break heap→io→heap cycles
   - Avoid dynamic allocation where possible

### Phase 5: Testing Integration

1. **Maintain existing test files** (.c tests can call Zig functions via C headers)
2. **Add Zig's built-in testing** alongside AgeRun tests
3. **Verify behavior matches** original C implementation exactly
4. **Check memory reports** for zero leaks

### Maintaining C Header Compatibility

**CRITICAL**: The existing C header file must remain unchanged to ensure compatibility:

1. **Keep Original Header**:
   ```c
   // ar_expression_ast.h remains exactly as it was
   typedef struct ar_expression_ast_s ar_expression_ast_t;  // Opaque type
   
   // All function declarations remain unchanged
   ar_expression_ast_t* ar_expression_ast__create_literal_int(int value);
   void ar_expression_ast__destroy(ar_expression_ast_t *own_node);
   // etc...
   ```

2. **Export Matching Functions from Zig**:
   ```zig
   // Zig must export functions that match these declarations exactly
   export fn ar_expression_ast__create_literal_int(value: c_int) ?*c.ar_expression_ast_t {
       // Zig implementation
   }
   ```

3. **Test C Compatibility**:
   - Existing C test files should compile and run without modification
   - No changes to #include statements in other modules
   - No changes to how other modules call these functions

4. **Linking Behavior**:
   - The Zig .o file replaces the C .o file in linking
   - Other modules continue to include the .h file
   - No source code changes needed in dependent modules

### Phase 6: Build Integration

1. **Update Makefile** (automatic detection):
   - Makefile will automatically detect .zig files
   - Build flags: `-lc -fno-stack-check` for C interop
   - Pattern rule: `$(ZIG) build-obj -O ReleaseSafe -target native`

2. **Verify build**:
   ```bash
   make clean build
   strings bin/*.o | grep "zig X.X.X"  # Confirm Zig compilation
   ```

### Phase 7: Cleanup and Documentation

1. **Delete the C file**:
   ```bash
   rm modules/ar_expression_ast.c  # EXAMPLE: Replace with your module
   git add modules/ar_expression_ast.c  # EXAMPLE: Stage deletion for your module
   ```

2. **Update all documentation references**:
   ```bash
   # Find all references to update
   grep -r "ar_expression_ast\.c" .  # EXAMPLE: Replace with your module name
   grep -r "\.c implementation" modules/*.md
   
   # Update module documentation
   # Update README.md for hybrid modules
   ```

3. **No Makefile changes needed** - build system auto-detects

## Best Practices

### Import Organization

```zig
// Clean imports (inline functions only)
const ar_assert = @import("ar_assert.zig");
const ar_assert__func = ar_assert.ar_assert__func;

// C header imports - group related headers
const c = @cImport({
    @cInclude("ar_heap.h");
    @cInclude("ar_data.h");  // Related to heap
});
```

### Struct Definition Patterns

```zig
// For opaque types with unions/complex fields
const ar_expression_ast_s = struct {
    node_type: c.ar_expression_ast_type_t,  // Avoid reserved words
    data: node_data_t,  // EXAMPLE: Union for different node types
};

// Union definitions for variant data
// EXAMPLE: Internal type definitions specific to this module
const node_data_t = union {  // EXAMPLE: Internal type for ar_expression_ast module
    literal_int: literal_int_data_t,      // EXAMPLE: Module-specific type
    literal_double: literal_double_data_t, // EXAMPLE: Module-specific type
    literal_string: literal_string_data_t, // EXAMPLE: Module-specific type
    memory_access: memory_access_data_t,   // EXAMPLE: Module-specific type
    binary_op: binary_op_data_t,          // EXAMPLE: Module-specific type
};

// Nested struct definitions
// EXAMPLE: Module-specific type definition
const memory_access_data_t = struct {  // EXAMPLE: Internal type for ar_expression_ast module
    own_base: ?[*:0]u8,        // Owned string
    own_path: ?*c.ar_list_t,   // Owned list of strings
};
```

### Memory Allocation Patterns

```zig
// Allocate with proper type and description
const own_node: ?*anyopaque = c.AR__HEAP__MALLOC(@sizeOf(ar_expression_ast_s), "Expression AST node (int)");
if (own_node == null) {
    return null;
}

// Cast and initialize
const ref_ast_node: *ar_expression_ast_s = @ptrCast(@alignCast(own_node));
ref_ast_node.node_type = c.AR_EXPRESSION_AST_TYPE__LITERAL_INT;

// Return as C type
return @ptrCast(own_node);
```

### String Duplication Pattern

```zig
// Always check for null and provide description
const own_string_copy: ?[*:0]u8 = @ptrCast(c.AR__HEAP__STRDUP(
    @as([*c]const u8, @ptrCast(ref_value)), 
    "String literal value"
));
if (own_string_copy == null) {
    // Clean up any partial allocations
    c.AR__HEAP__FREE(own_node);
    return null;
}
```

### Exit-Time Safety

```zig
// Check initialization state early
if (!g_initialized) {
    return;  // Never call init during cleanup
}
```

### Access Pattern for Exported Modules

```zig
// Zig modules with exports: Access via C headers
// This avoids duplicate symbols at link time
// Use the C header interface, not direct Zig imports
```

### Assert Module Handling

- **Zig modules**: Use `ar_assert.zig`
- **C modules**: Continue using `ar_assert.h` macros
- **Never mix**: Don't use C assert macros in Zig code

## Verification Checklist

- [ ] All tests pass with identical behavior
- [ ] Zero memory leaks in memory reports
- [ ] Build succeeds with Zig flags
- [ ] Documentation updated (module.md, README.md if hybrid)
- [ ] C file deleted and deletion staged in git
- [ ] Platform compatibility verified (especially Ubuntu CI)
- [ ] API compatibility maintained
- [ ] Ownership semantics preserved
- [ ] Compare implementation with C version line-by-line for critical functions
- [ ] Verify ar_list__items() usage matches C exactly
- [ ] Check all error paths have proper cleanup
- [ ] Ensure all exported functions have nullable parameters

## Common Issues and Solutions

### Variadic Function Incompatibility

**Problem**: Platform va_list incompatibility
**Solution**: Use hybrid approach - keep variadic functions in C

```c
// Keep in C file (e.g., ar_io_variadic.c)
// EXAMPLE: Hypothetical variadic function
// int ar_io__printf(const char *format, ...) {  // EXAMPLE: Hypothetical variadic function
//     // Variadic implementation
// }
```

### Header Path Issues

**Problem**: Ubuntu stricter about header paths
**Solution**: Test on Linux CI first, fix includes

### Runtime Platform Differences

**Problem**: Different runtime behavior on macOS vs Linux
**Solution**: Create platform-specific helper functions

### Memory Leak Patterns

**Problem**: Different memory management patterns between C and Zig can introduce leaks
**Solution**: Critical implementation patterns discovered during ar_expression_ast migration

#### 1. List Items Array Management

**C Implementation Pattern**:
```c
// ar_expression_ast__get_memory_path in C
char** ar_expression_ast__get_memory_path(const ar_expression_ast_t *ref_node, size_t *out_count) {
    // ...
    // Transfer ownership of the array to caller - they must free it
    return (char**)ar_list__items(ref_node->data.memory_access.own_path);
}
```

**WRONG Zig Implementation** (creates memory leak):
```zig
// This allocates a NEW array AND calls ar_list__items
const own_array = @ptrCast(@alignCast(c.AR__HEAP__MALLOC(count * @sizeOf([*:0]u8), "Memory path array")));
const own_items = c.ar_list__items(ref_path_list);
// Copy items and free ar_list__items array - but now we have TWO arrays!
```

**CORRECT Zig Implementation**:
```zig
// Match C exactly - just return the array from ar_list__items
return @ptrCast(c.ar_list__items(ref_path_list));
```

#### 2. Destruction Pattern for Lists

**C Implementation Pattern**:
```c
// Uses ar_list__items to get array, then frees items and array
void **own_items = ar_list__items(own_node->data.memory_access.own_path);
if (own_items) {
    for (size_t i = 0; i < count; i++) {
        if (own_items[i]) {
            AR__HEAP__FREE(own_items[i]);
        }
    }
    AR__HEAP__FREE(own_items);  // Free the array itself
}
ar_list__destroy(own_node->data.memory_access.own_path);
```

**Better Zig Implementation** (avoids extra allocation):
```zig
// Use ar_list__remove_first to avoid calling ar_list__items
var item: ?*anyopaque = c.ar_list__remove_first(ref_ast_node.data.memory_access.own_path);
while (item != null) : (item = c.ar_list__remove_first(ref_ast_node.data.memory_access.own_path)) {
    c.AR__HEAP__FREE(item);
}
c.ar_list__destroy(ref_ast_node.data.memory_access.own_path);
```

#### 3. Error Handling During Construction

**Problem**: Calling destroy on partially constructed objects
**Solution**: Manual cleanup when object isn't fully initialized

```zig
// WRONG: Calling destroy on uninitialized node
if (allocation_fails) {
    ar_expression_ast__destroy(@ptrCast(own_node));  // Node type not set yet!
}

// CORRECT: Manual cleanup
if (allocation_fails) {
    // Free any path components we've already added
    var item: ?*anyopaque = c.ar_list__remove_first(own_path_list);
    while (item != null) : (item = c.ar_list__remove_first(own_path_list)) {
        c.AR__HEAP__FREE(item);
    }
    c.ar_list__destroy(own_path_list);
    c.AR__HEAP__FREE(own_base_copy);
    c.AR__HEAP__FREE(own_node);
    return null;
}
```

### Zig-Specific Syntax Issues

#### Reserved Keywords

**Problem**: Zig has reserved words that C doesn't
**Solution**: Rename fields/variables appropriately

```zig
// C struct has field named 'type' (reserved in Zig)
const ar_expression_ast_s = struct {
    node_type: c.ar_expression_ast_type_t,  // Renamed from 'type'
    data: node_data_t,  // EXAMPLE: Internal union type
};
```

#### Nullable Parameters

**Problem**: C API compatibility requires all parameters to be nullable
**Solution**: Make all exported function parameters optional

```zig
// WRONG
export fn ar_method_evaluator__create(ref_log: *c.ar_log_t) ?*c.ar_method_evaluator_t {

// CORRECT
export fn ar_method_evaluator__create(ref_log: ?*c.ar_log_t) ?*c.ar_method_evaluator_t {
    if (ref_log == null) return null;
    // ...
}
```

#### Type Casting

**Problem**: Excessive @ptrCast operations reduce readability
**Solution**: Use C types directly when possible

```zig
// WRONG: Custom type requiring casts everywhere
const own_node: *ar_expression_ast_s = // ...
return @ptrCast(own_node);  // Cast needed

// CORRECT: Use C type directly
const own_node: ?*c.ar_expression_ast_t = @ptrCast(@alignCast(c.AR__HEAP__MALLOC(...)));
return own_node;  // No cast needed
```

### Common C API Compatibility Mistakes

1. **Forgetting Nullable Parameters**:
   ```zig
   // MISTAKE - discovered during testing
   export fn ar_expression_ast__create_binary_op(
       op: c.ar_binary_operator_t,
       own_left: *c.ar_expression_ast_t,    // WRONG: Not nullable
       own_right: *c.ar_expression_ast_t    // WRONG: Not nullable
   ) ?*c.ar_expression_ast_t {
   
   // CORRECT - C callers can pass NULL
   export fn ar_expression_ast__create_binary_op(
       op: c.ar_binary_operator_t,
       own_left: ?*c.ar_expression_ast_t,   // Nullable
       own_right: ?*c.ar_expression_ast_t   // Nullable
   ) ?*c.ar_expression_ast_t {
       if (own_left == null or own_right == null) {
           // Clean up if one operand is null
           if (own_left != null) ar_expression_ast__destroy(own_left);
           if (own_right != null) ar_expression_ast__destroy(own_right);
           return null;
       }
   ```

2. **Wrong Default Values**:
   ```zig
   // C implementation returns specific defaults for invalid inputs
   export fn ar_expression_ast__get_type(ref_node: ?*const c.ar_expression_ast_t) c.ar_expression_ast_type_t {
       if (ref_node == null) {
           // Must match C's default exactly
           return c.AR_EXPRESSION_AST_TYPE__LITERAL_INT;
       }
   ```

3. **Ownership Semantics Mismatch**:
   ```zig
   // Must understand if C function transfers ownership or not
   // Example: ar_list__items() transfers ownership of array to caller
   ```

4. **Export Keyword Missing**:
   ```zig
   // WRONG - not visible to C
   fn ar_expression_ast__create() ?*c.ar_expression_ast_t {
   
   // CORRECT - exported for C linkage
   export fn ar_expression_ast__create() ?*c.ar_expression_ast_t {
   ```

## Example Migrations

### ar_string Module
- Before: `ar_string.c` + `ar_string.h` (now deleted)  # EXAMPLE: C file no longer exists
- After: `ar_string.zig` + `ar_string.h` (header maintained for C compatibility)
- Tests: Existing `ar_string_tests.c` continues to work
- Build: Automatic detection and compilation

### ar_expression_ast Module (Complex Example)
- Before: `ar_expression_ast.c` + `ar_expression_ast.h` (now deleted)  # EXAMPLE: C file no longer exists
- After: `ar_expression_ast.zig` + `ar_expression_ast.h`
- Key Challenges:
  - Complex memory ownership with lists of strings
  - Reserved keyword 'type' → renamed to 'node_type'
  - Critical to match C's `ar_list__items()` behavior exactly
  - Error handling during partial construction
- Lessons:
  - Started with 4 memory leaks from misunderstanding `ar_list__items()` ownership
  - Fixed by exactly matching C implementation for get_memory_path
  - Used `ar_list__remove_first()` in destroy to avoid extra allocation
  - Manual cleanup required for error paths during construction

## Integration with TDD

When migrating during active development:

1. **Complete current TDD cycles** in C first
2. **Commit working state** before migration
3. **Migrate with behavior preservation** (implementation tests should still pass)
4. **Use migration as refactor phase** if part of larger TDD cycle
5. **Verify no behavior changes** before continuing development

This ensures migration doesn't interfere with feature development and maintains code quality throughout the process.

## Critical Success Factors

Based on the ar_expression_ast migration experience:

1. **Exact Function Behavior Matching**: Functions that transfer ownership (like get_memory_path) must match C implementation exactly
2. **Memory Leak Detection**: Use individual test memory reports (`bin/memory_report_<test_name>.log`)
3. **Iterative Debugging**: Start with simple implementation, then fix leaks one by one
4. **User Feedback Integration**: User review catches critical issues like missing nullable parameters
5. **Compare Implementations**: Always compare with C implementation when debugging, especially for memory management
6. **Test-Driven Verification**: Run tests after each change to catch regressions immediately

## Debugging Memory Leaks in Zig Modules

When you encounter memory leaks after migration:

1. **Check memory report for specific test**:
   ```bash
   cat bin/run-tests/memory_report_ar_expression_ast_tests.log
   ```

2. **Identify leak source** (e.g., "List items array" from ar_list.c:231)

3. **Trace usage of the leaking function**:
   ```bash
   grep -n "ar_list__items" modules/ar_expression_ast.zig
   ```

4. **Compare with C implementation**:
   ```bash
   # EXAMPLE: Compare with C implementation from git history
   # git show HEAD~5:modules/ar_expression_ast.c | grep -A20 "function_name"  # EXAMPLE: Historical reference
   ```

5. **Common fixes**:
   - Match C's ownership transfer exactly
   - Use ar_list__remove_first() instead of ar_list__items() in destructors
   - Ensure proper cleanup in error paths