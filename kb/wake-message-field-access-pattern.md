# Wake Message Field Access Pattern

## Learning
Methods that access message fields must handle special lifecycle messages (`__wake__` and `__sleep__`) which are strings, not maps. The pattern involves detecting these special messages and providing default values for fields that would exist on map messages.

## Importance
This pattern prevents "Cannot access field on STRING value" errors that systematically affect methods across the codebase. Without this pattern, methods fail when processing wake messages, requiring error whitelisting instead of fixing the root cause.

## Example
```c
// WRONG: Assumes message is always a map
ar_data_t* sender = ar_data__get_map_data(message, "sender");  // BAD: Fails on wake message
ar_data_t* content = ar_data__get_map_data(message, "content");  // BAD: Fails on wake message

// CORRECT: Detect special messages and provide defaults
// From echo-1.0.0.method implementation:
memory.is_wake := if(message = "__wake__", 1, 0)
memory.is_sleep := if(message = "__sleep__", 1, 0)
memory.is_special := memory.is_wake + memory.is_sleep
memory.sender := if(memory.is_special > 0, 0, message.sender)
memory.content := if(memory.is_special > 0, message, message.content)
send(memory.sender, memory.content)
```

The pattern in C code when implementing evaluators:
```c
// Check if message is a special string
ar_data_type_t message_type = ar_data__get_type(message);
if (message_type == AR_DATA_TYPE_STRING) {
    const char* str_value = ar_data__get_string(message);
    if (strcmp(str_value, "__wake__") == 0 || strcmp(str_value, "__sleep__") == 0) {
        // Provide default values instead of accessing fields
        ar_data_t* own_default_sender = ar_data__create_integer(0);
        ar_data_t* own_default_content = ar_data__create_string(str_value);
        // Use defaults instead of field access
    }
}
```

## Generalization
Any method that accesses fields on incoming messages must:
1. Check if the message equals `"__wake__"` or `"__sleep__"`
2. Use conditional logic (`if()` function) to provide appropriate defaults
3. Design the logic to work with both string and map messages
4. Consider using arithmetic on flags (is_wake + is_sleep) to create unified conditions

## Implementation
To fix methods with wake message errors:

1. **Identify the pattern**:
   ```bash
   grep "Cannot access field.*on STRING value.*wake" log_whitelist.yaml
   ```

2. **Apply the fix** in the method file:
   ```
   memory.is_wake := if(message = "__wake__", 1, 0)
   memory.is_sleep := if(message = "__sleep__", 1, 0)
   memory.is_special := memory.is_wake + memory.is_sleep
   memory.field := if(memory.is_special > 0, default_value, message.field)
   ```

3. **Remove whitelist entry** after verifying the fix

4. **Test thoroughly** to ensure both special and regular messages work

## Related Patterns
- [Agent Wake Message Processing](agent-wake-message-processing.md)
- [Defensive Programming Consistency](defensive-programming-consistency.md)
- [Message Ownership Flow](message-ownership-flow.md)