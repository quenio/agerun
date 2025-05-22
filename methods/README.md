# AgeRun Methods Directory

This directory contains AgeRun method definitions. Each file represents a single method that can be instantiated and executed by agents in the AgeRun system.

## File Naming Convention

Method files follow a strict naming convention:

```
<method-name>-<semver>.method
```

Where:
- `<method-name>` is the name of the method (alphanumeric characters and underscores)
- `<semver>` is the semantic version in the format `MAJOR.MINOR.PATCH` (e.g., `1.0.0`)
- `.method` is the required file extension

### Examples

- `echo-1.0.0.method` - Echo method version 1.0.0
- `calculator-2.1.3.method` - Calculator method version 2.1.3
- `data_processor-0.5.0.method` - Data processor method version 0.5.0

## Method File Format

Each `.method` file is a plain text file containing AgeRun instruction code. A method definition consists of a sequence of instructions, with each instruction on a separate line.

### Structure

According to the AgeRun grammar:
- Each instruction must be on its own line
- Instructions cannot be combined on the same line
- Even the last instruction must be followed by a newline
- Empty lines are ignored

### Instruction Types

Instructions can be one of two types:

1. **Assignment**: Stores a value in memory
   ```
   memory.variable := expression
   ```

2. **Function Instruction**: Calls a system function (optionally storing the result)
   ```
   function(parameters)
   memory.result := function(parameters)
   ```

### Example Method Files

**Simple Echo Method** (`echo-1.0.0.method`):
```
memory.sender := message.sender
memory.data := message.data
send(memory.sender, "Echo: " + memory.data)
```

**Calculator Method** (`calculator-1.0.0.method`):
```
memory.result := 0
memory.result := if(message.operation = "add", message.a + message.b, memory.result)
memory.result := if(message.operation = "multiply", message.a * message.b, memory.result)
memory.result := if(message.operation = "subtract", message.a - message.b, memory.result)
memory.result := if(message.operation = "divide", message.a / message.b, memory.result)
send(message.sender, memory.result)
```

**Agent Spawner Method** (`spawner-1.0.0.method`):
```
memory.child_id := agent(message.method_name, message.version, message.context)
send(memory.child_id, "__wake__")
memory.success := if(memory.child_id > 0, true, false)
send(message.sender, memory.success)
```

### Available Instructions

Methods can use the following system functions:

- **send(agent_id, message)**: Send a message to another agent
- **parse(template, input)**: Extract values from strings based on templates
- **build(template, values)**: Construct strings by replacing placeholders
- **method(name, instructions, version)**: Define a new method
- **agent(method_name, version, context)**: Create a new agent instance
- **destroy(agent_id)**: Destroy an agent
- **destroy(method_name, version)**: Destroy a method version
- **if(condition, true_value, false_value)**: Conditional evaluation

### Expressions

Expressions can include:
- **String literals**: `"Hello, World"`
- **Number literals**: `42`, `3.14`
- **Memory access**: `memory.variable`, `message.field`, `context.setting`
- **Arithmetic**: `a + b`, `x * y`, `m - n`, `p / q`
- **Comparisons**: `a = b`, `x <> y`, `m < n`, `p <= q`, `r > s`, `t >= u`
- **String concatenation**: `"Hello, " + name`

### Special Messages

Methods should handle these lifecycle messages:
- `__wake__`: Sent when an agent is created or resumed
- `__sleep__`: Sent before an agent is paused or destroyed

## Version Management

Methods follow semantic versioning:
- **MAJOR**: Incompatible changes to method behavior
- **MINOR**: New functionality added in a backward-compatible manner
- **PATCH**: Backward-compatible bug fixes

Multiple versions of the same method can coexist, allowing agents to use specific versions as needed.

## Usage

Methods in this directory are loaded by the AgeRun methodology system and can be instantiated by agents using:

```
agent("method_name", "version", context)
```

The system will look for the corresponding file in this directory based on the method name and version specified.