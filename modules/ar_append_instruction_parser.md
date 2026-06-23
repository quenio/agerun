# ar_append_instruction_parser

The `ar_append_instruction_parser` module parses standalone compatibility `append(...)`
instructions into `AR_INSTRUCTION_AST_TYPE__APPEND` AST nodes.

## Responsibility

- parse standalone `append(target, value)` function calls
- preserve the lower-level optional result path API for legacy/direct parser callers
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
- both arguments are parsed as normal expressions and may refer to memory, message, context, or
  literals
- the unified instruction parser routes assigned `append(...)` source text through ordinary
  assignment parsing because `append(...)` is also a pure expression call
- runtime checks such as target ownership, target existence, protected `memory.self`, and LIST type
  validation are handled by `ar_append_instruction_evaluator`
