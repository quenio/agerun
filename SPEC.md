# AgeRun: Agent Runtime Specification

## Overview

This specification defines a lightweight, message-driven agent system where each agent is characterized by a single method responsible for handling all incoming messages. Agents operate within a persistent runtime environment, allowing dynamic creation, pausing, resumption, and destruction.

The current runtime also includes a dedicated interactive shell executable, `arsh`, implemented by
`ar_shell`. The shell wraps each input line into a `{text = ...}` envelope, uses the built-in
`shell-1.0.0.method` to interpret the restricted shell subset, supports built-in `agents` / `list agents`
inspection commands for enumerating active agents with method name/version, stores shell-session
state in `ar_shell_session`, renders replies using only the runtime sender ID, and exits on EOF /
Ctrl-D.

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
                 | <complete-function>
                 | <compile-function>
                 | <spawn-function>
                 | <exit-function>
                 | <deprecate-function>
                 | <if-function>

<send-function> ::= 'send' '(' <expression> ',' <expression> ')'
<parse-function> ::= 'parse' '(' <expression> ',' <expression> ')'
<build-function> ::= 'build' '(' <expression> ',' <expression> ')'
<complete-function> ::= 'complete' '(' <expression> [',' <memory-access>] ')'
<compile-function> ::= 'compile' '(' <expression> ',' <expression> ',' <expression> ')'
<spawn-function> ::= 'spawn' '(' <expression> ',' <expression> ',' <expression> ')'
<exit-function> ::= 'exit' '(' <expression> ')'
<deprecate-function> ::= 'deprecate' '(' <expression> ',' <expression> ')'
<if-function> ::= 'if' '(' <comparison-expression> ',' <expression> ',' <expression> ')'
```

Instructions in an agent method can be of two types:
- An assignment, which stores the result of an expression in the agent's memory using the `:=` operator
- A function call instruction, which must be one of the supported system functions:
  - `send` - Send a message to an agent or delegate
  - `parse` - Extract values from a string using a template
  - `build` - Construct a string using a template and values
  - `complete` - Fill one or more `memory...` variables from a local completion template
  - `compile` - Define a new agent method
  - `spawn` - Spawn a new agent instance
  - `exit` - Exit an existing agent
  - `deprecate` - Deprecate an existing method
  - `if` - Evaluates a condition and returns one of two values based on the result

Function call instructions can optionally assign their result to a variable. For example:
- `send(agent_id, message)` - Call the function without storing the result
- `success := send(agent_id, message)` - Store the result in a memory variable
- `memory.ok := complete("The capital is {city}.", memory.location)` - Populate `memory.location.city` and store boolean success in `memory.ok`
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

### 1. Parsing, Building, and Completing Strings

- `parse(template: string, input: string) → map`: Extracts values from input based on the template. Always returns a map; if parsing fails, returns an empty map. The template parameter must be a STRING type.
- `build(template: string, values: map) → string`: Constructs a string by replacing placeholders in template with corresponding values from values. Always returns a string; placeholders without corresponding values remain unchanged. The template parameter must be a STRING type.
- `complete(template: string[, memory_path: memory...]) → boolean`: Uses a local CPU-only completion backend to populate placeholder variables as strings in `memory...` targets. One-argument calls write `{name}` to `memory.name`; two-argument calls write under the supplied direct `memory...` base path. On success, all writes occur atomically and the result is `true`. On failure, the result is `false`, actionable diagnostics are recorded, and no partial target writes occur.

### 2. Messaging

- `send(recipient_id: integer, message: data) → boolean`:
  - **Routing by ID**:
    - `recipient_id == 0`: No operation (no-op); returns `true`
    - `recipient_id > 0`: Routes to agent's message queue
    - `recipient_id < 0`: Routes to delegate's message queue
  - **Asynchronous Delivery**: All messages are enqueued for non-blocking operation
  - **Return Value**:
    - Returns `true` if the recipient exists and message is enqueued
    - Returns `false` if the recipient does not exist
  - The `message` parameter can be any supported data type (INTEGER, DOUBLE, STRING, LIST, or MAP)
  - **Ownership**: Message ownership transfers to the recipient's queue

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
- `complete(...)` is local-only in the first release, uses CPU-only execution, stores generated values as strings, rejects empty/outer-whitespace/braced generated values, and preserves every literal segment in the source template when successful values are substituted back into the template.
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

**Processing**: All messages, regardless of their data type, are handled by the agent's single method. At agent creation time, the agency sets `memory.self` to the agent ID; method instructions cannot assign or store any instruction result into `memory.self` or `memory.self.*`. Parse templates also cannot construct `self` or nested `self.*` result fields, and parse input cannot be `memory.self`.

## Agent Creation

- **Dynamic Instantiation**: Agents can be dynamically spawned at runtime by existing agents.
- **Context Sharing**: Parent agents can provide their memory map as a read-only context to child agents.

## Scalability

- **Horizontal and Vertical Scaling**: The agent system supports both horizontal and vertical scaling.
- **Agent Awareness**: Agents are designed to be unaware of the underlying scaling mechanisms, ensuring seamless scalability.

## External Communication via Delegates

### Delegate Concept

**Definition**: Delegates are specialized system components that provide controlled access to external resources and services. Each delegate type handles a specific communication channel (file, network, logging, etc.) with built-in security controls, enabling agents to interact with the outside world through a secure, message-based interface.

**Key Characteristics**:
- Implemented as C/Zig modules following Parnas principles
- Registered with the system at startup
- **Asynchronous Message Queues**: Delegates have FIFO message queues like agents for non-blocking I/O
- Communicate with agents exclusively via messages
- Enforce security policies (validation, limits, timeouts)
- Maintain the agent sandbox

### Delegate Registration
- **System Interface**: Runtime provides `register_delegate(type: string, handler: function)` for registering delegate implementations
- **Built-in Delegates**: Standard delegates (FileDelegate, NetworkDelegate, LogDelegate) are pre-registered
- **Custom Delegates**: Applications can register additional delegate types

### Delegate Communication Protocol
- **Delegate IDs**: Negative integers identify delegates (e.g., FileDelegate = -100, NetworkDelegate = -101)
- **Message Queuing**: Delegates have FIFO queues enabling asynchronous, non-blocking communication
- **Request Format**: Agents send MAP messages with action and parameters:
  ```
  send(-100, {"action": "read", "path": "/data.txt", "reply_to": agent_id})
  ```
- **Response Format**: Delegates reply by sending messages back to the requesting agent:
  ```
  send(agent_id, {"action": "read", "status": "success", "content": "file data"})
  ```
- **Processing**: The system processes both agent and delegate message queues

### Delegate Interface (C/Zig Implementation)

```c
typedef struct ar_delegate_s ar_delegate_t;

// Lifecycle management
ar_delegate_t* ar_delegate__create(ar_log_t* ref_log, const char* type);
void ar_delegate__destroy(ar_delegate_t* own_delegate);

// Property access
ar_log_t* ar_delegate__get_log(const ar_delegate_t* ref_delegate);
const char* ar_delegate__get_type(const ar_delegate_t* ref_delegate);

// Message handling (called by system when processing delegate's message queue)
bool ar_delegate__handle_message(ar_delegate_t* ref_delegate,
                                  ar_data_t* ref_message,
                                  int64_t sender_id);
```

**Note**: The `handle_message()` function is called by the system when processing a message from the delegate's queue, similar to how agent methods are invoked.

## System Startup

The runtime startup contract has two layers:

- **Embedded/runtime API**: The system can be started by providing a method name and version
  string, which is used to spawn the first agent—similar to the `spawn` instruction.
- **`agerun` executable default path**: On a fresh executable run without overrides, `agerun`
  starts with `bootstrap-1.0.0` and queues the standard `"__boot__"` startup message.
- **`agerun` executable override path**: On a fresh executable run, operators may pass
  `--boot-method <method-name-version>` to request a different boot method such as
  `boot-echo-1.0.0`.
- **Boot override contract**: Fresh-start boot overrides must handle the raw `"__boot__"` startup
  string sent by the executable; ordinary message handlers like `echo-1.0.0` are not valid boot
  methods unless wrapped by a boot-safe method.
- **Executable no-persistence mode**: Operators may pass `--no-persistence` to run `agerun`
  without loading persisted methodology or persisted agents and without saving either file on
  shutdown.
- **No-persistence semantics**: When `--no-persistence` is enabled, `agerun` follows the fresh-run
  startup path, leaves existing persisted files untouched, and still allows fresh-start boot
  overrides such as `--boot-method boot-echo-1.0.0`.
- **Persisted-agent precedence**: If persisted agents are restored from `agerun.agency`, the
  executable skips fresh boot-agent creation and reports any requested boot override as skipped.
- **Invalid override behavior**: Malformed or unavailable boot overrides fail clearly and do not
  silently fall back to `bootstrap-1.0.0`.
