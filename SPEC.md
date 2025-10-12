# AgeRun: Agent Runtime Specification

## Overview

This specification defines a lightweight, message-driven agent system where each agent is characterized by a single method responsible for handling all incoming messages. Agents operate within a persistent runtime environment, allowing dynamic creation, pausing, resumption, and destruction.

## Agent Definition

- **Method**: Each agent is defined by a single method that processes all messages.
- **Type Identification**: An agent's type is uniquely identified by the combination of its method's name and version.

### Versioning:

- **Semantic Versioning**: Method versions follow semantic versioning (e.g., "1.0.0"), where:
  - The first number is the major version (breaking changes)
  - The second number is the minor version (backward-compatible new features)
  - The third number is the patch version (backward-compatible bug fixes)
- **Immutable Versions**: Once a method version is created, it is immutable.
- **Backward Compatibility**: Compatibility between versions is determined by semantic versioning rules:
  - Existing agents automatically switch to the newest compatible version (same major version).
  - Agents must specify a version, but can use a partial version (e.g., "1") to get the latest matching version (e.g., latest "1.x.x").
  - When a new compatible method version is registered, agents using an older version will:
    1. Complete processing their current message
    2. Have their method reference updated to the newest compatible version
    3. Continue operation with the new version

### Version Transition Examples

**Case 1: Latest in Major Version**
- Method "Greeter" exists in versions: "1.0.0", "1.1.0", "1.2.0", "2.0.0"
- When an agent requests version "1", it automatically uses "1.2.0" as the latest 1.x.x version

**Case 2: New Compatible Version Added**
- Agent is using method "Calculator" version "1.2.0"
- When version "1.2.5" is added:
  - Agent finishes processing its current message
  - Agent's method reference is updated to version "1.2.5"
  - Agent continues processing with the new version

**Case 3: New Incompatible Version**
- Agent is using method "DataProcessor" version "1.3.2"
- When version "2.0.0" is added, the agent continues using "1.3.2"
- New agents requesting version "2" or "2.0.0" will use the new version
- Existing agents remain on 1.x.x versions for backward compatibility

**Case 4: Partial Version Matching**
- Method "Translator" exists in versions: "1.0.0", "1.1.0", "1.1.1", "1.1.2"
- When an agent requests version "1.1", it automatically uses "1.1.2" as the latest matching version

### Persistence:

- **Methodology**: All method definitions are persisted in a file named `methodology.agerun`. They are loaded and made available when the agent system restarts.
- **Agency**: Agents and their memory are not persisted by default. However, if an agent is created with the `persist: boolean` option set to `true` (default is `false`), its memory map and context are saved to a file named `agerun.agency`. The persisted agents are automatically restored at system startup before the first agent is executed.

## Resource Management

- **Paused Agents**: Retain their message queue and memory map.
- **Destroyed Agents**: All associated resources, including message queue and memory, are deleted.
- **Lifecycle Control**: The environment determines whether an agent is paused or destroyed.

## Data Types

The AgeRun system uses a type-safe data model for storing and manipulating values:

- **INTEGER**: Whole number values used for counters, IDs, etc.
- **DOUBLE**: Floating-point values for decimal numbers.
- **STRING**: Text values used for messages, names, and other textual data.
- **LIST**: Ordered collections of values (which can be of any data type).
- **MAP**: Key-value collections where keys are strings and values can be of any data type.

All data types are implemented as opaque structures with comprehensive memory management and type safety. Agent memory and context are implemented as MAPs that can store any of the supported data types, including nested structures.

### Memory Safety

The AgeRun system implements zero-tolerance memory management with:

- **Comprehensive heap tracking** using `AR_HEAP_MALLOC`, `AR_HEAP_FREE`, and `AR_HEAP_STRDUP` macros
- **Zero memory leaks** across all modules verified through rigorous testing
- **Automatic memory reporting** via `heap_memory_report.log` after program execution
- **Strict ownership semantics** with explicit prefixes (`own_`, `mut_`, `ref_`) throughout the codebase
- **Agent lifecycle memory management** ensuring proper message queue cleanup during agent destruction

## Agent Structure

- **Message Queue**: Each agent has an encapsulated FIFO (First-In-First-Out) message queue for asynchronous communication.
- **Memory Map**: Agents maintain an encapsulated key-value map (memory) for storing state.
- **Context Map**: Agents can be provided with a read-only context map (context) upon creation.

Both the list and map structures are implemented as opaque types to maintain encapsulation and protect internal implementation details.

## Method Definition

A method definition consists of a sequence of instructions, with each instruction on a separate line. The following BNF grammar defines the syntax of a method definition:

```
<method-definition> ::= <instruction> {<newline> <instruction>} <newline>
<newline> ::= '\n'
```

Important rules for method definitions:
- Each instruction must be on its own line
- Instructions cannot be combined on the same line
- Even the last instruction must be followed by a newline
- Empty lines are ignored

## Method Expressions and Instructions

Agents can use the following expressions and instructions within their method:

### Instruction Syntax

The following BNF grammar defines the syntax of individual instructions allowed in AgeRun methods:

```
<instruction> ::= <assignment>
               | <function-instruction>
               
<assignment> ::= <memory-access> ':=' <expression>

<function-instruction> ::= [<memory-access> ':='] <function-call>

<function-call> ::= <send-function>
                 | <parse-function>
                 | <build-function>
                 | <method-function>
                 | <spawn-function>
                 | <exit-function>
                 | <deprecate-function>
                 | <if-function>

<send-function> ::= 'send' '(' <expression> ',' <expression> ')'
<parse-function> ::= 'parse' '(' <expression> ',' <expression> ')'
<build-function> ::= 'build' '(' <expression> ',' <expression> ')'
<method-function> ::= 'method' '(' <expression> ',' <expression> ',' <expression> ')'
<spawn-function> ::= 'spawn' '(' <expression> ',' <expression> ',' <expression> ')'
<exit-function> ::= 'exit' '(' <expression> ')'
<deprecate-function> ::= 'deprecate' '(' <expression> ',' <expression> ')'
<if-function> ::= 'if' '(' <comparison-expression> ',' <expression> ',' <expression> ')'
```

Instructions in an agent method can be of two types:
- An assignment, which stores the result of an expression in the agent's memory using the `:=` operator
- A function call instruction, which must be one of the supported system functions:
  - `send` - Send a message to another agent
  - `parse` - Extract values from a string using a template
  - `build` - Construct a string using a template and values
  - `method` - Define a new agent method
  - `spawn` - Spawn a new agent instance
  - `exit` - Exit an existing agent
  - `deprecate` - Deprecate an existing method
  - `if` - Evaluates a condition and returns one of two values based on the result

Function call instructions can optionally assign their result to a variable. For example:
- `send(agent_id, message)` - Call the function without storing the result
- `success := send(agent_id, message)` - Store the result in a memory variable
- `exit(agent_id)` - Exit an agent without storing the result
- `success := exit(agent_id)` - Exit an agent and store the result
- `deprecate(method_name, method_version)` - Deprecate a method without storing the result
- `success := deprecate(method_name, method_version)` - Deprecate a method and store the result
- `if(condition, true_value, false_value)` - Evaluate without storing the result
- `result := if(condition, true_value, false_value)` - Store the result in a memory variable

Standalone expressions that are not part of an assignment or one of the allowed function calls are not permitted as instructions.

### Expression Syntax

The following BNF grammar defines the syntax of expressions allowed in AgeRun instructions:

```
<expression> ::= <string-literal>
              | <number-literal>
              | <memory-access>
              | <arithmetic-expression>
              | <comparison-expression>

<string-literal> ::= '"' <characters> '"'

<number-literal> ::= <integer>
                  | <double>

<integer> ::= ['-'] <digit> {<digit>}
<double>  ::= <integer> '.' <digit> {<digit>}

<memory-access> ::= 'message' {'.' <identifier>}
                 | 'memory' {'.' <identifier>}
                 | 'context' {'.' <identifier>}

<arithmetic-expression> ::= <expression> <arithmetic-operator> <expression>
<arithmetic-operator> ::= '+' | '-' | '*' | '/'

<comparison-expression> ::= <expression> <comparison-operator> <expression>
<comparison-operator> ::= '=' | '<>' | '<' | '<=' | '>' | '>='

<identifier> ::= <letter> {<letter> | <digit> | '_'}
<characters> ::= {<any-character-except-quote>}
<digit> ::= '0' | '1' | '2' | '3' | '4' | '5' | '6' | '7' | '8' | '9'
<letter> ::= 'a' | 'b' | ... | 'z' | 'A' | 'B' | ... | 'Z'
```

The expression evaluator follows these rules:
- String literals are enclosed in double quotes and represent string values
- Number literals can be either integers (whole numbers) or doubles (floating-point numbers)
- `message` refers to the current message being processed, and nested fields can be accessed using dot notation (e.g., `message.field`)
- `memory` provides access to the agent's memory map, and nested fields can be accessed using dot notation (e.g., `memory.field`)
- `context` provides access to the agent's read-only context map, and nested fields can be accessed using dot notation (e.g., `context.field`)
- In assignments, only `memory` paths can be used on the left side of the ':=' operator
- Arithmetic operations can be performed with basic operators: +, -, *, /
- Comparison operations use relational operators to compare values:
  - `=` equality (returns true if the values are equal)
  - `<>` inequality (returns true if the values are not equal)
  - `<` less than (returns true if the left value is less than the right value)
  - `<=` less than or equal to (returns true if the left value is less than or equal to the right value)
  - `>` greater than (returns true if the left value is greater than the right value)
  - `>=` greater than or equal to (returns true if the left value is greater than or equal to the right value)
- Type conversion is automatic where possible; integers are promoted to doubles, numeric types can be converted to strings

### 1. Parsing and Building Strings

- `parse(template: string, input: string) → map`: Extracts values from input based on the template. Always returns a map; if parsing fails, returns an empty map. The template parameter must be a STRING type.
- `build(template: string, values: map) → string`: Constructs a string by replacing placeholders in template with corresponding values from values. Always returns a string; placeholders without corresponding values remain unchanged. The template parameter must be a STRING type.

### 2. Messaging

- `send(agent_id: integer, message: data) → boolean`:
  - If `agent_id == 0`: No operation is performed; returns `true`.
  - If `agent_id != 0` and the target agent's queue exists: Enqueues the message; returns `true`.
  - If `agent_id != 0` and the target agent's queue does not exist: Returns `false`.
  - The `message` parameter can be any supported data type (INTEGER, DOUBLE, STRING, LIST, or MAP).

### 3. Memory Access

- **Reading**: Access values using dot notation with the root identifiers `message`, `memory`, or `context` (e.g., `message.field`, `memory.user.name`, `context.settings`).
- **Writing**: Assign values to memory using `memory.path := value`. Only `memory` paths can be used on the left side of an assignment.

### 4. Arithmetic Operations

- **Operators**: Standard arithmetic operators are used: `+`, `-`, `*`, `/`.
- **Type Inference**: When parsing strings, values are inferred in the following order: integer, double, string.

### 5. Conditional Evaluation

- `if(condition: boolean, true_value, false_value)`: Returns `true_value` if condition is true; otherwise, returns `false_value`.

### 6. Agent Management

- `compile(method_name: string, instructions: string, version: string) → boolean`: Defines a new method with the specified name, instruction code, and version string. The version string must follow semantic versioning (e.g., "1.0.0"). Compatibility between versions is determined based on semantic versioning rules: agents using version 1.x.x will automatically use the latest 1.x.x version. Returns true if the method was successfully defined, or false if the instructions cannot be parsed or compiled.
- `spawn(method_name: string | integer, version: string, context: map) → agent_id`: Spawns a new agent instance based on the specified method name and version string. The version parameter is required. If a partial version is specified (e.g., "1"), the latest matching version (e.g., latest "1.x.x") will be used. A context map must be provided as the third argument. Returns a unique agent ID. Special no-op cases: if method_name is 0 (integer) or "" (empty string), the instruction performs no operation but returns true and sets the result to 0 if assigned to a variable.
- `exit(agent_id: integer) → boolean`: Attempts to exit the specified agent. The agent is immediately destroyed. Returns true if successful, or false if the agent does not exist or is already destroyed.
- `deprecate(method_name: string, method_version: string) → boolean`: Attempts to deprecate the specified method version by unregistering it from the methodology. This allows deprecating methods even when agents are actively using them. Returns true if successful, or false if the method does not exist.

## Message Handling

### Message Types:

Messages can be any of the supported data types:
- **STRING**: Text messages for simple communication.
- **INTEGER** or **DOUBLE**: Numeric values for direct data exchange.
- **LIST**: Collections of values for batch processing.
- **MAP**: Structured data with named fields.

**Processing**: All messages, regardless of their data type, are handled by the agent's single method.

## Agent Creation

- **Dynamic Instantiation**: Agents can be dynamically spawned at runtime by existing agents.
- **Context Sharing**: Parent agents can provide their memory map as a read-only context to child agents.

## Scalability

- **Horizontal and Vertical Scaling**: The agent system supports both horizontal and vertical scaling.
- **Agent Awareness**: Agents are designed to be unaware of the underlying scaling mechanisms, ensuring seamless scalability.

## External Communication via Delegates

### Delegate Concept

**Definition**: Delegates are specialized system components that act on behalf of agents to interact with external resources. Each delegate type handles a specific communication channel (file, network, logging, etc.) with built-in security controls.

**Key Characteristics**:
- Implemented as C/Zig modules following Parnas principles
- Registered with the system at startup
- Communicate with agents exclusively via messages
- Enforce security policies (validation, limits, timeouts)
- Maintain the agent sandbox

### Delegate Registration
- **System Interface**: Runtime provides `register_delegate(type: string, handler: function)` for registering delegate implementations
- **Built-in Delegates**: Standard delegates (FileDelegate, NetworkDelegate, LogDelegate) are pre-registered
- **Custom Delegates**: Applications can register additional delegate types

### Delegate Communication Protocol
- **Delegate Instances**: Each delegate type has instances identified by reserved agent IDs (e.g., FileDelegate = -100, NetworkDelegate = -101)
- **Message Format**: Agents send structured MAP messages:
  ```
  send(-100, {"action": "read", "path": "/data.txt", "reply_to": agent_id})
  ```
- **Response Format**: Delegates reply with operation results:
  ```
  {"action": "read", "status": "success", "content": "file data"}
  ```

### Delegate Interface (C/Zig Implementation)
```c
typedef struct ar_delegate_s ar_delegate_t;  // EXAMPLE: Planned delegate type for future implementation

ar_delegate_t* ar_delegate__create(const char* type, ar_log_t* log);  // EXAMPLE: Future delegate creation
void ar_delegate__destroy(ar_delegate_t* delegate);  // EXAMPLE: Future delegate cleanup
bool ar_delegate__handle_message(ar_delegate_t* delegate, ar_data_t* message, ar_agent_t* sender);  // EXAMPLE: Future message handling
```

## System Startup

The system is started by providing a method name and version string, which is used to spawn the first agent—similar to the `spawn` instruction.
