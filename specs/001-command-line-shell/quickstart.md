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

- The `arsh` executable is implemented by the `ar_shell` module
- `ar_shell` is instantiated for the shell process
- The shell acknowledgement mode is selected by a CLI startup flag before the shell loop begins
- The `ar_shell` module creates and manages the shell session instance
- The runtime instantiates an `ar_shell_session` module that owns that session's state and lifecycle
- The selected acknowledgement mode is stored on that shell session for the full session lifetime
- A session-specific shell delegate starts reading from stdin for that shell session
- The runtime creates a dedicated agent from the built-in `shell` method
- The shell session is ready to report acknowledgements and render messages returned by that agent
- The shell reports readiness and remains open for repeated input until EOF / Ctrl-D is received

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
- The resulting agent ID is stored in the shell session memory map owned by `ar_shell_session` under `echo_id`
- The `ar_shell` module continues managing the overall shell session while `ar_shell_session` exposes that state to the shell method
- The shell reports handoff acknowledgement in normal mode

### Send a message to a runtime agent and capture the result

```text
memory.send_ok := send(memory.echo_id, "Hello")
```

Expected outcome:
- The shell method resolves `memory.echo_id` through the `ar_shell_session` module
- The `ar_shell_session` module owns that shell session state while remaining under `ar_shell` management
- The runtime queues the message to the target agent
- The shell reports handoff acknowledgement
- If the shell was started in verbose mode, it may also report acceptance and action outcome details
- Any later message returned by the target agent is routed back through the delegate callback path and rendered by the shell session to standard output

### Store a plain session value

```text
memory.prompt := "Ready"
```

Expected outcome:
- The built-in `shell` method interprets the assignment
- The `ar_shell_session` module stores `prompt = "Ready"` in its own memory map
- The value is available to later shell-driven interactions
- The `ar_shell` module continues managing the session as a whole

### Send using a stored session value

```text
send(memory.echo_id, memory.prompt)
```

Expected outcome:
- The shell method resolves both `memory.echo_id` and `memory.prompt` through `ar_shell_session`
- The shell session values ultimately come from the session state owned by `ar_shell_session`
- The target agent receives the resolved message
- Any returned message appears asynchronously in the shell session after the delegate callback triggers session rendering

## 3. Observe replies

Returned messages are displayed in the same terminal session.

Expected behavior:
- The session-scoped delegate callback routes returned runtime messages into the shell session
- The shell session renders shell-visible output to standard output
- Interactive terminal sessions label transcripts with `IN: ` prompts and `OUT: ` output prefixes
- The current rendered reply form for string payloads is `reply sender_id=<runtime-id> text=<reply>`
- Interactive terminal sessions display that reply line as `OUT: reply sender_id=<runtime-id> text=<reply>`
- The displayed reply shows only the runtime sender ID
- Returned messages may arrive after later input has already been entered
- Delayed returned messages do not terminate the shell session while it remains open

## 4. Exit the shell

The first implementation exits only when EOF / Ctrl-D is received.

When EOF / Ctrl-D is received:
- the shell session begins shutdown immediately
- later returned messages are discarded instead of being rendered
- the dedicated receiving agent is destroyed
- the `ar_shell_session` module is cleaned up
- the `ar_shell` module unregisters and releases the shell session
- the shell delegate shuts down
- the shell process exits cleanly

## Notes for the first implementation

- The shell executable is implemented by `ar_shell`, not by `ar_executable`
- Invalid shell syntax is reported as an error and leaves the session active
- The shell delegate is session-specific and only handles input capture plus callback routing back into the shell session
- Shell semantics live in the built-in `shell` method
- Session values live in `ar_shell_session`, not in the running agent's memory map
- The `ar_shell` module manages shell sessions, while `ar_shell_session` owns per-session state and lifecycle
