# AgeRun Development Guide for Claude

This document contains essential instructions for Claude Code to assist with AgeRun development.

## Project Overview

AgeRun is a lightweight, message-driven agent system where each agent is defined by a single method responsible for handling all incoming messages. Agents operate within a persistent runtime environment, allowing dynamic creation, versioning, pausing, resumption, and destruction.

## Quick Start

### Clean Build Script
Use `./clean_build.sh` for comprehensive build verification with minimal output (~20 lines):
- Cleans, builds, and runs static analysis
- Runs all tests (shows count only)
- Runs executable and sanitizer tests
- Reports memory leak status

**When to use**: Before commits, for quick verification, or when context space is limited.

### Detailed Build Commands
For detailed output during development:
```bash
make clean && make          # Clean and build
make analyze               # Static analysis on library
make analyze-tests         # Static analysis on tests
make test                  # Run tests (auto-cleans and rebuilds in debug)
make run                   # Run executable (auto-builds in debug)
make test-sanitize         # Run tests with ASan
make run-sanitize          # Run executable with ASan
```

**Key Points**:
- Always run from repository root
- Tests and executable run from `bin` directory automatically
- Debug mode is enabled by default for development
- Documentation-only changes don't require testing
- The Makefile automatically:
  - Runs `make clean debug` before tests to ensure heap tracking
  - Changes to `bin` directory for test/run targets
  - Builds with debug assertions active

## Project Structure

- **/modules**: Core implementation (.c/.h files and tests)
- **/bin**: Generated binaries (ignored by git, NEVER read these files)
- **/methods**: Method definitions (.method files with docs and tests)

## Critical Development Rules

### 1. Memory Management (ZERO TOLERANCE FOR LEAKS)

**Mandatory Practices**:
- Use heap tracking macros: `AR_HEAP_MALLOC`, `AR_HEAP_FREE`, `AR_HEAP_STRDUP`
- Include `#include "agerun_heap.h"` in all .c files that allocate memory
- Follow ownership naming conventions:
  - `own_`: Owned values that must be destroyed
  - `mut_`: Mutable references (read-write access)
  - `ref_`: Borrowed references (read-only access)
  - Apply to variables, parameters, and struct fields
  - Consistent across .h, .c, and _tests.c files
- Set pointers to NULL after ownership transfer
- Add `// Ownership transferred to caller` comment at return statements
- Follow the Memory Management Model (MMM.md) strictly
- Use debug assertions strategically:
  - `AR_ASSERT_OWNERSHIP()` for critical resource allocations
  - `AR_ASSERT_TRANSFERRED()` for complex ownership transfers
  - `AR_ASSERT_NOT_USED_AFTER_FREE()` for complex reuse patterns
  - See agerun_assert.md for complete guidelines
- Expression evaluation ownership rules:
  - Memory access (`memory.x`): returns reference, do NOT destroy
  - Arithmetic (`2 + 3`, `memory.x + 5`): returns new object, MUST destroy
  - String operations (`"Hello" + " World"`): returns new object, MUST destroy
  - Use `ar_expression_take_ownership()` to check ownership

**Memory Leak Detection**:
- Full test suite: Check console for "WARNING: X memory leaks detected"
- Individual debugging: Check `bin/heap_memory_report.log` after each test
  - **CRITICAL**: The report file is overwritten on each program run
  - Workflow: Run test → Check heap_memory_report.log → Run next test
- Always run `make test-sanitize` before committing
- Environment variables for debugging:
  - `ASAN_OPTIONS=halt_on_error=0` to continue after first error
  - `ASAN_OPTIONS=detect_leaks=1:leak_check_at_exit=1` for complex leaks

### 2. Test-Driven Development (MANDATORY)

**Red-Green-Refactor Cycle**:
1. **Red**: Write failing test FIRST
2. **Green**: Write MINIMUM code to pass
3. **Refactor**: Improve while keeping tests green

**Test Requirements**:
- Every module MUST have tests
- Use Given/When/Then structure:
  ```c
  // Given a description of the test setup
  /* Setup code here */
  
  // When describing the action being tested
  /* Action code here */
  
  // Then describing the expected result
  /* Assertion code here */
  ```
- One test per behavior
- Tests must be isolated and fast
- Zero memory leaks in tests
- Test files: `<module>_tests.c`

### 3. Parnas Design Principles (STRICTLY ENFORCED)

**Core Principles**:
- **Information Hiding**: Hide design decisions behind interfaces
- **Single Responsibility**: One module, one concern
- **No Circular Dependencies**: Uses hierarchy must be strict
- **Opaque Types**: Required for complex data structures
- **Minimal Interfaces**: Expose only what's necessary
- **Complete Documentation**: Every module must be fully documented

**Enforcement**: Violations result in automatic PR rejection.

### 4. Coding Standards

**Formatting**:
- 4-space indentation (no tabs)
- 100-character line limit
- ALL files must end with newline
  - C standard requires newline at EOF
  - Missing newlines cause compiler errors with -Wall -Werror
  - Always verify file ends with '\n' after edits
  - Use `./add_newline.sh <filename>` helper script to ensure proper EOF
- Function prefix: `ar_`
- Type suffix: `_t`

**Documentation**:
- Use `/**` style for all public APIs
- Include `@param`, `@return`, `@note` tags
- Document ownership semantics clearly
- Write complete sentences with punctuation
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

**File Operations**:
- Use IO module functions, not raw stdio
- `ar_io_open_file` instead of `fopen`
- `ar_io_close_file` instead of `fclose`
- `ar_io_fprintf` instead of `fprintf`
- `ar_io_read_line` instead of `fgets` or `getline`
- Check all return codes
- Use `ar_io_create_backup` before modifying critical files
- Use `ar_io_write_file` for atomic operations
- Use `ar_io_set_secure_permissions` for sensitive files

### 5. Module Development

**Code Modification Process**:
1. Understand codebase structure and dependencies
2. Make incremental changes with frequent compilation
3. Complete one functional area before moving to next
4. Verify each change with tests

**Opaque Types**:
- Forward declare in header: `typedef struct name_s name_t;`
- Define structure in .c file only
- Provide `ar_name_create()` and `ar_name_destroy()`
- No separate init functions
- Update all dependent code to use public API
- Avoid size-exposing functions
- Provide accessor functions for internal state
- Handle NULL checks consistently

**Best Practices**:
- Make incremental changes with frequent compilation
- Remove unused functions immediately
- Handle all compiler warnings (-Wall -Werror)
- Update documentation with code changes
  - Update modules/README.md when creating new modules
- Avoid code smells (see below)
- When showing code, provide only the raw code without commentary

**Common Code Smells to Avoid**:
- **Long Function**: Keep functions under 50 lines, single responsibility
- **Large Module**: Split modules with multiple unrelated concerns
- **Duplicate Code**: Extract common functionality, follow DRY principle
- **Long Parameter List**: Limit to 4-5 parameters, use structs for groups
- **Feature Envy**: Move functions to modules whose data they use most
- **Primitive Obsession**: Create typedefs/structs for domain concepts
- **Inappropriate Intimacy**: Use opaque types for proper encapsulation
- **Speculative Generality**: Remove unused code, implement only what's needed
- **Switch Statements**: Consider function pointer tables or dispatch tables
- **Message Chains**: Apply Law of Demeter (a->b()->c()->d() is bad)
- **Comments**: Refactor code to be self-documenting; comments explain "why" not "what"
- **Helper Functions**: No helper functions allowed - they indicate missing modules or data abstractions

### 6. Method Development

**Requirements**:
- Store in `methods/` as `<name>-<version>.method`
- Create corresponding `.md` documentation
- Create `<name>_tests.c` test file
- Tests must verify memory state after execution
- Use relative path `../methods/` from bin directory

**Method Language Rules**:
- Expressions: literals, memory access, arithmetic
- Instructions: assignments, function calls
- Function calls cannot be nested
- `if()` cannot be nested
- `send(0, message)` is a no-op returning true

### 7. Debug and Analysis

**Memory Debugging**:
- Use ASan via `make test-sanitize`
- Check `bin/heap_memory_report.log` for leaks
- Add DEBUG output (keep it for future sessions)
- Use static analyzer: `make analyze`
  - HTML reports in `bin/scan-build-results` (if scan-build installed)
  - Console output otherwise

**Guidelines**:
- NEVER read binary files (.o, executables)
- NEVER access /bin directory contents
- Focus on source files only
- Address all warnings immediately

**Binary File Restriction - STRICT ENFORCEMENT**:
- Before using the Read tool, ALWAYS check:
  1. Is the path in /bin directory? → REFUSE
  2. Does the file have a binary extension? → REFUSE
- Forbidden file types include but are not limited to:
  - Object files: .o, .obj
  - Libraries: .a, .lib, .so, .dylib, .dll
  - Executables: .exe, .out, files without extension in /bin
  - Compiled outputs: .pyc, .pyo, .class
  - Any file in /bin directory regardless of extension
- ONLY read source files: .c, .h, .md, .txt, .method, .agerun (config only)
- When in doubt, assume it's binary and don't read it

### 8. Agent Lifecycle

**Critical Points**:
- Agents receive `__wake__` on creation
- Agents receive `__sleep__` before destruction
- ALWAYS process messages after sending to prevent leaks
- Call `ar_system_process_next_message()` after `ar_agent_send()`

### 9. Building Individual Tests

Always use make to build tests:
```bash
make bin/test_name  # Build individual test
```
Never compile directly with gcc.

### 10. Session Management

When reviewing tasks:
- Check session todo list with `TodoRead`
- Check `TODO.md` file in repository
- Keep CLAUDE.md updated with new guidelines

### 11. Documentation Guidelines

**Understand Documentation Purpose**:
- **Developer Documentation** (README.md files in directories):
  - SHOULD contain implementation details, patterns, and examples
  - Intended for developers working on the codebase
  - Example: `methods/README.md` should include test writing patterns
  
- **API/Module Documentation** (individual .md files):
  - Should focus on public interface and usage
  - Should NOT contain implementation details
  - Example: `echo-1.0.0.md` describes the echo method's behavior, not how to test it

- **Project Documentation** (root-level .md files):
  - CLAUDE.md: Development guidelines and patterns
  - TODO.md: Task tracking and project status
  - README.md: Project overview and quick start

**Before Modifying Documentation**:
1. Identify the file's purpose and target audience
2. Consider whether the content is appropriate for that audience
3. Implementation details belong in developer-facing documentation
4. Don't remove content without understanding why it's there

**Common Documentation Patterns**:
- Directory README files often contain implementation guidance
- Individual component documentation focuses on usage
- Test patterns belong in developer documentation, not API docs

**Tool Result Interpretation**:
- Don't act mechanically on tool results without understanding context
- When tools report "issues", verify they are actually problems
- Consider the purpose of the file before making changes
- Question whether reported patterns are appropriate for their location

## AgeRun Language Notes

- No null type - use integer 0
- All parameters required
- Version strings explicit (e.g., "1.0.0")
- Map literals only in assignments
- Agent ID 0 indicates failure
- Always process `__wake__` messages
- Always process messages after sending to prevent memory leaks

## Method Test Template

Method tests should use the test fixture module to handle setup and teardown:

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "agerun_test_fixture.h"
#include "agerun_system.h"
#include "agerun_agent.h"
#include "agerun_data.h"

static void test_method_example(void) {
    printf("Testing method-name functionality...\n");
    
    // Create test fixture
    test_fixture_t *own_fixture = ar_test_fixture_create("test_name");
    assert(own_fixture != NULL);
    
    // Initialize test environment
    assert(ar_test_fixture_initialize(own_fixture));
    
    // Verify correct directory
    assert(ar_test_fixture_verify_directory(own_fixture));
    
    // Load required methods
    assert(ar_test_fixture_load_method(own_fixture, "method-name", 
                                       "../methods/method-name-1.0.0.method", "1.0.0"));
    
    // Create agent
    agent_id_t agent = ar_agent_create("method-name", "1.0.0", NULL);
    assert(agent > 0);
    
    // Process wake message
    ar_system_process_next_message();
    
    // ... test code ...
    
    // Check for memory leaks
    assert(ar_test_fixture_check_memory(own_fixture));
    
    // Destroy fixture (handles all cleanup)
    ar_test_fixture_destroy(own_fixture);
    
    printf("✓ Test passed\n");
}

int main(void) {
    printf("Running method-name tests...\n\n");
    
    test_method_example();
    
    printf("\nAll tests passed!\n");
    return 0;
}
```

**Key Points**:
- Use `ar_test_fixture_create()` to create fixture with unique test name
- Call `ar_test_fixture_initialize()` to set up clean environment
- Use `ar_test_fixture_verify_directory()` to ensure tests run from bin/
- Load methods with `ar_test_fixture_load_method()`
- Always check memory with `ar_test_fixture_check_memory()`
- Call `ar_test_fixture_destroy()` to clean up everything
- No manual cleanup code needed - fixture handles it all

## Quick Reference

**Never**:
- Read binary files or /bin contents
- Create circular dependencies
- Write code without tests
- Leave memory leaks
- Remove final newlines
- Use raw file operations
- Create files unless necessary

**Always**:
- Use heap tracking macros
- Follow TDD cycle
- Document ownership
- Run sanitizer before commit
- End files with newline
- Use make for builds
- Process messages after sending