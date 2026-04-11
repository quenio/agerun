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
- creates a shell session owned by `ar_shell`
- instantiates an `ar_shell_session` module for that shell session
- starts a session-specific shell delegate over stdio
- creates a dedicated receiving agent from the built-in `shell` method
- enters an interactive session that remains open until the user exits

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

- replies are returned toward the shell delegate as output envelopes
- the delegate unwraps them onto stdout
- each displayed reply identifies the sending runtime component
- the first implementation expects output envelopes to carry display text plus sender identity

## Shutdown Contract

When the user exits:
- the shell session begins shutdown
- the dedicated receiving agent is destroyed
- the `ar_shell_session` module is cleaned up
- the shell session owned by `ar_shell` is cleaned up
- the shell delegate is shut down
- the command exits cleanly
