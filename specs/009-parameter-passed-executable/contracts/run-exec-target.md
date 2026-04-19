# Contract: `make run-exec`

## Purpose

Define how the repository's `run-exec` Make target forwards boot-method override requests to the
main `agerun` executable.

## Invocation

```text
make run-exec [BOOT_METHOD=<method-name-version>] [RUN_EXEC_DIR=<dir>] [AGERUN_METHODS_DIR=<dir>]
```

## Inputs

### `BOOT_METHOD=<method-name-version>`

- Optional
- Supplies the combined boot method identifier forwarded to `./agerun --boot-method ...`
- Uses the same filename-stem convention as `methods/` (for example, `workflow-coordinator-1.0.0`)

### `RUN_EXEC_DIR=<dir>`

- Existing optional build/output directory override
- Retained for test isolation and parallel workflows

### `AGERUN_METHODS_DIR=<dir>`

- Existing optional methods-directory override
- Retained for fixture-driven executable tests

## Behavioral Contract

1. Without `BOOT_METHOD`, `make run-exec` preserves the current build-and-run behavior.
2. With `BOOT_METHOD`, `make run-exec` builds the executable and launches it with the matching
   `--boot-method` CLI argument.
3. `run-exec` does not create a second execution path or alternate target for the override case.
4. Invalid `BOOT_METHOD` values are surfaced through the executable's normal startup failure path;
   the Make target does not silently replace them with the default boot method.

## Observability Contract

The target remains operator-friendly:

- users can request an alternate boot method without running `./agerun` directly
- existing output-directory and methods-directory overrides continue to work
- startup output still reveals whether the default boot path, override path, or restored-agent skip
  path was used

## Non-Goals

- No new `run-exec-with-boot` target
- No second Make variable for boot version
- No environment-variable-only override contract hidden from the executable CLI
