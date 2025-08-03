# Debug Logging for Ownership Tracing

## Learning
When debugging data corruption issues, add debug logging at every ownership transfer point and data access point. This creates a trace showing exactly where and when data changes, revealing ownership bugs that would otherwise be invisible.

## Importance
Ownership bugs are nearly impossible to debug without visibility. Strategic logging reveals the exact moment corruption occurs, turning hours of debugging into minutes.

## Example
```c
// Add debug logging at key points:

// 1. When sending messages (ar_agency.c)
fprintf(stderr, "DEBUG [SEND]: Sending message to agent %lld, ", (long long)agent_id);
switch (ar_data__get_type(own_message)) {
    case AR_DATA_TYPE__INTEGER:
        fprintf(stderr, "type=INTEGER, value=%lld\n", (long long)ar_data__get_integer(own_message));
        break;
    case AR_DATA_TYPE__MAP:
        fprintf(stderr, "type=MAP\n");
        break;
    // ... other types
}

// 2. When creating frames (ar_frame.c)
fprintf(stderr, "DEBUG [FRAME_CREATE]: Creating frame with message type=%d", ar_data__get_type(ref_message));
if (ar_data__get_type(ref_message) == AR_DATA_TYPE__INTEGER) {
    fprintf(stderr, ", value=%lld", (long long)ar_data__get_integer(ref_message));
}
fprintf(stderr, "\n");

// 3. When accessing data (ar_expression_evaluator.zig)
if (map != null) {
    std.debug.print("DEBUG [MEMORY_ACCESS]: Accessing 'message', type={}, ", .{c.ar_data__get_type(map)});
    if (c.ar_data__get_type(map) == c.AR_DATA_TYPE__INTEGER) {
        std.debug.print("value={}\n", .{c.ar_data__get_integer(map)});
    } else {
        std.debug.print("\n", .{});
    }
}

// 4. When claiming ownership (ar_expression_evaluator.zig)
if (ref_value == own_value and c.ar_data__get_type(ref_value) == c.AR_DATA_TYPE__MAP) {
    std.debug.print("DEBUG [EVALUATE_EXPR]: Claimed ownership of MAP at {*}\n", .{ref_value});
}
```

## Generalization
Debug logging strategy for ownership issues:
1. **Log at every transfer point**: Send, receive, create, destroy
2. **Include key information**: Component name, operation, data type, data value (if simple), pointer address
3. **Use consistent prefixes**: [COMPONENT_ACTION] for easy grep filtering
4. **Log ownership changes**: When taking, dropping, or transferring ownership
5. **Keep logs after fixing**: Future debugging will benefit

## Implementation
```c
// Macro for consistent ownership logging
#define LOG_OWNERSHIP(component, action, data) \
    fprintf(stderr, "DEBUG [%s_%s]: data=%p, type=%d, owned=%s\n", \
            component, action, (void*)data, \
            ar_data__get_type(data), \
            ar_data__is_owned(data) ? "yes" : "no")  // EXAMPLE: ar_data__is_owned hypothetical

// Usage throughout codebase
LOG_OWNERSHIP("AGENT", "SEND", own_message);
LOG_OWNERSHIP("SYSTEM", "RECEIVE", own_message);
LOG_OWNERSHIP("FRAME", "STORE", ref_message);

// Filter logs for specific component
make test_name 2>&1 | grep "DEBUG \[AGENT"

// Trace specific data pointer
make test_name 2>&1 | grep "data=0x600001db5640"

// See ownership flow
make test_name 2>&1 | grep -E "SEND|RECEIVE|CLAIM|DESTROY"
```

## Related Patterns
- [Evidence-Based Debugging](evidence-based-debugging.md)
- [Memory Debugging Comprehensive Guide](memory-debugging-comprehensive-guide.md)
- [Ownership Gap Vulnerability Pattern](ownership-gap-vulnerability.md)
- [Stderr Redirection for Debugging](stderr-redirection-debugging.md)