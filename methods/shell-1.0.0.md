# shell-1.0.0

## Purpose

`shell-1.0.0.method` is the built-in method asset used by the command-line shell scaffold.
The current implementation goes beyond startup scaffolding and now covers the first restricted
shell-syntax slice needed for interactive shell-session behavior.

## Current Behavior

The current implementation supports a first restricted shell-syntax slice:

```text
memory.last_input := message.text
memory.assignment := parse("memory.prompt := {value}", message.text)
memory.store_request_input := build("action=ar_shell_session__store_value path=memory.prompt value={value}", memory.assignment)
memory.store_request := parse("action={action} path={path} value={value}", memory.store_request_input)
send(memory.shell_session_delegate_id, memory.store_request)
memory.last_error := "Invalid shell syntax"
```

Current behavior details:
- keeps tracking the last received shell input line in agent memory
- recognizes the `memory.prompt := ...` shell assignment form used by the new tests
- forwards that assignment to `ar_shell_session` through the session runtime delegate
- relies on `ar_shell_session` to normalize quoted string values before storing them in shell-session memory
- records a recoverable `memory.last_error` marker so invalid syntax handling can evolve without closing the session

The full shell behavior described in `specs/001-command-line-shell/spec.md` still requires broader
restricted-syntax support for additional assignment paths plus `spawn(...)`, `send(...)`, and the
assigned `spawn` / `send` forms.

## Validation

`methods/shell_tests.c` verifies that the method asset loads through `ar_method_fixture__load_method()`, parses to an AST, supports receiving-agent creation, redirects the tested prompt assignment into shell-session memory, and keeps invalid syntax recoverable.
