# ar_tail_instruction_evaluator

The `ar_tail_instruction_evaluator` module evaluates `tail(...)` instructions inside an
`ar_frame_t`.

## Responsibility

- verify the instruction AST is `AR_INSTRUCTION_AST_TYPE__TAIL`
- evaluate the single argument as a normal expression
- return a new LIST containing shallow copies of all items after the first
- return a new empty LIST for empty source lists and single-item source lists
- store integer `0` for missing, non-LIST, or not-copyable inputs
- never mutate the source list
- write optional result assignments with the returned value

## Public API

```c
ar_tail_instruction_evaluator_t* ar_tail_instruction_evaluator__create(
    ar_log_t *ref_log,
    ar_expression_evaluator_t *ref_expr_evaluator
);

void ar_tail_instruction_evaluator__destroy(
    ar_tail_instruction_evaluator_t *own_evaluator
);

bool ar_tail_instruction_evaluator__evaluate(
    const ar_tail_instruction_evaluator_t *ref_evaluator,
    const ar_frame_t *ref_frame,
    const ar_instruction_ast_t *ref_ast
);
```

## Ownership notes

- the evaluator owns only its internal structure
- the log and expression evaluator are borrowed references
- source expressions that evaluate to temporary values are discarded after result creation
- returned tail lists are newly allocated and contain shallow-copied items
- if result storage fails, the owned result value is destroyed before the evaluator returns `false`

## Empty-list and fallback behavior

`tail([])` and `tail([value])` both store a new empty LIST. Missing and non-LIST inputs store
integer `0`, so callers can distinguish invalid input from the valid tail of a single-item list.
Without result assignment, the computed value is discarded and the instruction completes
successfully so method execution can continue.

## Current limitation

Returned tail items are limited by `ar_data__shallow_copy()`. The evaluator can return primitives
and flat maps/lists, but any retained item that is a map or list containing nested maps or lists
cannot be copied. In that case `tail(...)` logs an error, stores integer `0` when assigned, and
otherwise completes without stopping method execution.
