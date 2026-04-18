# ar_complete_instruction_parser

The `ar_complete_instruction_parser` module parses `complete(...)` instructions into
`AR_INSTRUCTION_AST_TYPE__COMPLETE` AST nodes.

## Responsibility

- parse one-argument forms such as `complete("The largest country is {country}.")`
- parse two-argument forms such as `complete("The capital is {city}.", memory.location)`
- validate that placeholder markers use supported `{name}` syntax
- validate that the optional second argument is a direct `memory...` access path
- attach parsed expression ASTs for the template string and optional base path
- report parse failures through the shared `ar_log` instance

## Public API

```c
ar_complete_instruction_parser_t* ar_complete_instruction_parser__create(ar_log_t *ref_log);
void ar_complete_instruction_parser__destroy(ar_complete_instruction_parser_t *own_parser);
ar_instruction_ast_t* ar_complete_instruction_parser__parse(
    ar_complete_instruction_parser_t *mut_parser,
    const char *ref_instruction,
    const char *ref_result_path
);
```

## Current implementation notes

- the parser is implemented in Zig with a stable C-facing header
- successful parses produce a function-call AST with function name `"complete"`
- argument ASTs are stored on the returned instruction AST so the evaluator can re-use the parsed
  template string and optional base-path expression without reparsing text
- invalid placeholder syntax, unsupported interpolation shapes, and non-memory second arguments are
  rejected during parsing rather than deferred to runtime generation
- the parser itself does not perform model/runtime work; it only validates AgeRun syntax and builds
  AST state for `ar_complete_instruction_evaluator`
