# Tail Module

## Overview

The tail module (`ar_tail`) implements the pure value-level tail operation shared by expression
evaluation and standalone compatibility tail instruction evaluation. It reads one value and returns a
new result without mutating runtime state.

## API

```c
ar_data_t* ar_tail__create_result(
    const ar_data_t *ref_list
);
```

Creates a new result value. LIST values return a new LIST containing deep copies of every item after
the first. Empty and single-item LIST values return a new empty LIST. Missing values, non-LIST
values, and copy failures return integer `0`.

## Argument Semantics

The operation depends only on the evaluated value supplied to it. Paths such as `memory.self`,
`self`, or nested variable paths do not receive special argument handling. Protected identity
behavior is enforced only by assignment and result-storage rules.

## Ownership

- The returned value is owned by the caller.
- The input value is borrowed and never mutated.
- Returned lists own all retained deep-copy items.
- NULL is returned only if allocating the fallback result also fails.

## Dependencies

- `ar_data` for data values, list inspection, and deep-copy behavior
- `ar_allocator` for temporary list item arrays
