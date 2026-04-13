# Contract: `arsh` CLI

## Purpose

Define the user-facing command contract for the AgeRun shell.

## Command Name

- The shell command name is `arsh`

## Implementation Contract

- the `arsh` executable is implemented by the `ar_shell` module
- `arsh` is not implemented in `ar_executable`

## Startup Contract

Invoking `arsh`:
- instantiates an `ar_shell` module
- selects the shell session acknowledgement mode via a CLI startup flag before the shell loop begins
- creates and registers a shell session managed by `ar_shell`
- instantiates an `ar_shell_session` module that owns that session's state and lifecycle
- stores the selected acknowledgement mode on that shell session
- starts a session-specific shell delegate over stdio
- creates a dedicated receiving agent from the built-in `shell` method
- enters an interactive session that remains open until EOF / Ctrl-D is received

## Input Contract

### Accepted transport input

- stdin is read as raw text lines
- for each accepted line, the session-specific shell delegate creates an envelope map
- initial input envelope shape:

```text
{text = <input string>}
```

### Interpreted shell subset

The `shell` method interprets one line at a time using a restricted subset of AgeRun instruction
syntax:
- `spawn(...)`
- `send(...)`
- `memory... := ...`
- `memory... := spawn(...)`
- `memory... := send(...)`

## Output Contract

### Normal mode

The shell reports whether the delegate successfully handed the input envelope to the receiving
agent.

### Verbose mode

The shell may additionally report:
- receiving-agent acceptance for processing
- final runtime action outcome

### Replies

- replies returned by the session agent are delivered to the session-specific shell delegate callback path
- the delegate forwards each returned-message event back into the owning shell session
- the shell session renders shell-visible output to stdout
- each displayed reply is attributed using only the runtime sender ID

## Shutdown Contract

When EOF / Ctrl-D is received:
- the shell session begins shutdown immediately
- later returned messages are discarded instead of being rendered
- the dedicated receiving agent is destroyed
- the `ar_shell_session` module is cleaned up
- the shell session is unregistered from `ar_shell`
- the shell delegate is shut down
- the command exits cleanly
