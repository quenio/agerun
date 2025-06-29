# AgeRun Development Guide for Claude

This document contains essential instructions for Claude Code to assist with AgeRun development.

## Project Overview

AgeRun is a lightweight, message-driven agent system where each agent is defined by a single method responsible for handling all incoming messages. Agents operate within a persistent runtime environment, allowing dynamic creation, versioning, pausing, resumption, and destruction.

## Quick Start

**Primary Build Tool**: `./clean_build.sh` - runs everything with minimal output (~20 lines)
- Use before commits and for quick verification
- Includes: clean, build, static analysis, all tests, sanitizers, leak check

**Individual Commands** (when needed):
```bash
make clean && make     # Rebuild
make analyze          # Static analysis on library
make analyze-tests    # Static analysis on tests
make test             # Run tests (auto-rebuilds) 
make test-sanitize    # Run with ASan
make run              # Run executable
make bin/test_name    # Build/run specific test
```

**Note**: Always run from repo root. Makefile handles directory changes automatically (runs `make clean debug` before tests). Doc-only changes don't require testing. Always pause before build commands to check for custom scripts.

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

### 0. Documentation Protocol

**Always search CLAUDE.md first** when asked about procedures. Don't overthink - start with exact keywords.

### 1. Memory Management (ZERO TOLERANCE FOR LEAKS)

**Mandatory Practices**:
- Use heap tracking macros: `AR__HEAP__MALLOC`, `AR__HEAP__FREE`, `AR__HEAP__STRDUP`
- Include `#include "ar_heap.h"` in all .c files that allocate memory
- Follow ownership naming conventions:
  - `own_`: Owned values that must be destroyed
  - `mut_`: Mutable references (read-write access)
  - `ref_`: Borrowed references (read-only access)
  - Apply to ALL variables, parameters, and struct fields (including local variables)
  - **IMPORTANT**: When receiving ownership (e.g., from malloc/create functions), use `own_` prefix immediately
  - Consistent across .h, .c, and _tests.c files
- Set pointers to NULL after ownership transfer
- Add `// Ownership transferred to caller` comment at return statements
- Follow the Memory Management Model (MMM.md) strictly
- Use debug assertions strategically:
  - `AR_ASSERT_OWNERSHIP()` for critical resource allocations
  - `AR_ASSERT_TRANSFERRED()` for complex ownership transfers
  - `AR_ASSERT_NOT_USED_AFTER_FREE()` for complex reuse patterns
  - See ar_assert.md for complete guidelines
- Expression evaluation ownership rules:
  - Memory access (`memory.x`): returns reference, do NOT destroy
  - Arithmetic (`2 + 3`, `memory.x + 5`): returns new object, MUST destroy
  - String operations (`"Hello" + " World"`): returns new object, MUST destroy
  - Map iteration pattern:
  - Use `ar_data__get_map_keys()` to get all keys as a list
  - Remember to destroy both the list and its string elements after use
  - Write persistence files with key/type on one line, value on the next

**Memory Leak Detection**:
- Full test suite: Check console for "WARNING: X memory leaks detected"
- **Individual test memory reports**: Located at `bin/memory_report_<test_name>.log`
  - **IMPORTANT**: No longer uses generic `heap_memory_report.log`
  - Each test generates its own report file automatically
  - Example: `bin/memory_report_ar_string_tests.log`
  - Workflow: `make bin/test_name` → Check test-specific memory report
- Enhanced per-test reporting: The build system generates unique memory reports for each test
  - `make bin/test_name` automatically creates test-specific report files
  - Manual runs can use `AGERUN_MEMORY_REPORT` environment variable
  - Example: `AGERUN_MEMORY_REPORT=my_test.log ./bin/ar_string_tests`
- Always run `make test-sanitize` before committing
- Environment variables for debugging:
  - `ASAN_OPTIONS=halt_on_error=0` to continue after first error
  - `ASAN_OPTIONS=detect_leaks=1:leak_check_at_exit=1` for complex leaks

**Comprehensive Memory Debugging Guide**:

**Example: Debugging a Function Return Ownership Leak**
```c
// SYMPTOM: Memory leak detected in test_instruction_parser
// 1. Isolate the specific test causing the leak
// 2. Check memory report: bin/memory_report_instruction_parser_tests.log
//    Shows: "data_t (list) allocated at instruction_ast.c:142"

// 3. Examine the leaking function:
data_t* ar__instruction_ast__get_function_args(ast_t *ast) {
    return ar_data__create_list();  // Creates NEW list (ownership transfer)
}

// 4. Find usage in tests - variable naming reveals the bug:
data_t *ref_args = ar__instruction_ast__get_function_args(ast);  // WRONG: ref_ implies borrowed
// ... no ar_data__destroy(ref_args) call found

// 5. Fix by updating variable name and adding cleanup:
data_t *own_args = ar__instruction_ast__get_function_args(ast);  // Correct prefix
// ... use args ...
ar_data__destroy(own_args);  // Add cleanup

// 6. Common patterns that cause leaks:
// - Removal functions: ar_data__list_remove_first() returns owned value
// - Map iteration: ar_data__get_map_keys() creates new list
// - String operations: "Hello" + " World" creates new string
// - Buffer overflows: escape sequences need 2 bytes, not 1
// - Don't trust function names - check ownership docs
```

**Debug Strategy**: When leak detected → Check memory report → Trace allocation source → Verify ownership semantics → Fix variable naming → Add proper cleanup

### 2. Test-Driven Development (MANDATORY)

**Pre-modification Rule**: Run module tests BEFORE changing any module

**Red-Green-Refactor Cycle (ALL THREE PHASES REQUIRED)**:

**CRITICAL**: This is a CYCLE that repeats for each new behavior/feature. NO commits during the cycle!

For each new behavior/feature:
1. **Red Phase**: Write failing test FIRST
   - Write test for ONE specific behavior (or modify existing test to fail)
   - Run test to confirm it fails
   - Do NOT commit
   
2. **Green Phase**: Write MINIMUM code to pass
   - Make the test pass with simplest implementation
   - Run test to confirm it passes
   - Check memory report for leaks
   - Do NOT commit
   
3. **Refactor Phase (MANDATORY - DO NOT SKIP)**: Improve while keeping tests green
   - Look for improvements:
     - Remove code duplication
     - Improve naming
     - Extract helper functions
     - Simplify complex logic
     - Apply consistent patterns
   - Run tests after each change
   - Even if no refactoring needed, you MUST explicitly state: "Refactor phase: No improvements identified"
   - Do NOT commit

**REPEAT THE CYCLE** for next behavior/feature

**Only after ALL cycles are complete**:
- Update documentation
- Update TODO.md
- Update CHANGELOG.md
- THEN commit all changes together

**IMPORTANT**: 
- The cycle is NOT complete after just Red-Green
- Refactor phase is MANDATORY, not optional
- NO commits until ALL TDD cycles for the feature are done
- One commit per feature, not per cycle

**Example - Making a module instantiable**:
- Cycle 1: Red (test create/destroy) → Green (implement) → Refactor (check for improvements)
- Cycle 2: Red (test evaluate with instance) → Green (implement) → Refactor (check for improvements)
- Cycle 3: Red (test legacy wrapper) → Green (implement) → Refactor (check for improvements)
- Only NOW: Update docs → Update TODO.md → Update CHANGELOG.md → Commit once

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
  - `cd bin && ./ar_string_tests`
  - `AGERUN_MEMORY_REPORT=bin/test.memory_report.log ./bin/ar_string_tests`

**TDD for Large Refactoring (Advanced Pattern)**:
- **Breaking down architectural changes**: Divide large refactoring into multiple sequential TDD cycles
- **Each cycle addresses one behavior**: Don't try to implement multiple instruction types in one cycle
- **Build on previous cycles**: Later cycles can assume earlier cycles work correctly
- **Example pattern**: 9 cycles for facade refactoring (assignment, send, if, parse, build, method, agent, destroy, error handling)
- **Refactor phase is critical**: Use refactor phase to eliminate duplication and extract common patterns
- **All cycles before commit**: Complete ALL planned cycles before documentation and commit

### 3. Parnas Design Principles (STRICT ENFORCEMENT) ✅

**Status**: Full compliance achieved as of 2025-06-08. All interface violations have been fixed. Zero circular dependencies (except heap ↔ io).

**Core Principles**:
- **Information Hiding**: Hide design decisions behind interfaces
- **Single Responsibility**: One module, one concern
- **No Circular Dependencies**: Uses hierarchy must be strict
- **Opaque Types**: Required for complex data structures
- **Minimal Interfaces**: Expose only what's necessary
- **Complete Documentation**: Every module must be fully documented
- **Const-Correctness**: NEVER cast away const - fix interfaces instead
- **No Parallel Implementations**: Modify existing code, don't create _v2 versions

**Important Clarifications**:
- **Enums in Public APIs**: Some enums (like `data_type_t`) are part of the abstract model, not implementation details. These are acceptable when they represent abstract concepts needed by clients.
- **Internal Functions**: Never expose functions marked "internal use only" in public headers
- **Implementation Details**: Keep struct definitions, array indices, and storage mechanisms private

**Enforcement**: Violations result in automatic PR rejection.

### Preventing Circular Dependencies & Code Duplication

**Dependency Management**:
```bash
# Check before adding includes:
grep -n "#include.*ar_" module.h module.c
# Verify hierarchy: Foundation (io/list/map) → Data (heap/data) → Core (agent/method) → System (agency/interpreter)
```

**Architectural Patterns** (in order of preference):
1. **Interface Segregation**: Split large modules (agency → registry/store/update)
2. **Registry Pattern**: Central ownership of lifecycle (registry owns all agents)
3. **Facade Pattern**: ONLY coordinate, never implement business logic
4. **Parser/Executor Split**: Separate concerns for clarity
5. **Callbacks/DI**: Last resort - adds complexity

**Code Duplication Prevention (DRY - Don't Repeat Yourself)**:
```bash
# Before writing code, search for existing:
grep -r "function_name\|concept" modules/
```

**DRY Strategies**:
- If copying code, STOP and refactor instead
- Extract common functions → new abstractions/modules
- Use data tables instead of switch statements
- Parameterize variations instead of copying
- Template pattern with callbacks (use sparingly)
- Move code with diff verification, don't rewrite

**Red Flags**: Module A→B→C→A cycles, repeated validation logic, similar function names (_for_int, _for_string), parsing+execution together

**Exception**: heap ↔ io circular dependency is accepted and documented.

### 4. Coding Standards

**String Parsing**: Track quote state when scanning for operators (`:=`, `(`, `)`, `,`)
```c
bool in_quotes = false;
while (*p) {
    if (*p == '"' && (p == start || *(p-1) != '\\')) in_quotes = !in_quotes;
    else if (!in_quotes && /* check operator */) { /* process */ }
    p++;
}
```

**Naming Patterns**:
| Type | Pattern | Example |
|------|---------|---------|
| Module functions | `ar_<module>__<function>` | `ar_data__create_map()` |
| Static functions | `_<function>` | `_validate_file()` |
| Test functions | `test_<module>__<name>` | `test_string__trim()` |
| Heap macros | `AR__HEAP__<OP>` | `AR__HEAP__MALLOC` |
| Assert macros | `AR_ASSERT_<TYPE>` | `AR_ASSERT_OWNERSHIP` |
| Opaque types | `typedef struct ar_<module>_s ar_<module>_t;` | |
| Files | `ar_<module>.{h,c}` | `ar_data.h` |

**Key Standards**:
- 4-space indent, 100-char lines, newline at EOF
- `/** */` docs with `@param`, `@return`, `@note` ownership
- Use IO module (`ar_io__open_file` not `fopen`, etc) - check all return codes
- Use `PRId64`/`PRIu64` for portability, never `%lld`

### 5. Module Development

**Core Architecture**:
- **Parsing vs Evaluation**: Data owner parses (methodology→methods), consumer evaluates (interpreter→ASTs)
- **Opaque Types**: `typedef struct ar_type_name_s ar_type_name_t;` in header, definition in .c only
- **Module Size**: Split at ~850 lines into focused modules (e.g., agency→4 modules)

**Key Patterns**:
- Parse once, evaluate many times (store ASTs, not source)
- Use dynamic collections (lists/maps), not fixed arrays
- String-based IDs for reliable persistence
- Read interface first instead of guessing function names
- No platform-specific code (`#ifdef __linux__` forbidden)
- Create public APIs to eliminate duplication across modules

**Code Quality Checklist**:
✓ Functions < 50 lines (single responsibility)
✓ Parameters ≤ 5 (use structs for more)  
✓ No speculative generality  
✓ Self-documenting (comments = "why" not "what")  
✓ Incremental changes with frequent compilation
✓ Verify each change with tests
✓ Remove unused functions immediately
✓ Address all warnings immediately
✓ Think twice before adding global state  
✓ Update modules/README.md for new modules

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

### 7. Development Practices

**Directory Navigation**: Always use absolute paths
```bash
cd /Users/quenio/Repos/agerun/bin  # Correct
cd bin  # Wrong - avoid relative paths
```

**Backup Files**: Never create them - use git instead
- `git stash` for temporary saves
- `git diff > changes.patch` for patches
- If created accidentally: `git reset HEAD <file>`

**Debug Tools**:
- **Memory**: `make test-sanitize` → Check `bin/memory_report_<test_name>.log`
- **Static Analysis**: `make analyze` (requires scan-build: `brew install llvm` or `apt install clang-tools`)
- **Test Failures**: Often just wrong directory - 4-step check: pwd → cd /path → pwd → run
- **Pattern Testing**: Test regex/sed/awk patterns before using in scripts
- **Doc Validation**: clean_build.sh validates file refs, function names, types

**Expression Ownership** (CRITICAL):
- References (`memory.x`): Don't destroy - borrowed from memory/context
- New objects (`2+3`, `"a"+"b"`): Must destroy - evaluator creates them
- Self-ownership check: Never let `values_result == mut_memory`
- Context lifetime: NEVER destroy context or its elements in evaluators
- Debug ownership: Use `ar_data__hold_ownership()` to verify status

### 8. Agent Lifecycle

**Critical Points**:
- Agents receive `__wake__` on creation
- Agents receive `__sleep__` before destruction
- ALWAYS process messages after sending to prevent leaks
- Call `ar_system__process_next_message()` after `ar_agent__send()`

### 9. Building Individual Tests

Always use make to build tests:
```bash
make bin/test_name  # Build individual test
```
Never compile directly with gcc.

**Important Makefile Features**:
- When building individual tests, the test is automatically executed after building
- Module changes are automatically rebuilt (no need to rebuild the library separately)
- The Makefile handles all dependencies and runs the test from the correct directory
- Example: `make bin/ar_string_tests` will:
  1. Rebuild any changed modules
  2. Rebuild the test
  3. Run the test automatically from the bin directory
  4. Generate a memory report specific to that test

### 10. Session & Commit Management

**Task Management**:
- **Session todos**: Current TDD cycles, implementations, bug fixes
- **TODO.md**: Future architectural work, refactoring plans
- **User feedback**: May reveal design issues, not just implementation bugs

**Pre-Commit Checklist** (MANDATORY):
1. `./clean_build.sh` - Fix ALL issues before proceeding (includes doc validation)
2. Update module .md files if interfaces changed
3. Update TODO.md - Mark completed, add new tasks
4. Update CHANGELOG.md (NON-NEGOTIABLE)
5. `git diff` - Verify all changes intentional
6. Check for backup files outside ./bin (*.backup, *.bak, *.tmp)
7. Only then: `git commit`

**Remember**: Complete ALL TDD Cycles → Docs → TODO → CHANGELOG → Commit

**After Completing Major Tasks**:
- Document completion date in TODO.md (e.g., "Completed 2025-06-11")
- Move tasks from "In Progress" to "Completed Tasks" section
- Include brief summary of what was accomplished
- Update CLAUDE.md with any new patterns or learnings from the session

### 11. Refactoring Patterns

**Core Principles**:
- **Preserve behavior**: Tests define expected behavior - fix implementation, not tests
- **Move code, don't rewrite**: Use diff to verify code is moved, not reimplemented  
- **Clean state recovery**: If refactoring fails, revert completely rather than debug
- **Validate changes**: After adding validation, test with intentional errors to ensure no false negatives

**Bulk Renaming with sed**:
```bash
# Pattern: sed 's/old/new/g' file > file.tmp && mv file.tmp file
# Example for static functions (exclude test files):
for file in modules/*.c; do
  [[ ! "$file" == *"_tests.c" ]] && \
  sed -E 's/^static ([a-zA-Z][a-zA-Z0-9_]*)\(/static _\1(/g' "$file" > "$file.tmp" && \
  mv "$file.tmp" "$file"
done
# Always compile after bulk changes. Never use -i.bak (creates backups)
```

**Code Movement Verification**:
```bash
# MANDATORY: Verify code is moved, not rewritten
diff -u <(sed -n '130,148p' original.c) <(sed -n '11,29p' new.c)
```

**Key Patterns**:
- **Module cohesion**: Split large modules (e.g., 850-line agency → 4 focused modules)
- **Merging functions**: Move helpers first, then merge implementation (don't re-implement)
- **Instance migration**: Replace parameters with `self->field` references
- **File verification**: Check `wc -l` if content seems truncated; beware "[... rest of ...]"
- **Test preservation**: Tests define behavior - fix implementation, not tests
- **Diff verification**: MANDATORY when moving code between modules
- **Strategic analysis**: Check if modern solution exists before refactoring legacy code
- **Complexity warning**: If "simple" change touches many modules → approach is wrong
- **Complete implementations**: All cases or none - partial implementations create bugs

### 12. Plan Verification and Review

**When Creating Development Plans**:
- **Always include critical verification steps**: Plans must include diff verification, test running, memory checking
- **User feedback is valuable**: If user points out missing steps, update the plan immediately
- **Example**: "We're missing the comparison of previous and new implementation" - this feedback prevents bugs
- **Plan completeness checklist**:
  - [ ] Verification steps included (diff, tests, memory)
  - [ ] File paths and line numbers specified
  - [ ] Success criteria defined
  - [ ] Error handling considered

**Module-Specific Requirements**:
- **Different modules have different constraints**: Not all modules follow identical patterns
- **Examples**:
  - Some parsers accept only strings (method parser: all 3 args must be quoted)
  - Some accept expressions (build parser: second arg must be expression)
  - Some have fixed argument counts (method: exactly 3, build: exactly 2)
- **Always check**: Read existing tests to understand specific requirements

### 13. Task Tool Guidelines

**Core Rule**: Read before write - examine Task output before modifying

**Workflow**: 
1. Use Task to analyze/create files
2. Read the output with Read tool
3. Use Edit (not Write) for improvements
4. Be specific: "analyze only" vs "create files"

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
ar_system__shutdown();
ar__methodology__cleanup();
ar_agency__reset();
remove("methodology.agerun");
remove("agency.agerun");

// ... test code ...

// Initialize system after creating methods
ar_system__init(NULL, NULL);
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
