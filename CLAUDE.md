# AgeRun Development Guide for Claude

This document contains essential instructions for Claude Code to assist with AgeRun development.

## Project Overview

AgeRun is a lightweight, message-driven agent system where each agent is defined by a single method responsible for handling all incoming messages. Agents operate within a persistent runtime environment, allowing dynamic creation, versioning, pausing, resumption, and destruction.

## Quick Start

**Primary**: `make build 2>&1` → `make check-logs` (MANDATORY sequence) ([details](kb/quick-start-commands.md))
**Clean build**: `make clean build 2>&1` → `make check-logs`  
**Help**: `make` shows all targets
**Scripts**: Use make targets, not direct execution from `/scripts/`

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
**Check SPEC.md before implementing** features to avoid unnecessary work ([details](kb/specification-consistency-maintenance.md))
**When reading TODO.md**: Check [ ] = incomplete, [x] = complete. Read completion dates. Be explicit - list all components.
**KB articles need CLAUDE.md references** to be accessible in future sessions ([details](kb/claude-md-reference-requirement.md))
**Evidence-based debugging**: Always verify with concrete evidence, not assumptions ([details](kb/evidence-based-debugging.md))
**Build output review**: Review complete command output, not just final status ([details](kb/comprehensive-output-review.md))
**Documentation compaction**: Extract verbose content to KB while preserving links ([details](kb/documentation-compacting-pattern.md))
**Selective compaction**: Compact only completed items in mixed-state documents ([details](kb/selective-compaction-pattern.md))
**Quantitative metrics**: Use specific numbers instead of vague descriptions ([details](kb/quantitative-documentation-metrics.md))
**Self-documenting changes**: Add entries for significant modifications ([details](kb/self-documenting-modifications-pattern.md))
**Retroactive tracking**: Add completed unplanned tasks to TODO.md ([details](kb/retroactive-task-documentation.md))

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
- Message ownership flow: System takes ownership after dequeuing from agents ([details](kb/message-ownership-flow.md))
- Ownership gap vulnerability: Ensure atomic ownership transfer to prevent corruption ([details](kb/ownership-gap-vulnerability.md))
- Ownership patterns: Use `ar_data__claim_or_copy()` & `ar_data__destroy_if_owned()` ([details](kb/ownership-pattern-extraction.md))
- Temporary resources: Creator destroys contexts since agents store as references ([details](kb/temporary-resource-ownership-pattern.md))
- API ownership on failure: Verify implementation to understand cleanup requirements ([details](kb/api-ownership-on-failure.md))
- Context lifecycle: Destroy resources where variables are still in scope ([details](kb/context-ownership-lifecycle.md))

**Memory Leak Detection**:
- Full test suite: Check console for "WARNING: X memory leaks detected"
- Individual test reports: `bin/run-tests/memory_report_<test_name>.log` ([details](kb/memory-leak-detection-workflow.md))
- Complete verification: `grep "Actual memory leaks:" bin/run-tests/memory_report_*.log | grep -v "0 (0 bytes)"`
- Always run `make sanitize-tests 2>&1` before committing
- Tests using dlsym are automatically excluded from sanitizer builds ([details](kb/sanitizer-test-exclusion-pattern.md))
- Debug strategy: Check report → Trace source → Verify ownership → Fix naming → Add cleanup ([details](kb/memory-debugging-comprehensive-guide.md))
- Test leak analysis: Verify lifecycle management before adding cleanup ([details](kb/test-memory-leak-ownership-analysis.md))
- Environment variables: `ASAN_OPTIONS=halt_on_error=0` (continue on error), `detect_leaks=1` (complex leaks)
- Wake messages: Process with `ar_system__process_next_message_with_instance(own_system)` after agent creation ([details](kb/agent-wake-message-processing.md))

### 2. Test-Driven Development (MANDATORY)

**Pre-modification**: Run module tests BEFORE changes
**Cycle**: Red→Green→Refactor for EACH behavior, NO commits during ([details](kb/red-green-refactor-cycle.md), [cycle details](kb/tdd-cycle-detailed-explanation.md))
**Complete ALL cycles** → Update docs/TODO/CHANGELOG → Single commit

**Test Requirements**: BDD structure, one test per behavior, zero leaks ([details](kb/bdd-test-structure.md))
**Cleanup**: `ar_methodology__cleanup()` & `ar_agency__reset()`
**Messages**: Process all including wake messages ([details](kb/agent-wake-message-processing.md))
**Advanced patterns**: ([details](kb/tdd-advanced-large-refactoring.md), [API changes](kb/tdd-api-simplification.md))

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


**Note**: Examples assume `own_system`, `mut_agency`, and other instance variables are available. In practice, these would be created via fixtures or passed as parameters.
**Architectural Patterns**: Interface segregation > Registry > Facade (coordinate only) > Parser/Executor > Callbacks ([details](kb/architectural-patterns-hierarchy.md))

**Code Duplication Prevention (DRY - Don't Repeat Yourself)**:
```bash
# Before writing code, search for existing:
grep -r "function_name\|concept" modules/
```

**DRY**: Stop copying → extract common functions → use data tables → parameterize → verify moves with diff
**Domain types**: Create type-safe abstractions instead of primitive obsession ([details](kb/domain-specific-type-creation.md))

**Red Flags**: Module A→B→C→A cycles, repeated validation logic, similar function names (_for_int, _for_string), parsing+execution together

**Exception**: heap ↔ io circular dependency is accepted and documented.

### Additional Design Principles

**Core Standards**:
- **Const-Correctness**: NEVER cast away const - fix interfaces instead ([details](kb/const-correctness-principle.md))
- **No Parallel Implementations**: Modify existing code, don't create _v2 versions ([details](kb/no-parallel-implementations-principle.md))
- **Composition Over Inheritance**: Prefer composition patterns to create flexible, maintainable architectures ([details](kb/composition-over-inheritance-principle.md))
- **Regression Investigation**: Trace root causes with git history ([details](kb/regression-root-cause-analysis.md))

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
| C/C-ABI module functions | `ar_<module>__<function>` | `ar_data__create_map()` |
| Zig struct modules | `TitleCase` | `DataStore.zig` |
| Zig struct functions | `camelCase` | `getAllKeys()` |
| Static functions | `_<function>` | `_validate_file()` |
| Test functions (C) | `test_<module>__<name>` | `test_string__trim()` |
| Test files (Zig) | `<Module>Tests.zig` | `DataStoreTests.zig` |
| Heap macros | `AR__HEAP__<OP>` | `AR__HEAP__MALLOC` |
| Assert macros | `AR_ASSERT_<TYPE>` | `AR_ASSERT_OWNERSHIP` |
| Opaque types | `typedef struct ar_<module>_s ar_<module>_t;` | |
| C-ABI files | `ar_<module>.{h,c,zig}` | `ar_data.h` |

**Key Standards**:
- 4-space indent, 100-char lines, newline at EOF
- `/** */` docs with `@param`, `@return`, `@note` ownership
- Use IO module (`ar_io__open_file` not `fopen`, etc) - check all return codes
- Use `PRId64`/`PRIu64` for portability, never `%lld`
- **Documentation**: Real AgeRun types/functions only, validate with `make check-docs`
- **Doc checker supports**: C functions/types, Zig pub functions/types, and `module.function` syntax ([details](kb/multi-language-documentation-validation.md))

### 6. Module Development

**Architecture**: Owner parses→consumer evaluates, opaque types, ~850 line limit ([details](kb/module-development-patterns.md))
**Patterns**: Parse once→evaluate many, dynamic collections, string IDs ([details](kb/opaque-types-pattern.md))
**Quality**: <50 lines/function, ≤5 params, validate docs ([details](kb/module-quality-checklist.md))
**Instantiation**: Bottom-up by dependencies ([details](kb/module-instantiation-prerequisites.md))

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

**Navigation**: Use absolute paths only ([details](kb/absolute-path-navigation.md))
**Backups**: Use git stash/diff, never .bak files
**Scripts**: Add to `/scripts/` with make targets ([details](kb/progressive-tool-enhancement.md))
**Debug**: `make sanitize-tests 2>&1`, always redirect stderr ([details](kb/development-debug-tools.md))
**Ownership debugging**: Add logging at transfer points to trace corruption ([details](kb/debug-logging-ownership-tracing.md))
**Expression ownership**: References=borrowed, new objects=destroy ([details](kb/expression-ownership-rules.md))

### 9. Error Propagation Pattern

**Implementation**: Set errors at source → Store in struct → Propagate via get_error() → Print once at top level ([details](kb/error-propagation-pattern.md))
**Key Rule**: Evaluators set errors, interpreter prints them. Never print errors where they occur.

### 10. Agent Lifecycle

**Critical Points**:
- Agents receive `__wake__` on creation
- Agents receive `__sleep__` before destruction
- ALWAYS process messages after sending to prevent leaks
- Call `ar_system__process_next_message_with_instance(own_system)` after `ar_agent__send()`
- Agents mark themselves as owners of wake/sleep messages ([details](kb/ownership-drop-message-passing.md))
- Tests must process wake messages to prevent leaks ([details](kb/agent-wake-message-processing.md))

### 11. Building Individual Tests

Always use make to build tests:
```bash
make test_name 2>&1  # Build individual test (with stderr)
```
Never compile directly with gcc or run binaries directly ([details](kb/make-only-test-execution.md)).

**Parallel Build System**: Targets use isolated directories (bin/run-tests/, bin/sanitize-tests/) to enable parallel execution without conflicts.

**Important Makefile Features**:
- When building individual tests, the test is automatically executed after building
- Module changes are automatically rebuilt (no need to rebuild the library separately)
- The Makefile handles all dependencies and runs the test from the correct directory
- Pattern rules compile object files - update these directly for consistent flags (not target-specific vars)
- **Pattern rule updates**: Change all 6 targets when updating Zig flags ([details](kb/makefile-pattern-rule-management.md))
- **POSIX shell only**: Use `case` not `[[ ]]` - Make uses sh, not bash ([details](kb/makefile-posix-shell-compatibility.md))
- **Filtered variables**: Use `filter-out` for conditional compilation ([details](kb/makefile-filtered-variables-pattern.md))
- Example: `make ar_string_tests 2>&1` will:
  1. Rebuild any changed modules
  2. Rebuild the test
  3. Run the test automatically from the bin directory
  4. Generate a memory report specific to that test

### 12. Session & Commit Management

**Task Management**:
- **Session todos (TodoWrite/TodoRead)**: Current TDD cycles, implementations, bug fixes
- **TODO.md file**: Long-term architecture, future features (check [ ] vs [x] for completion)
- **User feedback**: May reveal design issues, not just implementation bugs. Listen for concerns about output/behavior/consistency. Verify assumptions before acting. ([details](kb/user-feedback-as-qa.md))
- **Architectural insights**: User feedback often reveals bigger transformations ([details](kb/architectural-review-in-feedback.md))
- **Be frank, not polite**: Only confirm correctness when certain - honest disagreement > polite agreement
- **Todo list integrity**: Mark items complete, never remove them - preserves task history

**Pre-Commit Checklist** (MANDATORY): ([details](kb/pre-commit-checklist-detailed.md))
1. `make clean build 2>&1` → verify exit 0 → `make check-logs` ([details](kb/build-verification-before-commit.md))
2. Update docs for API changes ([details](kb/documentation-language-migration-updates.md))
3. Check outdated refs: `grep -l "old_name" modules/*.md`
4. Update TODO.md & CHANGELOG.md
5. `git diff` full review → clean temp files → commit

**Remember**: Complete ALL TDD Cycles → Docs → TODO → CHANGELOG → Commit ([details](kb/tdd-feature-completion-before-commit.md))

**After Completing Major Tasks**:
- Document completion date in TODO.md (e.g., "Completed 2025-06-11")
- Move tasks from "In Progress" to "Completed Tasks" section
- Include brief summary of what was accomplished
- Update CLAUDE.md with any new patterns or learnings from the session ([details](kb/systematic-guideline-enhancement.md))

### 13. Refactoring Patterns

**Principles**: Preserve behavior, move don't rewrite, verify moves ([details](kb/refactoring-patterns-detailed.md))
**Bulk ops**: Use scripts not sed - `rename_symbols.py`, `batch_fix_docs.py` ([details](kb/script-enhancement-over-one-off.md))
**Verification**: `diff -u <(sed -n '130,148p' old.c) <(sed -n '11,29p' new.c)` ([details](kb/code-movement-verification.md))
**Key patterns**: ([details](kb/refactoring-key-patterns.md))

### 14. Plan Verification and Review ([details](kb/plan-verification-and-review.md))

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

**Docs**: https://ziglang.org/documentation/0.14.1/ (see #C for interop)
**When**: Performance, cross-platform, safety needs, defer patterns ([details](kb/zig-module-development-guide.md))
**Migration**: Rename .c→.bak, C takes precedence ([details](kb/c-to-zig-module-migration.md))
**Best practices**: Concrete types, ar_allocator only, ownership prefixes ([details](kb/zig-integration-comprehensive.md))
**Struct modules**: TitleCase internal-only modules ([details](kb/zig-struct-modules-pattern.md))

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
