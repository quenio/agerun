# AgeRun Terminology Glossary

This glossary defines the standard terminology used throughout the AgeRun project. These terms have specific meanings and are used consistently across documentation, commands, and code.

## TDD Planning Hierarchy

### Cycle
A logical grouping of related iterations in a TDD plan. Cycles organize iterations into manageable units for review and implementation.

**Usage:**
- Plans are divided into **Cycles** (Cycle 1, Cycle 2, Cycle 3, etc.)
- Each cycle contains 3-5 iterations
- Cycles start with 1, not 0
- Example: "Cycle 1: Basic Functionality", "Cycle 2: Error Handling"

**Context:** TDD plans, iteration organization

### Iteration
A single test-implementation pair in TDD. Each iteration tests exactly one assertion through RED-GREEN-REFACTOR.

**Usage:**
- Numbered with decimal notation: 1.1, 1.2, 2.1, 2.2, etc.
- First number matches the cycle: iterations 1.1-1.5 belong to Cycle 1
- Each iteration has exactly one assertion (one behavior to verify)
- Splits use additional decimals: 1.1 → 1.1.1, 1.1.2 (not renumbering)

**Context:** TDD plans, test implementation

### Phase
The three stages of the TDD cycle: RED, GREEN, and REFACTOR.

**Usage:**
- **RED Phase**: Write failing test with explicit assertion
- **GREEN Phase**: Minimal implementation to pass the test
- **REFACTOR Phase**: Improve code without changing behavior

**Important:** The term "phase" is **RESERVED EXCLUSIVELY** for TDD phases. Do not use "phase" to describe workflow stages, command sections, or any other concept.

**Context:** TDD cycle, test-driven development

## Command/Checkpoint Hierarchy

### Command
A Claude Code command that guides systematic execution of a specific task.

**Usage:**
- Located in `.opencode/command/ar/*.md`
- Invoked as `/ar:command-name`
- May contain stages and steps with checkpoint tracking
- Example: `/ar:commit`, `/ar:create-plan`, `/ar:review-changes`

**Context:** Claude Code commands, workflow automation

### Stage
A logical grouping of related steps within a command workflow. Stages organize steps and have gates for verification.

**Usage:**
- Marked with `### Stage N:` heading (level 3)
- Contains 3-6 related steps
- Followed by a gate checkpoint
- Example: "Stage 1: Pre-Commit Verification (Steps 1-5)"

**Context:** Command documentation, workflow organization

### Step
An individual checkpoint action within a stage. Steps are the atomic units of work in a checkpoint-tracked workflow.

**Usage:**
- Marked with `#### Step N:` heading (level 4)
- Numbered sequentially: Step 1, Step 2, Step 3, etc.
- Updated with `make checkpoint-update CMD=name STEP=N`
- Each step completes one specific task

**Context:** Checkpoint tracking, command execution

### Gate
A verification checkpoint between stages that enforces completion of required steps before allowing progression.

**Usage:**
- Executed with `make checkpoint-gate CMD=name GATE="Name" REQUIRED="1,2,3"`
- Named descriptively: "Build Quality", "Integration", "Documentation"
- Blocks progression if required steps are incomplete
- Returns exit code 1 on failure, 0 on success

**Context:** Quality enforcement, workflow gates

## API Evolution Terms

### API Cycle
A stage in API evolution, typically referring to the progression of an API through different design patterns.

**Usage:**
- **Cycle 1**: Original simple API
- **Cycle 2**: Advanced API with additional parameters
- **Cycle 3**: Consolidation and cleanup

**Note:** In API evolution documentation, "cycle" refers to major API redesign stages, not TDD iterations.

**Context:** API evolution, refactoring patterns

## Memory Management Terms

### Ownership Prefix
Variable name prefixes that indicate ownership semantics for memory management.

**Usage:**
- `own_`: Variable owns the data and is responsible for cleanup
- `ref_`: Variable borrows/references data (does not free)
- `mut_`: Variable has mutable access to data (may or may not own)

**Example:**
```c
ar_data_t *own_message = ar_data__create_string("hello");
ar_data_t *ref_log = ar_log__get_data(log);
ar_data_t *mut_list = ar_list__get(list, 0);
```

**Context:** Memory management, ownership semantics

## Module Organization Terms

### Module Layer
The hierarchical organization of modules in the AgeRun architecture.

**Usage:**
- **Foundation Layer**: ar_heap, ar_io (basic memory and I/O)
- **Data Layer**: ar_data, ar_string, ar_list, ar_map
- **Core Layer**: ar_expression, ar_instruction, ar_method
- **System Layer**: ar_agency, ar_agent, ar_executor

**Context:** Architecture, module dependencies

### Module Prefix
The `ar_` prefix used for all AgeRun types, functions, and modules.

**Usage:**
- Types: `ar_module_t*`
- Functions: `ar_module__function()`
- Constants: `AR_MODULE_CONSTANT`

**Context:** Naming conventions, code organization

## Testing Terms

### BDD Structure
The Given-When-Then-Cleanup pattern used in all AgeRun tests.

**Usage:**
```c
// Given [setup state]
ar_data_t *own_data = ar_data__create_string("test");

// When [action being tested]
bool result = ar_data__is_string(own_data);

// Then [verification]
AR_ASSERT(result, "Should be string");

// Cleanup
ar_data__destroy(own_data);
```

**Context:** Test structure, test-driven development

### Assertion
A verification statement in a test using `AR_ASSERT` macro.

**Usage:**
- Each iteration should have exactly ONE assertion
- Format: `AR_ASSERT(condition, "message")`
- Failure message should explain expected behavior

**Context:** Testing, TDD iterations

## Documentation Terms

### KB Article
A knowledge base article in the `kb/` directory documenting patterns, learnings, and best practices.

**Usage:**
- Located in `kb/*.md`
- Indexed in `kb/README.md`
- Referenced from commands and AGENTS.md
- Must use real AgeRun types and functions in examples

**Context:** Knowledge management, documentation

### Cross-Reference
A link between related KB articles or from commands/AGENTS.md to KB articles.

**Usage:**
- Inline: `([details](kb/article-name.md))`
- Related Patterns section: `- [Article Title](article-name.md)`
- Must use relative paths

**Context:** Documentation, knowledge integration

## Terminology Anti-Patterns

### ❌ DO NOT Use "Phase" for Non-TDD Contexts
**Wrong:** "Phase 1 of the migration", "commit phase", "build phase"
**Correct:** "Stage 1 of the migration", "commit stage", "build stage"

### ❌ DO NOT Use "Checkpoint" as a Heading
**Wrong:** `#### Checkpoint 3: Update Documentation`
**Correct:** `#### Step 3: Update Documentation`

### ❌ DO NOT Use Wrong Heading Levels
**Wrong:** `## Stage 1:` (level 2)
**Correct:** `### Stage 1:` (level 3)

### ❌ DO NOT Mix Terminology
**Wrong:** Using "phase" and "stage" interchangeably in the same document
**Correct:** Use "stage" consistently for workflows, "phase" only for TDD

## Quick Reference

| Term | Context | Example | Reserved? |
|------|---------|---------|-----------|
| **Cycle** | TDD Plans, API Evolution | "Cycle 1", "Cycle 2" | No |
| **Iteration** | TDD Implementation | "1.1", "2.3" | No |
| **Phase** | TDD Only | "RED Phase", "GREEN Phase" | **YES** |
| **Command** | Claude Code | `/ar:commit` | No |
| **Stage** | Workflow | "Stage 1: Pre-Commit" | No |
| **Step** | Checkpoint | "Step 3: Update Docs" | No |
| **Gate** | Verification | "Build Quality Gate" | No |

## Related Documentation

- [TDD Iteration Planning Pattern](kb/tdd-iteration-planning-pattern.md)
- [Checkpoint Implementation Guide](kb/checkpoint-implementation-guide.md)
- [Multi-Step Checkpoint Tracking Pattern](kb/multi-step-checkpoint-tracking-pattern.md)
- [Ownership Naming Conventions](kb/ownership-naming-conventions.md)
- [Module Hierarchy and Dependencies](modules/README.md)
