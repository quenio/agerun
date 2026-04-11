# Quickstart: Command-Line Shell (`arsh`)

This document describes the intended user flow for the first `arsh` implementation.

## Prerequisites

- AgeRun builds successfully with `make clean build 2>&1`
- The built-in shell method `shell-1.0.0.method` is available to the runtime
- The shell command is exposed as `arsh`

## 1. Start the shell

```bash
arsh
```

### Expected startup behavior

- The `arsh` entrypoint calls into the non-instantiable `ar_shell` module
- The `ar_shell` module creates and holds the shell session instance
- A session-specific shell delegate starts reading from stdin and writing to stdout
- The runtime creates a dedicated receiving agent from the built-in `shell` method
- The shell reports readiness and remains open for repeated input

## 2. Enter one-line shell instructions

The first implementation supports one-line inputs in a restricted AgeRun syntax subset.

### Spawn a runtime agent

```text
memory.echo_id := spawn("echo", "1.0.0", context)
```

Expected outcome:
- The shell delegate wraps the entered string as `{ text = <exact input> }`
- The built-in `shell` method interprets the line
- A runtime agent is spawned
- The resulting agent ID is stored in the shell session memory map owned by `ar_shell` under `echo_id`
- The shell reports handoff acknowledgement in normal mode

### Send a message to a runtime agent and capture the result

```text
memory.send_ok := send(memory.echo_id, "Hello")
```

Expected outcome:
- The shell method resolves `memory.echo_id` through the shell session owned by `ar_shell`
- The runtime queues the message to the target agent
- The shell reports handoff acknowledgement
- In verbose mode, the shell may also report acceptance and action outcome details
- Any later runtime reply is returned in an output envelope that the delegate unwraps for display

### Store a plain session value

```text
memory.prompt := "Ready"
```

Expected outcome:
- The built-in `shell` method interprets the assignment
- The shell session owned by `ar_shell` stores `prompt = "Ready"` in its own memory map
- The value is available to later shell-driven interactions

### Send using a stored session value

```text
send(memory.echo_id, memory.prompt)
```

Expected outcome:
- The shell method resolves both `memory.echo_id` and `memory.prompt` from the shell session owned by `ar_shell`
- The target agent receives the resolved message
- Any reply appears asynchronously in the shell session after delegate unwrapping

## 3. Observe replies

Returned messages are displayed in the same terminal session.

Expected behavior:
- The session-specific shell delegate unwraps returned output envelopes into displayed text
- Sender identity is shown for each displayed reply
- Replies may arrive after later input has already been entered
- Delayed replies do not terminate the shell session

## 4. Exit the shell

When the user exits:
- the shell session begins shutdown
- the dedicated receiving agent is destroyed
- the shell module releases the shell session
- the shell delegate shuts down
- the shell process exits cleanly

## Notes for the first implementation

- The shell delegate is session-specific and only transports input/output plus envelope wrap/unwrap
- Shell semantics live in the built-in `shell` method
- Session values live in the shell session owned by `ar_shell`, not in the receiving agent's memory map
- The shell method and shell session exchange state through messages only
- The `arsh` entrypoint stays thin by delegating shell lifecycle and session ownership to `ar_shell`
