# Method Creator Method

Version: 1.0.0

## Overview

The method-creator method allows agents to dynamically create new methods at runtime using the `compile()` function.

## Message Format

The method expects a message with the following fields:
- `method_name`: The name of the new method to create
- `instructions`: The instruction code for the method (one instruction per line)
- `version`: The semantic version string (e.g., "1.0.0")
- `sender`: The agent ID to send the result back to

## Behavior

1. Extracts the method name, instructions, and version from the message
2. Calls the `compile()` function to create and register the new method
3. Returns 1 if successful, 0 if failed
4. Sends the result back to the sender

## Example Usage

Input message:
```
{
  "method_name": "multiplier",
  "instructions": "memory.result := message.a * message.b\nsend(message.sender, memory.result)",
  "version": "1.0.0",
  "sender": 456
}
```

Output: 1 (success)

## Notes

- The created method is immediately available for use by agents
- Methods are not persisted by default
- Version must follow semantic versioning format
- Instructions must be valid AgeRun syntax