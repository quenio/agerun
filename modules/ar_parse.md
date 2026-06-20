# Parse Module

## Overview

The parse module (`ar_parse`) implements the pure value-level parse operation shared by expression
evaluation and parse instruction evaluation. It reads template and input values and returns a new map
without mutating runtime state.

## API

```c
ar_data_t* ar_parse__create_result(
    const ar_data_t *ref_template,
    const ar_data_t *ref_input
);
```

Creates a new MAP result. STRING, INTEGER, and DOUBLE arguments are interpreted as strings using the
same primitive conversion style used by template building. Missing values, LIST or MAP arguments,
malformed templates, non-matching input, and ambiguous templates return an empty MAP.

## Template Semantics

Templates use `{name}` placeholders. Literal text outside placeholders must match the input. Extracted
values are stored under the placeholder name and converted to INTEGER, DOUBLE, or STRING when possible.
Dotted placeholder names create nested map fields when the parent maps do not already exist.

Placeholder names are ordinary result keys. The parse operation does not reject `self`,
`self.anything`, or values read from `memory.self`; protected identity behavior is enforced only by
assignment and result-storage rules.

## Ownership

- The returned MAP is owned by the caller.
- The template and input values are borrowed and never mutated.
- Extracted values are owned by the returned map.
- NULL is returned only for allocation failure.

## Dependencies

- `ar_data` for data values and map storage
- `ar_allocator` for temporary string allocation
