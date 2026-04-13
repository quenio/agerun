# Contract: Shell Session Protocol

## Purpose

Define the runtime message contract between the session-specific shell delegate, the built-in
`shell` method, the instantiable `ar_shell_session` module that owns one session's state and
lifecycle, and the instantiated `ar_shell` module that manages shell sessions for the `arsh`
executable.

## 1. Delegate -> Receiving Agent

### Input envelope

For each accepted line of stdin input, the delegate forwards a map with exactly one field:

```text
text = <input string>
```

The delegate does not interpret the shell syntax beyond creating this envelope and targeting the
configured receiving agent for the session.

## 2. Receiving Agent / Runtime -> Delegate

### Returned-message callback payload

Replies intended for shell display are returned toward the delegate callback path carrying at least
these logical fields:

```text
text = <display string>
sender_id = <runtime sender identifier>
```

When a message is returned by the session agent, the session-specific delegate receives the
callback event, notifies the owning shell session, and the shell session renders terminal output.
Returned replies displayed in the terminal are attributed using only the runtime sender ID.

## 3. Built-in `shell` Method Responsibilities

The built-in `shell` method:
- interprets one shell input line at a time
- supports the restricted instruction subset documented in [arsh-cli.md](./arsh-cli.md)
- launches agents and sends runtime messages when requested
- exchanges shell-session state with `ar_shell_session` only through messages

## 4. Built-in `shell` Method <-> `ar_shell_session`

The exact implementation may evolve, but the protocol must support these logical operations:

### Store a shell-session value

```text
action = ar_shell_session__store_value
path = <session path>
value = <data>
```

### Load a shell-session value

```text
action = ar_shell_session__load_value
path = <session path>
request_id = <correlation value>
```

### Return a loaded shell-session value

```text
action = ar_shell_session__return_loaded_value
request_id = <correlation value>
value = <data>
```

### Report a failed shell-session operation

```text
action = ar_shell_session__report_operation_failure
request_id = <correlation value>
reason = <string>
```

## 5. `ar_shell` <-> `ar_shell_session`

- `ar_shell` creates, tracks, and destroys `ar_shell_session` instances
- `ar_shell_session` owns the shell session memory map and lifecycle for one shell session
- `ar_shell_session` exposes that shell session state to the built-in `shell` method through messages

## 6. Shell-mode assignment redirection

In shell mode, `memory... := ...` syntax is preserved for the user, but the assigned value is stored
in the shell session memory map owned by `ar_shell_session` rather than in the receiving agent's
memory map.

## 7. Acknowledgement Stages

The acknowledgement mode is selected when `arsh` starts and is stored on the active shell session.

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
- `ar_shell` creates and manages the shell session instance
- `ar_shell_session` is instantiated for that shell session and owns its state and lifecycle
- the session-specific shell delegate is bound to that shell session instance

## 9. Replies to the delegate

Runtime replies sent back to the shell delegate must:
- remain attributable using only the runtime sender ID
- be displayable asynchronously through the delegate callback path and shell-session rendering while the session remains open
- be discarded if they arrive after EOF / Ctrl-D has already closed the shell session
- not corrupt the active shell session when delayed
