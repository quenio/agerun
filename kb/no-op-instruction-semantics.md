# No-op Instruction Semantics

## Learning
Instruction evaluators should handle intentional no-op cases gracefully. Some no-ops continue in
evaluator control flow, such as a non-routable `send(...)` recipient and a non-string `spawn`
method-name selector.
Other no-spawn cases, such as a string method name that misses methodology lookup, may fail evaluator
control flow while still storing the documented language result. If the instruction has a
language-level result assignment, store the documented integer status/result value; AgeRun has no
boolean data type. For `send(...)`, integer status reports delivery: integer `1` means a message was
enqueued and integer `0` means no message was sent because the recipient was not a routable nonzero
INTEGER or because delivery failed.

## Importance
Proper no-op semantics enable:
- Conditional execution patterns in methods
- Graceful handling of optional operations
- Methods that adapt to different contexts
- Avoiding error propagation for intentional no-ops

## Example
```c
// In ar_spawn_instruction_evaluator.zig:

// Non-string method-name selectors do not spawn an agent, but preserve method control flow.
if (c.ar_data__get_type(own_method_name) != c.AR_DATA_TYPE__STRING) {
    if (c.ar_instruction_ast__has_result_assignment(ref_ast)) {
        const own_result = c.ar_data__create_integer(0);
        _ = c.ar_result_binding__bind(ref_log, ref_frame, ref_result_path, own_result);
    }
    return true; // Host bool: intentional no-op selector
}

// String method names that miss lookup also store the no-spawn result, but are lookup failures.
if (ref_method == null) {
    if (c.ar_instruction_ast__has_result_assignment(ref_ast)) {
        const own_result = c.ar_data__create_integer(0);
        _ = c.ar_result_binding__bind(ref_log, ref_frame, ref_result_path, own_result);
    }
    return false; // Host bool: lookup failed
}
```

AgeRun method usage:
```agerun
# Conditional spawning - no spawn when not booting
memory.method_name := if(memory.is_boot = 1, "echo", 0)
memory.echo_id := spawn(memory.method_name, memory.version, context)
# spawn does not create an agent and echo_id = 0 when method_name = 0
```

## Generalization
When implementing instruction evaluators:
1. Identify documented no-op/failure conditions; do not add extra sentinel values by analogy
2. Choose host evaluator success or failure according to the documented instruction contract
3. Set result variables to the documented result value, such as integer `0` when no agent is spawned
4. Document no-op behavior in specifications
5. Add tests specifically for no-op cases

## Implementation
```bash
# Find instructions that might need no-op handling
grep -r "return false" modules/*_instruction_evaluator.zig

# Add no-op checks before validation
# Succeed in evaluator control flow for no-op cases
# Set appropriate integer result values
```

## Related Patterns
- [No-op Semantics Pattern](no-op-semantics-pattern.md)
