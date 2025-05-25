# Agent Manager Method

Version: 1.0.0

## Overview

The agent-manager method provides functionality to create and destroy agents dynamically using the `agent()` and `destroy()` functions.

## Message Format

The method expects a message with the following fields:
- `action`: Either "create" or "destroy"
- `sender`: The agent ID to send the result back to

For "create" action:
- `method_name`: The name of the method for the new agent
- `version`: The version of the method (e.g., "1.0.0")
- `context`: The context map for the new agent

For "destroy" action:
- `agent_id`: The ID of the agent to destroy

## Behavior

1. Checks the action type
2. For "create": Creates a new agent with the specified method and context
3. For "destroy": Destroys the specified agent
4. Returns the agent ID for create, or 1/0 for destroy success/failure
5. Sends the result back to the sender

## Example Usage

Create agent:
```
{
  "action": "create",
  "method_name": "echo",
  "version": "1.0.0",
  "context": {"name": "Echo Bot"},
  "sender": 789
}
```
Output: 123 (new agent ID)

Destroy agent:
```
{
  "action": "destroy",
  "agent_id": 123,
  "sender": 789
}
```
Output: 1 (success)

## Notes

- Created agents automatically receive a `__wake__` message
- Destroyed agents receive a `__sleep__` message before destruction
- Returns 0 if the action fails or is unrecognized