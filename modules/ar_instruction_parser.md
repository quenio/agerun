# ar_instruction_parser

The `ar_instruction_parser` module is the facade that turns one instruction string into one
`ar_instruction_ast_t` node.

## Responsibility

- expose the unified public entry point `ar_instruction_parser__parse()`
- detect the instruction form from the source text
- dispatch to the appropriate specialized parser module
- return a fully built instruction AST on success
- report parse failures through the shared `ar_log` instance

## Public API

```c
ar_instruction_parser_t* ar_instruction_parser__create(ar_log_t *ref_log);
void ar_instruction_parser__destroy(ar_instruction_parser_t *own_parser);
ar_instruction_ast_t* ar_instruction_parser__parse(
    ar_instruction_parser_t *mut_parser,
    const char *ref_instruction
);
```

The deprecated helpers `ar_instruction_parser__get_error()` and
`ar_instruction_parser__get_error_position()` remain for compatibility but always return `NULL` and
`0`; normal parse diagnostics flow through `ar_log`.

## Supported instruction families

The facade currently dispatches to specialized parsers for:

- assignment instructions
- `send(...)`
- `if(...)`
- `compile(...)`
- `spawn(...)`
- `exit(...)`
- `deprecate(...)`
- `parse(...)`
- `build(...)`
- `complete(...)`

`complete(...)` is routed to `ar_complete_instruction_parser`, which validates supported
placeholder syntax and the optional direct `memory...` base path before returning an
`AR_INSTRUCTION_AST_TYPE__COMPLETE` node.

## Current implementation notes

- the facade itself stays small and only performs top-level dispatch decisions
- specialized parser instances are created once and reused across parse calls
- successful function-call parses store both string arguments and parsed expression ASTs on the
  returned instruction AST
- `complete(...)` parsing is intentionally kept out of the facade implementation so syntax rules for
  template placeholders and base-path validation remain isolated in the specialized parser module

## Typical usage

```c
ar_log_t *own_log = ar_log__create();
ar_instruction_parser_t *own_parser = ar_instruction_parser__create(own_log);

ar_instruction_ast_t *own_ast = ar_instruction_parser__parse(
    own_parser,
    "memory.ok := complete(\"The capital is {city}.\", memory.location)"
);

ar_instruction_ast__destroy(own_ast);
ar_instruction_parser__destroy(own_parser);
ar_log__destroy(own_log);
```
