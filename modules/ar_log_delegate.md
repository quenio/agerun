# ar_log_delegate Module

## Overview

The `ar_log_delegate` module provides a delegate that accepts structured log messages from agents
and routes them through the existing `ar_log` infrastructure. It formats log entries with
timestamps, severity levels, and agent context while enforcing a minimum log level.

## Purpose

LogDelegate enables consistent structured logging for agent workflows:
- **Structured log messages**: Accepts level, message, and agent_id fields
- **Formatted output**: Adds timestamp and context to log entries
- **Level filtering**: Rejects messages below a configured minimum severity
- **Error reporting**: Uses `ar_log_t` for error logging

## Interface

### Types

```c
typedef struct ar_log_delegate_s ar_log_delegate_t;
```

Opaque type representing a log delegate instance.

### Functions

#### ar_log_delegate__create

```c
ar_log_delegate_t* ar_log_delegate__create(ar_log_t *ref_log, const char *ref_min_level);
```

Creates a new log delegate instance.

**Parameters:**
- `ref_log`: The log instance for error reporting (borrowed reference, may be NULL)
- `ref_min_level`: Minimum log level ("info", "warning", "error"); NULL uses "info"

**Returns:**
- A new log delegate instance, or NULL on failure

**Ownership:**
- Returns an owned value that caller must destroy
- The delegate borrows the log reference - caller must ensure log outlives delegate

#### ar_log_delegate__create_delegate

```c
ar_delegate_t* ar_log_delegate__create_delegate(ar_log_t *ref_log, const char *ref_min_level);
```

Creates a base `ar_delegate_t` already wired to the log delegate handler.

This is the integration entry point used by `ar_system` and `ar_delegation` when they need a
registered delegate instance that can consume queued messages.

#### ar_log_delegate__destroy

```c
void ar_log_delegate__destroy(ar_log_delegate_t *own_delegate);
```

Destroys a log delegate instance and frees resources.

#### ar_log_delegate__get_type

```c
const char* ar_log_delegate__get_type(const ar_log_delegate_t *ref_delegate);
```

Returns the type identifier string (`"log"`).

#### ar_log_delegate__handle_message

```c
ar_data_t* ar_log_delegate__handle_message(
    ar_log_delegate_t *mut_delegate,
    ar_data_t *ref_message,
    int64_t sender_id);
```

Handles a log delegate message and returns a response map.

**Message format:**
- `{"level": "info", "message": "text", "agent_id": 123}`

**Response format:**
- Success: `{"status": "success"}`
- Error: `{"status": "error", "message": "..."}`

**Ownership:**
- Returns an owned response map that the caller must destroy.

## Implementation Status

**Current Status**: Phase 2 (Cycles 18-20) complete with structured logging and level filtering.

## Related Modules

- `ar_delegate`: Base delegate infrastructure for message queuing and handler dispatch
- `ar_delegation`: Delegation subsystem managing delegate instances
- `ar_log`: Error reporting and logging infrastructure

## Test File

Test file: `modules/ar_log_delegate_tests.c`

## Additional Walkthrough

The **LogDelegate** lets agents send **structured log messages** through AgeRun’s normal logging system.

## In plain terms

Instead of an agent writing to stdout or touching files directly, it can send a message to the log delegate, and the delegate:

- validates the message
- filters by severity
- formats it
- writes it via `ar_log`

So it is the agent-safe logging gateway.

## What it accepts

Its documented input format is a MAP like:

```json
{
  "level": "info",
  "message": "text",
  "agent_id": 123
}
```

Fields:
- `level`: log severity like `info`, `warning`, or `error`
- `message`: the actual log text
- `agent_id`: context about which agent emitted it

## What it returns

It returns a response map:

### success
```json
{ "status": "success" }
```

### error
```json
{ "status": "error", "message": "..." }
```

## Main behavior

From `modules/ar_log_delegate.md`, it does four important things:

### 1. Accepts structured log messages
Agents don’t send raw strings blindly; they send a message with fields.

### 2. Formats output
It adds:
- timestamp
- severity level
- agent context

So logs become more useful for debugging and tracing.

### 3. Filters by minimum log level
When the delegate is created, it gets a minimum severity, for example:
- `"info"`
- `"warning"`
- `"error"`

If an incoming message is below that threshold, it gets rejected/ignored according to the delegate’s logic.

Example:
- delegate min level = `"warning"`
- incoming `"info"` log → filtered out
- incoming `"error"` log → accepted

### 4. Uses existing `ar_log`
It doesn’t invent a separate logging backend; it routes everything through the project’s existing `ar_log` infrastructure.

## Why it exists

It preserves the AgeRun model:

- agents communicate by messages
- external side effects go through delegates
- the system can control and audit outside interactions

Logging is treated like any other external capability.

## Creation API

Documented constructors:

```c
ar_log_delegate_t* ar_log_delegate__create(ar_log_t *ref_log, const char *ref_min_level);
ar_delegate_t* ar_log_delegate__create_delegate(ar_log_t *ref_log, const char *ref_min_level);
```

Meaning:
- `ref_log`: existing log instance to write through
- `ref_min_level`: minimum allowed severity

Use `ar_log_delegate__create()` when interacting with the log delegate module directly.
Use `ar_log_delegate__create_delegate()` when registering the log delegate in delegation/system.
If `ref_min_level` is `NULL`, docs say it defaults to `"info"`.

## Type string

It identifies itself as:

```c
"log"
```

via `ar_log_delegate__get_type()`.

## Current implementation status

Per the docs:
- **Phase 2 complete**
- structured logging works
- minimum-level filtering works

Test file:
- `modules/ar_log_delegate_tests.c`

## Practical example

An agent could conceptually send something like:

```json
{
  "level": "warning",
  "message": "Session escalated to human",
  "agent_id": 42
}
```

to the log delegate, and the delegate would turn that into a normal AgeRun log entry with formatting and severity enforcement.

## Best use cases

The log delegate is useful for:

- audit trails
- agent diagnostics
- workflow tracing
- escalation reporting
- structured operational logs
