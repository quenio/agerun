# Agerun Development Guide for Claude

This document contains instructions and guidance for Claude Code to assist with the development of the Agerun project.

## Project Overview

Agerun is a lightweight, message-driven agent system where each agent is defined by a single method responsible for handling all incoming messages. Agents operate within a persistent runtime environment, allowing dynamic creation, versioning, pausing, resumption, and destruction.

## Build and Test

When making changes, always follow these steps to ensure code quality:

1. **Build the project**:
   ```
   make
   ```

2. **Build the example application**:
   ```
   make example
   ```

3. **Run tests**:
   ```
   make test
   ```

## Project Structure

- **/include**: Header files
  - `agerun.h`: Public API
  - `agerun_interpreter.h`: Interpreter interface

- **/src**: Implementation files
  - `agerun.c`: Core runtime implementation
  - `agerun_interpreter.c`: Instruction language interpreter

- **/examples**: Example applications
  - `example.c`: Simple example demonstrating usage

- **/tests**: Test files
  - `test_basics.c`: Basic functionality tests

## Coding Style Guidelines

1. **Formatting**:
   - 4-space indentation
   - No tabs, only spaces
   - Maximum line length of 100 characters

2. **Naming Conventions**:
   - Function names: `lowercase_with_underscores`
   - Variables: `lowercase_with_underscores`
   - Constants and macros: `UPPERCASE_WITH_UNDERSCORES`
   - Types: `lowercase_with_underscores_t` (with `_t` suffix)
   - Global variables: `g_lowercase_with_underscores`

3. **Comments**:
   - Use `//` for single-line comments
   - Use `/* */` for multi-line comments
   - All functions should have a descriptive comment
   - Comment complex logic within functions

4. **Memory Management**:
   - Always check return values from memory allocation functions
   - Free allocated memory in the appropriate scope
   - Use valgrind to check for memory leaks

## Implementation Notes

1. **Agent Lifecycle**:
   - Agents receive `__wake__` on creation/resumption
   - Agents receive `__sleep__` before pausing/destruction

2. **Instruction Language**:
   - Parse and build string operations
   - Memory access and modification
   - Message sending
   - Conditional evaluation
   - Agent management (creation, destruction)

3. **Persistence**:
   - Methods are stored in `agrun.methods`
   - Agents are stored in `agrun.agents` if persistent

## Future Enhancements

1. **Instruction Language Improvements**:
   - Better error handling and reporting
   - More comprehensive string operations
   - Support for arrays and complex data structures

2. **Runtime Improvements**:
   - Multithreading support
   - Better resource management
   - Network support for distributed agents

3. **Tools and Utilities**:
   - CLI tool for managing agents
   - Debugger for agent methods
   - Visualization of agent relationships

## Debugging Tips

1. **Add logging**:
   - Use printf statements for debugging
   - Include agent ID, method name, and version in logs

2. **Use gdb**:
   - Set breakpoints in key functions
   - Inspect memory state at runtime

3. **Memory issues**:
   - Use valgrind to check for memory leaks
   - Use AddressSanitizer for detecting memory errors