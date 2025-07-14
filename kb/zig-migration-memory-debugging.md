# Zig Migration Memory Debugging

## Learning
During Zig module migrations, memory leaks can arise from misunderstanding C function ownership semantics. The key discovery pattern involves comparing implementations line-by-line when tests show memory leaks that didn't exist before migration. Modern Zig modules should use ar_allocator for type-safe memory management that eliminates many casting and ownership issues.

## Importance
Memory management is critical in AgeRun. A Zig implementation that looks correct but misunderstands ownership transfer will introduce leaks. This pattern helps identify subtle ownership semantic differences between implementations.

## Example
```c
// C implementation that transfers ownership
char** ar_expression_ast__get_memory_path(const ar_expression_ast_t *ref_node, size_t *out_count) {
    // ar_list__items() creates a NEW array and transfers ownership to caller
    return (char**)ar_list__items(ref_node->data.memory_access.own_path);
}

// WRONG Zig implementation (creates leak)
export fn ar_expression_ast__get_memory_path(ref_node: ?*const c.ar_expression_ast_t, out_count: ?*usize) ?[*]?[*:0]u8 {
    // This creates TWO arrays - one from malloc, one from ar_list__items!
    const own_array = @ptrCast(@alignCast(c.AR__HEAP__MALLOC(count * @sizeOf([*:0]u8), "Memory path array")));
    const own_items = c.ar_list__items(ref_path_list);
    // ... copy and free own_items - but we leaked own_array!
}

// CORRECT Zig implementation (matches C behavior)
export fn ar_expression_ast__get_memory_path(ref_node: ?*const c.ar_expression_ast_t, out_count: ?*usize) ?[*]?[*:0]u8 {
    // Match C exactly - just return the array from ar_list__items
    return @ptrCast(c.ar_list__items(ref_path_list));
}

// PREFERRED: Modern approach for new Zig code
const ar_allocator = @import("ar_allocator.zig");

export fn ar_expression_ast__create_example() ?*c.ar_expression_ast_t { // EXAMPLE: New function pattern
    // ar_allocator provides type safety and automatic tracking integration
    const own_node = ar_allocator.create(ar_expression_ast_t, "expression AST node");
    if (own_node == null) return null;
    
    // No casting needed - direct field access
    own_node.?.node_type = c.AR_EXPRESSION_AST_TYPE__LITERAL_INT;
    return @ptrCast(own_node);
}
```

## Generalization
When migrating C to Zig and encountering memory leaks:

1. **Preferred**: Use ar_allocator for new Zig code to avoid casting/ownership issues
2. **Check memory reports**: `cat bin/memory_report_<test_name>.log`
3. **Identify leak source**: Note the allocation description (e.g., "List items array")
4. **Compare implementations line-by-line**:
   ```bash
   # Compare specific function implementations
   grep -A20 "function_name" modules/ar_module.zig  # EXAMPLE: Replace with actual module
   # If C file still exists:
   grep -A20 "function_name" modules/ar_module.c  # EXAMPLE: C file may be deleted
   ```
5. **Look for ownership transfer patterns**:
   - Functions returning allocated memory transfer ownership
   - Functions like `ar_list__items()` create new arrays
   - Match C behavior exactly, don't add extra allocations
   - For new code: ar_allocator eliminates many ownership complexity issues

## Implementation
```bash
# Step 1: Run test and check for leaks
make ar_expression_ast_tests
cat bin/memory_report_ar_expression_ast_tests.log

# Step 2: If leaks found, identify the source
# Example output: "ar_data_t (list) allocated at ar_list.c:231"

# Step 3: Find usage of the leaking function
grep -n "ar_list__items" modules/ar_expression_ast.zig

# Step 4: Compare with C implementation
# Check how C handles the same function
diff -u <(grep -A10 "get_memory_path" modules/ar_expression_ast.c) \  # EXAMPLE: C file no longer exists
        <(grep -A10 "get_memory_path" modules/ar_expression_ast.zig)

# Step 5: Fix by matching C's ownership semantics exactly
```

## Related Patterns
- [Zig Memory Allocation with ar_allocator](zig-memory-allocation-with-ar-allocator.md)
- [C to Zig Module Migration](c-to-zig-module-migration.md)
- [Zig-C Memory Tracking Consistency](zig-c-memory-tracking-consistency.md)
- [Memory Leak Detection Workflow](memory-leak-detection-workflow.md)
- [Ownership Naming Conventions](ownership-naming-conventions.md)