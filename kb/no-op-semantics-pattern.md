# No-op Semantics Pattern

## Learning
In AgeRun, `send(recipient, message)` only routes when the recipient evaluates to a nonzero INTEGER.
Integer `0` and non-INTEGER recipients are no-delivery sinks: the message expression is consumed
without processing or output. If the `send(...)` result is assigned, the language stores integer `0`
because no message was sent; AgeRun has no boolean data type.

## Importance
Understanding no-op semantics prevents confusion when messages seem to "disappear" without output. This pattern is documented in AGENTS.md but often surprises developers who expect agent 0 to represent console output or debugging.

## Example
```c
// Implementation in ar_send_instruction_evaluator.zig
if (!recipient_can_route) {
    // Non-routable recipients are sinks in evaluator control flow.
    // We need to destroy the message since it will not be sent.
    std.debug.print("DEBUG [SEND_EVAL]: Non-routable recipient - destroying message type={}\n",
                    .{c.ar_data__get_type(own_message)});
    c.ar_data__destroy_if_owned(own_message, ref_evaluator);
    send_result = false;  // Delivery status; assigned AgeRun status is integer 0
} else {
    // Normal case: send to actual agent
    send_result = c.ar_agency__send_to_agent(agent_id, own_message);
}

// Method using a no-delivery recipient for output (bootstrap.method)
memory.status := if(memory.initialized > 0, "Bootstrap initialized", "Bootstrap ready")
send(0, memory.status)  // This message is destroyed, not printed!
```

## Generalization
No-op patterns are useful for:
1. **Conditional execution**: Code can always call send() without checking
2. **Failure handling**: Any non-routable recipient indicates no valid destination
3. **Message sinking**: Unwanted messages can be discarded cleanly
4. **Simplifying logic**: No special cases needed in calling code

However, no-ops can be confusing when:
- Developers expect visible side effects (like console output)
- The no-op behavior isn't well documented
- Tests are written assuming the operation has an effect

## Implementation
```c
// Working with no-op semantics
void handle_conditional_message(int64_t condition, ar_data_t *own_message) {
    // Pattern 1: Use a non-routable recipient as a conditional sink
    ar_data_t *own_target = condition ? ar_data__create_integer(real_agent_id)
                                      : ar_data__create_integer(0);
    // Message is sent for a routable nonzero integer, destroyed otherwise
    
    // Pattern 2: Document no-op behavior in tests
    // Note: send(0, ...) and send("missing", ...) do not deliver messages
    // Don't expect console output from no-delivery recipients
    
    // Pattern 3: Use a nonzero agent for debugging output
    // Create a dedicated debug agent instead of using a no-delivery recipient
    int64_t debug_agent = ar_agency__create_agent("debug", "1.0.0", NULL);
    ar_agency__send_to_agent(debug_agent, own_debug_message);
}
```

## Related Patterns
- [System Message Flow Architecture](system-message-flow-architecture.md)
- [Test Expectation Reality Alignment](test-expectation-reality-alignment.md)
- [Message Processing Loop Pattern](message-processing-loop-pattern.md)
- [No-op Instruction Semantics](no-op-instruction-semantics.md)
