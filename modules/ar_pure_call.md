# Pure Call Module

## Overview

The pure call module (`ar_pure_call`) owns the shared metadata for registered pure expression
calls. It is the source of truth for pure-call names, expected arities, and pure-call identity.
Parsers use it to validate expression-call arity, and evaluators use it to classify generic `CALL`
AST nodes before evaluation.

## Registered Calls

| Name | Arity | Type |
|------|-------|------|
| `parse` | 2 | `AR_PURE_CALL_TYPE__PARSE` |
| `build` | 2 | `AR_PURE_CALL_TYPE__BUILD` |
| `if` | 3 | `AR_PURE_CALL_TYPE__IF` |
| `head` | 1 | `AR_PURE_CALL_TYPE__HEAD` |
| `tail` | 1 | `AR_PURE_CALL_TYPE__TAIL` |
| `append` | 2 | `AR_PURE_CALL_TYPE__APPEND` |

Every registered entry is a pure call. Effectful instruction calls such as `send(...)`,
`complete(...)`, `compile(...)`, `spawn(...)`, `deprecate(...)`, and `exit(...)` are not registered
here and remain statement-level instructions.

## API

```c
const ar_pure_call_t* ar_pure_call__find(const char *ref_name);
bool ar_pure_call__is_registered(const char *ref_name);
const char* ar_pure_call__get_name(const ar_pure_call_t *ref_call);
size_t ar_pure_call__get_arity(const ar_pure_call_t *ref_call);
ar_pure_call_type_t ar_pure_call__get_type(const ar_pure_call_t *ref_call);
size_t ar_pure_call__count(void);
const ar_pure_call_t* ar_pure_call__get_at(size_t index);
```

`ar_pure_call__find()` returns a borrowed metadata entry for a registered name or `NULL` for
unknown names. `ar_pure_call__is_registered()` is a convenience classification wrapper around that
lookup. Enumeration APIs expose the fixed registry for tests and documentation checks.

## Ownership

- Metadata entries are static borrowed references.
- Callers must not destroy or mutate returned metadata entries.
- Lookup and enumeration functions allocate no memory.
- `NULL` lookup names and out-of-range enumeration indexes return `NULL`.
- `NULL` metadata entries return name `NULL`, arity `0`, and `AR_PURE_CALL_TYPE__UNKNOWN`.

## Dependencies

- Standard `string.h` for name comparison.

## Testing

`ar_pure_call_tests.c` verifies the exact registered call set, arity, call type, enumeration order,
unknown-name rejection, and NULL safety.
