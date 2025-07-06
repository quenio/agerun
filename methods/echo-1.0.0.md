# Echo Method v1.0.0

## Overview

The Echo method is a simple agent that sends back the content field of any message it receives to the original sender. This is the most basic example of an AgeRun agent and demonstrates fundamental message handling.

## Behavior

When the echo agent receives a message, it extracts the content field and sends it back to the sender.

## Implementation

```
send(message.sender, message.content)
```

## Usage

### Creating an Echo Agent

```c
int64_t echo_agent = ar_agent__create("echo", "1.0.0", NULL);
```

### Sending Messages

```c
// Send a message with sender and content fields
data_t *message = ar_data__create_map();
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

The echo method does not specially handle lifecycle messages:
- `__wake__`: Echoed back like any other message
- `__sleep__`: Echoed back like any other message

## Use Cases

1. **Testing**: Verify that the messaging system is working correctly
2. **Debugging**: Confirm that messages are being sent and received
3. **Heartbeat**: Simple ping/pong mechanism for health checks
4. **Learning**: Understanding basic agent behavior in AgeRun

## Version History

- **1.0.0** - Initial release with basic echo functionality