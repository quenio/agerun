# String Builder Method

Version: 1.0.0

## Overview

The string-builder method demonstrates the use of the `parse()` and `build()` functions to extract values from strings and construct new strings using templates.

## Message Format

The method expects a message with the following fields:
- `template`: The template string for parsing (e.g., "name={name}, age={age}")
- `input`: The input string to parse (e.g., "name=John, age=30")
- `output_template`: The template for building the output (e.g., "Hello {name}, you are {age} years old!")
- `sender`: The agent ID to send the result back to

## Behavior

1. Parses the input string using the provided template
2. Extracts values into a map
3. Builds a new string using the output template and extracted values
4. Sends the result back to the sender

## Example Usage

Input message:
```
{
  "template": "user={username}, role={role}",
  "input": "user=alice, role=admin",
  "output_template": "Welcome {username}! Your role is: {role}",
  "sender": 123
}
```

Output: "Welcome alice! Your role is: admin"

## Implementation

```
memory.is_wake := if(message = "__wake__", 1, 0)
memory.is_sleep := if(message = "__sleep__", 1, 0)
memory.is_special := memory.is_wake + memory.is_sleep
memory.template := if(memory.is_special > 0, "", message.template)
memory.input := if(memory.is_special > 0, "", message.input)
memory.output_template := if(memory.is_special > 0, "", message.output_template)
memory.sender := if(memory.is_special > 0, 0, message.sender)
memory.parsed := parse(memory.template, memory.input)
memory.result := build(memory.output_template, memory.parsed)
send(memory.sender, memory.result)
```

The implementation handles special lifecycle messages (`__wake__` and `__sleep__`) which are strings, not maps. For these messages:
- All template fields are set to empty strings
- The sender is set to 0 (system)
- Parse and build operations work with empty strings
- Empty result is sent back
- This prevents errors when trying to access fields that don't exist on string messages

## Special Messages

The string builder method handles lifecycle messages specially:
- `__wake__`: Sent back to agent 0 (system) with empty string result
- `__sleep__`: Sent back to agent 0 (system) with empty string result

These special messages are strings, not maps, so the method detects them and provides default values to avoid field access errors.

## Error Handling

- If parsing fails (template doesn't match input), an empty map is used
- Placeholders in the output template without corresponding values are left empty
- Special messages are handled gracefully with empty string operations