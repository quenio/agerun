# Build Module

## Overview

The build module (`ar_build`) implements the pure value-level build operation shared by expression
evaluation and build instruction evaluation. It reads a template value and a values value, then
returns a new string without mutating runtime state.

## API

```c
ar_data_t* ar_build__create_result(
    const ar_data_t *ref_template,
    const ar_data_t *ref_values
);
```

Creates a new STRING result. STRING, INTEGER, and DOUBLE templates are interpreted as strings using
ordinary primitive conversion. Missing or non-primitive templates use an empty string fallback.

When the values argument is a MAP, placeholders are resolved with `ar_data__get_map_data()` and
STRING, INTEGER, and DOUBLE values are substituted. Missing placeholders, non-MAP values arguments,
and placeholder values that cannot be interpreted as strings leave the original placeholder text
unchanged.

## Template Semantics

Templates use `{name}` placeholders. Dotted placeholder names use the same map lookup semantics as
other data path reads. Placeholder resolution depends only on the evaluated values supplied to the
operation; syntactic paths such as `memory.self` do not receive special handling.

## Ownership

- The returned STRING is owned by the caller.
- The template and values inputs are borrowed and never mutated.
- NULL is returned only for allocation failure.

## Dependencies

- `ar_data` for data values and map lookup
- `ar_allocator` for temporary string allocation
