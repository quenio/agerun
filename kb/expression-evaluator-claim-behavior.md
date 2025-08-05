# Expression Evaluator Claim Behavior

## Learning
The expression evaluator can claim ownership of unowned data through `ar_data__claim_or_copy()` when evaluating expressions. This is particularly dangerous during comparisons like `message = "__wake__"` where the evaluator may take ownership of data that should remain available to other parts of the system.

## Importance
- Explains mysterious data corruption where values suddenly change type
- Critical for understanding why data must have an owner before evaluation
- Reveals a subtle behavior that can cause frame reference corruption
- Essential knowledge for debugging "Cannot access field X on INTEGER value 0" errors

## Example
```c
// Scenario: Wake message handling in dynamically generated method
// Generated method includes: memory.is_wake := if(message = "__wake__", 1, 0)

// PROBLEM: Message has no owner when passed to interpreter
ar_data_t *ref_message = /* unowned MAP from test */;
ar_frame_t *own_frame = ar_frame__create(mut_memory, ref_context, ref_message);

// During evaluation of (message = "__wake__"):
// 1. Evaluator accesses 'message' via ar_frame__get_message()
// 2. Calls ar_data__claim_or_copy(ref_value, ref_frame)
// 3. Since message is unowned, claim succeeds!
// 4. Debug output: "DEBUG [EVALUATE_EXPR]: Claimed ownership of MAP"
// 5. Message in frame now points to corrupted/destroyed data

// RESULT: Next access to message shows INTEGER 0 instead of MAP
// ERROR: Cannot access field 'text' on INTEGER value 0 (base: message)

// SOLUTION: Ensure message has an owner before evaluation
ar_data__take_ownership(message, owner);  // Now claim will fail, copy will be made
```

## Generalization
1. **Claim mechanism**: `ar_data__claim_or_copy()` tries to claim before copying
2. **Unowned vulnerability**: Data without an owner can be claimed by any component
3. **Comparison danger**: Expressions comparing data values trigger claim attempts
4. **Frame corruption**: Claimed data in frames leads to reference corruption
5. **Debug indicator**: "Claimed ownership" messages reveal this behavior

## Implementation
```c
// Inside ar_expression_evaluator.zig (actual code)
c.AR_EXPRESSION_AST_TYPE__MEMORY_ACCESS => {
    // Memory access returns a reference
    const ref_value = _evaluate_memory_access(ref_log, ref_frame, ref_node);
    
    // Use claim_or_copy to handle ownership properly
    const own_value = c.ar_data__claim_or_copy(ref_value, ref_frame);
    
    // Debug message when claim succeeds (data was unowned)
    if (ref_value == own_value and c.ar_data__get_type(ref_value) == c.AR_DATA_TYPE__MAP) {
        std.debug.print("DEBUG [EVALUATE_EXPR]: Claimed ownership of MAP at {*}\n", .{ref_value});
    }
    return own_value;
}

// The ar_data__claim_or_copy implementation
ar_data_t* ar_data__claim_or_copy(ar_data_t *ref_data, const void *owner) {
    if (!ref_data || !owner) {
        return NULL;
    }
    
    // Try to claim ownership first
    if (ar_data__take_ownership(ref_data, owner)) {
        // Successfully claimed - drop it back so caller owns it
        ar_data__drop_ownership(ref_data, owner);
        return ref_data;  // Return same pointer - we claimed it!
    } else {
        // Can't claim ownership - make a shallow copy
        return ar_data__shallow_copy(ref_data);
    }
}
```

## Related Patterns
- [Expression Ownership Rules](expression-ownership-rules.md)
- [Test Fixture Message Ownership](test-fixture-message-ownership.md)
- [Ownership Gap Vulnerability](ownership-gap-vulnerability.md)
- [Debug Logging for Ownership Tracing](debug-logging-ownership-tracing.md)