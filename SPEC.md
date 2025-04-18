# AgeRun: Agent Runtime Specification

## Overview

This specification defines a lightweight, message-driven agent system where each agent is characterized by a single method responsible for handling all incoming messages. Agents operate within a persistent runtime environment, allowing dynamic creation, pausing, resumption, and destruction.

## Agent Definition

- **Method**: Each agent is defined by a single method that processes all messages.
- **Type Identification**: An agent's type is uniquely identified by the combination of its method's name and version.

### Versioning:

- **Immutable Versions**: Once a method version is created, it is immutable.
- **Backward Compatibility**: New method versions can specify backward compatibility. If set:
  - Existing agents automatically switch to the newest compatible version.
  - New agents referencing older versions are instantiated with the latest compatible version.

### Persistence:

- **Methodology**: All method definitions are persisted in a file named `methodology.agerun`. They are loaded and made available when the agent system restarts.
- **Agency**: Agents and their memory are not persisted by default. However, if an agent is created with the `persist: boolean` option set to `true` (default is `false`), its memory map and context are saved to a file named `agency.agerun`. The persisted agents are automatically restored at system startup before the first agent is executed.

## Agent Lifecycle

- **Creation/Resumption**: Upon creation or resumption, the agent receives the special message `__wake__`.
- **Pausing/Destruction**: Before pausing or destruction, the agent receives the special message `__sleep__`.

### Resource Management:

- **Paused Agents**: Retain their message queue and memory map.
- **Destroyed Agents**: All associated resources, including message queue and memory, are deleted.
- **Lifecycle Control**: The environment determines whether an agent is paused or destroyed.

## Agent Structure

- **Message Queue**: Each agent has an implicit FIFO (First-In-First-Out) message queue for asynchronous communication.
- **Memory Map**: Agents maintain an implicit key-value map (memory) for storing state.
- **Context Map**: Agents can be provided with a read-only context map (context) upon creation.

## Method Expressions and Instructions

Agents can use the following expressions and instructions within their method:

### 1. Parsing and Building Strings

- `parse(template: string, input: string) → dict`: Extracts values from input based on the template. Always returns a dictionary; if parsing fails, returns an empty dictionary.
- `build(template: string, values: dict) → string`: Constructs a string by replacing placeholders in template with corresponding values from values. Always returns a string; placeholders without corresponding values remain unchanged.

### 2. Messaging

- `send(agent_id: integer, message: string) → boolean`:
  - If `agent_id == 0`: No operation is performed; returns `true`.
  - If `agent_id != 0` and the target agent's queue exists: Enqueues the message; returns `true`.
  - If `agent_id != 0` and the target agent's queue does not exist: Returns `false`.

### 3. Memory Access

- **Reading**: Access a value using `var`.
- **Writing**: Assign a value using `var := value`.

### 4. Arithmetic Operations

- **Operators**: Standard arithmetic operators are used: `+`, `-`, `*`, `/`.
- **Type Inference**: When parsing strings, values are inferred in the following order: integer, decimal, double, string.

### 5. Conditional Evaluation

- `if(condition: boolean, true_value, false_value)`: Returns `true_value` if condition is true; otherwise, returns `false_value`.

### 6. Agent Management

- `method(name: string, instructions: string, previous_version: integer = 0, backward_compatible: boolean = true, persist: boolean = false) → version`: Defines a new method with the specified name and instruction code. If `previous_version` is given, it builds on the prior version. If `backward_compatible` is true, existing agents will upgrade automatically. Returns the new version number. If the instructions cannot be parsed or compiled, returns 0.
- `create(method_name: string, version: integer = null, context: dict = null) → agent_id`: Creates a new agent instance based on the specified method name and (optionally) version. If no version is specified, the latest available version will be used. An optional context may be provided. Returns a unique agent ID.
- `destroy(agent_id: integer) → boolean`: Attempts to destroy the specified agent. Returns true if successful, or false if the agent does not exist or is already destroyed.

## Message Handling

### Special Messages:

- `__wake__`: Indicates the agent has been created or resumed.
- `__sleep__`: Indicates the agent is about to be paused or destroyed.

**Processing**: All messages, including special ones, are handled by the agent's single method.

## Agent Creation

- **Dynamic Instantiation**: Agents can be dynamically created at runtime by existing agents.
- **Context Sharing**: Parent agents can provide their memory map as a read-only context to child agents.

## Scalability

- **Horizontal and Vertical Scaling**: The agent system supports both horizontal and vertical scaling.
- **Agent Awareness**: Agents are designed to be unaware of the underlying scaling mechanisms, ensuring seamless scalability.

## System Startup

The system is started by providing a method name and version, which is used to create the first agent—similar to the `create` instruction. Immediately after creation, the system sends the special message `__wake__` to this initial agent.
