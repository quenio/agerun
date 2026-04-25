# ar_instruction_ast

The `ar_instruction_ast` module defines the parsed instruction nodes used throughout the AgeRun
parser and evaluator pipeline.

## Responsibility

- represent every supported instruction as an opaque AST node
- store either assignment data or function-call data
- preserve both original string arguments and parsed expression/instruction metadata
- provide ownership-safe creation, accessor, and destruction helpers

## Supported instruction types

The public enum `ar_instruction_ast_type_t` covers:

- `AR_INSTRUCTION_AST_TYPE__ASSIGNMENT`
- `AR_INSTRUCTION_AST_TYPE__SEND`
- `AR_INSTRUCTION_AST_TYPE__IF`
- `AR_INSTRUCTION_AST_TYPE__COMPILE`
- `AR_INSTRUCTION_AST_TYPE__SPAWN`
- `AR_INSTRUCTION_AST_TYPE__EXIT`
- `AR_INSTRUCTION_AST_TYPE__DEPRECATE`
- `AR_INSTRUCTION_AST_TYPE__PARSE`
- `AR_INSTRUCTION_AST_TYPE__BUILD`
- `AR_INSTRUCTION_AST_TYPE__COMPLETE`

`AR_INSTRUCTION_AST_TYPE__COMPLETE` represents `complete(template[, values])` and is used by
both the specialized complete parser and the instruction-evaluator facade.

## Public API highlights

### Node creation

```c
ar_instruction_ast_t* ar_instruction_ast__create_assignment(
    const char *ref_memory_path,
    const char *ref_expression
);

ar_instruction_ast_t* ar_instruction_ast__create_function_call(
    ar_instruction_ast_type_t type,
    const char *ref_function_name,
    const char **ref_args,
    size_t arg_count,
    const char *ref_result_path
);
```

### Assignment accessors

```c
const char* ar_instruction_ast__get_assignment_path(const ar_instruction_ast_t *ref_node);
const char* ar_instruction_ast__get_assignment_expression(const ar_instruction_ast_t *ref_node);
const ar_expression_ast_t* ar_instruction_ast__get_assignment_expression_ast(
    const ar_instruction_ast_t *ref_node
);
bool ar_instruction_ast__set_assignment_expression_ast(
    ar_instruction_ast_t *mut_node,
    ar_expression_ast_t *own_expression_ast
);
```

### Function-call accessors

```c
const char* ar_instruction_ast__get_function_name(const ar_instruction_ast_t *ref_node);
ar_list_t* ar_instruction_ast__get_function_args(const ar_instruction_ast_t *ref_node);
const ar_list_t* ar_instruction_ast__get_function_arg_asts(const ar_instruction_ast_t *ref_node);
bool ar_instruction_ast__set_function_arg_asts(
    ar_instruction_ast_t *mut_node,
    ar_list_t *own_arg_asts
);
const char* ar_instruction_ast__get_function_result_path(const ar_instruction_ast_t *ref_node);
bool ar_instruction_ast__has_result_assignment(const ar_instruction_ast_t *ref_node);
bool ar_instruction_ast__has_protected_memory_self_assignment(
    const ar_instruction_ast_t *ref_node
);
```

## Ownership notes

- all creation helpers return owned AST nodes
- `ar_instruction_ast__destroy()` takes ownership of the node and recursively frees its contents
- `ar_instruction_ast__get_function_args()` allocates and returns a new list; the caller must
  destroy that list with `ar_list__destroy()`
- `ar_instruction_ast__get_function_arg_asts()` returns a borrowed list reference
- setter helpers such as `ar_instruction_ast__set_assignment_expression_ast()` and
  `ar_instruction_ast__set_function_arg_asts()` take ownership of the supplied AST objects/lists

## Current implementation notes

- the module is implemented in Zig behind the stable `ar_instruction_ast.h` header
- instruction facades use `ar_instruction_ast__get_type()` for dispatch
- evaluators use `ar_instruction_ast__has_protected_memory_self_assignment()` to reject writes to
  agency-managed `memory.self` and `memory.self.*` result paths
- `complete(...)` support is represented with a normal function-call node carrying parsed argument
  ASTs for the template string and optional base path
- callers should treat the node as opaque and rely only on the documented accessor helpers
