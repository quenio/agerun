# AgeRun Instruction Parser Module

## Overview

The Instruction Parser module serves as a facade that coordinates all specialized instruction parsers in the AgeRun system. It provides both a unified interface (`ar_instruction_parser__parse()`) for parsing any type of instruction and legacy individual parse methods for backward compatibility. The parser automatically detects instruction types and dispatches to the appropriate specialized parser.

## Architecture

The module follows the facade pattern, maintaining instances of all specialized parsers:
- Assignment instruction parser
- Send instruction parser  
- Condition (if) instruction parser
- Parse instruction parser
- Build instruction parser
- Method instruction parser
- Agent instruction parser
- Destroy agent instruction parser
- Destroy method instruction parser

## Key Features

- **Unified Interface**: Single `ar_instruction_parser__parse()` function handles all instruction types
- **Automatic Detection**: Identifies instruction type by analyzing the input
- **Error Propagation**: Forwards errors from specialized parsers with context
- **Reusable**: Parser instances can be reused for multiple parse operations
- **Memory Safe**: Zero memory leaks with proper ownership management
- **Backward Compatible**: Legacy individual parse methods still available

## Instruction Types Supported

### Assignment Instructions
- Format: `memory.path := expression`
- Example: `memory.x := 42`
- Requires memory prefix for left-hand side

### Function Call Instructions
- **send**: `send(agent_id, message)` - Send message to agent
- **if**: `if(condition, true_value, false_value)` - Conditional expression
- **method**: `method(name, instructions, version)` - Create method
- **agent**: `agent(method_name, version, context)` - Create agent
- **destroy**: `destroy(agent_id)` or `destroy(method_name, version)` - Destroy agent/method
- **parse**: `parse(template, string)` - Parse string with template
- **build**: `build(template, map)` - Build string from template and map

Function calls can optionally have result assignment:
- `memory.result := send(0, "Hello")`

## API Reference

### Parser Creation and Destruction

```c
instruction_parser_t* ar__instruction_parser__create(void);
```
Creates a new parser instance that can be reused for multiple parse operations.

```c
void ar__instruction_parser__destroy(instruction_parser_t *own_parser);
```
Destroys a parser instance and frees all associated memory.

### Error Handling

```c
const char* ar__instruction_parser__get_error(const instruction_parser_t *ref_parser);
```
Returns the last error message, or NULL if no error occurred.

```c
size_t ar__instruction_parser__get_error_position(const instruction_parser_t *ref_parser);
```
Returns the character position where the last error occurred.

### Parse Methods

Each instruction type has its own parse method:

```c
ar_instruction_ast_t* ar__instruction_parser__parse_assignment(
    instruction_parser_t *mut_parser, 
    const char *ref_instruction
);
```

```c
ar_instruction_ast_t* ar__instruction_parser__parse_send(
    instruction_parser_t *mut_parser, 
    const char *ref_instruction, 
    const char *ref_result_path
);
```

```c
ar_instruction_ast_t* ar__instruction_parser__parse_if(
    instruction_parser_t *mut_parser, 
    const char *ref_instruction, 
    const char *ref_result_path
);
```

And similar methods for:
- `ar__instruction_parser__parse_method`
- `ar__instruction_parser__parse_agent`
- `ar__instruction_parser__parse_destroy`
- `ar__instruction_parser__parse_parse`
- `ar__instruction_parser__parse_build`

## Usage Examples

### Using the Unified Parser (Recommended)

```c
// Create parser
instruction_parser_t *parser = ar__instruction_parser__create();
if (!parser) {
    return;
}

// Parse any type of instruction
const char *instructions[] = {
    "memory.x := 42",
    "send(1, \"Hello\")",
    "if(memory.flag, \"yes\", \"no\")",
    "method(\"test\", \"code\", \"1.0\")",
    "agent(\"echo\", \"1.0\")",
    "destroy(1)",
    "parse(\"{x}\", \"x=42\")",
    "build(\"Hello {name}\", memory.vars)"
};

for (int i = 0; i < 8; i++) {
    ar_instruction_ast_t *ast = ar_instruction_parser__parse(parser, instructions[i]);
    
    if (ast) {
        printf("Parsed: %s (type: %d)\n", instructions[i], 
               ar__instruction_ast__get_type(ast));
        ar__instruction_ast__destroy(ast);
    } else {
        printf("Failed to parse: %s - %s\n", instructions[i],
               ar__instruction_parser__get_error(parser));
    }
}

ar__instruction_parser__destroy(parser);
```

### Parsing an Assignment

```c
// Create parser
instruction_parser_t *parser = ar__instruction_parser__create();
if (!parser) {
    // Handle error
    return;
}

// Parse assignment
const char *instruction = "memory.count := 42";
ar_instruction_ast_t *ast = ar__instruction_parser__parse_assignment(parser, instruction);

if (ast) {
    // Use the AST
    assert(ar__instruction_ast__get_type(ast) == AR_INST__ASSIGNMENT);
    
    // Clean up
    ar__instruction_ast__destroy(ast);
} else {
    // Handle parse error
    const char *error = ar__instruction_parser__get_error(parser);
    size_t position = ar__instruction_parser__get_error_position(parser);
    fprintf(stderr, "Parse error at position %zu: %s\n", position, error);
}

// Destroy parser
ar__instruction_parser__destroy(parser);
```

### Parsing Function Calls

```c
// Parse a send without assignment
ar_instruction_ast_t *ast1 = ar__instruction_parser__parse_send(
    parser, "send(0, \"Hello\")", NULL
);

// Parse a send with assignment
ar_instruction_ast_t *ast2 = ar__instruction_parser__parse_send(
    parser, "memory.result := send(1, \"Test\")", "memory.result"
);

// Parse an if expression
ar_instruction_ast_t *ast3 = ar__instruction_parser__parse_if(
    parser, "memory.status := if(count > 5, \"High\", \"Low\")", "memory.status"
);
```

### Parser Reusability

```c
instruction_parser_t *parser = ar__instruction_parser__create();

// Parse multiple instructions with the same parser
ar_instruction_ast_t *ast1 = ar__instruction_parser__parse_assignment(parser, "memory.x := 10");
ar__instruction_ast__destroy(ast1);

ar_instruction_ast_t *ast2 = ar__instruction_parser__parse_assignment(parser, "memory.y := 20");
ar__instruction_ast__destroy(ast2);

ar_instruction_ast_t *ast3 = ar__instruction_parser__parse_send(parser, "send(0, \"Done\")", NULL);
ar__instruction_ast__destroy(ast3);

ar__instruction_parser__destroy(parser);
```

## Design Decisions

### No General Parse Function
Unlike some parsers, this module does not provide a general `parse_instruction` function. Instead, callers must use the specific parse method for the instruction type they expect. This design:
- Provides better type safety
- Makes error handling more explicit
- Allows for instruction-specific parameters (like result_path)

### Reusable Parser Instances
The parser is designed to be created once and used multiple times. This:
- Reduces allocation overhead
- Allows for potential future optimizations (caching, etc.)
- Follows the pattern established by the expression_parser module

### Separate Parse Methods
Each instruction type has its own parse method because:
- Different instructions have different syntactic rules
- Some functions support optional result assignment, others don't
- Error messages can be more specific
- The API clearly shows what instruction types are supported

## Memory Management

The module follows the AgeRun Memory Management Model:

1. **Parser Lifetime**: Create parser once, use many times, destroy when done
2. **AST Ownership**: Parse methods return owned AST nodes that caller must destroy
3. **Error Messages**: Error messages are owned by the parser and valid until next parse
4. **String Parameters**: All string parameters are borrowed references
5. **Thread Safety**: Parser instances are not thread-safe

## Error Handling

Common parse errors include:
- Missing "memory" prefix in assignments
- Invalid assignment operator (using `=` instead of `:=`)
- Mismatched parentheses or quotes
- Wrong number of arguments for functions
- Malformed expressions
- Empty instructions

Error messages include the position where the error occurred, making it easier to provide helpful feedback to users.

## Testing

The module includes comprehensive tests covering:
- All instruction types
- Assignment variations (simple, nested paths, expressions)
- Function calls with and without result assignment
- Error cases and edge conditions
- Parser reusability
- Whitespace handling
