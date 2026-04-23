# Contracts: Executable Persistence Disable Option

This directory records the external interfaces affected by the executable persistence-disable
feature.

## Files

- [executable-persistence-cli.md](./executable-persistence-cli.md) - CLI contract for the
  `agerun` persistence-disable option
- [run-exec-persistence-mode.md](./run-exec-persistence-mode.md) - top-level build-file contract
  for forwarding no-persistence runs through `make run-exec`

## Scope

These contracts cover only operator-facing startup and shutdown persistence behavior. They do not
change the AgeRun method language, persisted file formats, runtime APIs, or the separate `arsh`
executable.
