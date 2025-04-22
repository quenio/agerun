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

### Persistence:

- **Methodology**: All method definitions are persisted in a file named `methodology.agerun`. They are loaded and made available when the agent system restarts.
- **Agency**: Agents and their memory are not persisted by default. However, if an agent is created with the `persist: boolean` option set to `true` (default is `false`), its memory map and context are saved to a file named `agency.agerun`. The persisted agents are automatically restored at system startup before the first agent is executed.

## Agent Lifecycle

- **Creation/Resumption**: Upon creation or resumption, the agent receives the special message `__wake__`.
- **Pausing/Destruction**: Before pausing or destruction, the agent receives the special message `__sleep__`. An agent is only destroyed after it has processed the `__sleep__` message and entered a sleeping state.

### Resource Management:

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

All data types are implemented as opaque structures with proper memory management and type safety. Agent memory and context are implemented as MAPs that can store any of the supported data types, including nested structures.

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
                 | <agent-function>
                 | <destroy-function>
                 | <if-function>

<send-function> ::= 'send' '(' <expression> ',' <expression> ')'
<parse-function> ::= 'parse' '(' <expression> ',' <expression> ')'
<build-function> ::= 'build' '(' <expression> ',' <expression> ')'
<method-function> ::= 'method' '(' <expression> ',' <expression> ',' <expression> ')'
<agent-function> ::= 'agent' '(' <expression> ',' <expression> [',' <expression>] ')'
<destroy-function> ::= 'destroy' '(' <expression> ')'
                     | 'destroy' '(' <expression> ',' <expression> ')'
<if-function> ::= 'if' '(' <comparison-expression> ',' <expression> ',' <expression> ')'
```

Instructions in an agent method can be of two types:
- An assignment, which stores the result of an expression in the agent's memory using the `:=` operator
- A function call instruction, which must be one of the supported system functions:
  - `send` - Send a message to another agent
  - `parse` - Extract values from a string using a template
  - `build` - Construct a string using a template and values
  - `method` - Define a new agent method
  - `agent` - Create a new agent instance
  - `destroy` - Destroy an existing agent or method
  - `if` - Evaluates a condition and returns one of two values based on the result

Function call instructions can optionally assign their result to a variable. For example:
- `send(agent_id, message)` - Call the function without storing the result
- `success := send(agent_id, message)` - Store the result in a memory variable
- `destroy(agent_id)` - Destroy an agent without storing the result
- `success := destroy(agent_id)` - Destroy an agent and store the result
- `destroy(method_name, method_version)` - Destroy a method without storing the result
- `success := destroy(method_name, method_version)` - Destroy a method and store the result
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

<memory-access> ::= <identifier>
                 | 'message'
                 | 'memory' '[' <expression> ']'

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
- The special identifier `message` refers to the current message being processed
- Memory access is done through `memory[key]` where key is any expression that evaluates to a string
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

- **Reading**: Access a value using `var`.
- **Writing**: Assign a value using `var := value`.

### 4. Arithmetic Operations

- **Operators**: Standard arithmetic operators are used: `+`, `-`, `*`, `/`.
- **Type Inference**: When parsing strings, values are inferred in the following order: integer, double, string.

### 5. Conditional Evaluation

- `if(condition: boolean, true_value, false_value)`: Returns `true_value` if condition is true; otherwise, returns `false_value`.

### 6. Agent Management

- `method(method_name: string, version: string, instructions: string) → boolean`: Defines a new method with the specified name, version string, and instruction code. The version string must follow semantic versioning (e.g., "1.0.0"). Compatibility between versions is determined based on semantic versioning rules: agents using version 1.x.x will automatically use the latest 1.x.x version. Returns true if the method was successfully defined, or false if the instructions cannot be parsed or compiled.
- `agent(method_name: string, version: string, context: map = null) → agent_id`: Creates a new agent instance based on the specified method name and version string. The version parameter is required. If a partial version is specified (e.g., "1"), the latest matching version (e.g., latest "1.x.x") will be used. An optional context may be provided. Returns a unique agent ID.
- `destroy(agent_id: integer) → boolean`: Attempts to destroy the specified agent. Before destruction, the agent receives the `__sleep__` message. The agent is only destroyed after it is in a sleeping state. Returns true if successful, or false if the agent does not exist or is already destroyed.
- `destroy(method_name: string, method_version: string) → boolean`: Attempts to destroy the specified method version. If any agents are using this method, they will all be sent the `__sleep__` message first, then destroyed once they are sleeping, before the method is removed. Returns true if successful, or false if the method does not exist.

## Message Handling

### Message Types:

Messages can be any of the supported data types:
- **STRING**: Text messages for simple communication.
- **INTEGER** or **DOUBLE**: Numeric values for direct data exchange.
- **LIST**: Collections of values for batch processing.
- **MAP**: Structured data with named fields.

### Special Messages:

- `__wake__`: Special STRING message indicating the agent has been created or resumed.
- `__sleep__`: Special STRING message indicating the agent is about to be paused or destroyed.

**Processing**: All messages, regardless of their data type and including special ones, are handled by the agent's single method.

## Agent Creation

- **Dynamic Instantiation**: Agents can be dynamically created at runtime by existing agents.
- **Context Sharing**: Parent agents can provide their memory map as a read-only context to child agents.

## Scalability

- **Horizontal and Vertical Scaling**: The agent system supports both horizontal and vertical scaling.
- **Agent Awareness**: Agents are designed to be unaware of the underlying scaling mechanisms, ensuring seamless scalability.

## System Startup

The system is started by providing a method name and version string, which is used to create the first agent—similar to the `agent` instruction. Immediately after creation, the system sends the special message `__wake__` to this initial agent.
