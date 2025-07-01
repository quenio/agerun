# Method Parser Module

## Overview

The method parser module is responsible for parsing method source code and creating the corresponding Abstract Syntax Tree (AST) representation. It transforms text-based method definitions into structured AST objects that can be executed by the interpreter.

## Architecture

### Opaque Type

- `ar_method_parser_t`: Parser instance that maintains parsing state and context

### Key Functions

#### Creation and Destruction

- `ar_method_parser__create(ar_log_t *ref_log)`: Creates a new parser instance
- `ar_method_parser__destroy()`: Destroys a parser instance

#### Parsing

- `ar_method_parser__parse()`: Parses method source code into an AST

#### Error Handling

- `ar_method_parser__get_error()`: Returns the last error message
- `ar_method_parser__get_error_line()`: Returns the line number where the error occurred

## Usage Example

```c
// Create a parser (with optional logging)
ar_log_t *own_log = ar_log__create(); // Optional - can be NULL
ar_method_parser_t *own_parser = ar_method_parser__create(own_log);
if (!own_parser) {
    // Handle allocation failure
    return;
}

// Use the parser (future functionality)
// ...

// Clean up
ar_method_parser__destroy(own_parser);
ar_log__destroy(own_log); // Only if log was created
```

## Memory Management

The module follows strict ownership rules:

- `ar_method_parser__create()` returns an owned parser that must be destroyed
- `ar_method_parser__destroy()` takes ownership and frees all resources
- The destroy function safely handles NULL pointers

## Implementation Status

- [x] Basic structure creation and destruction
- [x] Parse empty method
- [x] Parse single instruction
- [x] Parse multiple instructions
- [x] Skip empty lines
- [x] Skip comments (both full line and inline)
- [x] Error handling and reporting
- [ ] Integration with method module

## Design Decisions

1. **Opaque Type**: The parser structure is opaque to maintain encapsulation
2. **Stateful Parser**: The parser maintains state to support error reporting
3. **AST Output**: Produces method AST objects for execution
4. **Instruction Parser Integration**: Uses the instruction parser facade for parsing individual instructions
5. **Trimming**: Removes leading/trailing whitespace from method source before parsing
6. **Line-by-Line Parsing**: Splits source by newlines and parses each non-empty line as an instruction
7. **Empty Line Handling**: Automatically skips empty lines during parsing
8. **Line Ending Support**: Handles Unix (\n), Windows (\r\n), and Mac (\r) line endings
9. **Comment Support**: Supports `#` for comments (both full line and inline)
10. **Quote-Aware Parsing**: Correctly handles `#` inside quoted strings (not treated as comments)
11. **Error Reporting**: Provides line numbers and error messages for parse failures
12. **Error State Management**: Clears error state on successful parse