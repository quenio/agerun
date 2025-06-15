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
- Always pause before executing build commands to check for custom scripts/procedures

## Git Workflow (CRITICAL)

**After Every Git Push**:
1. **ALWAYS run `git status`** to verify:
   - Push completed successfully
   - Working tree remains clean
   - No uncommitted changes remain

This is a MANDATORY verification step. Never assume a push succeeded without checking.

## Project Structure

- **/modules**: Core implementation (.c/.h files and tests)
- **/bin**: Generated binaries (ignored by git, NEVER read these files)
- **/methods**: Method definitions (.method files with docs and tests)

## Critical Development Rules

### 0. Documentation Search Protocol (MANDATORY)

**When asked about procedures or workflows**:
1. **ALWAYS search CLAUDE.md first** using exact keywords from the question
2. **Use multiple search strategies**:
   - Search for exact phrases (e.g., "after push", "after commit")
   - Search for related terms (e.g., "git", "workflow", "verify")
   - Check relevant sections based on topic
3. **Start with the simplest interpretation** of the question
4. **Do NOT assume complexity** where none exists
5. **Do NOT jump to related topics** without checking for direct answers first

**Common mistake pattern to avoid**:
- Question: "What should I do after X?"
- Wrong approach: Jumping to complex workflow sections
- Right approach: Search for "after X" first, check simple procedures

This prevents overthinking and ensures accurate responses based on documented procedures.

### 1. Memory Management (ZERO TOLERANCE FOR LEAKS)

**Mandatory Practices**:
- Use heap tracking macros: `AR__HEAP__MALLOC`, `AR__HEAP__FREE`, `AR__HEAP__STRDUP`
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
  - Use `ar__expression__take_ownership()` to check ownership
- Map iteration pattern:
  - Use `ar__data__get_map_keys()` to get all keys as a list
  - Remember to destroy both the list and its string elements after use
  - Write persistence files with key/type on one line, value on the next

**Memory Leak Detection**:
- Full test suite: Check console for "WARNING: X memory leaks detected"
- Individual debugging: Check `bin/heap_memory_report.log` after each test
  - **CRITICAL**: The report file is overwritten on each program run
  - Workflow: `make bin/test_name` → Check generated memory report
- Enhanced per-test reporting: The build system generates unique memory reports for each test
  - `make bin/test_name` automatically creates test-specific report files
  - Manual runs can use `AGERUN_MEMORY_REPORT` environment variable
  - Example: `AGERUN_MEMORY_REPORT=my_test.log ./bin/agerun_string_tests`
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
- Follow the 4-step directory check process (see Section 7) before running tests
- To run tests, use `make bin/test_name` which automatically builds and runs
- If you need to run tests manually from bin/ directory:
  - `cd bin && ./agerun_string_tests`
  - `AGERUN_MEMORY_REPORT=bin/test.memory_report.log ./bin/agerun_string_tests`

### 3. Parnas Design Principles (STRICTLY ENFORCED) ✅

**Status**: Full compliance achieved as of 2025-06-08. All interface violations have been fixed. Zero circular dependencies (except heap ↔ io).

**Core Principles**:
- **Information Hiding**: Hide design decisions behind interfaces
- **Single Responsibility**: One module, one concern
- **No Circular Dependencies**: Uses hierarchy must be strict
- **Opaque Types**: Required for complex data structures
- **Minimal Interfaces**: Expose only what's necessary
- **Complete Documentation**: Every module must be fully documented

**Important Clarifications**:
- **Enums in Public APIs**: Some enums (like `data_type_t`) are part of the abstract model, not implementation details. These are acceptable when they represent abstract concepts needed by clients.
- **Internal Functions**: Never expose functions marked "internal use only" in public headers
- **Implementation Details**: Keep struct definitions, array indices, and storage mechanisms private

**Enforcement**: Violations result in automatic PR rejection.

### Preventing Circular Dependencies (MANDATORY)

**Before Adding Any #include**:
1. **Check Current Dependencies**: Run `grep -n "#include.*agerun_" module.h module.c` to see existing includes
2. **Analyze Dependency Direction**: Ensure the new include follows the hierarchical layers:
   - Foundation (assert, io, list, map, string) → Data (heap, data) → Core (agent, method, expression) → System (agency, methodology, interpreter)
3. **Question the Need**: If you need to include a "higher" module in a "lower" one, STOP and reconsider the design

**Architectural Patterns to Prevent/Resolve Circular Dependencies** (in order of preference):

1. **Interface Segregation** (PREFERRED - split large modules):
   ```c
   // Instead of one large module with bidirectional dependencies:
   // Split into: core functionality, registry, store, updater
   // Example: agency split into agency, agent_registry, agent_store, agent_update
   ```

2. **Registry Pattern** (PREFERRED - for managing collections):
   ```c
   // Instead of modules directly managing each other:
   // Use a registry that owns and manages the lifecycle
   // Example: agent_registry owns all agents, not individual modules
   ```

3. **Facade Pattern** (PREFERRED - for complex subsystems):
   ```c
   // Instead of exposing internal module interactions:
   // Provide a simple interface that coordinates multiple modules
   // Example: agency is a facade for agent_registry, agent_store, agent_update
   ```

4. **Parser/Executor Separation** (PREFERRED - for language processing):
   ```c
   // Instead of mixing parsing and execution in one module:
   // Separate into parser (returns AST) and executor (processes AST)
   // Example: instruction parses, interpreter executes
   ```

5. **Callback Pattern** (LAST RESORT - adds complexity):
   ```c
   // Use ONLY when upward communication is absolutely necessary
   // and other patterns don't work
   // BAD: in low_module.c
   #include "high_module.h"
   void low_function() { high_notify(); }
   
   // ACCEPTABLE AS LAST RESORT: in low_module.h
   typedef void (*notify_callback_t)(int event_type, void *data);
   void ar__low__register_callback(notify_callback_t callback);
   // WARNING: Callbacks make code harder to understand and debug
   ```

6. **Dependency Inversion** (LAST RESORT - adds abstraction layers):
   ```c
   // Use ONLY when two modules absolutely must share behavior
   // and cannot be restructured
   // Define abstract interface that both modules can use
   // WARNING: This adds complexity and indirection
   ```

7. **Event Bus Pattern** (AVOID - too complex for this codebase):
   ```c
   // Generally too heavyweight for AgeRun's needs
   // Consider simpler patterns first
   ```

**Red Flags to Watch For**:
- Module A needs to notify Module B of changes (use callbacks)
- Two modules need to share data structures (extract to common module)
- Module needs to know about its container (use registry pattern)
- Parsing and execution in same module (separate concerns)
- Manager module becoming too large (split into focused modules)
- Implementation file including a module that uses it (probable circular dependency)

**Testing for Circular Dependencies**:
```bash
# Before committing, check for potential circular dependencies:
grep -r "#include.*agerun_" modules/*.c modules/*.h | sort

# Look for patterns like:
# - A.c includes B.h and B.c includes A.h
# - A includes B includes C includes A
# Update CIRCULAR_DEPS_ANALYSIS.md if adding new modules
```

**Accepted Exception**:
- `heap ↔ io`: This fundamental circular dependency exists because memory tracking needs error reporting and error reporting needs memory allocation. This is documented and accepted.

### Preventing Code Duplication (MANDATORY)

**DRY Principle**: Don't Repeat Yourself - every piece of knowledge must have a single, unambiguous, authoritative representation.

**Before Writing Any Code**:
1. **Search for Similar Functionality**: `grep -r "function_name\|concept" modules/`
2. **Check Existing Modules**: Review module documentation to find existing implementations
3. **Question the Duplication**: If you're copying code, STOP and refactor instead

**Strategies to Eliminate Duplication**:

1. **Extract Common Functions** (PREFERRED):
   ```c
   // BAD: Same validation logic in multiple places
   // in module_a.c
   if (strlen(name) > 0 && strlen(name) < 256 && strchr(name, '/') == NULL) { ... }
   // in module_b.c  
   if (strlen(name) > 0 && strlen(name) < 256 && strchr(name, '/') == NULL) { ... }
   
   // GOOD: Extract to string module
   bool ar__string__is_valid_name(const char *name);
   ```

2. **Create Utility Modules** (PREFERRED):
   ```c
   // When multiple modules need similar functionality:
   // Create a dedicated utility module (e.g., validation, conversion, formatting)
   // Example: string module provides trim, split, join for all modules
   ```

3. **Use Data-Driven Approaches** (PREFERRED):
   ```c
   // BAD: Repetitive switch/if statements
   if (type == TYPE_INT) return "integer";
   else if (type == TYPE_STRING) return "string";
   else if (type == TYPE_MAP) return "map";
   
   // GOOD: Use lookup table
   static const char* type_names[] = {
       [TYPE_INT] = "integer",
       [TYPE_STRING] = "string", 
       [TYPE_MAP] = "map"
   };
   return type_names[type];
   ```

4. **Parameterize Variations** (PREFERRED):
   ```c
   // BAD: Multiple similar functions
   void process_agent_create() { /* 90% same code */ }
   void process_agent_update() { /* 90% same code */ }
   void process_agent_delete() { /* 90% same code */ }
   
   // GOOD: Single parameterized function
   void process_agent_operation(operation_type_t op);
   ```

5. **Template Pattern with Callbacks** (USE SPARINGLY):
   ```c
   // Only when behavior varies in the middle of algorithm
   void process_file(const char *path, 
                    bool (*process_line)(const char *line, void *ctx),
                    void *ctx);
   ```

**Red Flags for Duplication**:
- Copy-pasting code between functions/modules
- Similar function names with slight variations (e.g., `_for_int`, `_for_string`)
- Repeated error handling patterns
- Multiple functions with same structure but different types
- Hardcoded values that appear in multiple places
- Similar test setup/teardown code

**Acceptable Duplication** (rare cases):
- Performance-critical code where abstraction overhead matters (document why)
- When modules must remain independent (no shared dependencies)
- Test code that needs explicit clarity over DRY
- When abstraction would be more complex than duplication (document why)

**Testing for Duplication**:
```bash
# Find potential duplicate function patterns
grep -r "^[a-zA-Z_].*(" modules/*.c | sed 's/.*://g' | sort | uniq -c | sort -rn

# Find similar code structures (manual review needed)
# Look for files with similar sizes that might contain duplicated logic
ls -la modules/*.c | awk '{print $5, $9}' | sort -n

# Find repeated string literals
grep -r "\".*\"" modules/*.c | grep -v "printf\|fprintf\|error" | sort | uniq -c | sort -rn
```

### 4. Coding Standards

**Naming Conventions** (Updated 2025-06-11):
- **Module Functions**: Use double underscore pattern `ar__<module>__<function>`
  - Examples: `ar__data__create_map()`, `ar__agent__send()`, `ar__system__init()`
  - Applied to all 21 modules consistently
- **Static Functions**: Use single underscore prefix `_<function_name>` (Completed 2025-06-11)
  - Examples: `_validate_file()`, `_find_method_idx()`, `_allocate_node()`
  - Indicates internal/private functions within a module
  - Similar to Python convention for private members
  - **IMPORTANT**: Only applies to static functions in implementation files, NOT test functions
  - Test functions in `*_tests.c` files keep their original names
- **Heap Macros**: Use double underscore pattern `AR__HEAP__<OPERATION>`
  - Examples: `AR__HEAP__MALLOC`, `AR__HEAP__FREE`, `AR__HEAP__STRDUP`
  - Applied to all 5 heap macros consistently
- **Assert Macros**: EXCEPTION - Keep original pattern `AR_ASSERT_<TYPE>`
  - Examples: `AR_ASSERT_OWNERSHIP`, `AR_ASSERT_TRANSFERRED`, `AR_ASSERT_NOT_USED_AFTER_FREE`
  - Rationale: Assert module contains only macros, not functions; follows different naming pattern
- **Type suffix**: `_t`

**Formatting**:
- 4-space indentation (no tabs)
- 100-character line limit
- ALL files must end with newline
  - C standard requires newline at EOF
  - Missing newlines cause compiler errors with -Wall -Werror
  - Always verify file ends with '\n' after edits

**Documentation**:
- Use `/**` style for all public APIs
- Include `@param`, `@return`, `@note` tags
- Document ownership semantics clearly
- Write complete sentences with punctuation
- When making functions static (Parnas compliance), mark them "(INTERNAL USE ONLY)" in comments
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
- `ar__io__open_file` instead of `fopen`
- `ar__io__close_file` instead of `fclose`
- `ar__io__fprintf` instead of `fprintf`
- `ar__io__read_line` instead of `fgets` or `getline`
- Check all return codes
- Use `ar__io__create_backup` before modifying critical files
- Use `ar__io__write_file` for atomic operations
- Use `ar__io__set_secure_permissions` for sensitive files

**Portable Format Specifiers**:
- Always use portable format specifiers for fixed-width integer types
- Include `<inttypes.h>` when formatting `int64_t`, `uint64_t`, etc.
- Use `PRId64` for `int64_t`: `printf("%" PRId64 "\n", value);`
- Use `PRIu64` for `uint64_t`: `printf("%" PRIu64 "\n", value);`
- NEVER use `%lld` or cast to `(long long)` - this breaks on Linux where `int64_t` is `long`
- Example:
  ```c
  #include <inttypes.h>
  int64_t agent_id = 42;
  printf("Agent ID: %" PRId64 "\n", agent_id);  // Correct
  printf("Agent ID: %lld\n", (long long)agent_id);  // Wrong - not portable
  ```

### 5. Module Development

**Dependency Management**:
- **Circular Dependencies**: Always check for and eliminate circular dependencies
- **Unidirectional Flow**: Ensure dependencies flow in one direction (e.g., agency → agent_update → agent_registry)
- **Delegation Pattern**: Higher-level modules can pass their dependencies to lower-level modules as parameters
- **Module Naming**: Follow `ar__<module>__<function>` pattern consistently (e.g., `ar__agent_update__update_methods`)
  - **IMPORTANT**: All module functions now use double underscores (updated 2025-06-08)

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

**Module Boundaries** (strict Parnas enforcement):
- **No Internal Headers**: Functions are either public (in .h) or private (static in .c)
- **No Friend Modules**: No special access between modules - use public APIs only
- **Clean Interfaces**: If modules need to communicate, design proper public interfaces
- **Public Enums**: Enums representing abstract concepts (like `data_type_t`) are acceptable in public APIs when clients need them
- **No Internal Function Exposure**: Never expose functions with "_internal" suffix in public headers
- **Registry Pattern**: When modules need to manage collections, use a separate registry module (e.g., agent_registry)
- **Facade Pattern**: For complex subsystems, use a facade module to coordinate multiple focused modules

**Best Practices**:
- Make incremental changes with frequent compilation
- Remove unused functions immediately
- Handle all compiler warnings (-Wall -Werror)
- Update documentation with code changes
  - Update modules/README.md when creating new modules
- Avoid code smells (see below)
- When showing code, provide only the raw code without commentary
- Think twice before adding global state to modules - prefer opaque structures
- When using other modules, read their interface first instead of guessing function names
- **Module Splitting**: When a module exceeds ~850 lines, consider splitting into focused modules
- **Dynamic Collections**: Use list/map structures instead of fixed arrays (no MAX_AGENTS limits)
- **String-Based IDs**: Use string keys in persistent maps for reliable serialization
- **Parameter Control**: Add boolean parameters to control optional behaviors (e.g., `send_lifecycle_events`)
- **Shutdown Order**: In system shutdown, call cleanup functions before marking as uninitialized
- **Avoid Platform-Specific Code**: Write portable C code that works across all platforms
  - NO `#ifdef __linux__`, `#ifdef __APPLE__`, or similar platform checks
  - Use standard POSIX/C library functions that work everywhere
  - If platform differences seem necessary, find a portable solution instead
  - Example: Use `errno` for error reporting instead of reading `/proc/meminfo`

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

### 7. Command Execution Guidelines

**ALWAYS Check Directory Before Running Commands**:
When executing any command, especially build/test commands, follow this 4-step process:
1. Check where you are: `pwd`
2. Change to the directory you want to be in: `cd <directory>`
3. Verify you're in the correct directory: `pwd`
4. Only then run the command you want

**Directory Navigation Rules**:
- **ALWAYS use full/absolute paths with `cd`**: Never use relative paths like `cd bin` or `cd ..`
- **Correct**: `cd /Users/quenio/Repos/agerun/bin`
- **Incorrect**: `cd bin`, `cd ../modules`, `cd ./tests`
- **Rationale**: Prevents confusion about current location and ensures commands work regardless of starting directory

This prevents common errors like:
- Running tests from wrong directory
- File not found errors
- Incorrect relative paths
- Wasted time debugging non-existent problems

### 8. Backup File Handling

**NEVER Create Backup Files**:
- Git already provides version history - use `git diff` and `git show` instead
- If you absolutely must create a backup:
  1. First check `.gitignore` for appropriate patterns
  2. Use extensions already in `.gitignore` if possible
  3. Or add the pattern to `.gitignore` BEFORE creating the file
  4. Delete backup files immediately after use

**Better Alternatives**:
- Use `git stash` to temporarily save changes
- Create a branch for experimental changes
- Use `git diff > changes.patch` to save a patch file (if .patch is in .gitignore)
- Simply rely on git's reflog for recovery

**If Backup Files Are Accidentally Staged**:
- Use `git reset HEAD <file>` to unstage
- Or `git rm --cached <file>` to remove from staging
- NEVER commit them

### 9. Debug and Analysis

**Installing clang-tools (for Static Analysis)**:

The AgeRun build system uses `scan-build` for static analysis. If `scan-build` is not available on your system, install it using the appropriate package manager:

#### macOS (using Homebrew)
```bash
# Install LLVM which includes scan-build
brew install llvm

# The scan-build command will be available at:
# /opt/homebrew/opt/llvm/bin/scan-build

# Add to your PATH:
export PATH="/opt/homebrew/opt/llvm/bin:$PATH"
```

#### Ubuntu/Debian
```bash
# Install clang-tools which includes scan-build
apt-get update && apt-get install -y clang-tools

# scan-build should now be available in your PATH
```

#### Verifying Installation
```bash
which scan-build
# Should output the path to scan-build

scan-build --version
# Should show the version information
```

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

**Expression Ownership Rules**:
- Memory access expressions (`memory.x`) return references - do NOT destroy
- Arithmetic/string operations return new objects - MUST destroy
- `ar__expression__take_ownership()` returns NULL for references
- Send instruction requires ownership of message parameter
- Currently no support for sending memory references directly

**Test Debugging Best Practices**:
- **ALWAYS verify test output carefully** - don't assume a test is failing based on assertion line numbers alone
- **Follow the 4-step directory check process** - many tests require being run from the `bin` directory (see Section 7)
- **Add debug output to confirm assumptions** - use fprintf(stderr, ...) to trace execution
- **Read the FULL error output** - early errors (like directory checks) can mask the real test status
- **When a test seems to be failing**:
  1. Follow the 4-step directory check process from Section 7
  2. Add debug output to see what's actually happening
  3. Verify the actual values being compared
  4. Don't assume the implementation is broken - the test expectations might be wrong
- **Remember**: A test that exits early due to directory checks is NOT the same as a test that fails assertions
- **Common pitfalls**:
  - Seeing "Assertion failed" and assuming the feature doesn't work
  - Not noticing that later tests are passing when run from correct directory
  - Jumping to fix code before verifying what's actually happening with debug output

### 10. Agent Lifecycle

**Critical Points**:
- Agents receive `__wake__` on creation
- Agents receive `__sleep__` before destruction
- ALWAYS process messages after sending to prevent leaks
- Call `ar__system__process_next_message()` after `ar__agent__send()`

### 11. Building Individual Tests

Always use make to build tests:
```bash
make bin/test_name  # Build individual test
```
Never compile directly with gcc.

**Important Makefile Features**:
- When building individual tests, the test is automatically executed after building
- Module changes are automatically rebuilt (no need to rebuild the library separately)
- The Makefile handles all dependencies and runs the test from the correct directory
- Example: `make bin/agerun_string_tests` will:
  1. Rebuild any changed modules
  2. Rebuild the test
  3. Run the test automatically from the bin directory
  4. Generate a memory report specific to that test

### 12. Session Management

When reviewing tasks:
- Check session todo list with `TodoRead`
- Check `TODO.md` file in repository
- Keep CLAUDE.md updated with new guidelines

**Before Committing (MANDATORY CHECKLIST - ALWAYS CHECK THIS)**:
1. **Run Tests**: Ensure all changes work correctly and no memory leaks
2. **Update TODO.md**: Mark completed tasks and add any new tasks identified
3. **Update CHANGELOG.md**: Document completed milestones and achievements (NON-NEGOTIABLE)
4. **Review Changes**: Use `git diff` to verify all changes are intentional
5. **Check for temporary/backup files**: NEVER commit backup files (*.backup, *.bak, *.tmp, etc.)
6. **Then Commit**: Only after completing steps 1-5

**IMPORTANT**: Always perform steps 1-5 BEFORE running `git commit`. Never skip the checklist.

**Critical Reminders**:
- CHANGELOG update is MANDATORY for every commit that completes tasks
- Always check this checklist BEFORE running `git commit`
- Build this mental model: Changes → Tests → CHANGELOG → Check for backups → Commit
- Treat CHANGELOG updates as part of the work, not as optional documentation
- NEVER commit temporary or backup files to version control

**After Completing Major Tasks**:
- Document completion date in TODO.md (e.g., "Completed 2025-06-11")
- Move tasks from "In Progress" to "Completed Tasks" section
- Include brief summary of what was accomplished
- Update CLAUDE.md with any new patterns or learnings from the session

### 13. Refactoring Patterns

**Visitor Pattern to List-Based**:
- When refactoring from visitor pattern, use list-based approach for better memory management
- Return lists of data that callers can iterate over and destroy
- Avoids complex callback memory ownership issues

**Module Cohesion**:
- Create focused modules with single responsibilities
- Example: Split agency (850+ lines) into:
  - agent_registry (ID management)
  - agent_store (persistence)  
  - agent_update (version updates)
  - agency (81-line facade coordinating the others)

**File Editing Best Practices**:
- Always verify file content thoroughly before making edits
- Be suspicious of placeholder-looking text like "[... rest of ...]"
- Check file sizes or line counts when something seems off (e.g., `wc -l filename`)
- Never assume placeholder text is a display artifact - it might be literal content
- If a file seems truncated, investigate the git history to find the complete version

**Bulk Renaming Guidelines**:
- **Use `sed` for bulk renaming**: When renaming functions, macros, variables, or parameters across the codebase, `sed` is the most efficient and safe approach
- **Pattern**: `sed 's/old_name/new_name/g' file > file.tmp && mv file.tmp file`
- **Batch processing**: Use loops for multiple files: `for file in modules/*.c; do sed 's/pattern/replacement/g' "$file" > "$file.tmp" && mv "$file.tmp" "$file"; done`
- **Always verify current directory**: Run `pwd` before executing sed commands to ensure correct location
- **Advantages**: Fast, reliable, handles large codebases efficiently, preserves file structure
- **Examples from naming convention refactoring**: 
  - Module functions: `sed 's/ar_data_/ar__data__/g'`
  - Static functions: `sed -E 's/^static ([a-zA-Z][a-zA-Z0-9_]*)\(/static _\1(/g'`
  - Function calls: `sed 's/\bfunction_name(/\b_function_name(/g' file > file.tmp && mv file.tmp file`
  - Macros: `sed 's/AR_HEAP_/AR__HEAP__/g'`
- **Important Considerations**:
  - Global variables may get accidentally renamed - revert these manually
  - Function calls within renamed functions need separate updates
  - Always compile and test after bulk renaming to catch issues
  - NEVER use `-i.bak` as it creates backup files - use temporary files instead

**Static Function Renaming Workflow**:
1. **Identify target files**: Exclude test files when renaming static functions
   ```bash
   ls modules/*.c | grep -v '_tests\.c$'
   ```
2. **Rename function definitions**:
   ```bash
   for file in modules/*.c; do 
     if [[ ! "$file" == *"_tests.c" ]]; then 
       sed -E 's/^static ([a-zA-Z][a-zA-Z0-9_]*)\(/static _\1(/g' "$file" > "$file.tmp" && mv "$file.tmp" "$file"
     fi
   done
   ```
3. **Update function calls**: Use more specific patterns to avoid renaming unrelated items
   ```bash
   sed 's/\([^_]\)function_name(/\1_function_name(/g' file.c > file.c.tmp && mv file.c.tmp file.c
   ```
4. **Fix any double underscores**: Sometimes sed patterns can create `__function` instead of `_function`
5. **Revert global variable changes**: Check for and fix any accidentally renamed globals
6. **Compile and test**: Always run `make clean && make` followed by `./clean_build.sh`

**Git Workflow**:
- **CRITICAL**: See "Git Workflow (CRITICAL)" section near the top of this document
- Always run `git status` after `git push` to ensure push completed successfully
- Verify working tree remains clean after operations
- Follow the 4-step directory check process (Section 7) before running commands with relative paths

### 14. Task Tool Usage Guidelines

**Preventing Content Loss When Using Task Tool**:

When working with the Task tool, follow these guidelines to prevent accidentally overwriting valuable content:

1. **Read Before Write**: Always use the `Read` tool to examine files created by the `Task` tool before deciding to modify them. This ensures you know what content exists before making changes.

2. **Use Edit Instead of Write**: When improving existing content, use the `Edit` tool to make specific changes rather than completely rewriting with `Write`. This preserves the original work while allowing improvements.

3. **Clear Task Instructions**: When using the `Task` tool, be specific about whether you want it to:
   - Create files (and then you'll review them)
   - Just analyze and report back (and then you'll create files)
   - Create drafts for you to refine

4. **Document Workflow Intentions**: Before starting a task, clearly state your intended workflow:
   - "I'll use Task to analyze, then create files myself"
   - "I'll use Task to create files, then review and edit if needed"

5. **Backup Before Overwrite**: If you do need to completely rewrite a file, first copy its contents to a temporary location or include the original content in your response before overwriting.

6. **Trust Task Tool Output**: The Task tool is designed to create high-quality output. Trust its work and only make targeted improvements rather than assuming you need to rewrite everything.

**Key Principle**: Always read and understand existing content before modifying it, even if you were involved in creating it originally.

## AgeRun Language Notes

- No null type - use integer 0
- All parameters required
- Version strings explicit (e.g., "1.0.0")
- **No map literals** - `{}` syntax is not supported in AgeRun expressions
- Agent ID 0 indicates failure
- Always process `__wake__` messages
- Always process messages after sending to prevent memory leaks
- **Function calls are NOT expressions** - per BNF grammar specification
- **Send with memory references not supported** - send() needs ownership of message

## Method Test Template

```c
#include <unistd.h>
// ... other includes ...

// Directory check
char cwd[1024];
if (getcwd(cwd, sizeof(cwd)) != NULL) {
    size_t len = strlen(cwd);
    if (len < 4 || strcmp(cwd + len - 4, "/bin") != 0) {
        fprintf(stderr, "ERROR: Tests must be run from the bin directory!\n");
        return 1;
    }
}

// Clean state
ar__system__shutdown();
ar__methodology__cleanup();
ar__agency__reset();
remove("methodology.agerun");
remove("agency.agerun");

// ... test code ...

// Initialize system after creating methods
ar__system__init(NULL, NULL);
```

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
- Read README.md for project overview and user documentation
