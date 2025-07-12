# AgeRun Development Guide for Claude

This document contains essential instructions for Claude Code to assist with AgeRun development.

## Project Overview

AgeRun is a lightweight, message-driven agent system where each agent is defined by a single method responsible for handling all incoming messages. Agents operate within a persistent runtime environment, allowing dynamic creation, versioning, pausing, resumption, and destruction.

## Quick Start

**Primary Build Tool**: `make full-build` - runs everything with minimal output (~20 lines)
- Use before commits and for quick verification
- Includes: clean, build, static analysis, all tests, sanitizers, leak check, doc validation

**Individual Commands** (when needed):
```bash
make                  # Shows help with all available targets
make clean            # Clean all build artifacts
make analyze-exec     # Static analysis on executable code
make analyze-tests    # Static analysis on tests
make run-tests        # Run tests (auto-rebuilds) 
make sanitize-tests   # Run with ASan
make run-exec         # Build and run executable
make test_name        # Build/run specific test
make check-naming     # Check naming conventions
make check-docs       # Check documentation validity
make check-all        # Run all code quality checks
make add-newline FILE=<file>  # Add missing newline to file
```

**Makefile Usage Note**: Running `make` without any target shows a helpful list of all available targets. Use this as a guide to discover available commands.

**Note**: Always run from repo root. Makefile handles directory changes automatically. Doc-only changes don't require testing. Always pause before build commands to check for custom scripts.

**Scripts Directory**: All build and utility scripts are located in `/scripts/`. Never run these scripts directly - always use the corresponding make targets. Scripts will fail with an error message if run outside the repository root.

## Git Workflow (CRITICAL)

**After Every Git Push**:
1. **ALWAYS run `git status`** to verify:
   - Push completed successfully
   - Working tree remains clean
   - No uncommitted changes remain

This is a MANDATORY verification step. Never assume a push succeeded without checking. ([details](kb/git-push-verification.md))

## Project Structure

- **/modules**: Core implementation (.c/.h files and tests)
- **/bin**: Generated binaries (ignored by git, NEVER read these files)
- **/methods**: Method definitions (.method files with docs and tests)
- **/scripts**: Build and utility scripts (run via make targets, not directly - includes slash commands)
- **.claude/commands**: Slash command definitions (must use make targets, not direct scripts)

## Critical Development Rules

### 0. Documentation Protocol

**Always search CLAUDE.md first** when asked about procedures. Don't overthink - start with exact keywords.
**When reading TODO.md**: Check [ ] = incomplete, [x] = complete. Read completion dates. Be explicit - list all components.

### 1. Memory Management (ZERO TOLERANCE FOR LEAKS)

**Mandatory Practices**:
- Use heap tracking macros: `AR__HEAP__MALLOC`, `AR__HEAP__FREE`, `AR__HEAP__STRDUP`
- Include `#include "ar_heap.h"` in all .c files that allocate memory
- Follow ownership naming conventions ([details](kb/ownership-naming-conventions.md)):
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
- **Individual test memory reports**: Located at `bin/memory_report_<test_name>.log` ([details](kb/memory-leak-detection-workflow.md))
  - **IMPORTANT**: No longer uses generic `heap_memory_report.log`
  - Each test generates its own report file automatically
  - Example: `bin/memory_report_ar_string_tests.log`
  - Workflow: `make test_name` → Check test-specific memory report
- **Complete verification**: `grep "Actual memory leaks:" bin/memory_report_*.log | grep -v "0 (0 bytes)"` checks ALL reports
- **CI visibility**: full_build.sh prints leak reports to stdout for immediate CI debugging
- Enhanced per-test reporting: The build system generates unique memory reports for each test
  - `make test_name` automatically creates test-specific report files
  - Manual runs can use `AGERUN_MEMORY_REPORT` environment variable
  - Example: `AGERUN_MEMORY_REPORT=my_test.log ./bin/ar_string_tests`
- Always run `make sanitize-tests` before committing
- Environment variables for debugging:
  - `ASAN_OPTIONS=halt_on_error=0` to continue after first error
  - `ASAN_OPTIONS=detect_leaks=1:leak_check_at_exit=1` for complex leaks

**Comprehensive Memory Debugging Guide**:

**Example: Debugging a Function Return Ownership Leak**
```c
// SYMPTOM: Memory leak detected in test_instruction_parser
// 1. Isolate the specific test causing the leak
// 2. Check memory report: bin/memory_report_instruction_parser_tests.log
//    Shows: "ar_data_t (list) allocated at instruction_ast.c:142"

// 3. Examine the leaking function:
ar_data_t* ar_instruction_ast__get_function_args(ast_t *ast) {
    return ar_data__create_list();  // Creates NEW list (ownership transfer)
}

// 4. Find usage in tests - variable naming reveals the bug:
ar_data_t *ref_args = ar_instruction_ast__get_function_args(ast);  // WRONG: ref_ implies borrowed
// ... no ar_data__destroy(ref_args) call found

// 5. Fix by updating variable name and adding cleanup:
ar_data_t *own_args = ar_instruction_ast__get_function_args(ast);  // Correct prefix
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

**Use-After-Free Prevention**: Watch error paths + ownership transfer interactions. Run ASan always when both present.

### 2. Test-Driven Development (MANDATORY)

**Pre-modification Rule**: Run module tests BEFORE changing any module

**Red-Green-Refactor Cycle (ALL THREE PHASES REQUIRED)** ([details](kb/red-green-refactor-cycle.md)):

**CRITICAL**: This is a CYCLE that repeats for each new behavior/feature. NO commits during the cycle!

For each new behavior/feature:
1. **Red Phase**: Write failing test FIRST
   - Write test for ONE specific behavior (or modify existing test to fail)
   - Run test to confirm it ACTUALLY FAILS (comments don't count as failure!)
   - If you can't create a failing test, document as "consistency fix" not TDD
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
- **MANDATORY BDD structure**: Use Given/When/Then comments ([details](kb/bdd-test-structure.md)):
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
- **Test names must reflect actual behavior** - update names when refactoring changes behavior ([details](kb/test-function-naming-accuracy.md))
- Zero memory leaks in tests
- **Global state cleanup**: Tests registering methods/agents MUST call `ar_methodology__cleanup()` and `ar_agency__reset()`
- Process all messages before cleanup: `while (ar_system__process_next_message());`
- Test files: `<module>_tests.c`
- **Use test fixtures**: Check for `ar_*_fixture.h` before writing boilerplate setup; NEVER manually destroy fixture-tracked resources (causes double-free)
- Follow the 4-step directory check process (see Section 7) before running tests
- To run tests, use `make test_name` which automatically builds and runs
- **ALWAYS rebuild after code changes**: `make test_name` (not just `./test_name`)
- If you need to run tests manually from bin/ directory:
  - `cd bin && ./ar_string_tests`
  - `AGERUN_MEMORY_REPORT=bin/test.memory_report.log ./bin/ar_string_tests`

**TDD for Large Refactoring (Advanced Pattern)**:
- **Breaking down architectural changes**: Divide large refactoring into multiple sequential TDD cycles
- **Each cycle addresses one behavior**: Don't try to implement multiple instruction types in one cycle
- **Build on previous cycles**: Later cycles can assume earlier cycles work correctly
- **Example pattern**: 9 cycles for facade refactoring (assignment, send, if, parse, build, method, agent, destroy, error handling)
- **Refactor phase is critical**: Use refactor phase to eliminate duplication and extract common patterns
- **Accept partial improvements**: Better implementation with some duplication > wrong abstraction
- **Verify integration points**: Check that related cycles properly connect (e.g., log propagation through hierarchies)
- **All cycles before commit**: Complete ALL planned cycles before documentation and commit
- **MANDATORY facade updates**: Always include facade/integration updates as separate TDD cycles in same plan

### 3. Parnas Design Principles (STRICT ENFORCEMENT) ✅

**Status**: Full compliance achieved as of 2025-06-08. All interface violations have been fixed. Zero circular dependencies (except heap ↔ io).

**Core Principles**:
- **Information Hiding**: Hide design decisions behind interfaces ([details](kb/information-hiding-principle.md))
- **Single Responsibility**: One module, one concern ([details](kb/single-responsibility-principle.md))
- **No Circular Dependencies**: Uses hierarchy must be strict ([details](kb/no-circular-dependencies-principle.md))
- **Opaque Types**: Required for complex data structures ([details](kb/opaque-types-principle.md))
- **Minimal Interfaces**: Expose only what's necessary ([details](kb/minimal-interfaces-principle.md))
- **Complete Documentation**: Every module must be fully documented ([details](kb/complete-documentation-principle.md))
- **Program Families**: Design for anticipated variations and multiple versions ([details](kb/program-families-principle.md))
- **Design for Change**: Decompose based on what's likely to change, not workflow ([details](kb/design-for-change-principle.md))
- **Separation of Concerns**: Each module addresses a distinct concern ([details](kb/separation-of-concerns-principle.md))

**Important Clarifications**:
- **Enums in Public APIs**: Some enums (like `ar_data_type_t`) are part of the abstract model, not implementation details. These are acceptable when they represent abstract concepts needed by clients.
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
3. **Facade Pattern**: ONLY coordinate, never implement business logic; update creation when interfaces change; run facade tests after sub-component changes; frame-based evaluators→create upfront, not lazily ([details](kb/facade-pattern-coordination.md))
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

### Additional Design Principles

**Core Standards**:
- **Const-Correctness**: NEVER cast away const - fix interfaces instead ([details](kb/const-correctness-principle.md))
- **No Parallel Implementations**: Modify existing code, don't create _v2 versions ([details](kb/no-parallel-implementations-principle.md))
- **Composition Over Inheritance**: Prefer composition patterns to create flexible, maintainable architectures ([details](kb/composition-over-inheritance-principle.md))

### 4. Code Smells Detection and Prevention

**Code smells** are surface indicators of deeper problems in code design. Based on Martin Fowler's Refactoring catalog, watch for these common issues:

**Bloaters** (code that has grown too large):
- **Long Method**: Methods > 20 lines should be broken down ([details](kb/code-smell-long-method.md))
- **Large Class**: Modules > 800 lines or > 12 functions need decomposition ([details](kb/code-smell-large-class.md))
- **Long Parameter List**: Functions with > 4 parameters need parameter objects ([details](kb/code-smell-long-parameter-list.md))
- **Data Clumps**: Related parameters appearing together frequently ([details](kb/code-smell-data-clumps.md))
- **Primitive Obsession**: Using primitives instead of domain objects ([details](kb/code-smell-primitive-obsession.md))

**Duplication** (most critical smell):
- **Duplicate Code**: Identical logic in multiple places - extract common functions ([details](kb/code-smell-duplicate-code.md))

**Change Preventers** (code that resists modification):
- **Divergent Change**: One module changing for multiple reasons
- **Shotgun Surgery**: One change requiring edits across many modules

**Couplers** (excessive coupling between modules):
- **Feature Envy**: Method using more data from another module than its own ([details](kb/code-smell-feature-envy.md))
- **Message Chains**: Long chains of method calls across modules

**Quick Detection**:
```bash
# Find long methods
grep -n "^[a-zA-Z_][a-zA-Z0-9_]*(" modules/*.c | while read line; do
  file=$(echo $line | cut -d: -f1); line_num=$(echo $line | cut -d: -f2)
  lines=$(awk -v start=$line_num 'NR >= start && /^}/ && --brace_count <= 0 {print NR-start; exit} /{/ {brace_count++} /}/ {brace_count--}' $file)
  [ "$lines" -gt 20 ] && echo "$file:$line_num - $lines lines"
done

# Find parameter list issues
grep -n "([^)]*,[^)]*,[^)]*,[^)]*," modules/*.h

# Find duplicate error messages
grep -r "ar_log__error" modules/ | cut -d'"' -f2 | sort | uniq -c | sort -nr | awk '$1 > 1'
```

### 5. Coding Standards

**String Parsing**: Track quote state when scanning for operators (`:=`, `(`, `)`, `,`) ([details](kb/string-parsing-quote-tracking.md))
```c
bool in_quotes = false;
while (*p) {
    if (*p == '"' && (p == start || *(p-1) != '\\')) in_quotes = !in_quotes;
    else if (!in_quotes && /* check operator */) { /* process */ }
    p++;
}
```

**Naming Patterns** (verify with grep before large changes; fix errors before warnings):
| Type | Pattern | Example |
|------|---------|---------|
| Module functions | `ar_<module>__<function>` | `ar_data__create_map()` (C & Zig) |
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

### 6. Module Development

**Core Architecture**:
- **Parsing vs Evaluation**: Data owner parses (methodology→methods), consumer evaluates (interpreter→ASTs)
- **Opaque Types**: `typedef struct ar_type_name_s ar_type_name_t;` in header, definition in .c only ([details](kb/opaque-types-pattern.md))
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
✓ Named constants > magic numbers (e.g., MEMORY_PREFIX_LEN = 7)
✓ Incremental changes with frequent compilation
✓ Verify each change with tests
✓ Remove unused functions immediately
✓ Address all warnings immediately
✓ Think twice before adding global state  
✓ Update modules/README.md for new modules

### 7. Method Development

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

### 8. Development Practices

**Directory Navigation**: Always use absolute paths ([details](kb/absolute-path-navigation.md))
```bash
cd /Users/quenio/Repos/agerun/bin  # Correct
cd bin  # Wrong - avoid relative paths
```

**Backup Files**: Never create them - use git instead
- `git stash` for temporary saves
- `git diff > changes.patch` for patches
- If created accidentally: `git reset HEAD <file>`

**Script Development**: All new, reusable Python/Bash scripts should be added under the `./scripts` dir
- Add directory check to ensure scripts run from repo root
- Create corresponding make target for user-facing scripts
- Scripts should fail with helpful error messages suggesting the make target
- Document the make target in the Makefile help section
- NEVER use Makefile variables $(VAR) in bash scripts - causes command substitution errors
- Build output: quiet success, verbose failure (hide output when working, show full errors)
- **Parallel builds**: Shared .PHONY deps → hoist to parent target (e.g., install-scan-build → full-build)

**Debug Tools**:
- **Memory**: `make sanitize-tests` → Check `bin/memory_report_<test_name>.log`
- **Static Analysis**: `make analyze-exec` (requires scan-build: `brew install llvm` or `apt install clang-tools`)
- **Abort traps**: `lldb -o "run" -o "bt" -o "quit" ./test_binary` for crash backtraces
- **Build verification**: `strings bin/*/agerun | grep DEBUG` confirms debug builds
- **Test Failures**: Often just wrong directory - 4-step check: pwd → cd /path → pwd → run
- **Pattern Testing**: Test regex/sed/awk patterns before using in scripts
- **Doc Validation**: `make full-build` validates file refs, function names, types
- **CI Debugging**: Add log capture + display on failure (see full_build.sh analyze-exec handling)

**Expression Ownership** (CRITICAL):
- References (`memory.x`): Don't destroy - borrowed from memory/context
- New objects (`2+3`, `"a"+"b"`): Must destroy - evaluator creates them
- Self-ownership check: Never let `values_result == mut_memory`
- Context lifetime: NEVER destroy context or its elements in evaluators
- Debug ownership: Use `ar_data__hold_ownership()` to verify status

### 9. Error Propagation Pattern

**Implementation**: Set errors at source → Store in struct → Propagate via get_error() → Print once at top level ([details](kb/error-propagation-pattern.md))
**Key Rule**: Evaluators set errors, interpreter prints them. Never print errors where they occur.

### 10. Agent Lifecycle

**Critical Points**:
- Agents receive `__wake__` on creation
- Agents receive `__sleep__` before destruction
- ALWAYS process messages after sending to prevent leaks
- Call `ar_system__process_next_message()` after `ar_agent__send()`

### 11. Building Individual Tests

Always use make to build tests:
```bash
make test_name  # Build individual test
```
Never compile directly with gcc.

**Parallel Build System**: Targets use isolated directories (bin/run-tests/, bin/sanitize-tests/) to enable parallel execution without conflicts.

**Important Makefile Features**:
- When building individual tests, the test is automatically executed after building
- Module changes are automatically rebuilt (no need to rebuild the library separately)
- The Makefile handles all dependencies and runs the test from the correct directory
- Pattern rules compile object files - update these directly for consistent flags (not target-specific vars)
- Example: `make ar_string_tests` will:
  1. Rebuild any changed modules
  2. Rebuild the test
  3. Run the test automatically from the bin directory
  4. Generate a memory report specific to that test

### 12. Session & Commit Management

**Task Management**:
- **Session todos (TodoWrite/TodoRead)**: Current TDD cycles, implementations, bug fixes
- **TODO.md file**: Long-term architecture, future features (check [ ] vs [x] for completion)
- **User feedback**: May reveal design issues, not just implementation bugs. Listen for concerns about output/behavior/consistency. Verify assumptions before acting.
- **Todo list integrity**: Mark items complete, never remove them - preserves task history

**Pre-Commit Checklist** (MANDATORY - NO EXCEPTIONS):
1. `make full-build` - Fix ALL issues before proceeding (includes doc validation) ([details](kb/build-verification-before-commit.md))
   - **Exception**: Type renames only need `make check-naming && make run-tests`
   - **Exception**: Doc-only changes only need `make check-docs`
   - **Exception**: Comment-only changes only need `make check-naming`
   - **Exception**: Skip tests if just run successfully (avoid redundant execution)
2. **Update module .md files if interfaces changed** - CRITICAL: Interface changes MUST include docs in same commit
3. **Review ALL documentation for outdated references** - Refactoring often leaves stale docs
4. `grep -l "function_name" modules/*.md` - Check docs for any API changes
   - **Hybrid modules**: Update both ar_io.md AND README.md to note Zig/C split approach
5. Update TODO.md - Mark completed, add new tasks
6. Update CHANGELOG.md (NON-NEGOTIABLE)
7. `git diff` - Verify all changes intentional
8. Check for backup files outside ./bin (*.backup, *.bak, *.tmp)
9. Remove any log files: `find . -name "*.log" -type f | grep -v bin/ | xargs rm -f`
10. Only then: `git commit`

**Remember**: Complete ALL TDD Cycles → Docs → TODO → CHANGELOG → Commit

**After Completing Major Tasks**:
- Document completion date in TODO.md (e.g., "Completed 2025-06-11")
- Move tasks from "In Progress" to "Completed Tasks" section
- Include brief summary of what was accomplished
- Update CLAUDE.md with any new patterns or learnings from the session ([details](kb/systematic-guideline-enhancement.md))

### 13. Refactoring Patterns

**Core Principles**:
- **Preserve behavior**: Tests define expected behavior - fix implementation, not tests
- **Move code, don't rewrite**: Use diff to verify code is moved, not reimplemented  
- **Clean state recovery**: If refactoring fails, revert completely rather than debug
- **Validate changes**: After adding validation, test with intentional errors to ensure no false negatives
- **Incremental commits**: Commit logical chunks even with remaining issues - note them for future work
- **Frame migration**: Convert evaluators incrementally; facade manages both patterns during transition
- **Complete API verification**: `grep -r "old_api_pattern" .` after interface changes - check ALL clients ([details](kb/api-migration-completion-verification.md))

**Bulk Renaming Pattern**:
```bash
# PREFERRED: Use rename_symbols.py for safe whole-word renaming
python3 scripts/rename_symbols.py --group <group-name> --live

# AVOID sed for bulk renames - error-prone with partial matches
# If rename_symbols.py doesn't support your case, enhance it first ([details](kb/script-enhancement-over-one-off.md))
# Script supports both type renaming (TYPE_RENAMES) and module renaming (MODULE_RENAMES) - see kb/module-renaming-script-enhancement.md
# Only use sed for one-off changes with careful verification
```

**Code Movement Verification**:
```bash
# MANDATORY: Verify code is moved, not rewritten ([details](kb/code-movement-verification.md))
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
- **Deprecation pattern**: `(void)param; return NULL/0;` with DEPRECATED docs/comments
- **Facade redundancy**: If specialized modules log errors, facades shouldn't duplicate
- **Migration verification**: Use TDD cycles even for mechanical refactoring (test→break→fix→verify)
- **Frame migration**: Include facade update as separate TDD cycle in same plan; create evaluators upfront
- **Agent context**: Agent instructions use separate context objects, not memory as context

### 14. Plan Verification and Review

**When Creating Development Plans**:
- **Single task focus**: Create plans for one todo item at a time, not multi-task plans
- **Always include critical verification steps**: Plans must include diff verification, test running, memory checking
- **User feedback is valuable**: If user points out missing steps, update the plan immediately ([details](kb/user-feedback-as-qa.md))
- **Example**: "We're missing the comparison of previous and new implementation" - this feedback prevents bugs
- **Documentation oversight**: User catching missing docs is common - verify docs are part of plan
- **Plan completeness checklist**:
  - [ ] Verification steps included (diff, tests, memory)
  - [ ] Documentation updates specified for interface changes
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

**Generic Module Design Patterns**:
- **Consider generic solutions**: User may suggest specific module (e.g., memory_path), but generic approach may be better
- **Example**: Creating ar_path instead of ar_memory_path provides broader utility
- **Benefits of generic design**:
  - Reduces overall code duplication
  - Provides reusable functionality
  - Supports multiple use cases (variable paths, file paths)
  - Better long-term maintainability
- **Always propose generic alternative**: When user suggests specific module, explain generic benefits ([details](kb/oo-to-c-adaptation.md))

### 15. Task Tool Guidelines

**Core Rule**: Read before write - examine Task output before modifying

**Workflow**: 
1. Use Task to analyze/create files
2. Read the output with Read tool
3. Use Edit (not Write) for improvements
4. Be specific: "analyze only" vs "create files"

### 16. Leveraging Zig for AgeRun Development

**Zig Documentation Reference**: https://ziglang.org/documentation/0.14.1/
**C Interop Guide**: https://ziglang.org/documentation/0.14.1/#C - Follow for full C compatibility

**When to Consider Zig**:
- Performance-critical components requiring zero-cost abstractions
- Cross-platform modules where C portability becomes complex
- New modules that would benefit from compile-time safety guarantees
- Components requiring precise memory layout control
- Pure computation modules (parsers, validators) - often need no heap

**Key Zig Features for AgeRun**:
- **Manual Memory Management**: Aligns with AgeRun's ownership model
- **Compile-time Safety**: Catches errors before runtime
- **C Interoperability**: Can interface directly with existing AgeRun C modules
- **No Hidden Control Flow**: Matches AgeRun's explicit design philosophy
- **Error Handling**: Explicit error unions complement AgeRun's error propagation

**Integration Guidelines**:
- Maintain C API compatibility for Zig modules
- Follow AgeRun's ownership conventions (own_, mut_, ref_)
- Use Zig's allocator pattern with AgeRun's heap tracking
- Compile-time validation for method language constraints
- Zero-cost abstractions for performance-critical paths
- Type mappings: `c_int`→`c_int`, `char*`→`?[*:0]u8`, `c_uchar`→`u8`, string literals→`@as([*c]const u8, "str")`
- Debug detection: `builtin.mode == .Debug or builtin.mode == .ReleaseSafe` (not `#ifdef DEBUG`)
- Exit-time safety: Check `!g_initialized` early, never call init during cleanup
- Avoid C macros returning void/anyopaque (e.g., AR_ASSERT_*)
- Audit dependencies before conversion - remove unused includes
- Delete C file when creating Zig replacement + update all .md refs (no Makefile changes needed)
- Zig funcs use same `ar_<module>__<function>` naming as C, static funcs use `_<name>` with snake_case
- Clean imports (inline functions only): `const ar_assert = @import("ar_assert.zig"); const ar_assert__func = ar_assert.ar_assert__func;`
- C header imports: Keep related headers in same @cImport block (macros need dependencies)
- Zig modules with exports: Access via C headers to avoid duplicate symbols at link time
- Create ar_assert.zig for Zig modules (C modules keep using ar_assert.h macros)
- Circular dependencies: Use stack allocation to break heap→io→heap cycles
- Platform differences: Handle stderr/stdout as functions on macOS with c.stderr()
- errno access: Create helper functions like getErrno() for cross-platform compatibility
- **Variadic functions**: Implement in C, not Zig (platform va_list incompatibility) - use hybrid approach
- **Build flags**: Add `-lc -fno-stack-check` to Zig build-obj for C interop compatibility
- **Ubuntu strictness**: Test on Linux CI first - catches header paths, linking, runtime issues early

**Example Integration Pattern**:
```zig
// Zig module exposing C-compatible interface
const std = @import("std");
const c = @cImport({
    @cInclude("ar_heap.h");
    @cInclude("ar_data.h");
});

// Export functions with C ABI
export fn ar_zigmodule__create() ?*c.ar_data_t {
    // Use AgeRun's heap tracking
    const data = c.ar_data__create_string("Zig integrated!");
    return data;
}
```

**Build Integration**:
- Add Zig compiler detection to Makefile
- Create hybrid build rules for C/Zig modules
- Maintain consistent test infrastructure
- Use Zig's built-in testing alongside AgeRun tests
- Verify Zig builds: check strings in .o for "zig X.X.X"
- Pattern rule: `$(ZIG) build-obj -O ReleaseSafe -target native`

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
ar_methodology__cleanup();
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
- Assume error causes - always verify with evidence

**Always**:
- Use heap tracking macros
- Follow TDD cycle
- Document ownership
- Run sanitizer before commit
- End files with newline
- Use make for builds
- Process messages after sending
- Read README.md for project overview and user documentation
- Verify working directory before commands
- Listen to user feedback as quality assurance
