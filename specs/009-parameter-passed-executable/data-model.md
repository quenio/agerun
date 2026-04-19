# Data Model: Executable Boot Method Override

This document describes the runtime selection data involved in overriding the executable's initial
boot method.

## 1. Boot Override Request

### Description
The operator-supplied request to replace the default boot method during a fresh executable startup.

### Fields
- `method_identifier` - required when an override is requested; provided through `--boot-method` or
  the `BOOT_METHOD` Make variable; follows the methods-folder filename-stem pattern
  `<method-name>-<version>`
- `request_source` - one of `default_startup`, `cli_override`, or `make_passthrough`

### Validation Rules
- `method_identifier` must be non-empty when an override is requested.
- `method_identifier` must follow the same combined naming convention used by files in `methods/`.
- Unknown executable flags or missing values are startup usage errors.

## 2. Parsed Boot Selection

### Description
The executable's internal breakdown of the combined identifier into the values needed for
`ar_system__init`.

### Fields
- `selected_method_name` - either the default boot method (`bootstrap`) or the parsed override
  method name
- `selected_method_version` - either the default boot version (`1.0.0`) or the parsed override
  version
- `selection_reason` - one of `default_requested`, `override_requested`, or `override_skipped`

### Validation Rules
- On a fresh startup path, `selected_method_name` and `selected_method_version` must both be set
  before calling `ar_system__init`.
- The split point is the final hyphen in the combined identifier.
- Identifiers that do not produce both a name and a version are rejected.

## 3. Startup State

### Description
The startup mode that determines whether a boot override can be applied.

### Fields
- `agents_loaded_from_disk` - boolean indicating whether persisted agents were restored
- `startup_mode` - one of `fresh_startup` or `restored_startup`
- `override_applicability` - one of `apply_override`, `use_default_boot`, or `skip_override`
- `skip_reason` - populated when an override was requested but not used

### State Rules
- `fresh_startup` means the executable must create exactly one initial boot agent.
- `restored_startup` means no fresh boot agent is created.
- `skip_override` is only valid when an override was requested and persisted agents were restored.

## 4. Boot Launch Outcome

### Description
The observable result of the executable startup selection process.

### Fields
- `boot_agent_created` - boolean
- `boot_agent_id` - integer identifier when creation succeeds
- `boot_message_queued` - boolean indicating whether `"__boot__"` was enqueued
- `failure_category` - one of `usage_error`, `boot_creation_failed`, `boot_message_failed`, or
  `none`
- `operator_message` - human-readable startup output explaining the outcome

### State Transitions
1. `parsed` - startup arguments and Make passthrough values have been interpreted
2. `validated` - request shape is acceptable
3. `selected` - effective boot method/version determined
4. `applied` - boot agent created and `"__boot__"` queued
5. `skipped` - override explicitly not used because agents were restored
6. `rejected` - startup stopped due to invalid input or failed boot-agent creation

## 5. Make Run-Exec Override Surface

### Description
The user-facing Make contract that forwards override intent to the executable.

### Fields
- `BOOT_METHOD` - optional Make variable supplying the combined method identifier
- `RUN_EXEC_DIR` - existing output-directory override retained for test isolation
- `AGERUN_METHODS_DIR` - existing method-directory override retained for test isolation

### Validation Rules
- `run-exec` must preserve its current behavior when `BOOT_METHOD` is not supplied.
- `BOOT_METHOD` values must be passed as executable parameters rather than creating a second
  execution path.
- Invalid combined identifiers are executable usage/runtime errors, not a signal to fall back to the
  default boot method silently.

## Relationships

- One `Boot Override Request` influences zero or one `Parsed Boot Selection`.
- One `Startup State` determines whether the override request becomes a `Parsed Boot Selection` or
  is recorded as a skipped request.
- One `Parsed Boot Selection` produces one `Boot Launch Outcome` on fresh startup.
- One `Make Run-Exec Override Surface` may generate one `Boot Override Request` when users launch
  through Make.

## Notes

- This feature does not alter agent memory, method persistence, or method-language data structures.
- The override feature changes startup selection only; once the boot agent is created, the existing
  message-processing and persistence flow continues unchanged.
- Final implementation confirmed the modeled outcomes: malformed identifiers transition to
  `rejected`, unavailable boot methods transition to `rejected` without fallback, and restored-agent
  startup transitions to `skipped` with an explicit operator-visible message.
