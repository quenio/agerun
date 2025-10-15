# AgeRun Concepts

This document defines the core concepts, terminology, and organizational principles used throughout the AgeRun project. These concepts form the foundation for understanding the project's architecture, development workflows, and documentation structure.

## Table of Contents

- [TDD Planning Hierarchy](#tdd-planning-hierarchy)
- [Command/Checkpoint Hierarchy](#commandcheckpoint-hierarchy)
- [API Evolution Terms](#api-evolution-terms)
- [Memory Management Terms](#memory-management-terms)
- [Module Organization Terms](#module-organization-terms)
- [Testing Terms](#testing-terms)
- [Documentation Terms](#documentation-terms)
- [Terminology Anti-Patterns](#terminology-anti-patterns)
- [Agent System Concepts](#agent-system-concepts)
- [Build & Verification Concepts](#build--verification-concepts)
- [File Naming Conventions](#file-naming-conventions)
- [Data Type Concepts](#data-type-concepts)
- [Semantic Versioning](#semantic-versioning)
- [Workflow Status Markers](#workflow-status-markers)
- [Quick Reference](#quick-reference)
- [Related Documentation](#related-documentation)

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

## Agent System Concepts

### Agent
A runtime entity that executes methods and processes messages. Agents are the fundamental units of computation in AgeRun.

**Usage:**
- Created with `ar_agency__create_agent(agency, method_name, version, context)`
- Each agent has a unique ID (int64_t)
- Maintains its own memory state
- Processes messages asynchronously through a queue

**Context:** Runtime execution, message-driven architecture

### Method
A versioned, executable definition that specifies an agent's behavior. Methods are written in AgeRun's instruction language.

**Usage:**
- Defined with semantic versioning: `echo-1.0.0`
- Stored as `.method` files or compiled at runtime
- Contains instructions for processing messages
- Supports versioned upgrades and backward compatibility

**Context:** Agent behavior definition, method versioning

### Message
Data passed asynchronously between agents. Messages drive all agent interactions in the system.

**Usage:**
- Created as `ar_data_t*` values
- Sent with `ar_agency__send_to_agent(agency, agent_id, own_message)`
- Ownership transferred to receiving agent's queue
- Processed in FIFO order

**Context:** Inter-agent communication, asynchronous processing

### Agency
The collection and manager of all active agents in the runtime environment.

**Usage:**
- Created with `ar_agency__create(methodology)`
- Maintains agent registry and message routing
- Handles agent lifecycle (create, pause, resume, destroy)
- Routes messages between agents

**Context:** Agent management, runtime coordination

### Methodology
The registry of all method definitions available in the system.

**Usage:**
- Contains versioned method definitions
- Supports method lookup by name and version
- Can be persisted to disk and restored
- Shared across all agents in an agency

**Context:** Method registry, version management

### Delegate
A lightweight proxy that queues messages for an agent. Delegates enable asynchronous message passing without requiring direct agent access.

**Usage:**
- Created with `ar_delegate__create(ref_log, name)`
- Queues messages with `ar_delegate__send(mut_delegate, own_message)`
- Check for messages: `ar_delegate__has_messages(ref_delegate)`
- Retrieve messages: `ar_delegate__take_message(mut_delegate)`
- Transfers message ownership to/from agent

**Context:** Message queuing, asynchronous communication

### Delegation
The system for managing delegates and routing messages between agents through indirect references.

**Usage:**
- Provides decoupling between message sender and receiver
- Enables message queuing when agent is busy
- Supports agent-to-agent communication patterns
- Maintains message ordering (FIFO)

**Context:** Message routing, agent communication patterns

## Build & Verification Concepts

### Make Target
Standard build commands accessed through the Makefile that provide consistent build operations.

**Usage:**
- `make build` - Full build with all tests and checks
- `make clean` - Remove all build artifacts
- `make run-tests` - Execute test suite only
- `make run-exec` - Build and run the executable
- `make sanitize-tests` - Run tests with AddressSanitizer
- `make analyze-exec` - Run static analysis

**Context:** Build system, development workflow

### Sanitizer
Runtime error detection tools that catch memory and threading issues during execution.

**Usage:**
- **AddressSanitizer (ASan)**: Detects memory errors (use-after-free, leaks, overflows)
- **ThreadSanitizer (TSan)**: Detects data races and threading issues
- Enabled with `make sanitize-tests` or `make tsan-tests`
- macOS uses clang for sanitizers, Linux uses gcc-13

**Context:** Memory safety, runtime verification

### Static Analysis
Compile-time code analysis that identifies potential issues without executing code.

**Usage:**
- Clang Static Analyzer examines code paths
- Detects null pointer dereferences, memory leaks, logic errors
- Run with `make analyze-exec` or `make analyze-tests`
- Results in HTML reports (with scan-build) or console output

**Context:** Compile-time verification, bug detection

### Check-Logs
A CI gate requirement that analyzes build logs for hidden errors that don't fail the build.

**Usage:**
- Run with `make check-logs` (after `make build`)
- Detects errors in test output, warnings, unexpected messages
- Uses `log_whitelist.yaml` to filter known intentional errors
- **Must pass** or CI will fail - non-negotiable requirement

**Context:** CI/CD, quality gates, hidden issue detection

## File Naming Conventions

### Module Files
Source code files following the standard AgeRun module structure.

**Usage:**
- **Header**: `ar_module.h` - Public API declarations
- **Implementation**: `ar_module.c` - Module implementation
- **Tests**: `ar_module_tests.c` - Test suite for the module
- **Documentation**: `ar_module.md` - Module documentation

**Example:** `ar_data.h`, `ar_data.c`, `ar_data_tests.c`, `ar_data.md`

**Context:** Code organization, module structure

### Method Files
Standalone method definition files with semantic versioning.

**Usage:**
- Format: `<method-name>-<version>.method`
- Example: `echo-1.0.0.method`, `counter-2.1.0.method`
- Contains method instructions in plain text
- One instruction per line with newline termination
- Located in `methods/` directory

**Context:** Method definitions, reusable behaviors

### Plan Files
TDD plan documents describing test-driven development cycles.

**Usage:**
- Format: `<task-name>_plan.md`
- Example: `agent_store_fixture_plan.md`, `message_queue_plan.md`
- Contains cycles, iterations, RED/GREEN phases
- Tracks status with markers (PENDING REVIEW, REVIEWED, etc.)
- Located in `plans/` directory

**Context:** TDD planning, development plans

### Documentation Files
Module documentation following consistent structure.

**Usage:**
- Format: `<module-name>.md` in `modules/` directory
- Example: `modules/ar_data.md`, `modules/ar_agent.md`
- Documents module purpose, API, dependencies, examples
- Must pass `make check-docs` validation
- Uses real AgeRun types in examples

**Context:** Module documentation, API reference

### KB Article Files
Knowledge base articles documenting patterns, learnings, and best practices.

**Usage:**
- Format: `<topic-name>.md` in `kb/` directory (kebab-case)
- Example: `kb/tdd-iteration-planning-pattern.md`, `kb/ownership-naming-conventions.md`
- Common suffixes: `-pattern.md`, `-guide.md`, `-principle.md`
- Indexed in `kb/README.md` under category sections
- Must use real AgeRun types and functions in code examples
- Cross-referenced from commands via `([details](kb/article.md))`
- Include "Related Patterns" section linking to other KB articles

**Context:** Knowledge management, development patterns, best practices

## Data Type Concepts

### ar_data_t
The universal data container that can hold any type of value (string, integer, list, map, etc.).

**Usage:**
- Type-safe value storage with runtime type checking
- Created with `ar_data__create_string()`, `ar_data__create_integer()`, etc.
- Accessed with `ar_data__get_string()`, `ar_data__is_integer()`, etc.
- Ownership-aware with reference counting
- Destroyed with `ar_data__destroy()`

**Context:** Data representation, type system

### Opaque Type
A type whose internal structure is hidden from clients, exposing only through API functions.

**Usage:**
- Declared as `typedef struct module_name_s module_name_t;`
- Implementation details in `.c` file only
- Enforces information hiding (Parnas principle)
- Example: `ar_agent_t`, `ar_method_t`, `ar_list_t`

**Context:** Information hiding, module encapsulation

### Type-Safe Wrapper
Functions that provide type-safe access to ar_data_t contents.

**Usage:**
- Check type: `ar_data__is_string(data)`
- Get value: `ar_data__get_string(data)` (returns NULL if wrong type)
- Set value: `ar_data__set_map_string(map, key, value)`
- Prevents type confusion errors at runtime

**Context:** Type safety, runtime validation

## Semantic Versioning

### Version Format
AgeRun methods use semantic versioning to manage compatibility and evolution.

**Usage:**
- Format: `major.minor.patch` (e.g., "1.2.3")
- **Major**: Breaking changes (incompatible API changes)
- **Minor**: New features (backward-compatible additions)
- **Patch**: Bug fixes (backward-compatible corrections)

**Context:** Method versioning, compatibility management

### Version Compatibility
Rules for determining which method versions can work together.

**Usage:**
- Same major version → compatible (1.0.0 works with 1.5.3)
- Different major version → incompatible (1.x.x vs 2.x.x)
- Higher minor/patch → safe upgrade (1.2.0 → 1.2.1)
- Method calls specify exact or minimum version

**Context:** Runtime method selection, upgrades

### Version Selection
Strategy for choosing which method version to use when multiple versions exist.

**Usage:**
- Exact match: Use specific version (e.g., "1.2.3")
- Latest compatible: Use highest compatible version
- Fallback: Use older version if newer not available
- Stored in methodology registry

**Context:** Method lookup, runtime selection

## Workflow Status Markers

### Plan Status Markers
Lifecycle indicators tracking the progress of TDD plan iterations.

**Usage:**
- `PENDING REVIEW` - Newly created, awaiting review
- `REVIEWED` - Approved, ready for implementation
- `REVISED` - Updated after review, ready for implementation
- `IMPLEMENTED` - RED-GREEN-REFACTOR complete, awaiting commit
- `✅ COMMITTED` - Committed to git
- `✅ COMPLETE` - Full plan complete (plan-level marker)

**Context:** TDD plan tracking, iteration status

### Status Lifecycle
The progression of an iteration through the development workflow.

**Usage:**
```
Creation → PENDING REVIEW (by ar:create-plan)
         ↓
Review → REVIEWED or REVISED (by ar:review-plan)
         ↓
Implementation → IMPLEMENTED (by ar:execute-plan)
         ↓
Commit → ✅ COMMITTED (before git commit)
         ↓
Completion → ✅ COMPLETE (plan-level, optional)
```

**Context:** Development workflow, progress tracking

### Marker Placement
Where status markers appear in plan documents.

**Usage:**
- Iteration headings only: `#### Iteration 1.1: Description - PENDING REVIEW`
- NOT on cycle headings: `### Cycle 1: Basic Functionality` (no marker)
- Plan-level completion: `# Plan - ✅ COMPLETE` (optional)
- Batch updates before commit: all IMPLEMENTED → ✅ COMMITTED

**Context:** Plan formatting, status tracking

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
| **Agent** | Runtime Entity | "agent_id: 12345" | No |
| **Method** | Behavior Definition | "echo-1.0.0" | No |
| **Message** | Agent Communication | `ar_data_t* own_msg` | No |
| **Delegate** | Message Queue Proxy | `ar_delegate_t* mut_del` | No |
| **Agency** | Agent Manager | `ar_agency_t* mut_agency` | No |
| **Methodology** | Method Registry | `ar_methodology_t* mut_meth` | No |

## Related Documentation

- [TDD Iteration Planning Pattern](kb/tdd-iteration-planning-pattern.md)
- [Checkpoint Implementation Guide](kb/checkpoint-implementation-guide.md)
- [Multi-Step Checkpoint Tracking Pattern](kb/multi-step-checkpoint-tracking-pattern.md)
- [Ownership Naming Conventions](kb/ownership-naming-conventions.md)
- [Module Hierarchy and Dependencies](modules/README.md)
