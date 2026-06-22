# ar_condition

The `ar_condition` module owns shared AgeRun condition truthiness rules.

## Purpose

`ar_condition` keeps condition selection consistent between instruction-level `if(...)` evaluation
and expression-level pure `if(...)` evaluation.

## Interface

```c
bool ar_condition__is_true(
    const ar_data_t *ref_value
);
```

Returns `true` only for nonzero integer values. Integer `0`, non-integer values, and missing values
are false. The function is pure and does not take ownership of the inspected value.
