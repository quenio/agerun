# ar_head_instruction_parser

The `ar_head_instruction_parser` module parses `head(...)` instructions into
`AR_INSTRUCTION_AST_TYPE__HEAD` AST nodes.

## Responsibility

- parse `head(list)` function calls
- support optional result assignment such as `memory.next := head(memory.targets)`
- attach the parsed expression AST for the list argument
- report parse failures through the shared `ar_log` instance

## Public API

```c
ar_head_instruction_parser_t* ar_head_instruction_parser__create(ar_log_t *ref_log);
void ar_head_instruction_parser__destroy(ar_head_instruction_parser_t *own_parser);
ar_instruction_ast_t* ar_head_instruction_parser__parse(
    ar_head_instruction_parser_t *mut_parser,
    const char *ref_instruction,
    const char *ref_result_path
);
```

The deprecated helpers `ar_head_instruction_parser__get_error()` and
`ar_head_instruction_parser__get_error_position()` remain for compatibility but always return
`NULL` and `0`; normal parse diagnostics flow through `ar_log`.

## Current implementation notes

- successful parses produce a function-call AST with function name `"head"`
- the single argument is parsed as a normal expression and may refer to memory, message, context, or
  literals
- runtime checks such as LIST type validation, empty-list behavior, protected `memory.self`, and
  copy-limit handling are performed by `ar_head_instruction_evaluator`
