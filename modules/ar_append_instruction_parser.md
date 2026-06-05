# ar_append_instruction_parser

The `ar_append_instruction_parser` module parses `append(...)` instructions into
`AR_INSTRUCTION_AST_TYPE__APPEND` AST nodes.

## Responsibility

- parse `append(memory.results, value)` function calls
- support optional result assignment such as `memory.append_ok := append(memory.results, value)`
- require the first argument to be a non-root `memory` access
- reject `message` and `context` targets during parsing
- attach parsed expression ASTs for both arguments
- report parse failures through the shared `ar_log` instance

## Public API

```c
ar_append_instruction_parser_t* ar_append_instruction_parser__create(ar_log_t *ref_log);
void ar_append_instruction_parser__destroy(ar_append_instruction_parser_t *own_parser);
ar_instruction_ast_t* ar_append_instruction_parser__parse(
    ar_append_instruction_parser_t *mut_parser,
    const char *ref_instruction,
    const char *ref_result_path
);
```

The deprecated helpers `ar_append_instruction_parser__get_error()` and
`ar_append_instruction_parser__get_error_position()` remain for compatibility but always return
`NULL` and `0`; normal parse diagnostics flow through `ar_log`.

## Current implementation notes

- successful parses produce a function-call AST with function name `"append"`
- the target argument remains an expression AST so the evaluator can inspect the memory path without
  evaluating it as a copied value
- the value argument is parsed as a normal expression and may refer to memory, message, context, or
  literals
- runtime checks such as target existence and LIST type validation are handled by
  `ar_append_instruction_evaluator`
