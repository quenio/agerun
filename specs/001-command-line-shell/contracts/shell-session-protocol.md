# Contract: Shell Session Protocol

## Purpose

Define the runtime message contract between the session-specific shell delegate, the built-in
`shell` method, the instantiable `ar_shell_session` module, and the shell session owned by the
instantiated `ar_shell` module that backs the `arsh` executable.

## 1. Delegate -> Receiving Agent

### Input envelope

For each accepted line of stdin input, the delegate forwards a map with exactly one field:

```text
text = <input string>
```

The delegate does not interpret the shell syntax beyond creating this envelope and targeting the
configured receiving agent for the session.

## 2. Receiving Agent / Runtime -> Delegate

### Output envelope

Replies intended for shell display are returned toward the delegate as an envelope map carrying at
least these logical fields:

```text
text = <display string>
sender_id = <runtime sender identifier>
```

The delegate unwraps this envelope into terminal output and sender attribution.

## 3. Built-in `shell` Method Responsibilities

The built-in `shell` method:
- interprets one shell input line at a time
- supports the restricted instruction subset documented in [arsh-cli.md](./arsh-cli.md)
- launches agents and sends runtime messages when requested
- exchanges shell-session state with `ar_shell_session` only through messages

## 4. Built-in `shell` Method <-> `ar_shell_session`

The exact implementation may evolve, but the protocol must support these logical operations:

### Set a shell-session value

```text
action = set
path = <session path>
value = <data>
```

### Get a shell-session value

```text
action = get
path = <session path>
request_id = <correlation value>
```

### Return a resolved shell-session value

```text
action = resolved
request_id = <correlation value>
value = <data>
```

### Report a state-operation failure

```text
action = error
request_id = <correlation value>
reason = <string>
```

## 5. `ar_shell_session` -> Shell Session owned by `ar_shell`

- `ar_shell_session` mediates runtime access to shell session state held by `ar_shell`
- `ar_shell_session` does not directly own or directly handle the shell session memory map
- the shell session memory map remains owned by the shell session held by `ar_shell`

## 6. Shell-mode assignment redirection

In shell mode, `memory... := ...` syntax is preserved for the user, but the assigned value is stored
in the shell session memory map owned by `ar_shell` rather than in the receiving agent's memory
map, with `ar_shell_session` mediating the runtime-facing access path.

## 7. Acknowledgement Stages

### Normal mode

Minimum required stage:
- input envelope handed from delegate to receiving agent

### Verbose mode

Optional additional stages:
- receiving agent accepted shell input for processing
- runtime action succeeded or failed

## 8. Shell session ownership

- the `arsh` executable is implemented by `ar_shell`
- `ar_shell` is instantiated for the shell process
- `ar_shell` creates and holds the shell session instance
- `ar_shell_session` is instantiated for that shell session
- the session-specific shell delegate is bound to that shell session instance

## 9. Replies to the delegate

Runtime replies sent back to the shell delegate must:
- remain attributable to the correct sender
- be displayable asynchronously after delegate unwrapping
- not corrupt the active shell session when delayed
