# AgeRun Development Guide for Claude

This document contains instructions and guidance for Claude Code to assist with the development of the AgeRun project.

## Project Overview

AgeRun is a lightweight, message-driven agent system where each agent is defined by a single method responsible for handling all incoming messages. Agents operate within a persistent runtime environment, allowing dynamic creation, versioning, pausing, resumption, and destruction.

## Build and Test

When making changes, always follow these steps to ensure code quality:

1. **Clean and build the project**:
   ```
   make clean && make
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
- Always do a clean build before running tests and the executable to ensure all changes are properly incorporated.
- Always run the executable after running tests to verify changes work in practical application contexts, not just in test environments.
- Always run `make` from the top-level of the repository.
- Always run tests and the executable with the `bin` directory as the current directory.
- The Makefile has been set up to handle this directory requirement:
  - The `test` target automatically changes to the `bin` directory before running tests
  - The `run` target automatically changes to the `bin` directory before running the executable
  - When running these targets, you can simply use `make test` and `make run` from the top level
- When updating markdown files or other documentation, it is not necessary to run tests and executable as these changes don't affect functionality

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
   - IMPORTANT: Every text file MUST end with a newline character
     - The C standard requires all source files to end with a newline
     - This includes .c, .h, and documentation files
     - When creating new files, always add a final empty line
     - When using echo/printf to write files, end the content with '\n'
     - This avoids compiler warnings and errors about "no newline at end of file"

2. **Naming Conventions**:
   - Function names: `ar_lowercase_with_underscores` (with `ar_` prefix)
   - Variables: `lowercase_with_underscores`
   - Constants and macros: `UPPERCASE_WITH_UNDERSCORES`
   - Types: `lowercase_with_underscores_t` (with `_t` suffix)
   - Global variables: `g_lowercase_with_underscores`

3. **Comments and Documentation**:
   - Use `//` for single-line comments
   - Use `/* */` for multi-line comments
   - All functions should have a descriptive comment
   - Comment complex logic within functions
   - Standardized documentation comments:
     - All modules, types, and functions must have doc comments using the `/**` style
     - Function doc comments must include:
       - A clear description of what the function does
       - `@param` tags for all parameters with their name, purpose, and ownership semantics
       - `@return` tags explaining the return value and its ownership
       - `@note` tags for ownership transfer information
     - Type documentation must explain the type's purpose and ownership model
     - Doc comments should be consistent between header file declarations and implementations
     - Example:
       ```c
       /**
        * Processes data and creates a new result object
        * @param mut_context The context to use (mutable reference)
        * @param ref_data The data to process (borrowed reference)
        * @return A newly created result object
        * @note Ownership: Returns an owned value that caller must destroy.
        *       The function does not take ownership of the data parameter.
        */
       result_t* ar_module_process(context_t *mut_context, const data_t *ref_data);
       ```

4. **Memory Management**:
   - Always check return values from memory allocation functions
   - Free allocated memory in the appropriate scope
   - Use valgrind to check for memory leaks
   - When freeing containers/structures and their contents, always free containers first, then their contents
   - This prevents use-after-free bugs when containers might access their contents during cleanup
   - Use consistent variable name prefixes to denote ownership type:
     - Use `own_` prefix for owned values (RValues) that must be destroyed by the owner
     - Use `mut_` prefix for mutable references (LValues) that provide read-write access
     - Use `ref_` prefix for borrowed references (BValues) that provide read-only access
   - Be explicit about resource ownership in all function documentation:
     - Clearly document when ownership is transferred (e.g., `ar_data_set_map_data()` transfers ownership of the value)
     - Document when functions return references versus new objects
     - Comment variables that should not be used after ownership transfer (e.g., "Don't use after this point")
     - Mark transferred pointers with NULL after ownership transfer (e.g., `own_value = NULL;`)
     - Add a comment at every return statement that returns locally owned memory with the text "// Ownership transferred to caller"
     - This makes ownership transfer points explicitly visible in the implementation
   - For expression evaluation, follow these memory ownership rules:
     - Direct memory access expressions (e.g., `memory.x`) return references that should NOT be destroyed
     - Arithmetic expressions (e.g., `2 + 3`) return new objects that MUST be destroyed
     - Arithmetic expressions with memory access (e.g., `memory.x + 5`) return new objects that MUST be destroyed
     - String expressions (e.g., `"Hello" + " World"`) return new objects that MUST be destroyed
     - String+number concatenation (e.g., `"Price: $" + 42.99`) returns new objects that MUST be destroyed
   - Follow the AgeRun Memory Management Model (MMM) to ensure memory safety:
     - STRICTLY adhere to all rules in the MMM.md document
     - After transferring ownership of a pointer, NEVER use that pointer again
     - Immediately set transferred pointers to NULL after ownership transfer
     - Use the correct ownership prefix for ALL variables in the codebase
     - Document ALL ownership semantics in function comments
     - Include assertions in debug builds to verify ownership invariants
     - Test all ownership semantics thoroughly, including error cases
     - Treat ownership violations as critical issues that must be fixed immediately

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
   - Always place debug log files under the bin directory
   - Debug log files should be named with the pattern bin/debug*.log

2. **Use gdb**:
   - Set breakpoints in key functions
   - Inspect memory state at runtime

3. **Memory issues**:
   - Use valgrind to check for memory leaks
   - Use AddressSanitizer for detecting memory errors
   - Add debug print statements before and after critical memory operations
   - When debugging complex memory ownership patterns:
     - Add print statements with object addresses and types
     - Log object creation and destruction to track lifecycle
     - Isolate problematic sections by testing only one module at a time
     - Temporarily skip cleanup in non-critical sections to identify crash points
     - Check for double-free errors by setting pointers to NULL after freeing
     - Be careful with references vs. new objects in expression evaluation

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
     - Provide a complementary destruction function (e.g., `ar_name_destroy()`) that handles cleanup
     - Avoid size-exposing functions (like `ar_name_size()`) as these break encapsulation
     - Remove any constants related to internal implementation details from header files
     - Provide a single creation function rather than separating allocation and initialization
     - Consolidate initialization logic directly into the creation function
     - Never expose separate initialization functions if there's only one way to initialize
     - Implement accessor functions for any internal state that needs to be exposed
     - Document opaque types by describing their purpose and behavior, not their implementation details
     - Use complete sentences with proper punctuation in all documentation
     - Update all dependent modules to use the new opaque type API instead of direct structure access
     - Update tests to use the public API rather than accessing internal structure fields
     - Handle NULL pointer checks consistently in all accessor functions

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
   - Apply memory ownership naming conventions consistently across all files of a module:
     - Header files (.h): Function parameters should use the ownership prefixes (`own_`, `mut_`, `ref_`)
     - Implementation files (.c): Variables should use the ownership prefixes
     - Test files (_tests.c): Test code should also follow the same naming conventions
     - Struct field names should follow the same ownership prefix conventions:
       - `own_` prefix for fields the struct owns and must destroy
       - `mut_` prefix for mutable fields the struct doesn't own
       - `ref_` prefix for read-only references to data owned elsewhere
     - This consistent naming scheme makes ownership semantics explicit and transparent

5. **Handle compiler warnings and errors**:
   - All compilation warnings and errors must be fixed before committing files
   - Consider temporarily disabling -Werror during development if needed
   - Address all warnings before finalizing code
   - Document reasons for any deliberately ignored warnings
   - Remember that the project enables all compiler warnings with -Wall and treats warnings as errors with -Werror
   - Be especially careful with:
     - Type safety and casting (particularly const correctness)
     - Memory allocation and ownership
     - Potential null pointer dereferences
     - Unused variables and parameters
     - Missing return values
     - Strict aliasing violations
   - When dealing with third-party interfaces (like the map module), carefully follow the documented ownership model
   - Prefer explicit, well-documented approaches over clever tricks that might trigger warnings

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
   - For each module, create a separate markdown documentation file (e.g., `agerun_queue.md`) that includes:
     - An overview of the module's purpose
     - Key features and behaviors
     - A complete API reference with function descriptions
     - Usage examples demonstrating how to use the module
     - Implementation notes that describe design decisions but not specific details
     - For opaque type modules, emphasize the public API and avoid revealing internal structure details
   - When creating a new module, update the `modules/README.md` to:
     - Add the module to the Module Dependency Tree diagram
     - Add the module to the Module Layers diagram if it's a core or foundation module
     - Add a section describing the module with its key features and dependencies
     - Ensure dependency information in the tree accurately reflects the new module's relationships
     - Update any existing module descriptions that now depend on the new module

8. **Keep modules with minimal interfaces**:
   - Remove functions that are not used in the implementation code
   - Only expose what's truly needed by clients of the module
   - Aim for focused, cohesive modules with well-defined responsibilities
   - Smaller interfaces are easier to understand, test, and maintain
   - Avoid exposing functions that are only used in tests

9. **Update development guidelines**:
   - When new important guidelines are discovered, add them to this file
   - Keep this CLAUDE.md file updated with current best practices
   - Consider this file the source of truth for development procedures
   - Documenting guidelines here ensures consistency across development sessions

10. **Code Presentations**:
   - When asked to show "the actual code" for a structure declaration or similar, provide only the exact code without additional commentary
   - Present the raw code as it appears in the source files
   - For example, when asked to show a struct definition, show only the typedef struct {...} code block