# AgeRun Development Guide for Claude

This document contains essential instructions for Claude Code to assist with AgeRun development.

## Project Overview

AgeRun is a lightweight, message-driven agent system where each agent is defined by a single method responsible for handling all incoming messages. Agents operate within a persistent runtime environment, allowing dynamic creation, versioning, pausing, resumption, and destruction.

## Quick Start

**Primary Build Tool**: `make build` - runs everything with minimal output (~20 lines)
- Use before commits and for quick verification
- Includes: build, static analysis, all tests, sanitizers, leak check, doc validation

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

**Build behavior**: `make build` builds from current state (no clean). To build clean, run: `make clean build`

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
- **/reports**: Analysis reports and technical comparisons (all .md analysis files go here)
- **.claude/commands**: Slash command definitions (must use make targets, not direct scripts)

## Critical Development Rules

### Knowledge Base Usage (MANDATORY) ([details](kb/knowledge-base-consultation-protocol.md))

**KB articles are mandatory prerequisites, NOT optional references**:
- **Before ANY task**: Search for relevant KB articles using grep on CLAUDE.md
- **MUST read BEFORE executing**: KB articles contain critical patterns to prevent errors
- **Task planning**: First todo item must be "Read kb/relevant-article.md"
- **Search first**: When asked about a topic, check if there's a kb link before answering
- **Example**: Before refactoring → read kb/refactoring-key-patterns.md FIRST
- **Example**: Before debugging → read kb/memory-debugging-comprehensive-guide.md FIRST

**Workflow**: Task requested → Search KB → Read articles → THEN execute task

**Markdown Link Resolution** ([details](kb/markdown-link-resolution-patterns.md)):
- **Always use relative paths** - Never use `/path` format which breaks on GitHub
- **Resolve from file location** - `kb/article.md` from CLAUDE.md → `./kb/article.md`
- **Navigate directories** - Use `../kb/article.md` from subdirectories
- **Validation** - Links must work identically on GitHub, VS Code, and Claude Code

### 0. Documentation Protocol

**Always search CLAUDE.md first** when asked about procedures. Don't overthink - start with exact keywords.
**When reading TODO.md**: Check [ ] = incomplete, [x] = complete. Read completion dates. Be explicit - list all components.

**Documentation Standards (MANDATORY)** ([details](kb/documentation-standards-integration.md)):
- **Real code only**: All examples must use actual AgeRun types/functions ([details](kb/validated-documentation-examples.md))
- **Validation required**: Run `make check-docs` before committing any .md files ([details](kb/documentation-validation-enhancement-patterns.md))
- **Specification consistency**: Keep SPEC.md updated after language changes ([details](kb/specification-consistency-maintenance.md))
- **Proper tagging**: Use `// EXAMPLE:`, `// BAD:`, `// ERROR:` for hypothetical code ([details](kb/example-marker-granular-control.md))
- **Markdown links**: Use relative paths only - see link resolution rules ([details](kb/markdown-link-resolution-patterns.md))
- **Preferred types**: `ar_data_t*`, `ar_agent_t*`, `ar_expression_ast_t*`, `ar_instruction_ast_t*`
- **Helper reference**: `python3 scripts/get_real_types.py --guide` for available APIs

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
- Expression ownership: memory.x=reference, arithmetic/strings=new object ([details](kb/expression-ownership-rules.md))
- Map iteration: get keys, destroy list & elements; persist with key/type then value
- Wake/sleep messages: Agents mark ownership, system drops before destroy ([details](kb/ownership-drop-message-passing.md))
- Ownership patterns: Use `ar_data__claim_or_copy()` & `ar_data__destroy_if_owned()` ([details](kb/ownership-pattern-extraction.md))

**Memory Leak Detection**:
- Full test suite: Check console for "WARNING: X memory leaks detected"
- Individual test reports: `bin/memory_report_<test_name>.log` ([details](kb/memory-leak-detection-workflow.md))
- Complete verification: `grep "Actual memory leaks:" bin/memory_report_*.log | grep -v "0 (0 bytes)"`
- Always run `make sanitize-tests` before committing
- Debug strategy: Check report → Trace source → Verify ownership → Fix naming → Add cleanup ([details](kb/memory-debugging-comprehensive-guide.md))
- Environment variables: `ASAN_OPTIONS=halt_on_error=0` (continue on error), `detect_leaks=1` (complex leaks)
- Wake messages: Process with `ar_system__process_next_message()` after agent creation ([details](kb/agent-wake-message-processing.md))

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

**Example**: Multiple cycles (create/destroy, evaluate, wrapper) → Only then docs/commit

**Test Requirements**:
- Every module MUST have tests with BDD structure ([details](kb/bdd-test-structure.md))
- One test per behavior, isolated & fast, zero leaks
- Names reflect behavior ([details](kb/test-function-naming-accuracy.md))
- Global cleanup: `ar_methodology__cleanup()` & `ar_agency__reset()`
- Process messages: `while (ar_system__process_next_message());`
- Process wake messages after agent creation to prevent leaks ([details](kb/agent-wake-message-processing.md))
- Use fixtures when available, run with `make test_name`
- Adapt fixtures when APIs change ([details](kb/test-fixture-api-adaptation.md))

**TDD Advanced**: Break large refactoring into sequential cycles, one behavior each ([details](kb/tdd-advanced-large-refactoring.md))
**API Simplification**: Use TDD for safe API changes ([details](kb/tdd-api-simplification.md))

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

**Architectural Patterns**: Interface segregation > Registry > Facade (coordinate only) > Parser/Executor > Callbacks ([details](kb/architectural-patterns-hierarchy.md))

**Code Duplication Prevention (DRY - Don't Repeat Yourself)**:
```bash
# Before writing code, search for existing:
grep -r "function_name\|concept" modules/
```

**DRY**: Stop copying → extract common functions → use data tables → parameterize → verify moves with diff

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

**Quick Detection**: Use automated scripts to find long methods, parameter issues, duplicate errors ([details](kb/code-smell-quick-detection.md))

### 5. Coding Standards

**String Parsing**: Track quote state when scanning for operators ([details](kb/string-parsing-quote-tracking.md))

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
- **Documentation**: Real AgeRun types/functions only, validate with `make check-docs`
- **Doc checker supports**: C functions/types, Zig pub functions/types, and `module.function` syntax ([details](kb/multi-language-documentation-validation.md))

### 6. Module Development

**Core Architecture**:
- **Parsing vs Evaluation**: Data owner parses (methodology→methods), consumer evaluates (interpreter→ASTs)
- **Opaque Types**: `typedef struct ar_<module>_s ar_<module>_t;` in header, definition in .c only ([details](kb/opaque-types-pattern.md))
- **Module Size**: Split at ~850 lines into focused modules (e.g., agency→4 modules)
- **Parser aliases**: Support all function name variants ([details](kb/parser-function-alias-support.md))

**Key Patterns**:
- Parse once, evaluate many times (store ASTs, not source)
- Use dynamic collections (lists/maps), not fixed arrays
- String-based IDs for reliable persistence
- Read interface first instead of guessing function names
- No platform-specific code (`#ifdef __linux__` forbidden)
- Create public APIs to eliminate duplication across modules
- Delegation: Pass log through layers for error propagation ([details](kb/module-delegation-error-propagation.md))
- Simplify instructions to single responsibility ([details](kb/instruction-behavior-simplification.md))

**Code Quality**: Functions <50 lines, params ≤5, named constants, remove unused code, validate docs ([details](kb/module-quality-checklist.md))

### 7. Method Development

**Requirements**:
- Store in `methods/` as `<name>-<version>.method`
- Create corresponding `.md` documentation **with real AgeRun types/functions only**
- Create `<name>_tests.c` test file
- Tests must verify memory state after execution
- Use relative path `../methods/` from bin directory
- **Validate documentation**: Run `make check-docs` before committing

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

**Script Development**: All new, reusable Python/Bash scripts should be added under the `./scripts` dir ([details](kb/progressive-tool-enhancement.md))
- Add directory check to ensure scripts run from repo root
- Create corresponding make target for user-facing scripts
- Scripts should fail with helpful error messages suggesting the make target
- Document the make target in the Makefile help section
- NEVER use Makefile variables $(VAR) in bash scripts - causes command substitution errors
- Build output: quiet success, verbose failure (hide output when working, show full errors)
- **Parallel builds**: Shared .PHONY deps → hoist to parent target (e.g., install-scan-build → build)

**Debug Tools**: Memory (`make sanitize-tests`), static analysis, crashes (lldb), patterns testing ([details](kb/development-debug-tools.md))

**Expression Ownership** (CRITICAL):
- References (`memory.x`): Don't destroy - borrowed from memory/context
- New objects (`2+3`, `"a"+"b"`): Must destroy - evaluator creates them
- Self-ownership check: Never let `values_result == mut_memory`
- Context lifetime: NEVER destroy context or its elements in evaluators
- Debug ownership: Use `ar_data__take_ownership()` to verify status

### 9. Error Propagation Pattern

**Implementation**: Set errors at source → Store in struct → Propagate via get_error() → Print once at top level ([details](kb/error-propagation-pattern.md))
**Key Rule**: Evaluators set errors, interpreter prints them. Never print errors where they occur.

### 10. Agent Lifecycle

**Critical Points**:
- Agents receive `__wake__` on creation
- Agents receive `__sleep__` before destruction
- ALWAYS process messages after sending to prevent leaks
- Call `ar_system__process_next_message()` after `ar_agent__send()`
- Agents mark themselves as owners of wake/sleep messages ([details](kb/ownership-drop-message-passing.md))
- Tests must process wake messages to prevent leaks ([details](kb/agent-wake-message-processing.md))

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
- **Pattern rule updates**: Change all 6 targets when updating Zig flags ([details](kb/makefile-pattern-rule-management.md))
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
1. `make clean build` - Fix ALL issues before proceeding (includes doc validation) ([details](kb/build-verification-before-commit.md))
   - **Report build time**: Include duration from output (e.g., "took 1m 3s") ([details](kb/build-time-reporting.md))
   - **Exception**: Type renames only need `make check-naming && make run-tests`
   - **Exception**: Doc-only changes only need `make check-docs`
   - **Exception**: Comment-only changes only need `make check-naming`
   - **Exception**: Skip tests if just run successfully (avoid redundant execution)
2. **Update module .md files if interfaces changed** - CRITICAL: Interface changes MUST include docs in same commit; language migrations need dependency tree updates ([details](kb/documentation-language-migration-updates.md))
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
- **Ownership transitions**: Update naming immediately when changing ref_→own_ ([details](kb/refactoring-ownership-transitions.md))
- **Dependency injection**: Avoid when 1:1 relationships exist ([details](kb/dependency-injection-anti-pattern.md))
- **Post-refactoring cleanup**: Remove obsolete modules after major refactoring ([details](kb/module-removal-checklist.md))

**Bulk Operations Patterns**:
- **Symbol renaming**: Use `rename_symbols.py` for safe whole-word renaming ([details](kb/script-enhancement-over-one-off.md))
- **Function renaming**: Follow systematic pattern for codebase-wide function renames ([details](kb/systematic-function-renaming-pattern.md))
- **Documentation fixes**: Use `batch_fix_docs.py` for large-scale documentation error resolution ([details](kb/automated-batch-documentation-fixes.md))
- **Instruction renaming**: Follow systematic pattern for language changes ([details](kb/language-instruction-renaming-pattern.md))
- **Search-and-replace precision**: Avoid unintended changes with careful targeting ([details](kb/search-replace-precision.md))

```bash
# PREFERRED: Use specialized scripts for bulk operations
python3 scripts/rename_symbols.py --group <group-name> --live
python3 scripts/batch_fix_docs.py --dry-run  # Preview changes first

# AVOID sed for bulk renames - error-prone with partial matches
# If existing scripts don't support your case, enhance them first
# Script supports both type renaming (TYPE_RENAMES) and module renaming (MODULE_RENAMES) - see kb/module-renaming-script-enhancement.md
# Only use sed for one-off changes with careful verification
```

**Code Movement Verification**:
```bash
# MANDATORY: Verify code is moved, not rewritten ([details](kb/code-movement-verification.md))
diff -u <(sed -n '130,148p' original.c) <(sed -n '11,29p' new.c)
```

**Key Patterns**: Module cohesion, code movement verification, test preservation, complete implementations ([details](kb/refactoring-key-patterns.md))
**Defensive consistency**: Always use defensive APIs even when ownership is "known" ([details](kb/defensive-programming-consistency.md))

### 14. Plan Verification and Review

**When Creating Development Plans**:
- **Single task focus**: Create plans for one todo item at a time, not multi-task plans
- **Always include critical verification steps**: Plans must include diff verification, test running, memory checking
- **User feedback is valuable**: If user points out missing steps, update the plan immediately ([details](kb/user-feedback-as-qa.md))
- **Example**: "We're missing the comparison of previous and new implementation" - this feedback prevents bugs
- **Documentation oversight**: User catching missing docs is common - verify docs are part of plan
- **Debugging assumptions**: Challenge all hypotheses with evidence ([details](kb/user-feedback-debugging-pattern.md))
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
- Modules with complex error cleanup patterns - use defer ([details](kb/zig-defer-error-cleanup-pattern.md))
- Evaluator migrations: Start simple→complex for risk reduction ([details](kb/evaluator-migration-priority-strategy.md))

**Key Zig Features for AgeRun**:
- **Manual Memory Management**: Aligns with AgeRun's ownership model
- **Compile-time Safety**: Catches errors before runtime
- **C Interoperability**: Can interface directly with existing AgeRun C modules
- **No Hidden Control Flow**: Matches AgeRun's explicit design philosophy
- **Error Handling**: Explicit error unions complement AgeRun's error propagation

**Integration Guidelines**: See comprehensive guide for all details ([details](kb/zig-integration-comprehensive.md))

**Migration Process**: Rename .c→.bak before testing Zig implementation - C takes precedence ([details](kb/c-to-zig-module-migration.md))

**Zig Best Practices** (NEW):
- **Type Usage**: Use concrete Zig types for your module, C types for others ([details](kb/zig-type-usage-patterns.md))
- **No unnecessary casts**: Return/accept concrete types to eliminate `@ptrCast`
- **Direct field access**: Use `param.?.field` instead of creating temporary variables
- **Const correctness**: Make parameters const if never mutated - update headers too
- **Skip helper functions**: Direct calls often cleaner than C-style helpers
- **Verify API behavior**: Read function docs/impl before use ([details](kb/api-behavior-verification.md))
- **Defer limitations**: Manual cleanup still needed when resource used later

**Zig Integration**: Use ar_allocator, maintain C API, follow ownership conventions ([details](kb/zig-integration-comprehensive.md))
**Ownership with claim_or_copy**: Essential pattern for evaluator migrations ([details](kb/zig-ownership-claim-or-copy-pattern.md))

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
- **Message accessor** - `message.field` returns references like memory/context ([details](kb/expression-evaluator-accessor-extension.md))

## Method Test Template

Directory check, clean state, test code, init system ([details](kb/method-test-template.md))

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
