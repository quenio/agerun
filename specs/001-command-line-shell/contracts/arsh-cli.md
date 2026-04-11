# Contract: `arsh` CLI

## Purpose

Define the user-facing command contract for the AgeRun shell.

## Command Name

- The shell command name is `arsh`

## Startup Contract

Invoking `arsh`:
- starts the stdio shell delegate
- creates a dedicated receiving agent from the built-in shell method
- creates a shell session module with its own memory map
- enters an interactive session that remains open until the user exits

## Input Contract

### Accepted transport input

- stdin is read as raw text lines
- for each accepted line, the stdio delegate creates an envelope map
- initial envelope shape:

```text
{text = <input string>}
```

### Interpreted shell subset

The shell method interprets one line at a time using a restricted subset of AgeRun instruction
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

- replies are shown asynchronously on stdout
- each reply identifies the sending runtime component

## Shutdown Contract

When the user exits:
- the shell session begins shutdown
- the dedicated receiving agent is destroyed
- the shell session module is cleaned up
- the command exits cleanly
