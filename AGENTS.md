# AgeRun Development Guide for Claude

This document contains essential instructions for Claude Code to assist with AgeRun development.

## Project Overview

AgeRun is a lightweight, message-driven agent system where each agent is defined by a single method responsible for handling all incoming messages. Agents operate within a persistent runtime environment, allowing dynamic creation, versioning, pausing, resumption, and destruction.

## Terminology Reference

**TDD Planning**: **Cycle** = logical grouping of iterations (e.g., "Cycle 1: Basic Functionality"), **Iteration** = single test-implementation pair (e.g., "1.1", "1.2"), **Phase** = TDD stages (RED/GREEN/REFACTOR) *[Reserved exclusively for TDD]*

**Workflows**: **Stage** = logical grouping of related steps (e.g., "Stage 1: Pre-Commit Verification"), **Step** = individual checkpoint action (e.g., "Step 1: Build"), **Gate** = verification checkpoint between stages (e.g., "Build Quality Gate")

**Runtime System**: **Agent** = runtime entity executing methods and processing messages, **Method** = versioned executable definition (e.g., "echo-1.0.0"), **Message** = data passed asynchronously between agents, **Agency** = collection manager of active agents, **Methodology** = registry of method definitions, **Delegate** = lightweight proxy queuing messages for agents

**Command**: Claude Code command that guides systematic execution (e.g., `/ar:commit`)

See [CONCEPTS.md](CONCEPTS.md) for complete definitions and context.

## Navigation Quick Reference

**Finding TDD Plans**: Located in `plans/` directory with format `<task-name>_plan.md`. See [CONCEPTS.md - Plan Files](CONCEPTS.md#plan-files) for structure and status markers.

**Finding Commands**: Located in `.opencode/command/ar/` directory. Invoked as `/ar:command-name`. Contains stages, steps, and checkpoint tracking. See [CONCEPTS.md - Command/Checkpoint Hierarchy](CONCEPTS.md#commandcheckpoint-hierarchy) for structure.

**Finding Modules**: Located in `/modules` directory with pattern:
- **Header**: `ar_<module>.h` - Public API declarations
- **Implementation**: `ar_<module>.c` - Module implementation
- **Tests**: `ar_<module>_tests.c` - Test suite for the module
- **Documentation**: `ar_<module>.md` - Module documentation

See [CONCEPTS.md - Module Organization Terms](CONCEPTS.md#module-organization-terms) and [CONCEPTS.md - File Naming Conventions](CONCEPTS.md#file-naming-conventions) for details.

**Finding KB Articles**: Located in `kb/` directory with format `<topic-name>.md` (kebab-case). Indexed in `kb/README.md`. Referenced from AGENTS.md and commands via links like `([details](kb/tdd-plan-review-checklist.md))`. See [CONCEPTS.md - KB Article Files](CONCEPTS.md#kb-article-files) for details.

## Quick Start

**Primary**: `make build 2>&1` → `make check-logs` (MANDATORY sequence) ([details](kb/quick-start-commands.md), [relationship](kb/build-logs-relationship-principle.md))
**Clean build**: `make clean build 2>&1` → `make check-logs`  
**CI requirement**: `check-logs` must pass or CI will fail ([details](kb/ci-check-logs-requirement.md), [deep analysis](kb/check-logs-deep-analysis-pattern.md), [network timeouts](kb/ci-network-timeout-diagnosis.md))
**Help**: `make` shows all targets
**Scripts**: Use make targets, not direct execution; dry-run mode essential ([details](kb/dry-run-mode-requirement.md))

## Command Execution Protocol

**When user asks you to execute a command**: Look for a markdown file with the command name under `.opencode/command/ar/`, read this file, and follow its instructions.

**When user asks to list commands**: List the files under `.opencode/command/ar/` folder.

## Git Workflow (CRITICAL)

**After Every Git Push**:
1. **ALWAYS run `git status`** to verify:
   - Push completed successfully
   - Working tree remains clean
   - No uncommitted changes remain

This is a MANDATORY verification step. Never assume a push succeeded without checking. ([details](kb/git-push-verification.md))

**Preserving Changes**: Always create patches BEFORE reverting files ([details](kb/git-patch-preservation-workflow.md))

## Project Structure

- **/modules**: Core implementation (.c/.h files and tests)
- **/bin**: Generated binaries (ignored by git, NEVER read these files)
- **/methods**: Method definitions (.method files with docs and tests)
- **/scripts**: Build and utility scripts (run via make targets, not directly - includes slash commands)
- **/reports**: Analysis reports and technical comparisons (all .md analysis files go here)
- **.claude/commands**: Slash command definitions (must use make targets, not direct scripts) ([role clarity](kb/role-clarification-pattern.md))

## Critical Development Rules

### Knowledge Base Usage (MANDATORY - SHOW SEARCHES IN CONVERSATION)

**Check KB BEFORE planning**: grep kb/README.md before creating any plan ([details](kb/kb-consultation-before-planning-requirement.md), [protocol](kb/knowledge-base-consultation-protocol.md), [enforcement](kb/command-kb-consultation-enforcement.md))
**Make KB searches VISIBLE**: Run grep, show results, quote guidance, apply patterns ([details](kb/kb-search-patterns.md), [priming](kb/session-start-priming-pattern.md))
**Systematic analysis required**: Apply [systematic task analysis protocol](kb/systematic-task-analysis-protocol.md) for priority decisions
**Trigger words**: "Why are you"/"Actually"/"You should" → Search NOW
**Markdown links**: Use relative paths only, resolve from file location ([details](kb/markdown-link-resolution-patterns.md))
**Complete searches**: Never limit results without checking totals ([details](kb/search-result-completeness-verification.md))

### Systematic Task Analysis (MANDATORY - PREVENT INCOMPLETE ANALYSIS)

**Complete data gathering FIRST**: Before priority recommendations, get full scope with `grep "^- \[ \]" TODO.md | wc -l` ([details](kb/systematic-task-analysis-protocol.md))
**Categorize before prioritizing**: Group tasks by complexity/dependencies, never rush to conclusions
**Evidence-based verification**: Apply quantitative metrics, question unexpected results (e.g., very few tasks in active project)
**Show methodology**: Present data gathering process and categorization logic, not just conclusions
**Cross-verify searches**: Use multiple grep patterns to ensure comprehensive coverage
**KB target enforcement**: Enforce quantitative targets with FAIL gates, not warnings ([details](kb/kb-target-compliance-enforcement.md))

### 0. Documentation Protocol

**Always search AGENTS.md first** when asked about procedures. Don't overthink - start with exact keywords.
**Check SPEC.md before implementing** features to avoid unnecessary work ([details](kb/specification-consistency-maintenance.md))
**When reading TODO.md**: Check [ ] = incomplete, [x] = complete. Read completion dates. Be explicit - list all components. Verify tasks are actually needed before implementing ([details](kb/task-verification-before-execution.md)).
**KB articles need AGENTS.md references** to be accessible in future sessions ([details](kb/claude-md-reference-requirement.md))
**Cross-reference new KB articles**: MANDATORY bidirectional linking when creating KB articles ([details](kb/new-learnings-cross-reference-requirement.md))
**Complete integration**: Cross-refs + command updates often missed ([details](kb/new-learnings-complete-integration-pattern.md))
**Evidence-based debugging**: Always verify with concrete evidence, not assumptions ([details](kb/evidence-based-debugging.md))
**Bug-to-architecture**: Bugs can reveal architectural improvements ([details](kb/bug-to-architecture-pattern.md))
**Build output review**: Review complete command output, not just final status ([details](kb/comprehensive-output-review.md))
**Documentation compaction**: Extract verbose content to KB while preserving links ([details](kb/documentation-compacting-pattern.md))
**Selective compaction**: Compact only completed items in mixed-state documents ([details](kb/selective-compaction-pattern.md))
**Quantitative metrics**: Use specific numbers instead of vague descriptions ([details](kb/quantitative-documentation-metrics.md))
**Priority setting**: Use metrics like "97.6% failures" for objective prioritization ([details](kb/quantitative-priority-setting.md))
**Self-documenting changes**: Add entries for significant modifications ([details](kb/self-documenting-modifications-pattern.md))
**Retroactive tracking**: Add completed unplanned tasks to TODO.md ([details](kb/retroactive-task-documentation.md))
**Report-driven planning**: Create analysis reports before large task sets ([details](kb/report-driven-task-planning.md))
**Script archaeology**: Examine scripts to understand what work was actually completed ([details](kb/script-archaeology-pattern.md))
**Learning extraction**: Multi-pass review reveals hidden patterns ([details](kb/comprehensive-learning-extraction-pattern.md), [thoroughness](kb/command-thoroughness-requirements-pattern.md))
**Session continuity**: Preserve context across session boundaries; resume without prompting when context clear ([details](kb/context-preservation-across-sessions.md), [resumption](kb/session-resumption-without-prompting.md))

**Documentation Standards (MANDATORY)** ([details](kb/documentation-standards-integration.md), [unmissable](kb/unmissable-documentation-pattern.md), [sync](kb/documentation-implementation-sync.md)):
- **Real code only**: All examples must use actual AgeRun types/functions ([details](kb/validated-documentation-examples.md))
- **Validation required**: Run `make check-docs` before committing any .md files ([details](kb/documentation-validation-enhancement-patterns.md))
- **Error classification**: Categorize doc errors as automatable vs judgment-based ([details](kb/documentation-error-type-classification.md))
- **Validation loops**: Use iterate validate→fix→re-validate cycles ([details](kb/validation-feedback-loop-effectiveness.md))
- **Placeholder marking**: Use EXAMPLE tags for hypothetical references ([details](kb/documentation-placeholder-validation-pattern.md))
- **Command outputs**: Document expected outputs for all scripts/tools ([details](kb/command-output-documentation-pattern.md))
- **Specification consistency**: Keep SPEC.md updated after language changes ([details](kb/specification-consistency-maintenance.md))
- **Proper tagging**: Use `// EXAMPLE:`, `// BAD:`, `// ERROR:` for hypothetical code ([details](kb/example-marker-granular-control.md))
- **Markdown links**: Use relative paths only - see link resolution rules ([details](kb/markdown-link-resolution-patterns.md))
- **Preferred types**: `ar_data_t*`, `ar_agent_t*`, `ar_expression_ast_t*`, `ar_instruction_ast_t*`
- **Helper reference**: `python3 scripts/get_real_types.py --guide` for available APIs
- **Feature removal**: Question if "core" features are truly essential ([details](kb/architectural-simplification-through-feature-removal.md))
**Doc updates**: Follow cascade order: methods→modules→project ([details](kb/documentation-update-cascade-pattern.md))

### 1. Memory Management (ZERO TOLERANCE FOR LEAKS)

**Mandatory Practices**:
- Use heap tracking macros: `AR__HEAP__MALLOC`, `AR__HEAP__FREE`, `AR__HEAP__STRDUP`
- Include `#include "ar_heap.h"` in all .c files that allocate memory
- Follow ownership naming conventions ([details](kb/ownership-naming-conventions.md)):
  - `own_`: Owned values that must be destroyed
  - `mut_`: Mutable references (read-write access)
  - `ref_`: Borrowed references (read-only access)
  - Apply to ALL variables, parameters, and struct fields (including local variables)
  - Use `take_` for ownership transfer, `get_` for read-only access ([details](kb/function-naming-state-change-convention.md))
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
- Test resources: Use dynamic allocation with ownership transfer ([details](kb/dynamic-test-resource-allocation.md))
- Expression evaluator claims: Unowned data can be claimed during evaluation ([details](kb/expression-evaluator-claim-behavior.md))
- Map iteration: get keys, destroy list & elements; persist with key/type then value
- Message ownership flow: System takes ownership after dequeuing from agents ([details](kb/message-ownership-flow.md))
- Ownership gap vulnerability: Ensure atomic ownership transfer to prevent corruption ([details](kb/ownership-gap-vulnerability.md))
- Ownership patterns: Use `ar_data__claim_or_copy()` & `ar_data__destroy_if_owned()` ([details](kb/ownership-pattern-extraction.md))
- Temporary resources: Creator destroys contexts since agents store as references ([details](kb/temporary-resource-ownership-pattern.md))
- API ownership on failure: Verify implementation to understand cleanup requirements ([details](kb/api-ownership-on-failure.md))
- Context lifecycle: Destroy resources where variables are still in scope ([details](kb/context-ownership-lifecycle.md))
- Shared context: System maintains ONE context for all agents ([details](kb/shared-context-architecture-pattern.md))
- Frame prerequisites: ar_frame__create needs non-NULL memory, context, message ([details](kb/frame-creation-prerequisites.md))
- Memory-efficient streaming: Direct file I/O without intermediate strings ([details](kb/memory-efficient-streaming-patterns.md))

**Memory Leak Detection**:
- Full test suite: Check console for "WARNING: X memory leaks detected"
- Individual test reports: `bin/run-tests/memory_report_<test_name>.log` ([details](kb/memory-leak-detection-workflow.md))
- Complete verification: `grep "Actual memory leaks:" bin/run-tests/memory_report_*.log | grep -v "0 (0 bytes)"`
- Always run `make sanitize-tests 2>&1` before committing
- Tests using dlsym are automatically excluded from sanitizer builds ([details](kb/sanitizer-test-exclusion-pattern.md))
- Debug strategy: Check report → Trace source → Verify ownership → Fix naming → Add cleanup ([details](kb/memory-debugging-comprehensive-guide.md))
- Test leak analysis: Verify lifecycle management before adding cleanup ([details](kb/test-memory-leak-ownership-analysis.md))
- Environment variables: `ASAN_OPTIONS=halt_on_error=0` (continue on error), `detect_leaks=1` (complex leaks)

### 2. Test-Driven Development (MANDATORY)

**Pre-modification**: Run module tests BEFORE changes
**Cycle**: Red→Green→Refactor for EACH behavior, NO commits during ([details](kb/red-green-refactor-cycle.md), [cycle details](kb/tdd-cycle-detailed-explanation.md))
**RED phase**: Must produce assertion failures, not compilation errors ([details](kb/tdd-red-phase-assertion-requirement.md))
**Verify fix**: Run single test first, then full suite ([details](kb/test-first-verification-practice.md))
**Test reality**: Align expectations with actual behavior ([details](kb/test-expectation-reality-alignment.md))
**Coverage verification**: Check existing implementations before adding ([details](kb/error-coverage-verification-before-enhancement.md))
**Test effectiveness**: Break implementation to verify tests catch failures ([details](kb/test-effectiveness-verification.md))
**Test assertion strength**: Verify specific outcomes, not just success/failure ([details](kb/test-assertion-strength-patterns.md))
**DLSym interception**: Test untestable conditions with function mocking ([details](kb/dlsym-test-interception-technique.md))
**Mock at right level**: Mock functions, not system calls for simpler tests ([details](kb/mock-at-right-level-pattern.md))
**Check existing solutions**: Look at other tests before creating complex infrastructure ([details](kb/check-existing-solutions-first.md))
**Documentation validation**: Tests verify documented behavior is accurate ([details](kb/test-driven-documentation-validation.md))
**Multi-session**: Use session tracking for large changes ([details](kb/multi-session-tdd-planning.md))
**Effort estimation**: Estimate in TDD cycles not hours/days ([details](kb/tdd-cycle-effort-estimation.md))
**Iteration planning**: One assertion per iteration ([details](kb/tdd-iteration-planning-pattern.md))
**Iteration splitting**: Split multi-assertion iterations into .1/.2 sub-iterations ([details](kb/tdd-plan-iteration-split-pattern.md))
**GREEN minimalism**: Hardcoded returns valid if they pass tests ([details](kb/tdd-green-phase-minimalism.md))
**Temporary cleanup**: Add manual cleanup in .1 iterations, remove in .2 ([details](kb/temporary-test-cleanup-pattern.md))
**Lifecycle separation**: Handle creation/destruction in separate cycles ([details](kb/lifecycle-event-separation-tdd.md))
**Test modification**: Ask permission before changing tests ([details](kb/permission-based-test-modification.md))
**Compilation-based TDD**: Use compilation failure as RED phase for infrastructure removal ([details](kb/compilation-based-tdd-approach.md))
**Documentation-only changes**: Skip TDD for .md updates, use direct editing ([details](kb/documentation-only-change-pattern.md))
**Cycle verification**: Systematically verify all iterations complete ([details](kb/tdd-cycle-completion-verification-pattern.md))
**Plan completion**: Update plan docs with completion status ([details](kb/plan-document-completion-status-pattern.md))
**Complete ALL cycles** → Update docs/TODO/CHANGELOG → Single commit

**Test Requirements**: BDD structure, one test per behavior, AR_ASSERT macros (never plain assert()), zero leaks ([details](kb/bdd-test-structure.md), [assertions](kb/ar-assert-descriptive-failures.md), [standards](kb/standards-over-expediency-principle.md))
**Test simplification**: Use literals not complex expressions ([details](kb/test-input-simplification-pattern.md))
**Context convention**: Pass 'context' not 'memory' ([details](kb/context-parameter-convention.md))
**Requirement precision**: Verify exact requirements before implementing ([details](kb/requirement-precision-in-tdd.md))
**Test completeness**: Enumerate & verify each outcome individually ([details](kb/test-completeness-enumeration.md))
**Method tests**: Verify AST after loading to catch parse errors ([details](kb/method-test-ast-verification.md))
**Cleanup**: `ar_methodology__cleanup()` & `ar_agency__reset()`
**Messages**: Process all messages to prevent memory leaks
**Test isolation**: Comment out tests to isolate errors ([details](kb/test-isolation-through-commenting.md)); cleanup shared files ([details](kb/test-isolation-shared-directory-pattern.md))
**Test cleanup**: Remove persisted files before tests run ([details](kb/test-file-cleanup-pattern.md))
**Test redundancy**: Avoid cleanup already handled by fixtures ([details](kb/redundant-test-cleanup-anti-pattern.md))
**Test updates**: Apply improvements retroactively to all tests ([details](kb/test-standardization-retroactive.md))
**Test complexity**: Simplify to minimal failing case for debugging ([details](kb/test-complexity-reduction-pattern.md))
**Test complexity smell**: Complex tests with retry loops indicate wrong abstraction ([details](kb/test-complexity-as-code-smell.md))
**Debug recompilation**: Rebuild debug programs after library changes ([details](kb/debug-program-recompilation-pattern.md))
**Fixture dependencies**: Map which tests use which fixtures before changes ([details](kb/fixture-dependency-mapping.md))
**Fixture evolution**: Add helpers as needed ([details](kb/test-fixture-evolution-pattern.md))
**Integration tests**: Run actual binary with popen(), verify output ([details](kb/integration-test-binary-execution.md))
**Exit codes**: popen() multiplies by 256 ([details](kb/exit-code-propagation-popen.md))
**Permission testing**: chmod to force failures ([details](kb/permission-based-failure-testing.md))
**Error marking**: Mark intentional errors clearly in tests ([details](kb/test-error-marking-strategy.md))
**Test strings**: Use clearly synthetic test data, not system-like strings ([details](kb/test-string-selection-strategy.md))
**Signal reporting**: Report signal numbers before assertions ([details](kb/test-signal-reporting-practice.md))
**Whitelist specificity**: Use unique fields for test errors to prevent masking ([details](kb/whitelist-specificity-pattern.md))
**Fixture ownership**: Tests bypassing system must manage message ownership ([details](kb/test-fixture-message-ownership.md))
**Fixture modules**: Extract helpers into proper modules with opaque types ([details](kb/test-fixture-module-creation-pattern.md))
**Fixture simplification**: Remove complex logic for obsolete features ([details](kb/test-fixture-simplification-pattern.md))
**Regression test removal**: Delete tests verifying absence of removed features ([details](kb/regression-test-removal-criteria.md))
**Advanced patterns**: ([details](kb/tdd-advanced-large-refactoring.md), [API changes](kb/tdd-api-simplification.md), [decoupling](kb/progressive-system-decoupling-tdd.md))

### 3. Parnas Design Principles (STRICT ENFORCEMENT) ✅

**Status**: Full compliance achieved as of 2025-06-08. All interface violations have been fixed. Zero circular dependencies (except heap ↔ io).

**Core Principles**:
- **Information Hiding**: Hide design decisions behind interfaces ([details](kb/information-hiding-principle.md))
- **Single Responsibility**: One module, one concern ([details](kb/single-responsibility-principle.md))
- **No Circular Dependencies**: Uses hierarchy must be strict ([details](kb/no-circular-dependencies-principle.md))
- **Opaque Types**: Required for complex data structures ([details](kb/opaque-types-principle.md))
- **Minimal Interfaces**: Expose only what's necessary ([details](kb/minimal-interfaces-principle.md))
- **Complete Documentation**: Every module must be fully documented ([details](kb/complete-documentation-principle.md))
- **Observable State**: Objects need queryable validity beyond existence ([details](kb/observable-internal-state.md))
- **Validation at Creation**: Check internal state, not just object existence ([details](kb/validation-at-creation-time.md))
- **Program Families**: Design for anticipated variations and multiple versions ([details](kb/program-families-principle.md))
- **Design for Change**: Decompose based on what's likely to change, not workflow ([details](kb/design-for-change-principle.md))
- **Separation of Concerns**: Each module addresses a distinct concern ([details](kb/separation-of-concerns-principle.md))

**Important Clarifications**:
- **Enums in Public APIs**: Some enums (like `ar_data_type_t`) are part of the abstract model, not implementation details. These are acceptable when they represent abstract concepts needed by clients.
- **Internal Functions**: Never expose functions marked "internal use only" in public headers
- **Implementation Details**: Keep struct definitions, array indices, and storage mechanisms private

**Enforcement**: Violations result in automatic PR rejection.

### Preventing Circular Dependencies & Code Duplication

**Dependency hierarchy**: Foundation → Data → Core → System ([details](kb/dependency-management-examples.md))
**Check dependencies**: `grep -n "#include.*ar_" module.h module.c`
**Architectural patterns**: Interface segregation > Registry > Facade > Parser/Executor > Callbacks; ID sign routing for subsystems ([details](kb/architectural-patterns-hierarchy.md), [facade](kb/facade-pattern-coordination.md), [routing](kb/id-based-message-routing-pattern.md))
**DRY principle**: Search existing → extract functions → use data tables → parameterize ([details](kb/domain-specific-type-creation.md))
**Red flags**: A→B→C→A cycles, validation duplication, _for_int/_for_string patterns
**Exception**: heap ↔ io circular dependency is accepted and documented.

### Additional Design Principles

**Core Standards**:
- **Const-Correctness**: NEVER cast away const - fix interfaces instead ([details](kb/const-correctness-principle.md))
- **No Parallel Implementations**: Modify existing code, don't create _v2 versions ([details](kb/no-parallel-implementations-principle.md))
- **Composition Over Inheritance**: Prefer composition patterns to create flexible, maintainable architectures ([details](kb/composition-over-inheritance-principle.md))
- **Header Inclusion**: Include actual headers, not forward declarations ([details](kb/header-inclusion-over-forward-declaration.md))
- **Regression Investigation**: Trace root causes with git history ([details](kb/regression-root-cause-analysis.md))
- **Resource-Aware APIs**: Design to avoid hidden memory costs ([details](kb/resource-aware-api-design.md))
- **Functional Value**: Validate features provide value, not decoration ([details](kb/functional-value-validation-pattern.md))

### 4. Code Smells Detection and Prevention

**Bloaters**: Long methods (>20 lines), large modules (>800 lines), long parameters (>4), data clumps, primitive obsession
**Duplication**: Most critical smell - extract common functions ([details](kb/code-smell-duplicate-code.md))
**Change preventers**: Divergent change, shotgun surgery
**Couplers**: Feature envy, message chains ([details](kb/code-smell-feature-envy.md))
**Detection**: Automated scripts for quick identification ([details](kb/code-smell-quick-detection.md))

### 5. Coding Standards

**String Parsing**: Track quote state when scanning for operators ([details](kb/string-parsing-quote-tracking.md))
**Make targets**: Use generic parameterized targets, avoid duplication ([details](kb/generic-make-targets-pattern.md))

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
**Resources**: Use instance-specific not global ([details](kb/instance-vs-global-resource-pattern.md))
**Error logging**: Utilize ar_log instances for all error conditions ([details](kb/error-logging-instance-utilization.md))
**Parser errors**: Comprehensive logging with position info ([details](kb/parser-error-logging-enhancement-pattern.md))
**Consistency**: Verify sister modules need same improvements; update modules/README.md after integration ([details](kb/module-consistency-verification.md), [symmetric](kb/symmetric-module-architecture-pattern.md), [systematic](kb/systematic-consistency-verification.md), [index](kb/documentation-index-consistency-pattern.md))
**Stateless**: Ensure no global state beyond instance management ([details](kb/stateless-module-verification.md))
**API cleanup**: Remove suffixes & dead code after migration ([details](kb/api-suffix-cleanup-pattern.md), [dead params](kb/dead-code-after-mandatory-parameters.md), [suffix consolidation](kb/api-suffix-cleanup-after-consolidation.md))
**Null instance**: Silent failure acceptable when instance is NULL ([details](kb/error-logging-null-instance-limitation.md))

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
- `send(0, message)` is a no-op returning true ([details](kb/no-op-semantics-pattern.md))

### 8. Development Practices

**Grouped by category** for easier reference ([details](kb/development-practice-groups.md)):

**Navigation & Files**: Absolute paths, git not .bak, ar_io backups ([details](kb/absolute-path-navigation.md), [backups](kb/file-io-backup-mechanism.md))
**Build & Debug**: `make sanitize-tests 2>&1`, make targets only, parallel jobs ([details](kb/development-debug-tools.md), [make](kb/make-target-testing-discipline.md), [compile](kb/compilation-driven-refactoring-pattern.md), [shell diagnostics](kb/shell-configuration-diagnostic-troubleshooting.md))
**Checkpoints**: Track complex tasks, gates, progress, concise output ([details](kb/multi-step-checkpoint-tracking-pattern.md), [workflows](kb/checkpoint-based-workflow-pattern.md), [gates](kb/gate-enforcement-exit-codes-pattern.md), [concise](kb/concise-script-output-principle.md))
**YAML & Persistence**: 2-space indent, escape multiline, direct I/O, validate contracts ([details](kb/yaml-serialization-direct-io-pattern.md), [multiline](kb/multi-line-data-persistence-format.md), [contracts](kb/yaml-implicit-contract-validation-pattern.md))
**Error & Logs**: Context filtering, precise grep, fix root causes ([details](kb/systematic-whitelist-error-resolution.md), [logs](kb/build-log-extraction-patterns.md), [grep](kb/grep-or-syntax-differences.md), [config migration](kb/configuration-migration-troubleshooting-strategy.md))
**CI/CD**: gh CLI debugging, version verification ([details](kb/github-actions-debugging-workflow.md), [versions](kb/tool-version-selection-due-diligence.md))
**Quality**: Stop after 3 failures, staged cleanup, impact analysis ([details](kb/struggling-detection-pattern.md), [cleanup](kb/staged-cleanup-pattern.md), [impact](kb/comprehensive-impact-analysis.md))

### 9. Error Propagation Pattern

**Implementation**: Set errors at source → Store in struct → Propagate via get_error() → Print once at top level ([details](kb/error-propagation-pattern.md))
**Key Rule**: Evaluators set errors, interpreter prints them. Never print errors where they occur.
**Graceful degradation**: Non-critical ops warn & continue ([details](kb/graceful-degradation-pattern.md))

### 10. Agent Lifecycle

**Critical Points**:
- ALWAYS process messages after sending to prevent leaks
- Call `ar_system__process_next_message(own_system)` after `ar_agent__send()`

### 11. Building Individual Tests

Always use make to build tests:
```bash
make test_name 2>&1  # Build individual test (with stderr)
```
Never compile directly with gcc or run binaries directly ([details](kb/make-only-test-execution.md)).

**Parallel Build System**: Targets use isolated directories (bin/run-tests/, bin/sanitize-tests/) to enable parallel execution without conflicts ([details](kb/compiler-output-conflict-pattern.md)).

**Important Makefile Features**:
- When building individual tests, the test is automatically executed after building
- Module changes are automatically rebuilt (no need to rebuild the library separately)
- The Makefile handles all dependencies and runs the test from the correct directory
- Pattern rules compile object files - update these directly for consistent flags (not target-specific vars)
- **Pattern rule updates**: Change all 6 targets when updating Zig flags ([details](kb/makefile-pattern-rule-management.md))
- **POSIX shell only**: Use `case` not `[[ ]]` - Make uses sh, not bash ([details](kb/makefile-posix-shell-compatibility.md))
- **Bash parsing**: Avoid complex chained commands, use sequential steps ([details](kb/bash-command-parsing-patterns.md))
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
- **Task authorization**: Wait for explicit instruction before starting tasks ([details](kb/task-authorization-pattern.md))
- **User feedback**: May reveal design issues, not just implementation bugs. Listen for concerns about output/behavior/consistency. Verify assumptions before acting. ([details](kb/user-feedback-as-qa.md), [architecture gate](kb/user-feedback-as-architecture-gate.md), [assumptions](kb/assumption-verification-before-action.md))
- **Architectural insights**: User feedback often reveals bigger transformations ([details](kb/architectural-review-in-feedback.md))
- **Be frank, not polite**: Only confirm correctness when certain - honest disagreement > polite agreement
- **Todo list integrity**: Mark items complete, never remove them - preserves task history

**Checkpoint Process Discipline** (MANDATORY):
- **Sequential execution required**: Work → Verify → Mark complete (never parallelize) ([details](kb/checkpoint-sequential-execution-discipline.md))
- **Work verification required**: NEVER mark steps complete without actual work ([details](kb/checkpoint-work-verification-antipattern.md))
- **Evidence-based completion**: Critical steps require evidence files and work summaries
- **No shortcut marking**: `make checkpoint-update` only after completing required work
- **Step instruction reading**: Always understand what each step requires before executing
- **Audit trail logging**: All checkpoint completions are logged with timestamps and evidence
- **Question step applicability**: If step seems irrelevant, document why rather than skip it

**Pre-Commit Checklist** (MANDATORY): ([details](kb/pre-commit-checklist-detailed.md))
1. `make clean build 2>&1` → verify exit 0 → `make check-logs` ([details](kb/build-verification-before-commit.md))
2. `make check-docs` → validate all documentation ([details](kb/documentation-validation-enhancement-patterns.md))
3. `make check-commands` → ensure 90%+ excellence ([details](kb/command-documentation-excellence-gate.md))
4. Update docs for API changes ([details](kb/documentation-language-migration-updates.md))
5. Check outdated refs: `grep -l "old_name" modules/*.md`
6. Update TODO.md & CHANGELOG.md in same commit ([details](kb/atomic-commit-documentation-pattern.md), [verify](kb/documentation-completion-verification.md))
7. `git diff --stat` review full scope → verify message describes ALL changes ([details](kb/incomplete-commit-message-recovery-pattern.md))
8. Include Claude Code attribution in commit message ([details](kb/claude-code-commit-attribution.md))
9. Clean temp files → commit

**Remember**: Complete ALL TDD Cycles → Docs → TODO → CHANGELOG → Commit ([details](kb/tdd-feature-completion-before-commit.md))

**After Completing Major Tasks**:
- Document completion date in TODO.md (e.g., "Completed 2025-06-11")
- Move tasks from "In Progress" to "Completed Tasks" section
- Capture improvements discovered during work as new TODO items ([details](kb/post-session-task-extraction-pattern.md))
- Update AGENTS.md with any new patterns or learnings from the session ([details](kb/systematic-guideline-enhancement.md))

### 13. Refactoring Patterns

**Principles**: Preserve behavior, move don't rewrite, verify moves ([details](kb/refactoring-patterns-detailed.md))
**Renaming**: Systematic approach for features/commands ([details](kb/comprehensive-renaming-workflow.md))
**Bulk ops**: Use scripts not sed - `rename_symbols.py`, `batch_fix_docs.py` ([details](kb/script-enhancement-over-one-off.md), [workflow](kb/systematic-file-modification-workflow.md), [removal](kb/global-function-removal-script-pattern.md))
**Verification**: `diff -u <(sed -n '130,148p' old.c) <(sed -n '11,29p' new.c)` ([details](kb/code-movement-verification.md))
**Key patterns**: ([details](kb/refactoring-key-patterns.md), [line refs](kb/implementation-plan-line-references.md))
**Non-functional code**: Detect and remove code that never executes ([details](kb/non-functional-code-detection-pattern.md))
**Parameter removal**: Systematically remove unused parameters across layers ([details](kb/systematic-parameter-removal-refactoring.md))
**Feature remnants**: Clean ALL traces when removing features ([details](kb/feature-remnant-cleanup-pattern.md))
**Verify by removal**: Test if code is needed by removing it ([details](kb/verification-through-removal-technique.md))
**Pattern spread**: Technical debt propagates through copy-paste ([details](kb/cross-file-pattern-propagation.md))
**Dead comments**: Delete commented code, don't accumulate ([details](kb/commented-code-accumulation-antipattern.md))

### 14. Plan Verification and Review

**Single task focus**: One TODO = one plan, include verification steps ([details](kb/plan-verification-checklist.md), [review](kb/plan-verification-and-review.md))
**Completion tracking**: Update plan docs with completion status after verification ([details](kb/plan-document-completion-status-pattern.md))
**Review status tracking**: Use REVIEWED/PENDING markers for multi-session reviews ([details](kb/plan-review-status-tracking.md))
**Iteration-by-iteration review**: Review 3-5 iterations per session to maintain quality ([details](kb/iterative-plan-review-protocol.md))
**Iterative refinement**: Expect multiple feedback rounds to get methodology right ([details](kb/iterative-plan-refinement-pattern.md))
**User feedback**: Valuable QA - update plans immediately when corrected ([details](kb/user-feedback-as-qa.md))
**Module constraints**: Different modules have different requirements - check tests
**Generic design**: Consider ar_path vs ar_memory_path for broader utility ([details](kb/oo-to-c-adaptation.md))

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
- Always process messages after sending to prevent memory leaks
- Message processing loop required for complete execution ([details](kb/message-processing-loop-pattern.md))
- All messages flow through system layer ([details](kb/system-message-flow-architecture.md))
- **Function calls are NOT expressions** - cannot nest in other calls ([details](kb/agerun-method-language-nesting-constraint.md))
- **Send with memory references not supported** - send() needs ownership of message
- **Message accessor** - `message.field` returns references like memory/context ([details](kb/expression-evaluator-accessor-extension.md))
- **Language constraints** - No type checking, if() returns values ([details](kb/agerun-language-constraint-workarounds.md))
- **No-op instructions** - spawn(0,..) & spawn("",..) return true ([details](kb/no-op-instruction-semantics.md))

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
- Assume error causes - always verify with evidence ([details](kb/issue-currency-verification-pattern.md))

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
