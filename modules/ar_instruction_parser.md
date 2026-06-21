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
- `append(...)`
- `head(...)`
- `tail(...)`

`complete(...)` is routed to `ar_complete_instruction_parser`, which validates supported
placeholder syntax and stores the optional values expression before returning an
`AR_INSTRUCTION_AST_TYPE__COMPLETE` node.

`append(...)` is routed to `ar_append_instruction_parser`, which parses both arguments as
expressions and returns an `AR_INSTRUCTION_AST_TYPE__APPEND` node.

`head(...)` and `tail(...)` are routed to their specialized parsers, each of which parses one list
expression and returns `AR_INSTRUCTION_AST_TYPE__HEAD` or `AR_INSTRUCTION_AST_TYPE__TAIL`.

Assigned pure calls, such as `memory.result := parse(template, input)` or
`memory.result := build(template, values)`, are parsed as normal assignment instructions whose
right-hand side is a pure function call expression. Standalone `parse(...)` remains accepted as a
function instruction for compatibility and discards its result. Standalone `build(...)` keeps the
existing build instruction behavior.

## Current implementation notes

- the facade itself stays small and only performs top-level dispatch decisions
- specialized parser instances are created once and reused across parse calls
- successful function-call parses store both string arguments and parsed expression ASTs on the
  returned instruction AST
- function-call argument boundaries and argument AST-list creation are centralized in
  `ar_function_call_parser`
- assignment dispatch only treats a right-hand side as an instruction-result call when it starts with
  an instruction function; registered pure calls are left for the expression parser
- `complete(...)` parsing is intentionally kept out of the facade implementation so syntax rules for
  template placeholders remain isolated in the specialized parser module
- `append(...)` parsing leaves target ownership and LIST validation to the evaluator so non-memory
  target expressions can compile and resolve to no-op results at runtime
- `head(...)` and `tail(...)` parsing accepts any expression; LIST validation, empty-list handling,
  and copy-limit behavior are evaluator concerns

## Typical usage

```c
ar_log_t *own_log = ar_log__create();
ar_instruction_parser_t *own_parser = ar_instruction_parser__create(own_log);

ar_instruction_ast_t *own_ast = ar_instruction_parser__parse(
    own_parser,
    "memory.result := complete(\"The capital of {country} is {city}.\", memory.values)"
);

ar_instruction_ast__destroy(own_ast);
ar_instruction_parser__destroy(own_parser);
ar_log__destroy(own_log);
```
