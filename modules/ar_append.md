# Append Module

## Overview

The append module (`ar_append`) implements the pure value-level append operation used by expression
evaluation. It reads one list value and one value to append, then returns a new result without
mutating runtime state.

## API

```c
ar_data_t* ar_append__create_result(
    const ar_data_t *ref_list,
    const ar_data_t *ref_value
);
```

Creates a new result value. LIST inputs return a new LIST containing deep copies of every source
item followed by a deep copy of the appended value. Empty LIST inputs return a new one-item LIST.
Missing list inputs, non-LIST list inputs, missing values, and copy failures return integer `0`.

## Argument Semantics

The operation depends only on the evaluated values supplied to it. Paths such as `memory.self`,
`self`, or nested variable paths do not receive special argument handling. Protected identity
behavior is enforced only by assignment and result-storage rules.

## Ownership

- The returned value is owned by the caller.
- Both input values are borrowed and never mutated.
- Returned lists own all deep-copy items.
- NULL is returned only if allocating the fallback result also fails.

## Dependencies

- `ar_data` for data values, list inspection, and deep-copy behavior
- `ar_allocator` for temporary list item arrays
