# Echo Method v1.0.0

## Overview

The Echo method is a simple agent that sends back the content field of any message it receives to the original sender. This is the most basic example of an AgeRun agent and demonstrates fundamental message handling.

## Behavior

When the echo agent receives a message, it extracts the content field and sends it back to the sender.

## Implementation

```
memory.is_wake := if(message = "__wake__", 1, 0)
memory.is_sleep := if(message = "__sleep__", 1, 0)
memory.is_special := memory.is_wake + memory.is_sleep
memory.sender := if(memory.is_special > 0, 0, message.sender)
memory.content := if(memory.is_special > 0, message, message.content)
send(memory.sender, memory.content)
```

The implementation handles special lifecycle messages (`__wake__` and `__sleep__`) which are strings, not maps. For these messages:
- The sender is set to 0 (system)
- The content is the message itself
- This prevents errors when trying to access fields that don't exist on string messages

## Usage

### Creating an Echo Agent

```c
int64_t echo_agent = ar_agent__create("echo", "1.0.0", NULL);
```

### Sending Messages

```c
// Send a message with sender and content fields
ar_data_t *message = ar_data__create_map();
ar_data__set_map_integer(message, "sender", 0); // 0 = system
ar_data__set_map_string(message, "content", "Hello, World!");
ar_agent__send(echo_agent, message);
// Receives back: "Hello, World!"
```

## Message Format

The echo method expects messages with the following structure:
- `sender`: The ID of the sender (integer)
- `content`: The actual content to echo back (any type)

## Special Messages

The echo method handles lifecycle messages specially:
- `__wake__`: Sent back to agent 0 (system) with the message itself as content
- `__sleep__`: Sent back to agent 0 (system) with the message itself as content

These special messages are strings, not maps, so the method detects them and provides default values for the `sender` and `content` fields to avoid field access errors.

## Use Cases

1. **Testing**: Verify that the messaging system is working correctly
2. **Debugging**: Confirm that messages are being sent and received
3. **Heartbeat**: Simple ping/pong mechanism for health checks
4. **Learning**: Understanding basic agent behavior in AgeRun

## Version History

- **1.0.0** - Initial release with basic echo functionality