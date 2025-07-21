# Zig Type Usage Patterns

## Overview

When migrating C modules to Zig, proper type usage significantly improves code clarity and reduces unnecessary casting. This guide documents patterns discovered during evaluator migrations.

## Key Principle: Use Concrete Types Where Possible

Zig allows us to use concrete types in function signatures instead of opaque C types, eliminating many casts and improving type safety.

## Pattern 1: Return Concrete Types from Create Functions

### Before (Using Opaque C Type)
```zig
pub export fn ar_assignment_instruction_evaluator__create(
    ref_log: ?*c.ar_log_t,
    ref_expr_evaluator: ?*c.ar_expression_evaluator_t
) ?*c.ar_assignment_instruction_evaluator_t {  // Opaque C type
    // ...
    return @ptrCast(own_evaluator);  // Cast required
}
```

### After (Using Concrete Zig Type)
```zig
pub export fn ar_assignment_instruction_evaluator__create(
    ref_log: ?*c.ar_log_t,
    ref_expr_evaluator: ?*c.ar_expression_evaluator_t
) ?*ar_assignment_instruction_evaluator_t {  // Concrete Zig type
    // ...
    return own_evaluator;  // No cast needed!
}
```

## Pattern 2: Accept Concrete Types in Function Parameters

### Before (Using Opaque C Type)
```zig
pub export fn ar_assignment_instruction_evaluator__evaluate(
    mut_evaluator: ?*c.ar_assignment_instruction_evaluator_t,  // Opaque C type
    ref_frame: ?*const c.ar_frame_t,
    ref_ast: ?*const c.ar_instruction_ast_t
) bool {
    // Required cast to use the evaluator
    const evaluator: *ar_assignment_instruction_evaluator_t = @ptrCast(@alignCast(mut_evaluator.?));
    // Use evaluator.ref_log, etc.
}
```

### After (Using Concrete Zig Type)
```zig
pub export fn ar_assignment_instruction_evaluator__evaluate(
    ref_evaluator: ?*const ar_assignment_instruction_evaluator_t,  // Concrete type
    ref_frame: ?*const c.ar_frame_t,
    ref_ast: ?*const c.ar_instruction_ast_t
) bool {
    // Direct usage - no temporary variable needed!
    c.ar_log__error(ref_evaluator.?.ref_log, "Error message");
}
```

## Pattern 3: Type Selection Guidelines

### Use Concrete Zig Types When:
- It's your own module's type (e.g., `ar_assignment_instruction_evaluator_t`)
- You need to access struct fields
- You want to avoid casting

### Use C Types When:
- The module is still ABI-compatible with C (even if implemented in Zig)
- You're calling C functions that expect C types
- The type is from an external C library

## Pattern 4: Eliminate Unnecessary Variables

### Before (With Temporary Variable)
```zig
const evaluator = ref_evaluator.?;
if (!c.ar_path__is_memory_path(own_path)) {
    c.ar_log__error(evaluator.ref_log, "Error");
    return false;
}
```

### After (Direct Usage)
```zig
if (!c.ar_path__is_memory_path(own_path)) {
    c.ar_log__error(ref_evaluator.?.ref_log, "Error");
    return false;
}
```

### Additional Examples

**Eliminate temporary result variables:**
```zig
// Before - unnecessary temporary variables
const method_result = c.ar_expression_evaluator__evaluate(ref_expr_evaluator, ref_frame, ref_method_ast);
const own_method_name = c.ar_data__claim_or_copy(method_result, @constCast(@ptrCast(ref_evaluator)));

// After - direct evaluation
const own_method_name = c.ar_data__claim_or_copy(
    c.ar_expression_evaluator__evaluate(ref_expr_evaluator, ref_frame, ref_method_ast),
    @constCast(@ptrCast(ref_evaluator))
);
```

**Eliminate boolean flag variables:**
```zig
// Before - unnecessary boolean flag
var context_valid = false;
if (ref_context_data != null and c.ar_data__get_type(ref_context_data) == c.AR_DATA_TYPE__MAP) {
    context_valid = true;
}
if (context_valid) {
    // do work
}

// After - direct condition check
if (ref_context_data != null and c.ar_data__get_type(ref_context_data) == c.AR_DATA_TYPE__MAP) {
    // do work
}
```

## Pattern 5: Const Correctness

When migrating, identify parameters that are never mutated and make them const:

```zig
// If evaluator is never mutated, change from:
mut_evaluator: ?*ar_assignment_instruction_evaluator_t

// To:
ref_evaluator: ?*const ar_assignment_instruction_evaluator_t
```

This may require updating C headers and dependent functions to maintain consistency.

## Benefits

1. **No Casting**: Eliminates `@ptrCast` and `@alignCast` in most cases
2. **Type Safety**: Zig compiler knows the actual types
3. **Cleaner Code**: Less boilerplate, more direct usage
4. **Better Error Messages**: Compiler errors reference actual types

## Common Pitfalls

1. **Forgetting Nullability**: All C-compatible parameters should be optional (`?*T`)
2. **Wrong Type Selection**: Using C types for your own module's types
3. **Const Casting**: If you need `@constCast`, consider updating the API instead
4. **Including Own Header**: Don't include the module's own header in @cImport
5. **Unnecessary Helper Functions**: In Zig, you don't need to check arguments for functions like ar_log__error
6. **Variable Ownership Naming**: Maintain ownership prefixes even for local variables (own_, ref_, mut_)

## Related Patterns

- [C to Zig Module Migration](c-to-zig-module-migration.md) - General migration guide
- [Zig Defer for Error Cleanup](zig-defer-error-cleanup-pattern.md) - Resource management
- [Const Correctness Principle](const-correctness-principle.md) - API improvements