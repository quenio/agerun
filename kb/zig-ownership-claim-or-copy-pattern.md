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
5. **Do not use claim_or_copy for container children that remain in the container** - use
   `ar_data__deep_copy()` unless you remove/transfer the child first

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
unowned temporaries can be consumed by the instruction. It is not safe for child values that are still
inside a container. For example, `head([ ... ])` evaluates a temporary list and then reads the first
item. If the first item is unowned and `claim_or_copy()` claims it, destroying the temporary source
list later tries to destroy a child that is now owned elsewhere.

Use `ar_data__deep_copy()` when an instruction returns or stores a subvalue without removing it from
its source container:

```zig
// Correct for head/tail-style extraction: source list keeps owning ref_item
const own_copy = c.ar_data__deep_copy(ref_item) orelse return false;
```

Use `claim_or_copy()` only after the value has been removed/transferred out of the container, or when
the value is the top-level expression result being consumed by the instruction.

## Related Patterns

- [Expression Ownership Rules](expression-ownership-rules.md)
- [Ownership Pattern Extraction](ownership-pattern-extraction.md)
- [Zig Type Usage Patterns](zig-type-usage-patterns.md)
