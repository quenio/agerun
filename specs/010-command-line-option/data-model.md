# Data Model: Executable Persistence Disable Option

This document describes the runtime selection data involved in disabling executable persistence for a
single `agerun` run.

## 1. Persistence Disable Request

### Description
The operator-supplied request to run the executable without loading or saving persisted state.

### Fields
- `persistence_disabled` - boolean indicating whether `--no-persistence` was supplied
- `request_source` - one of `default_startup`, `cli_flag`, or `make_passthrough`
- `operator_flag` - the explicit operator-facing input used to activate the mode (`--no-persistence`)

### Validation Rules
- `persistence_disabled` defaults to `false` when the option is omitted.
- Unknown executable flags remain usage errors.
- The flag is boolean and does not require a value.

## 2. Persistence Mode

### Description
The executable's internal interpretation of whether persisted state should participate in startup and
shutdown.

### Fields
- `load_methodology` - boolean
- `load_agency` - boolean
- `save_methodology` - boolean
- `save_agency` - boolean
- `mode_reason` - one of `default_persistence` or `persistence_disabled`

### State Rules
- In `default_persistence`, all four booleans are `true` unless an existing failure path prevents
  successful load/save.
- In `persistence_disabled`, all four booleans are `false` for the current run.
- The feature does not support partial persistence modes.

## 3. Startup Persistence Outcome

### Description
The startup result produced after evaluating persistence mode and pre-existing persisted files.

### Fields
- `methodology_restore_attempted` - boolean
- `agency_restore_attempted` - boolean
- `agents_loaded_from_disk` - boolean
- `startup_mode` - one of `fresh_startup`, `restored_startup`, or `fresh_startup_persistence_disabled`
- `operator_message` - human-readable output describing whether persistence was active or skipped

### State Rules
- `fresh_startup_persistence_disabled` means the executable skipped both restore paths by operator
  choice and proceeded as a fresh run.
- `restored_startup` is impossible when `persistence_disabled` is `true`.
- If persistence is disabled, restore-related file errors are irrelevant because no restore attempt
  occurs.

## 4. Shutdown Persistence Outcome

### Description
The shutdown result that determines whether persisted state is written back to disk.

### Fields
- `methodology_save_attempted` - boolean
- `agency_save_attempted` - boolean
- `methodology_saved` - boolean
- `agency_saved` - boolean
- `shutdown_mode` - one of `persistence_enabled_shutdown` or `persistence_disabled_shutdown`

### State Rules
- In `persistence_disabled_shutdown`, both save-attempt booleans are `false`.
- Existing persisted files are preserved but not rewritten when shutdown persistence is disabled.
- The feature changes whether save is attempted, not the file format used when save remains enabled.

## 5. Make Run-Exec Persistence Surface

### Description
The Make contract that forwards no-persistence intent into the executable CLI.

### Fields
- `NO_PERSISTENCE` - optional non-empty Make variable enabling `--no-persistence` (with
  `NO_PERSISTENCE=1` as the documented example)
- `BOOT_METHOD` - existing optional Make variable retained for fresh-start boot overrides
- `RUN_EXEC_DIR` - existing output-directory override retained for tests
- `AGERUN_METHODS_DIR` - existing methods-directory override retained for tests

### Validation Rules
- Without `NO_PERSISTENCE`, `make run-exec` preserves current behavior.
- With any non-empty `NO_PERSISTENCE` value, `make run-exec` forwards `--no-persistence` to the
  executable.
- `NO_PERSISTENCE=1` remains the documented example invocation.
- `NO_PERSISTENCE` complements rather than replaces the existing boot-override surface.

## Relationships

- One `Persistence Disable Request` determines one `Persistence Mode`.
- One `Persistence Mode` determines one `Startup Persistence Outcome`.
- One `Persistence Mode` determines one `Shutdown Persistence Outcome`.
- One `Make Run-Exec Persistence Surface` may generate one `Persistence Disable Request`.

## Notes

- This feature does not change the persisted methodology or agency file formats.
- This feature does not introduce new agent, method, or message data structures.
- The main behavioral change is whether persisted state participates in executable startup and
  shutdown for the current run.
