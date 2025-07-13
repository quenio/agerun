# Agent Manager Method

Version: 1.0.0

## Overview

The agent-manager method provides functionality to spawn and exit agents dynamically using the `spawn()` and `exit()` functions.

## Message Format

The method expects a message with the following fields:
- `action`: Either "spawn" or "exit"
- `sender`: The agent ID to send the result back to

For "spawn" action:
- `method_name`: The name of the method for the new agent
- `version`: The version of the method (e.g., "1.0.0")
- `context`: The context map for the new agent

For "exit" action:
- `agent_id`: The ID of the agent to exit

## Behavior

1. Checks the action type
2. For "spawn": Creates a new agent with the specified method and context
3. For "exit": Exits the specified agent
4. Returns the agent ID for spawn, or 1/0 for exit success/failure
5. Sends the result back to the sender

## Example Usage

Spawn agent:
```
{
  "action": "spawn",
  "method_name": "echo",
  "version": "1.0.0",
  "context": {"name": "Echo Bot"},
  "sender": 789
}
```
Output: 123 (new agent ID)

Exit agent:
```
{
  "action": "exit",
  "agent_id": 123,
  "sender": 789
}
```
Output: 1 (success)

## Notes

- Spawned agents automatically receive a `__wake__` message
- Exited agents receive a `__sleep__` message before exit
- Returns 0 if the action fails or is unrecognized