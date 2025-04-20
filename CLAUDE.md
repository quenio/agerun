# AgeRun Development Guide for Claude

This document contains instructions and guidance for Claude Code to assist with the development of the AgeRun project.

## Project Overview

AgeRun is a lightweight, message-driven agent system where each agent is defined by a single method responsible for handling all incoming messages. Agents operate within a persistent runtime environment, allowing dynamic creation, versioning, pausing, resumption, and destruction.

## Build and Test

When making changes, always follow these steps to ensure code quality:

1. **Build the project**:
   ```
   make
   ```

2. **Build the executable application**:
   ```
   make executable
   ```

3. **Run tests**:
   ```
   make test
   ```
   (The Makefile handles changing to the bin directory)

4. **Run the executable**:
   ```
   make run
   ```
   (The Makefile handles changing to the bin directory)

IMPORTANT: 
- Always run the executable after running tests to verify changes work in practical application contexts, not just in test environments.
- Always run `make` from the top-level of the repository.
- Always run tests and the executable with the `bin` directory as the current directory.
- The Makefile has been set up to handle this directory requirement:
  - The `test` target automatically changes to the `bin` directory before running tests
  - The `run` target automatically changes to the `bin` directory before running the executable
  - When running these targets, you can simply use `make test` and `make run` from the top level

## Project Structure

- **/modules**: Implementation, header files, and tests
  - `agerun_system.h`/`.c`: Public API and core runtime implementation
  - `agerun_interpreter.h`/`.c`: Interpreter interface and instruction language interpreter
  - `agerun_data.h`/`.c`: Data type definitions and operations
  - `agerun_string.h`/`.c`: String utility functions
  - `agerun_executable.h`/`.c`: Main executable implementation
  - `agerun_*_tests.c`: Test files for each module

- **/bin**: Generated object files and executables
  - All compiled object files, libraries, and executable files are placed here
  - Main executable is generated as `bin/agerun`
  - This directory is ignored by git

## Coding Style Guidelines

1. **Formatting**:
   - 4-space indentation
   - No tabs, only spaces
   - Maximum line length of 100 characters
   - All source files must end with a newline

2. **Naming Conventions**:
   - Function names: `ar_lowercase_with_underscores` (with `ar_` prefix)
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
   - Methods are stored in `agerun.methods`
   - Agents are stored in `agerun.agents` if persistent


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

## Code Modification Guidelines

1. **Understand the codebase structure**:
   - Thoroughly explore dependencies between files before making changes
   - Identify all affected interfaces when modifying shared functionality

2. **Use proper header files**:
   - Share data structures and function declarations through headers
   - Use forward declarations or opaque pointers to break circular dependencies
   - Keep interface definitions consistent across modules
   - Prefer opaque types for complex data structures to improve encapsulation
   - When implementing opaque types:
     - Use a forward declaration in the header file: `typedef struct name_s name_t;`
     - Define the full structure in the implementation file
     - Provide a creation function (e.g., `ar_name_create()`) that handles both allocation and initialization
     - Avoid size-exposing functions (like `ar_name_size()`) as these break encapsulation
     - Remove any constants related to internal implementation details from header files
     - Provide a single creation function rather than separating allocation and initialization
     - Consolidate initialization logic directly into the creation function
     - Never expose separate initialization functions if there's only one way to initialize
     - Implement accessor functions for any internal state that needs to be exposed
     - Document opaque types by describing their purpose and behavior, not their implementation details
     - Use complete sentences with proper punctuation in all documentation

3. **Make incremental changes**:
   - Implement small changes with frequent compilation checks
   - Complete one functional area before moving to the next
   - Verify each change with tests before proceeding

4. **Use proper C techniques**:
   - Maintain consistent function signatures throughout development
   - Provide complete implementations or appropriate stubs for all functions
   - Use proper type declarations and avoid implicit conversions
   - Never add unused functions, and always remove them if found
   - Encapsulate implementation details using opaque types
   - Design clean, well-defined interfaces for modules
   - When working with opaque types, update tests to use the public API rather than accessing internal structures directly
   - For opaque types, provide a single creation function that handles both allocation and initialization

5. **Handle compiler warnings and errors**:
   - All compilation warnings and errors must be fixed before committing files
   - Consider temporarily disabling -Werror during development if needed
   - Address all warnings before finalizing code
   - Document reasons for any deliberately ignored warnings

6. **Test extensively**:
   - Test changes with both normal and edge cases
   - Verify behavior matches the specification
   - Use the test suite to catch regressions
   - EVERY module MUST have tests
   - For each module, create a test file with the suffix `_tests.c` in the `modules` directory
   - Name test files after the module they test (e.g., `agerun_data_tests.c` for testing `agerun_data.c`, `agerun_system_tests.c` for testing `agerun_system.c`)
   - Use white-box testing approaches that directly inspect the internal state of modules when needed
   - Encapsulation is about hiding implementation details from client code, not from tests
   - Each test function should implement a single scenario
   - Each test function should follow the Gherkin-style structure with Given/When/Then comments:
     ```c
     // Given a description of the test setup
     /* Setup code here */
     
     // When describing the action being tested
     /* Action code here */
     
     // Then describing the expected result
     /* Assertion code here */
     ```

7. **Update documentation appropriately**:
   - Always update documentation after making code changes
   - Make sure tests and executable work before updating documentation
   - Documentation updates should include README.md, SPEC.md, and any other relevant files
   - Never leave documentation in an inconsistent state with the code
   - Focus documentation on the purpose and behavior of components, not implementation details
   - Write all documentation as complete sentences with proper punctuation
   - For types and functions, describe what they do rather than how they are implemented

8. **Update development guidelines**:
   - When new important guidelines are discovered, add them to this file
   - Keep this CLAUDE.md file updated with current best practices
   - Consider this file the source of truth for development procedures
   - Documenting guidelines here ensures consistency across development sessions