# Result Binding Module

## Overview

The `ar_result_binding` module owns statement-level result binding for assigned effectful
instructions. It is the shared domain owner for storing results from `send(...)`, `complete(...)`,
`compile(...)`, `spawn(...)`, `deprecate(...)`, and `exit(...)`.

Pure expression assignment, such as `memory.value := parse(template, input)` or
`memory.total := memory.x + 1`, remains handled by the assignment evaluator and is not part of this
module.

## Responsibility

- validate effectful result-binding targets before side effects run
- reject protected `memory.self` and `memory.self.*` result targets with the established error
  message
- bind owned result values into the mutable memory map from an `ar_frame_t`
- transfer ownership to frame memory when storage succeeds
- destroy owned result values when storage fails or the target cannot consume them
- leave each effectful evaluator responsible for its own instruction return behavior

## Public Interface

```c
bool ar_result_binding__validate_target(
    ar_log_t *ref_log,
    const char *ref_result_path
);
```

Validates the target path early enough that protected identity writes can stop an effectful
instruction before it performs side effects. A `NULL` path means the instruction has no result
binding and is accepted.

```c
bool ar_result_binding__bind(
    ar_log_t *ref_log,
    const ar_frame_t *ref_frame,
    const char *ref_result_path,
    ar_data_t *own_result
);
```

Consumes an owned result value. On success, ownership transfers to frame memory. On failure, the
owned result is destroyed. The return value reports whether storage consumed the result; callers keep
their instruction-specific success/failure semantics.

## Binding Rules

- `memory.path` targets are stored in the frame memory map.
- `memory.self` and `memory.self.*` are rejected before side effects and during binding as a safety
  backstop.
- Non-`memory` paths do not store into frame memory; the owned result is destroyed.
- `NULL` result values cannot be stored.
- The module does not evaluate expressions and does not parse instructions.

## Dependencies

- `ar_data`: for result values and memory-map storage
- `ar_frame`: for access to frame memory
- `ar_log`: for protected-target error reporting

## Testing

The module tests cover accepted memory targets, protected `memory.self` and `memory.self.*`
rejection, ownership transfer on successful storage, preservation of existing `memory.self`, and
owned-result destruction when storage fails.
