# Contracts: Executable Boot Method Override

This directory records the external interfaces affected by the executable boot-method override
feature.

## Files

- [executable-startup-cli.md](./executable-startup-cli.md) - CLI contract for the `agerun`
  executable boot override
- [run-exec-target.md](./run-exec-target.md) - Makefile contract for forwarding boot overrides
  through `make run-exec`

## Scope

These contracts cover only startup selection and operator-facing invocation. They do not change the
AgeRun method language, agent persistence format, or the separate `arsh` executable.

The final implementation remains within this contract scope: one executable flag, one `run-exec`
passthrough variable, explicit restored-agent precedence, and explicit failure behavior for
malformed or unavailable override requests.
