# Head Module

## Overview

The head module (`ar_head`) implements the pure value-level head operation shared by expression
evaluation and standalone compatibility head instruction evaluation. It reads one value and returns a
new result without mutating runtime state.

## API

```c
ar_data_t* ar_head__create_result(
    const ar_data_t *ref_list
);
```

Creates a new result value. LIST values with at least one item return a deep copy of the first item.
Missing values, empty LIST values, non-LIST values, and copy failures return integer `0` per the
central [SPEC.md sentinel contract](../SPEC.md#integer-0-sentinel-semantics). A first item whose
ordinary value is integer `0` is indistinguishable from that sentinel. Callers using `head(...) = 0`
as a stop condition need an item domain that excludes integer `0` or a wrapper container.

## Argument Semantics

The operation depends only on the evaluated value supplied to it. Paths such as `memory.self`,
`self`, or nested variable paths do not receive special argument handling. Protected identity
behavior is enforced only by assignment and result-storage rules.

## Ownership

- The returned value is owned by the caller.
- The input value is borrowed and never mutated.
- Nested list/map values are deep-copied when returned.
- NULL is returned only if allocating the fallback result also fails.

## Dependencies

- `ar_data` for data values, list inspection, and deep-copy behavior
