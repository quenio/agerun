# ar_complete_instruction_parser

The `ar_complete_instruction_parser` module parses `complete(...)` instructions.

## Planned responsibility

- parse one-argument and two-argument `complete(...)` calls
- validate supported placeholder syntax and direct `memory...` base paths
- produce `AR_INSTRUCTION_AST_TYPE__COMPLETE` function-call AST nodes
