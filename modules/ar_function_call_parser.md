# ar_function_call_parser

The `ar_function_call_parser` module owns AgeRun's function-call argument-list grammar.
It is implemented in Zig with a C-compatible header so both C instruction parsers and Zig
instruction parsers use the same source of truth for argument splitting.

## Responsibility

- split function-call arguments at top-level delimiters
- preserve nested parentheses, one-line list literals, one-line map literals, and quoted strings
- parse argument strings into expression AST lists
- centralize cleanup for owned argument strings and owned argument AST lists
- report parse failures through the shared `ar_log` instance

## Public API

```c
char* ar_function_call_parser__extract_argument(
    ar_log_t *ref_log,
    const char *ref_str,
    size_t *mut_pos,
    char delimiter
);
bool ar_function_call_parser__parse_exact(
    ar_log_t *ref_log,
    const char *ref_str,
    size_t *mut_pos,
    char ***out_args,
    size_t *out_count,
    size_t expected_count
);
ar_list_t* ar_function_call_parser__parse_arg_asts(
    ar_log_t *ref_log,
    char **ref_args,
    size_t arg_count,
    size_t error_offset
);
void ar_function_call_parser__destroy_arg(char *own_arg);
void ar_function_call_parser__destroy_args(char **own_args, size_t arg_count);
void ar_function_call_parser__destroy_arg_asts(ar_list_t *own_arg_asts);
```

## Ownership

- extracted argument strings are owned by the caller
- argument arrays returned by `ar_function_call_parser__parse_exact()` are owned by the caller
- AST lists returned by `ar_function_call_parser__parse_arg_asts()` are owned by the caller until
  transferred to `ar_instruction_ast__set_function_arg_asts()`

## Current Implementation Notes

- this module does not decide which function names exist
- this module does not decide instruction-specific arity exceptions
- expression parsing still rejects nested function calls until the language adds pure call
  expression AST support
