# ar_instruction_evaluator

The `ar_instruction_evaluator` module is the facade that executes parsed instruction AST nodes
inside an `ar_frame_t`.

## Responsibility

- own the shared expression evaluator used by specialized instruction evaluators
- create and coordinate specialized evaluators for each instruction type
- dispatch each `ar_instruction_ast_t` to the correct specialized evaluator
- keep log, agency, delegation, and frame-based execution flow consistent across instruction types

## Architecture

The facade currently coordinates specialized evaluators for:

- assignment
- send
- condition / `if(...)`
- parse
- build
- complete
- compile
- spawn
- exit
- deprecate

`complete(...)` support is split between:
- `ar_complete_instruction_evaluator`, which enforces AgeRun semantics such as atomic writes,
  boolean status results, and failure handling
- `ar_local_completion`, which owns local backend initialization and placeholder-value generation

## Public API

```c
ar_instruction_evaluator_t* ar_instruction_evaluator__create(
    ar_log_t *ref_log,
    ar_agency_t *ref_agency,
    ar_delegation_t *ref_delegation
);

void ar_instruction_evaluator__destroy(ar_instruction_evaluator_t *own_evaluator);

bool ar_instruction_evaluator__evaluate(
    const ar_instruction_evaluator_t *ref_evaluator,
    const ar_frame_t *ref_frame,
    const ar_instruction_ast_t *ref_ast
);
```

## Current implementation notes

- the facade is implemented in Zig
- all specialized evaluators are created during facade initialization
- `ar_instruction_evaluator__evaluate()` uses `ar_instruction_ast__get_type()` for dispatch
- `complete(...)` failures are handled as normal instruction outcomes: the specialized evaluator
  logs actionable diagnostics, preserves prior memory on failure, and writes boolean `0` when the
  call has a result assignment
- post-failure execution continues normally for later non-`complete(...)` instructions

## Typical usage

```c
ar_instruction_evaluator_t *own_evaluator = ar_instruction_evaluator__create(
    own_log,
    own_agency,
    own_delegation
);

ar_frame_t *own_frame = ar_frame__create(own_memory, own_context, own_message);
bool ok = ar_instruction_evaluator__evaluate(own_evaluator, own_frame, own_instruction_ast);

ar_frame__destroy(own_frame);
ar_instruction_evaluator__destroy(own_evaluator);
```
