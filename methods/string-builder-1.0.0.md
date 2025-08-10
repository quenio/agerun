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
memory.parsed := parse(message.template, message.input)
memory.result := build(message.output_template, memory.parsed)
send(message.sender, memory.result)
```

The implementation uses the parse() function to extract values from the input string based on the template pattern, then uses build() to construct a new string using the output template and the parsed values.

## Error Handling

- If parsing fails (template doesn't match input), an empty map is used
- Placeholders in the output template without corresponding values are left empty
- Special messages are handled gracefully with empty string operations