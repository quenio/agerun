# Method Parser Module

## Overview

The method parser module is responsible for parsing method source code and creating the corresponding Abstract Syntax Tree (AST) representation. It transforms text-based method definitions into structured AST objects that can be executed by the interpreter.

## Architecture

### Opaque Type

- `ar_method_parser_t`: Parser instance that maintains parsing state and context

### Key Functions

#### Creation and Destruction

- `ar_method_parser__create()`: Creates a new parser instance
- `ar_method_parser__destroy()`: Destroys a parser instance

## Usage Example

```c
// Create a parser
ar_method_parser_t *own_parser = ar_method_parser__create();
if (!own_parser) {
    // Handle allocation failure
    return;
}

// Use the parser (future functionality)
// ...

// Clean up
ar_method_parser__destroy(own_parser);
```

## Memory Management

The module follows strict ownership rules:

- `ar_method_parser__create()` returns an owned parser that must be destroyed
- `ar_method_parser__destroy()` takes ownership and frees all resources
- The destroy function safely handles NULL pointers

## Implementation Status

- [x] Basic structure creation and destruction
- [ ] Parse empty method
- [ ] Parse single instruction
- [ ] Parse multiple instructions
- [ ] Skip comments and empty lines
- [ ] Error handling and reporting
- [ ] Integration with method AST

## Design Decisions

1. **Opaque Type**: The parser structure is opaque to maintain encapsulation
2. **Stateful Parser**: The parser maintains state to support error reporting
3. **AST Output**: Will produce method AST objects for execution