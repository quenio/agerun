# Expression Parser Module

## Overview

The expression parser module (`ar_expression_parser`) provides a recursive descent parser for converting AgeRun expression strings into Abstract Syntax Trees (ASTs). It uses the expression AST module to represent parsed expressions and implements proper operator precedence and associativity rules.

## Purpose

The expression parser module serves as the foundation for separating expression parsing from evaluation. It provides:

1. **Stateful Parsing**: Opaque parser structure that tracks parsing position and errors
2. **Recursive Descent**: Clean implementation of operator precedence through recursive functions
3. **Error Reporting**: Detailed error messages with position information
4. **Complete Parsing**: Support for all AgeRun expression types with proper precedence

## Design Principles

### Opaque Parser Structure

The module uses an opaque type to hide parsing state:

```c
typedef struct expression_parser_s ar_expression_parser_t;
```

The parser maintains:
- A copy of the expression string being parsed
- Current position in the string
- Error message for parse failures

### Operator Precedence

The parser implements proper operator precedence (from highest to lowest):

1. **Primary**: Literals, memory access, parenthesized expressions
2. **Multiplicative**: `*`, `/`
3. **Additive**: `+`, `-`
4. **Relational**: `<`, `<=`, `>`, `>=`, `<>` (not equal)
5. **Equality**: `=`

### Recursive Descent Architecture

Each precedence level has its own parsing function:
- `_parse_primary()` - Literals, memory access, parentheses
- `_parse_term()` - Multiplication and division
- `_parse_additive()` - Addition and subtraction
- `_parse_relational()` - Comparison operators
- `_parse_equality()` - Equality operator

## API Functions

### Parser Lifecycle

- `ar_expression_parser__create(ar_log_t *log, const char *expression)` - Creates parser instance with error logging
- `ar_expression_parser__destroy(parser)` - Destroys parser instance

### Parser State

- `ar_expression_parser__get_position(parser)` - DEPRECATED: Always returns 0
- `ar_expression_parser__get_error(parser)` - DEPRECATED: Always returns NULL. Use ar_log for error reporting

### Parsing Functions

- `ar_expression_parser__parse_expression(parser)` - Main entry point, parses complete expression
- `ar_expression_parser__parse_literal(parser)` - Parses integer, double, or string literals
- `ar_expression_parser__parse_memory_access(parser)` - Parses memory/message/context access
- `ar_expression_parser__parse_arithmetic(parser)` - Parses arithmetic expressions
- `ar_expression_parser__parse_comparison(parser)` - Parses comparison expressions

## Supported Expression Types

### Literals

- **Integer**: `42`, `-10`, `0`
- **Double**: `3.14`, `-2.5`, `0.0`
- **String**: `"hello"`, `"world"`, `""`

### Memory Access

- **Simple**: `memory`, `message`, `context`
- **With Path**: `memory.x`, `message.user.name`, `context.request_id`

### Binary Operations

- **Arithmetic**: `2 + 3`, `x * 5`, `a - b`, `c / d`
- **Comparison**: `x > 5`, `a <= b`, `name = "test"`, `x <> y`

### Parenthesized Expressions

- `(2 + 3) * 4` - Override default precedence
- `((a + b) * c) / d` - Nested parentheses

## Error Handling

The parser reports errors through the ar_log instance provided during creation:

```c
ar_log_t *log = ar_log__create();
ar_expression_parser_t *parser = ar_expression_parser__create(log, "2 + + 3");
ar_expression_ast_t *ast = ar_expression_parser__parse_expression(parser);
if (!ast) {
    // Check the log for error details
    // The get_error() function is deprecated and always returns NULL
}
```

Common errors:
- Unterminated string literals
- Invalid number formats
- Missing closing parentheses
- Unexpected characters
- Invalid identifiers after dots

## Memory Management

The parser follows strict ownership semantics:

1. **Parser Creation**: Makes a copy of the expression string
2. **AST Creation**: Returns owned AST nodes that caller must destroy
3. **Error Messages**: Reported through ar_log instance (get_error() is deprecated)
4. **Path Arrays**: Temporary arrays properly cleaned up during parsing

## Usage Example

```c
// Create parser
ar_expression_parser_t *own_parser = ar__expression_parser__create("memory.x + 5");

// Parse expression
ar_expression_ast_t *own_ast = ar__expression_parser__parse_expression(own_parser);
if (!own_ast) {
    // Errors are reported via the ar_log instance provided during creation
    // The get_error() function is deprecated and always returns NULL
    ar__expression_parser__destroy(own_parser);
    return;
}

// Use the AST
assert(ar_expression_ast__get_type(own_ast) == AR_EXPR__BINARY_OP);
assert(ar_expression_ast__get_operator(own_ast) == AR_OP__ADD);

// Clean up
ar_expression_ast__destroy(own_ast);
ar__expression_parser__destroy(own_parser);
```

## Dependencies

- **ar_expression_ast** - For AST node structures and creation
- **ar_heap** - For memory allocation and tracking
- **ar_list** - For managing path components during parsing
- **ar_string** - For string manipulation utilities

## Testing

The module includes comprehensive tests (`ar_expression_parser_tests.c`) that verify:

- Parsing of all literal types
- Memory access with various path depths
- All binary operators with correct precedence
- Parenthesized and nested expressions
- Error handling through ar_log (get_error/get_error_position are deprecated)
- NULL safety
- Zero memory leaks (234 allocations, all freed)

All tests follow the Given/When/Then pattern and achieve 100% coverage of parsing functionality.