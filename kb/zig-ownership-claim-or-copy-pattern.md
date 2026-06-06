# Zig Ownership with claim_or_copy Pattern

## Overview

When migrating evaluators to Zig, understanding the `ar_data__claim_or_copy` pattern is crucial for proper memory management. This function ensures ownership while avoiding unnecessary copies.

## The Problem

Expression evaluators return `ar_data_t*` values with uncertain ownership:
- References from memory/context are borrowed (not owned)
- New objects from arithmetic/string operations are owned
- We need ownership to safely pass values or store them

## The Solution: ar_data__claim_or_copy

This function provides a consistent way to obtain ownership:
```zig
// Takes ownership if unowned, copies if already owned
const own_value = c.ar_data__claim_or_copy(
    expression_result,  // Result from evaluator (ownership unknown)
    owner_ptr          // Who will own the result
);
```

## Implementation Pattern

### Direct Evaluation Pattern (Recommended)
```zig
// Combine evaluation and ownership claim in one expression
const own_method_name = c.ar_data__claim_or_copy(
    c.ar_expression_evaluator__evaluate(ref_expr_evaluator, ref_frame, ref_method_ast),
    @constCast(@ptrCast(ref_evaluator))
);
if (own_method_name == null) {
    c.ar_log__error(ref_evaluator.?.ref_log, "Error message");
    return false;
}
defer c.ar_data__destroy_if_owned(own_method_name, @constCast(@ptrCast(ref_evaluator)));
```

### Why Not Direct Assignment?

You cannot directly assign the evaluation result:
```zig
// WRONG - ownership is uncertain
const own_value = c.ar_expression_evaluator__evaluate(...);  // May be borrowed!

// CORRECT - ensures ownership
const own_value = c.ar_data__claim_or_copy(
    c.ar_expression_evaluator__evaluate(...),
    owner
);
```

## Key Points

1. **Always use claim_or_copy** when you need ownership of evaluation results
2. **Use destroy_if_owned** for cleanup - it pairs with claim_or_copy
3. **Context references** are special - they're always borrowed, never owned
4. **Combine operations** - no need for temporary variables between evaluate and claim_or_copy
5. **Container children require correct ownership metadata** - public `ar_data` container APIs mark
   children as owned by their parent, so `claim_or_copy` deep-copies them instead of stealing them

## Common Use Cases

### Function Arguments
```zig
// Getting owned copies of function arguments
const own_arg1 = c.ar_data__claim_or_copy(
    c.ar_expression_evaluator__evaluate(evaluator, frame, arg1_ast),
    @constCast(@ptrCast(self))
);
defer c.ar_data__destroy_if_owned(own_arg1, @constCast(@ptrCast(self)));
```

### Borrowed Context
```zig
// Context is borrowed - no claim_or_copy needed
const ref_context = c.ar_expression_evaluator__evaluate(evaluator, frame, context_ast);
// Pass directly - agency expects borrowed reference
ar_agency__create_agent(method, version, ref_context);
```

### Container Child Extraction

`ar_data__claim_or_copy()` is safe for the top-level value returned by expression evaluation because
unowned temporaries can be consumed by the instruction. It is also safe for child values stored by
the public `ar_data` list/map APIs, because those APIs mark children as owned by the source container.
For example, `head([ ... ])` evaluates a temporary list, reads the first item, and `claim_or_copy()`
deep-copies that item because the item is owned by the list.

```zig
// Correct for head/tail-style extraction when ref_item came from an ar_data container
const own_copy = c.ar_data__claim_or_copy(ref_item, ref_evaluator) orelse return false;
```

Do not bypass the `ar_data` container APIs and then rely on `claim_or_copy()`: if a raw list/map holds
an unowned child, `claim_or_copy()` can still claim that child while the parent later tries to destroy
it. In that case, fix the container ownership metadata, remove/transfer the child first, or make an
explicit `ar_data__deep_copy()`.

## Related Patterns

- [Expression Ownership Rules](expression-ownership-rules.md)
- [Ownership Pattern Extraction](ownership-pattern-extraction.md)
- [Zig Type Usage Patterns](zig-type-usage-patterns.md)
