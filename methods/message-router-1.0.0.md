# Message Router Method

Version: 1.0.0

## Overview

The message-router method demonstrates routing messages to different agents based on routing criteria, leveraging the `send()` function's no-op behavior when agent_id is 0.

## Message Format

The method expects a message with the following fields:
- `route`: The routing destination ("echo" or "calc")
- `payload`: The message to route to the target agent
- `echo_agent`: The agent ID of the echo service
- `calc_agent`: The agent ID of the calculator service
- `sender`: The agent ID to send the result back to

## Behavior

1. Determines the target agent based on the route
2. Routes "echo" to the echo_agent
3. Routes "calc" to the calc_agent
4. Sends the payload to the target agent (or no-op if target is 0)
5. Returns true if sent successfully, false if target doesn't exist

## Example Usage

Route to echo:
```
{
  "route": "echo",
  "payload": "Hello, World!",
  "echo_agent": 10,
  "calc_agent": 20,
  "sender": 200
}
```
Output: true (message sent to agent 10)

Invalid route:
```
{
  "route": "unknown",
  "payload": {"data": 123},
  "echo_agent": 10,
  "calc_agent": 20,
  "sender": 200
}
```
Output: true (no-op since target is 0)

## Implementation

```
memory.is_wake := if(message = "__wake__", 1, 0)
memory.is_sleep := if(message = "__sleep__", 1, 0)
memory.is_special := memory.is_wake + memory.is_sleep
memory.route := if(memory.is_special > 0, "none", message.route)
memory.echo_agent := if(memory.is_special > 0, 0, message.echo_agent)
memory.calc_agent := if(memory.is_special > 0, 0, message.calc_agent)
memory.payload := if(memory.is_special > 0, message, message.payload)
memory.sender := if(memory.is_special > 0, 0, message.sender)
memory.is_echo := if(memory.route = "echo", 1, 0)
memory.is_calc := if(memory.route = "calc", 1, 0)
memory.target := if(memory.is_echo = 1, memory.echo_agent, 0)
memory.target := if(memory.is_calc = 1, memory.calc_agent, memory.target)
memory.sent := send(memory.target, memory.payload)
send(memory.sender, memory.sent)
```

The implementation handles special lifecycle messages (`__wake__` and `__sleep__`) which are strings, not maps. For these messages:
- The route is set to "none" (no routing performed)
- Agent IDs are set to 0
- The payload is the message itself
- The sender is set to 0 (system)
- Result of send (true for no-op) is sent back
- This prevents errors when trying to access fields that don't exist on string messages

## Special Messages

The message router method handles lifecycle messages specially:
- `__wake__`: Sent back to agent 0 (system) with result true (no-op send)
- `__sleep__`: Sent back to agent 0 (system) with result true (no-op send)

These special messages are strings, not maps, so the method detects them and provides default values to avoid field access errors.

## Implementation Notes

This method showcases:
- Conditional routing logic using `if()` functions
- Leveraging `send(0, message)` as a no-op for invalid routes
- Clean handling of routing without complex conditional statements
- The sent status is automatically determined by the send function