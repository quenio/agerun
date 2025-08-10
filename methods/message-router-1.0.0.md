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
memory.is_echo := if(message.route = "echo", 1, 0)
memory.is_calc := if(message.route = "calc", 1, 0)
memory.target := if(memory.is_echo = 1, message.echo_agent, 0)
memory.target := if(memory.is_calc = 1, message.calc_agent, memory.target)
memory.sent := send(memory.target, message.payload)
send(message.sender, memory.sent)
```

The implementation checks the route field to determine which agent to send the message to. It supports "echo" and "calc" routes, sending the payload to the corresponding agent. The result of the send operation (true/false) is returned to the original sender.

## Implementation Notes

This method showcases:
- Conditional routing logic using `if()` functions
- Leveraging `send(0, message)` as a no-op for invalid routes
- Clean handling of routing without complex conditional statements
- The sent status is automatically determined by the send function