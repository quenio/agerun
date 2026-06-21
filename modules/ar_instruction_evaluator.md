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
- append
- head
- tail
- compile
- spawn
- exit
- deprecate

`complete(...)` support is split between:
- `ar_complete_instruction_evaluator`, which enforces AgeRun semantics such as atomic writes,
  boolean status results, and failure handling
- `ar_local_completion`, which owns local backend initialization and placeholder-value generation

`append(...)` support is handled by `ar_append_instruction_evaluator`, which mutates only an
existing memory-owned list and stores integer `1` or `0` when the function call has a result
assignment.

Standalone compatibility `head(...)` and `tail(...)` support is handled by their specialized
evaluators. Assigned `head(...)` and `tail(...)` calls normally run through ordinary assignment of
pure expression-call results. Both paths share the same value-level head/tail semantics.

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
- `append(...)` evaluates the target expression, accepts it only when it resolves to a
  memory-owned LIST, and transfers the claimed or copied value to that list
- standalone `head(...)` delegates to `ar_head` and returns a deep copy of the first list item, or
  integer `0` for empty/invalid input
- standalone `tail(...)` delegates to `ar_tail` and returns a new LIST of deep-copied items after
  the first, a new empty LIST for an empty or single-item source list, or integer `0` for invalid
  input/copy failure
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
