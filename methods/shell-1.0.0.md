# shell-1.0.0

## Purpose

`shell-1.0.0.method` is the built-in method asset used by the command-line shell scaffold.
The current scaffold provides a valid method body so the runtime can register the method, build an
AST, and create a receiving agent for shell-session startup tests.

## Current Behavior

The current scaffold stores the last received text line into agent memory:

```text
memory.last_input := message.text
```

This is a temporary implementation scaffold for startup and transport validation. The full shell
behavior described in `specs/001-command-line-shell/spec.md` still requires the restricted
`spawn(...)`, `send(...)`, and shell-session message-mediation logic.

## Validation

`methods/shell_tests.c` verifies that the method asset loads through `ar_method_fixture__load_method()`, parses to an AST, and supports receiving-agent creation.
