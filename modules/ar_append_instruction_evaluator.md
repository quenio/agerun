# ar_append_instruction_evaluator

The `ar_append_instruction_evaluator` module evaluates `append(...)` instructions inside an
`ar_frame_t`.

## Responsibility

- verify the instruction AST is `AR_INSTRUCTION_AST_TYPE__APPEND`
- resolve the first argument as a mutable path under the frame memory map
- reject `message`, `context`, root `memory`, and protected `memory.self` targets
- require the resolved target to be an existing LIST
- evaluate the second argument as a normal expression
- transfer the append value into the target list with `ar_data__list_add_last_data()`
- write optional result assignments as integer `1` or `0`

## Public API

```c
ar_append_instruction_evaluator_t* ar_append_instruction_evaluator__create(
    ar_log_t *ref_log,
    ar_expression_evaluator_t *ref_expr_evaluator
);

void ar_append_instruction_evaluator__destroy(
    ar_append_instruction_evaluator_t *own_evaluator
);

bool ar_append_instruction_evaluator__evaluate(
    const ar_append_instruction_evaluator_t *ref_evaluator,
    const ar_frame_t *ref_frame,
    const ar_instruction_ast_t *ref_ast
);
```

## Ownership notes

- the evaluator owns only its internal structure
- the log and expression evaluator are borrowed references
- fresh literal values are transferred directly into the list
- borrowed values are passed through `ar_data__claim_or_copy()` before appending
- `ar_data__list_add_last_data()` takes ownership only on success
- if append fails after a value has been claimed or copied, the evaluator destroys the owned value

## Current limitation

Borrowed nested containers cannot be appended yet. `ar_data__claim_or_copy()` shallow-copies
primitives and flat containers, but returns `NULL` for borrowed maps or lists that contain nested
maps or lists. In that case `append(...)` logs an error and returns false, or stores integer `0`
when the instruction has a result assignment.
