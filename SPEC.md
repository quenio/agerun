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

- **Methodology**: All method definitions are persisted in a file named `agerun.methodology`. They
  are loaded and made available when the agent system restarts. On fresh executable startup without
  persisted methodology state, AgeRun loads `.method` files from the existing `methods/` directory
  and then loads `.method` files from each immediate subdirectory under `methodologies/`, with each
  subdirectory treated as a methodology instance source.
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

### Message Processing Order

The runtime processes one dequeued agent message by executing that agent's method once, to
completion, before processing another message. A single agent is therefore not re-entrant in the
current runtime, and two invocations of the same agent method cannot mutate that agent's memory
simultaneously.

Messages sent with `send(...)` are enqueued; they do not execute the recipient method inline. When
an agent sends a message to itself, that self-sent message is appended to the agent's FIFO queue
behind any messages already queued for that agent. Therefore, self-sent continuation messages can be
interleaved with older queued messages, but this interleaving occurs across completed method
invocations rather than through concurrent or re-entrant execution.

See [Agent FIFO Queue and Continuation Interleaving](kb/agent-fifo-queue-continuation-interleaving.md)
for the review-triage implications of this ordering model.

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

### Language Design Principles

The following principles guide language design. They are aspirations for language evolution, not a
claim that every current construct fully satisfies them.

- **Line-Based Parsing and Evaluation**: Each nonempty source line should parse to one complete
  instruction, and instructions should evaluate in source order. Line boundaries should remain
  visible parser and evaluation boundaries.
- **Expression Purity**: Expressions should have no side effects. Evaluating an expression should
  not mutate memory, send messages, create agents, deprecate methods, exit agents, complete work, or
  change observable runtime state.
- **Single Source of Semantics**: Each language behavior should have one clear owner in the parser,
  AST, evaluator, and documentation so behavior cannot drift across duplicate paths.
- **Syntax-Directed Semantics**: Each syntax construct should map to one evaluation behavior, and
  intentional semantic differences should be visible in the syntax or documented as explicit
  exceptions.
- **Explicit Exceptions**: Any behavior that is intentionally non-composable, non-orthogonal, or
  context-specific should be documented as a named language rule and covered by tests.
- **Composability**: Every pure value-producing construct should be usable in every compatible
  expression context unless the language defines a specific exception.
- **Orthogonality**: Equivalent values should behave the same regardless of whether they came from a
  literal, accessor, selected branch, function result, or intermediate assignment.

### Current Language State

The current language partially satisfies those principles:

- **Line-Based Parsing and Evaluation**: Methods are line-oriented: one instruction per line,
  ordered evaluation, required final newline, and ignored empty lines. Multi-line list and map
  literals are an explicit assignment-only, strictly line-bound source-format exception.
- **Expression Purity**: Current expression nodes are value-producing and side-effect free. Registered
  pure built-in calls are expression calls; effectful operations remain sequenced as method lines.
- **Single Source of Semantics**: The shared function-call argument parser centralizes argument
  boundary rules. Ordinary assignment and function-result assignment still use separate AST/evaluator
  paths, but assigned effectful result binding now shares one storage owner. Some
  instruction-specific exceptions remain documented outside one consolidated semantics section.
- **Syntax-Directed Semantics**: Memory assignment, function-result assignment, standalone
  compatibility mutation such as `append(memory.items, value)`, and the language positions that use
  integer `0` as a sentinel still have documented special cases.
- **Explicit Exceptions**: Current exceptions include assignment-only multi-line literals,
  memory-only mutation targets, protected `memory.self` paths, centralized integer `0` sentinel
  behavior, and raw string values with boundary-only quote/backslash parsing.
- **Composability**: Literals, accessors, operators, registered pure calls such as `parse(...)`,
  `build(...)`, `if(...)`, `append(...)`, `head(...)`, and `tail(...)`, and one-line list/map
  literals compose as expressions. Effectful built-in calls are not expressions and remain
  sequenced instructions.
  Multi-line literals are assignment-only and use linefeeds, not commas, as item/entry separators.
- **Orthogonality**: Current documented exceptions include memory-only mutation targets, integer `0`
  as the documented sentinel in specific language positions, and boundary-level quote handling that
  does not define value-level string escape sequences.

### Integer `0` Sentinel Semantics

AgeRun does not have a null or absence data type. Integer `0` remains ordinary integer data in normal
expression evaluation, comparison, storage, list/map values, and message payloads. It is interpreted
as the documented fallback, absence, or no-op sentinel only in the language positions listed here:

- Missing path references under the frame roots `message`, `memory`, and `context` evaluate to
  integer `0` (for example, `message.field`, `memory.field`, and `context.field`). Inside a
  multi-line map assignment block, missing block-local `.key` and `.key.nested` accesses also
  evaluate to integer `0`. Once produced, that `0` composes as ordinary integer data.
- Condition truthiness treats integer `0` as false and nonzero integers as true. Non-integer and
  missing condition values are false. This rule is condition truthiness, not a general conversion of
  data values into absence. It exists so methods can use integer status and flag values directly in
  conditions when no relational operator is needed.
- Expression-level `if(condition, true_value, false_value)` returns integer `0` when the selected
  branch cannot produce a value. If the selected branch explicitly evaluates to integer `0`, that
  result is still ordinary integer data selected by the branch.
- Pure `head(list)` returns integer `0` for empty LIST inputs, missing inputs, non-LIST inputs, and
  copy failures. A real first item whose value is integer `0` is indistinguishable from the sentinel,
  so methods that use `head(...) = 0` as a stop condition must exclude integer `0` from the item
  domain or wrap arbitrary items in containers.
- Pure `tail(list)` returns integer `0` for missing inputs, non-LIST inputs, and copy failures. Empty
  LIST inputs and single-item LIST inputs return a new empty LIST, not integer `0`.
- Pure `append(list, value)` returns integer `0` for missing, non-LIST, invalid, or not-copyable
  inputs and copy failures. Appending the ordinary integer value `0` to a valid list is normal data
  construction, not a sentinel; this includes a missing path reference that has already evaluated to
  integer `0`.
- `send(0, message)` is an instruction-level no-op destination. The message is destroyed, the
  instruction succeeds, and no message is delivered. If the result is assigned, the stored status is
  integer `1`. Integer `0` elsewhere in the message payload remains ordinary data.
- `spawn(0, version, context)` and `spawn("", version, context)` are instruction-level no-op method
  selections. The instruction performs no spawn and stores integer `0` as the assigned result when a
  result target is present.

New language features must not add ad hoc meanings for integer `0`. A new feature that needs
fallback, absence, or no-op semantics must either explicitly reuse this central sentinel contract or
document a deliberate exception in this section and in the feature-specific documentation.

## Method Expressions and Instructions

Agents can use the following expressions and instructions within their method:

### Instruction Syntax

The following BNF grammar defines the syntax of individual instructions allowed in AgeRun methods:

```
<instruction> ::= <assignment>
               | <function-instruction>
               
<assignment> ::= <memory-access> ':=' <expression>
               | <memory-access> ':=' <multiline-list-literal>
               | <memory-access> ':=' <multiline-map-literal>

<function-instruction> ::= [<memory-access> ':='] <instruction-function-call>
                         | <pure-function-call>

<instruction-function-call> ::= <send-function>
                              | <build-function>
                              | <complete-function>
                              | <append-function>
                              | <head-function>
                              | <tail-function>
                              | <compile-function>
                              | <spawn-function>
                              | <exit-function>
                              | <deprecate-function>
                              | <if-function>

<pure-function-call> ::= <parse-function>
                       | <build-function>
                       | <if-function>
                       | <append-function>
                       | <head-function>
                       | <tail-function>

<send-function> ::= 'send' '(' <two-function-arguments> ')'
<parse-function> ::= 'parse' '(' <two-function-arguments> ')'
<build-function> ::= 'build' '(' <two-function-arguments> ')'
<complete-function> ::= 'complete' '(' <one-or-two-function-arguments> ')'
<append-function> ::= 'append' '(' <two-function-arguments> ')'
<head-function> ::= 'head' '(' <one-function-argument> ')'
<tail-function> ::= 'tail' '(' <one-function-argument> ')'
<compile-function> ::= 'compile' '(' <three-function-arguments> ')'
<spawn-function> ::= 'spawn' '(' <three-function-arguments> ')'
<exit-function> ::= 'exit' '(' <one-function-argument> ')'
<deprecate-function> ::= 'deprecate' '(' <two-function-arguments> ')'
<if-function> ::= 'if' '(' <condition-function-arguments> ')'

<one-function-argument> ::= <function-argument>
<one-or-two-function-arguments> ::= <one-function-argument>
                                  | <two-function-arguments>
<two-function-arguments> ::= <function-argument> <function-argument-tail>
<three-function-arguments> ::= <function-argument> <function-argument-tail> <function-argument-tail>
<condition-function-arguments> ::= <expression> <function-argument-tail> <function-argument-tail>
<function-argument-tail> ::= <function-argument-separator> <function-argument>
<function-argument> ::= <expression>
<function-argument-separator> ::= ','
```

Instructions in an agent method can be of two types:
- An assignment, which stores the result of an expression in the agent's memory using the `:=` operator
- A function call instruction, which must be one of the supported instruction functions:
  - `send` - Send a message to an agent or delegate
  - `build` - Construct a string using a template and values
  - `complete` - Complete template placeholders with local LLM-generated values and return a map
  - `append` - Construct a new list in expression form, or mutate an existing memory list when used
    as a standalone compatibility instruction
  - `head` - Return the first item from a list
  - `tail` - Return a new list containing all but the first item
  - `compile` - Define a new agent method
  - `spawn` - Spawn a new agent instance
  - `exit` - Exit an existing agent
  - `deprecate` - Deprecate an existing method
  - `if` - Evaluates a condition and returns one of two values based on the result

Pure function calls are expressions. Registered pure calls such as `parse(...)`, `build(...)`,
`if(...)`, `append(...)`, `head(...)`, and `tail(...)` can appear anywhere an expression is accepted,
including assignment right-hand sides, function-call arguments, list items, map values, nested pure
calls, and selected `if(...)` branch values. Expression-level `if(...)` is lazy and evaluates only
the selected branch. Standalone pure-call names remain accepted for compatibility; `parse(...)`
discards its returned map, `build(...)` uses the existing top-level build instruction behavior,
standalone `if(...)` uses the existing condition instruction behavior, standalone `append(...)`
mutates an existing memory-owned list, and standalone `head(...)` / `tail(...)` calls compute and
discard their values.

Function call instructions can optionally assign their result to a variable. For example:
- `send(agent_id, message)` - Call the function without storing the result
- `memory.status := send(agent_id, message)` - Store integer `1`/`0` delivery status in memory
- `memory.result := complete("The capital of {country} is {city}.", memory.values)` - Return a new map containing values from `memory.values` plus generated values for missing placeholders
- `append(memory.results, message.value)` - Standalone compatibility append: mutate the existing
  `memory.results` list
- `memory.results := append(memory.results, message.value)` - Pure append expression: store a new
  list containing copied source items plus the copied message value
- `exit(agent_id)` - Exit an agent without storing the result
- `success := exit(agent_id)` - Exit an agent and store the result
- `deprecate(method_name, method_version)` - Deprecate a method without storing the result
- `success := deprecate(method_name, method_version)` - Deprecate a method and store the result
- `if(condition, true_value, false_value)` - Evaluate without storing the result
- `memory.result := if(condition, true_value, false_value)` - Store the pure expression result in memory
- `memory.parsed := parse("name={name}", message.text)` - Store a pure parse expression result
- `memory.text := build("Hello {name}", {name: "Ada"})` - Store a pure build expression result
- `memory.next := head(memory.targets)` - Store a pure head expression result
- `memory.remaining := tail(memory.targets)` - Store a pure tail expression result

All function-call argument lists use the same language rule for argument boundaries:
- Arguments are separated only by top-level commas and the closing parenthesis for the call.
- Commas and closing parentheses inside quoted strings, parenthesized expression groups, one-line
  list literals, or one-line map literals do not terminate an argument.
- While splitting function-call arguments, a double quote toggles quoted-string state only when it
  is preceded by zero or an even number of consecutive backslashes. A quote preceded by an odd
  number of consecutive backslashes is treated as part of the quoted span for boundary detection.
- Function-specific arity rules still apply after this shared splitting rule. For example,
  `send(...)` requires exactly two arguments, while `complete(...)` accepts one or two arguments.
- Function-call argument lists do not allow trailing commas before the closing parenthesis.
- Registered pure function calls can be nested anywhere expressions are accepted. Effectful
  instruction calls are not valid expressions.

Standalone expressions that are not part of an assignment or one of the allowed function calls are not permitted as instructions. Registered pure calls are expressions; effectful instruction calls cannot be nested inside assignment expressions or other function-call arguments.

### Expression Syntax

The following BNF grammar defines the syntax of expressions allowed in AgeRun instructions:

```
<expression> ::= <string-literal>
              | <number-literal>
              | <list-literal>
              | <map-literal>
              | <memory-access>
              | <pure-function-call>
              | <parenthesized-expression>
              | <arithmetic-expression>
              | <comparison-expression>

<string-literal> ::= '"' <characters> '"'

<number-literal> ::= <integer>
                  | <double>

<list-literal> ::= '[' [<expression> {',' <expression>}] ']'

<map-literal> ::= '{' [<identifier> ':' <expression> {',' <identifier> ':' <expression>}] '}'

<parenthesized-expression> ::= '(' <expression> ')'

<multiline-list-literal> ::= '[' <newline> {<indent> <expression> <newline>} <assignment-indent> ']'

<multiline-map-literal> ::= '{' <newline> {<indent> <identifier> ':=' <expression> <newline>} <assignment-indent> '}'

<integer> ::= ['-'] <digit> {<digit>}
<double>  ::= <integer> '.' <digit> {<digit>}

<memory-access> ::= 'message' {'.' <identifier>}
                 | 'memory' {'.' <identifier>}
                 | 'context' {'.' <identifier>}
                 | <block-local-access>

<block-local-access> ::= '.' <identifier> {'.' <identifier>}

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
- String literal values preserve their source characters between delimiters; backslashes are not
  decoded into escape sequences
- The current expression grammar does not specify an escaped double quote as a string value
  character; quote/backslash handling above is a function-call argument boundary rule
- Number literals can be either integers (whole numbers) or doubles (floating-point numbers)
- List literals create list values, e.g. `[1, 2]`
- Map literals create map values with identifier-only keys, e.g. `{name: "Ada"}`
- One-line list and map literals can appear anywhere expressions are accepted, including assignment values and function arguments
- One-line list and map literals do not allow trailing commas before the closing delimiter
- Multi-line list and map literals are accepted only as the top-level right side of an assignment
- Multi-line list item lines contain one expression; multi-line map entry lines use `identifier := expression`
- Multi-line literal item/entry lines must use identical indentation, the closing delimiter must align with the assignment line, and linefeeds are the only item/entry separators
- Multi-line literal item/entry lines do not allow trailing commas
- In a multi-line map assignment, an entry right-hand expression may use `.key` or `.key.nested`
  to read a key assigned on an earlier line in the same map block
- Block-local `.key` access is rejected in ordinary one-line expressions and one-line map literals
- In those same entry right-hand expressions, `memory.`, `message.`, and `context.` retain their
  ordinary frame-root meaning
- Multi-line literals cannot appear as function arguments, list elements, or map values; nested list and map values must be written as one-line literals
- `message` refers to the current message being processed, and nested fields can be accessed using dot notation (e.g., `message.field`)
- `memory` provides access to the agent's memory map, and nested fields can be accessed using dot notation (e.g., `memory.field`)
- `context` provides access to the agent's read-only context map, and nested fields can be accessed using dot notation (e.g., `context.field`)
- In assignments, only `memory` paths can be used on the left side of the ':=' operator
- Arithmetic operations can be performed with basic operators: +, -, *, /
- Comparison operations use relational operators to compare values:
  - `=` equality (produces integer `1` if the values are equal, otherwise integer `0`)
  - `<>` inequality (produces integer `1` if the values are not equal, otherwise integer `0`)
  - `<` less than (produces integer `1` if the left value is less than the right value, otherwise integer `0`)
  - `<=` less than or equal to (produces integer `1` if the left value is less than or equal to the right value, otherwise integer `0`)
  - `>` greater than (produces integer `1` if the left value is greater than the right value, otherwise integer `0`)
  - `>=` greater than or equal to (produces integer `1` if the left value is greater than or equal to the right value, otherwise integer `0`)
- Type conversion is automatic where possible; integers are promoted to doubles, numeric types can be converted to strings

### 1. Parsing, Building, and Completing Strings

- `parse(template: data, input: data) → map`: Pure expression call that extracts values from input based on the template and returns a new map. STRING, INTEGER, and DOUBLE arguments are interpreted as strings using ordinary conversion rules. Missing values, LIST or MAP arguments, malformed templates, non-matching input, and values that cannot be interpreted as strings return an empty map. Placeholder names are ordinary result keys, so `self` and nested `self.*` fields may appear in the returned map when it is stored outside protected paths.
- `build(template: data, values: data) → string`: Pure expression call that constructs a string by replacing placeholders in the template with corresponding values from a values map. STRING, INTEGER, and DOUBLE templates are interpreted as strings. Missing or non-primitive templates use an empty string fallback. When values is not a MAP, placeholders remain unchanged. Missing placeholders and values that cannot be interpreted as STRING, INTEGER, or DOUBLE also remain unchanged. The operation never mutates the values map. Top-level build instructions preserve the existing compatibility contract: the template argument must evaluate to STRING and the values argument must evaluate to MAP before the shared build operation is applied.
- `complete(template: string[, values: map]) → map`: Uses a local CPU-only completion backend to complete placeholder variables as strings and returns a new map. When a values map is provided, placeholders with corresponding primitive values are first replaced in the template using build-style substitution, the original values are deep-copied into the result map, and only placeholders still missing from the values map are sent to the local completion backend. When no values map is provided, all placeholders are completed by the backend. The input map is never mutated.

### 2. List Operations

- `append(list: expression, value: expression) → list | integer`: Pure expression call that
  evaluates both arguments without mutating either source value. When the first argument is a LIST,
  `append(...)` returns a new LIST containing deep copies of every source item followed by a deep
  copy of the value. Empty LIST input returns a new one-item LIST. Missing, non-LIST, invalid, or
  not-copyable inputs return integer `0` per [Integer `0` Sentinel Semantics](#integer-0-sentinel-semantics).
  Argument handling is path-neutral: `memory.self`, `self`, and nested paths are ordinary values,
  while protected identity behavior is enforced only by assignment and result-storage rules.
- Standalone compatibility `append(target, value)` instructions keep the existing mutating behavior:
  they mutate only when the target expression resolves to an existing LIST directly or indirectly
  owned by the frame memory map, such as `memory.results` or `memory.wrapper.results`. Targets under
  `message` or `context`, fresh literal/list expression results, missing targets, non-LIST targets,
  and protected `memory.self` targets are no-ops. Assigned and nested `append(...)` calls use the
  pure expression behavior above, not instruction result storage.
- For standalone compatibility mutation, fresh literal values are transferred directly into the
  target list. Borrowed values from `memory`, `message`, or `context` are claimed when possible or
  deep-copied before append, preserving nested list/map structure. Target no-ops and append failures
  complete without mutating memory further.

- `head(list: expression) → data | integer`: Pure expression call that evaluates the list expression
  and never mutates the source list. When the value is a LIST with at least one item, `head(...)`
  returns a deep copy of the first item. Empty lists, missing values, non-LIST values, and values that
  cannot be safely copied return integer `0` per
  [Integer `0` Sentinel Semantics](#integer-0-sentinel-semantics). Standalone compatibility
  `head(...)` instructions discard the computed value when no result assignment is present.
- `tail(list: expression) → list | integer`: Pure expression call that evaluates the list expression
  and never mutates the source list. When the value is a LIST, `tail(...)` returns a new LIST
  containing deep copies of every item after the first. Empty lists and single-item lists return a new
  empty LIST. Missing values, non-LIST values, and values that cannot be safely copied return integer
  `0` per [Integer `0` Sentinel Semantics](#integer-0-sentinel-semantics), allowing callers to
  distinguish invalid input from the valid tail of a single-item list. Standalone compatibility
  `tail(...)` instructions discard the computed value when no result assignment is present.
- `head(...)` and `tail(...)` can be nested and used anywhere expressions are accepted. They preserve
  nested list/map structure in returned values. Copy failure returns integer `0`: for `head(...)`,
  when the first item cannot be copied; for `tail(...)`, when any retained item cannot be copied.
- Together, `head(...)` and `tail(...)` let a method consume an unbounded list through self-messages without loops or indexed access. Because `head(...)` uses integer `0` as the empty/invalid sentinel, callers that use `0` as a stop condition must choose an item domain where integer `0` is not a valid item, or wrap arbitrary values in maps/lists so a valid item cannot be confused with the sentinel. For example, a method can process a list of nonzero target IDs one message at a time:

```
memory.next_target := head(message.targets)
memory.remaining_targets := tail(message.targets)
memory.target := if(memory.next_target = 0, 0, memory.next_target)
memory.next_self := if(memory.next_target = 0, 0, memory.self)
send(memory.target, message.payload)
send(memory.next_self, {targets: memory.remaining_targets, payload: message.payload})
```

### 3. Messaging

- `send(recipient_id: integer, message: data) → integer status`:
  - **Routing by ID**:
    - `recipient_id == 0`: No operation (no-op); succeeds with integer status `1` per
      [Integer `0` Sentinel Semantics](#integer-0-sentinel-semantics)
    - `recipient_id > 0`: Routes to agent's message queue
    - `recipient_id < 0`: Routes to delegate's message queue
  - **Asynchronous Delivery**: All messages are enqueued for non-blocking operation
  - **Return Value**:
    - Produces integer `1` if the recipient exists and message is enqueued, or if the recipient is
      the no-op destination `0`
    - Produces integer `0` if the recipient does not exist
  - The `message` parameter can be any supported data type (INTEGER, DOUBLE, STRING, LIST, or MAP)
  - **Ownership**: Message ownership transfers to the recipient's queue

### 4. Memory Access

- **Reading**: Access frame values using dot notation with the root identifiers `message`,
  `memory`, or `context` (e.g., `message.field`, `memory.user.name`, `context.settings`). Inside a
  multi-line map assignment block, `.key` reads keys already assigned earlier in that same block.
- Missing path references under `message`, `memory`, `context`, and block-local `.` inside
  multi-line map assignment blocks evaluate to integer `0` per
  [Integer `0` Sentinel Semantics](#integer-0-sentinel-semantics). Ordinary stored integer `0`
  values remain ordinary data.
- **Writing**: Assign values to memory using `memory.path := value`. Only `memory` paths can be used on the left side of an assignment.

### 5. Arithmetic Operations

- **Operators**: Standard arithmetic operators are used: `+`, `-`, `*`, `/`.
- **Type Inference**: When parsing strings, values are inferred in the following order: integer, double, string.

### 6. Conditional Evaluation

- `if(condition: expression, true_value: expression, false_value: expression) → data | integer`: Pure
  expression call that evaluates the condition first, treats integer `0` as false, a non-zero integer
  as true, and non-integer or missing condition values as false, then evaluates and returns only the
  selected branch expression. The unselected branch is never evaluated. If the selected branch cannot
  produce a value, expression-level `if(...)` returns integer `0` per
  [Integer `0` Sentinel Semantics](#integer-0-sentinel-semantics).

### 7. Agent Management

- `compile(method_name: string, instructions: string, version: string) → integer status`: Defines a new method with the specified name, instruction code, and version string. The version string must follow semantic versioning (e.g., "1.0.0"). Compatibility between versions is determined based on semantic versioning rules: agents using version 1.x.x will automatically use the latest 1.x.x version. Produces integer `1` if the method was successfully defined, or integer `0` if the instructions cannot be parsed or compiled.
- `complete(...)` is local-only in the first release, uses CPU-only execution, returns generated values as strings in a new map, rejects empty/outer-whitespace/braced generated values, applies build-style substitution for provided values, and never mutates the provided values map.
- `spawn(method_name: string | integer, version: string, context: map) → agent_id`: Spawns a new agent
  instance based on the specified method name and version string. The version parameter is required.
  If a partial version is specified (e.g., "1"), the latest matching version (e.g., latest "1.x.x")
  will be used. A context map must be provided as the third argument. Returns a unique agent ID.
  Special no-op cases: if method_name is integer `0` or an empty string, the instruction performs no
  operation and sets the result to integer `0` if assigned to a variable, per
  [Integer `0` Sentinel Semantics](#integer-0-sentinel-semantics).
- `exit(agent_id: integer) → integer status`: Attempts to exit the specified agent. The agent is immediately destroyed. Produces integer `1` if successful, or integer `0` if the agent does not exist or is already destroyed.
- `deprecate(method_name: string, method_version: string) → integer status`: Attempts to deprecate the specified method version by unregistering it from the methodology. This allows deprecating methods even when agents are actively using them. Produces integer `1` if successful, or integer `0` if the method does not exist.

## Message Handling

### Message Types:

Messages can be any of the supported data types:
- **STRING**: Text messages for simple communication.
- **INTEGER** or **DOUBLE**: Numeric values for direct data exchange.
- **LIST**: Collections of values for batch processing.
- **MAP**: Structured data with named fields.

**Processing**: All messages, regardless of their data type, are handled by the agent's single method. At agent creation time, the agency sets `memory.self` to the agent ID; method instructions cannot assign or store any instruction result into `memory.self` or `memory.self.*`. Protected identity behavior is enforced by storage rules, not by parse argument names or paths.

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
