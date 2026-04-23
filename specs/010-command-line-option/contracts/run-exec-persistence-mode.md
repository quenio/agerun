# Contract: `make run-exec` Persistence Mode

## Purpose

Define how the repository's `run-exec` Make target forwards persistence-disable requests to the main
`agerun` executable.

## Invocation

```text
make run-exec [NO_PERSISTENCE=<non-empty>] [BOOT_METHOD=<method-name-version>] [RUN_EXEC_DIR=<dir>] [AGERUN_METHODS_DIR=<dir>]
```

## Inputs

### `NO_PERSISTENCE=<non-empty>`

- Optional
- Any non-empty value enables the executable's `--no-persistence` CLI flag for the run
- `NO_PERSISTENCE=1` remains the documented example invocation
- Applies to both persisted methodology and persisted agency behavior

### `BOOT_METHOD=<method-name-version>`

- Existing optional Make variable retained for fresh-start boot overrides
- Continues to work when `NO_PERSISTENCE` is also supplied

### `RUN_EXEC_DIR=<dir>`

- Existing optional build/output directory override
- Retained for test isolation and parallel workflows

### `AGERUN_METHODS_DIR=<dir>`

- Existing optional methods-directory override
- Retained for fixture-driven executable tests

## Behavioral Contract

1. Without `NO_PERSISTENCE`, `make run-exec` preserves current build-and-run behavior.
2. With any non-empty `NO_PERSISTENCE` value, `make run-exec` builds the executable and launches
   it with `--no-persistence`.
3. `run-exec` does not create a second target or alternate execution path for the no-persistence
   case.
4. `NO_PERSISTENCE` may be combined with `BOOT_METHOD=<name-version>` to request a fresh startup
   with a specific boot method while still skipping persisted load/save behavior.
5. Existing output-directory and methods-directory overrides continue to work unchanged.

## Observability Contract

The target remains operator-friendly:

- users can request a non-persistent run without launching `./agerun` directly
- startup output still reveals that persistence was disabled for the current session
- the target remains compatible with existing fresh-start boot-override workflows

## Non-Goals

- No dedicated `run-exec-no-persistence` target
- No environment-variable-only contract hidden from the executable CLI
- No separate Make variables for methods-only or agents-only persistence
