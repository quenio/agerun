# Echo Method v1.0.0

## Overview

The Echo method is a simple agent that sends back any message it receives to the original sender. This is the most basic example of an AgeRun agent and demonstrates fundamental message handling.

## Behavior

When the echo agent receives a message, it immediately sends that exact message back to the sender.

## Implementation

```
send(sender, message)
```

## Usage

### Creating an Echo Agent

```c
agent_id_t echo_agent = ar_agent_create("echo", "1.0.0", NULL);
```

### Sending Messages

```c
// Send a simple text message
ar_agent_send(echo_agent, "Hello, World!");
// Receives back: "Hello, World!"

// Send a structured message
ar_agent_send(echo_agent, "{\"type\": \"ping\", \"id\": 123}");
// Receives back: "{\"type\": \"ping\", \"id\": 123}"
```

## Message Format

The echo method accepts any message format and returns it unchanged:
- Plain text strings
- JSON-formatted data
- Numeric values
- Any other string representation

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