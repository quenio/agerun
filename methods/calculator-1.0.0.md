# Calculator Method v1.0.0

## Overview

The Calculator method implements a basic arithmetic calculator that can perform addition, subtraction, multiplication, and division operations. It maintains a result value in memory and responds with the calculated result.

## Behavior

The calculator expects messages with structured data containing:
- `operation`: The arithmetic operation to perform ("add", "multiply", "subtract", "divide")
- `a`: The first operand
- `b`: The second operand
- `sender`: The agent ID to send the result back to

The calculator performs the requested operation and sends the result back to the sender.

## Implementation

```
memory.result := 0
memory.result := if(message.operation = "add", message.a + message.b, memory.result)
memory.result := if(message.operation = "multiply", message.a * message.b, memory.result)
memory.result := if(message.operation = "subtract", message.a - message.b, memory.result)
memory.result := if(message.operation = "divide", message.a / message.b, memory.result)
send(message.sender, memory.result)
```

## Usage

### Creating a Calculator Agent

```c
int64_t calc = ar__agent__create("calculator", "1.0.0", NULL);
```

### Performing Operations

```c
// Addition: 10 + 5 = 15
ar__data__t *msg = ar__data__create_map();
ar__data__set_map_data(msg, "operation", ar__data__create_string("add"));
ar__data__set_map_data(msg, "a", ar__data__create_integer(10));
ar__data__set_map_data(msg, "b", ar__data__create_integer(5));
ar__data__set_map_data(msg, "sender", ar__data__create_integer(my_agent_id));
ar__agent__send_data(calc, msg);
// Receives back: 15

// Multiplication: 7 * 8 = 56
ar__data__set_map_data(msg, "operation", ar__data__create_string("multiply"));
ar__data__set_map_data(msg, "a", ar__data__create_integer(7));
ar__data__set_map_data(msg, "b", ar__data__create_integer(8));
ar__agent__send_data(calc, msg);
// Receives back: 56

// Division: 100 / 4 = 25
ar__data__set_map_data(msg, "operation", ar__data__create_string("divide"));
ar__data__set_map_data(msg, "a", ar__data__create_integer(100));
ar__data__set_map_data(msg, "b", ar__data__create_integer(4));
ar__agent__send_data(calc, msg);
// Receives back: 25
```

## Message Format

### Input Message Structure

The calculator expects a map with the following fields:

| Field | Type | Description |
|-------|------|-------------|
| `operation` | String | The operation to perform: "add", "multiply", "subtract", or "divide" |
| `a` | Number | The first operand (integer or double) |
| `b` | Number | The second operand (integer or double) |
| `sender` | Integer | The agent ID to send the result to |

### Output Message

The calculator sends back a single numeric value representing the result of the operation.

## Supported Operations

1. **Addition** (`"add"`): Returns a + b
2. **Multiplication** (`"multiply"`): Returns a * b  
3. **Subtraction** (`"subtract"`): Returns a - b
4. **Division** (`"divide"`): Returns a / b

## Error Handling

- If the operation is not recognized, the calculator returns the last computed result (initially 0)
- Division by zero follows standard floating-point behavior (returns infinity)
- Missing fields in the message may cause undefined behavior

## Memory State

The calculator maintains a single value in memory:
- `memory.result`: Stores the last computed result (initialized to 0)

## Use Cases

1. **Mathematical Computations**: Perform basic arithmetic in agent workflows
2. **Data Processing**: Calculate aggregates, averages, or other metrics
3. **Game Logic**: Handle score calculations or game mechanics
4. **Financial Calculations**: Simple interest, percentage calculations, etc.

## Limitations

- Only supports binary operations (two operands)
- No support for complex expressions or operator precedence
- Results are not persistent across agent restarts unless the agent is saved

## Version History

- **1.0.0** - Initial release with add, multiply, subtract, and divide operations