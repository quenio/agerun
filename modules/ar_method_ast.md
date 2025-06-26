# Method AST Module (ar_method_ast)

## Overview

The Method AST module provides the Abstract Syntax Tree (AST) structure for parsed methods. It represents a method as a collection of parsed instruction ASTs, enabling separation between parsing (text to AST) and execution (AST evaluation).

## Key Concepts

- **Method AST**: A container holding a list of instruction ASTs representing the parsed instructions of a method
- **Instruction AST**: Individual parsed instructions (created by the instruction parser modules)
- **Line-based access**: Instructions can be accessed by line number (1-based) for error reporting

## Public Interface

### Types

```c
typedef struct ar_method_ast_s ar_method_ast_t;
```

An opaque type representing a parsed method.

### Functions

#### ar__method_ast__create
```c
ar_method_ast_t* ar__method_ast__create(void);
```
Creates a new empty method AST.

**Returns**: A newly created method AST (owned by caller), or NULL on failure
**Ownership**: Caller must destroy the returned AST using `ar__method_ast__destroy()`

#### ar__method_ast__destroy
```c
void ar__method_ast__destroy(ar_method_ast_t* own_ast);
```
Destroys a method AST and all instruction ASTs it contains.

**Parameters**:
- `own_ast`: The method AST to destroy (ownership transferred)

**Ownership**: Takes ownership of the AST and all contained instruction ASTs
**Note**: Safe to call with NULL

#### ar__method_ast__add_instruction
```c
void ar__method_ast__add_instruction(ar_method_ast_t* mut_ast, ar_instruction_ast_t* own_instruction);
```
Adds an instruction AST to the method AST.

**Parameters**:
- `mut_ast`: The method AST to add to (mutable reference)
- `own_instruction`: The instruction AST to add (ownership transferred)

**Ownership**: Takes ownership of the instruction AST
**Note**: If either parameter is NULL, the instruction is destroyed to prevent memory leaks

#### ar__method_ast__get_instruction_count
```c
size_t ar__method_ast__get_instruction_count(const ar_method_ast_t* ref_ast);
```
Gets the number of instructions in the method AST.

**Parameters**:
- `ref_ast`: The method AST to query (borrowed reference)

**Returns**: The number of instructions (0 if AST is NULL)

## Design Decisions

1. **Container Pattern**: The method AST is purely a container for instruction ASTs, with no knowledge of method names or versions (handled by the method module)

2. **List Storage**: Uses the list module internally to store instruction ASTs in order

3. **Ownership Model**: The method AST owns all instruction ASTs it contains and is responsible for their cleanup

4. **Future Line Numbers**: Designed to support line-based access (1-based) for better error reporting, though accessor functions are not yet implemented

## Usage Example

```c
// Create a method AST
ar_method_ast_t *own_ast = ar__method_ast__create();
if (!own_ast) {
    // Handle error
}

// Parse and add instructions
instruction_parser_t *parser = ar__instruction_parser__create();
ar_instruction_ast_t *own_instr = ar_instruction_parser__parse(parser, "memory.x := 42");
ar__method_ast__add_instruction(own_ast, own_instr);

// Check instruction count
size_t count = ar__method_ast__get_instruction_count(own_ast);
printf("Method has %zu instructions\n", count);

// Cleanup
ar__instruction_parser__destroy(parser);
ar__method_ast__destroy(own_ast);
```

## Dependencies

- `ar_heap.h`: Memory management
- `ar_list.h`: List data structure for storing instructions
- `ar_instruction_ast.h`: Instruction AST types

## Future Enhancements

The following functions are planned for upcoming TDD cycles:
- `ar__method_ast__get_instruction()`: Access instruction by line number (1-based)

## Memory Management

The module follows the AgeRun Memory Management Model:
- Uses heap tracking macros for all allocations
- Properly destroys all owned instruction ASTs during cleanup
- Zero memory leaks verified through testing