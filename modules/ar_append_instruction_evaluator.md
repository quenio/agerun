# ar_append_instruction_evaluator

The `ar_append_instruction_evaluator` module evaluates `append(...)` instructions inside an
`ar_frame_t`.

## Responsibility

- verify the instruction AST is `AR_INSTRUCTION_AST_TYPE__APPEND`
- evaluate the first argument as a normal expression
- mutate the target only when it resolves to an existing LIST owned directly or indirectly by memory
- treat `message`, `context`, fresh expression, non-LIST, missing, and protected `memory.self` targets as no-ops
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
- target expressions that evaluate to temporary values are discarded on no-op paths
- fresh literal values are transferred directly into the list
- borrowed values are passed through `ar_data__claim_or_copy()` before appending, preserving nested maps and lists
- `ar_data__list_add_last_data()` takes ownership only on success
- if append fails after a value has been claimed or copied, the evaluator destroys the owned value

## No-op behavior

Target expressions that do not resolve to a memory-owned LIST do not mutate anything. Value
evaluation, copy, and list-insert failures also leave the target unchanged after the failing step.
If the instruction has a result assignment, the evaluator stores integer `0`; otherwise the
instruction still completes successfully so method execution can continue.

## Result assignment safety

Assigned append instructions validate the result path before mutating the target list. Non-memory
result paths and protected `memory.self` paths fail without appending. For successful appends, the
evaluator stores integer `1`; if that success result cannot be stored after the list append, the
evaluator removes the appended item before failing.

## Nested values

Borrowed maps and lists are deep-copied before append, so nested list/map structure is preserved and
the source value is not mutated. If the value cannot be copied or inserted, `append(...)` logs an
error, stores integer `0` when the instruction has a result assignment, and otherwise completes
without stopping method execution.
