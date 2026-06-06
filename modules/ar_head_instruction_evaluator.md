# ar_head_instruction_evaluator

The `ar_head_instruction_evaluator` module evaluates `head(...)` instructions inside an
`ar_frame_t`.

## Responsibility

- verify the instruction AST is `AR_INSTRUCTION_AST_TYPE__HEAD`
- evaluate the single argument as a normal expression
- return a shallow copy of the first item when the argument resolves to a non-empty LIST
- store integer `0` for empty, missing, non-LIST, or not-copyable inputs
- never mutate the source list
- write optional result assignments with the returned value

## Public API

```c
ar_head_instruction_evaluator_t* ar_head_instruction_evaluator__create(
    ar_log_t *ref_log,
    ar_expression_evaluator_t *ref_expr_evaluator
);

void ar_head_instruction_evaluator__destroy(
    ar_head_instruction_evaluator_t *own_evaluator
);

bool ar_head_instruction_evaluator__evaluate(
    const ar_head_instruction_evaluator_t *ref_evaluator,
    const ar_frame_t *ref_frame,
    const ar_instruction_ast_t *ref_ast
);
```

## Ownership notes

- the evaluator owns only its internal structure
- the log and expression evaluator are borrowed references
- source expressions that evaluate to temporary values are discarded after result creation
- returned values are new shallow copies, so `head(...)` does not claim or move items out of the
  source list
- if result storage fails, the owned result value is destroyed before the evaluator returns `false`

## No-op and fallback behavior

If the input is empty, missing, non-LIST, or cannot be shallow-copied safely, assigned
`head(...)` stores integer `0`. Without result assignment, the computed fallback is discarded and
the instruction completes successfully so method execution can continue.

When callers use `head(...) = 0` as a stop condition, their list item domain must exclude integer
`0`, or arbitrary values should be wrapped in flat containers so a valid item cannot collide with
the sentinel.

## Current limitation

Returned nested containers are limited by `ar_data__shallow_copy()`. The evaluator can return
primitives and flat maps/lists, but a first item that is a map or list containing nested maps or
lists cannot be copied. In that case `head(...)` stores integer `0` when assigned, and otherwise
completes without stopping method execution.
