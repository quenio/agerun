# Feature Specification: Assignment List Append

**Feature Branch**: `007-assignment-list-append`
**Created**: 2026-04-18
**Status**: Draft
**Input**: Enhancement proposal: "extend assignment syntax with append semantics so methods can create and grow lists while building structured parse results"

## Clarifications

### Session 2026-04-18

- Q: What syntax should append use? → A: Use an empty trailing index on the left-hand side, such as `memory.items[] := value`.
- Q: Should append work only on `memory...` paths? → A: Yes.
- Q: What should happen if the target path does not exist? → A: The runtime creates a list at that path and appends the value.
- Q: What should happen if the target path already exists as a list? → A: The runtime appends the value to the end of the existing list.
- Q: What should happen if the target path already exists but is not a list? → A: The assignment fails safely and leaves the existing value unchanged.
- Q: Should append support dynamic target paths? → A: Yes, including paths such as `memory.node[memory.key][]`.

## User Scenarios & Testing *(mandatory)*

### User Story 1 - Build a list incrementally (Priority: P1)

A method author can append one value at a time into a list using normal assignment syntax.

**Why this priority**: YAML sequences cannot be represented without a way to grow lists across
multiple parsing steps.

**Independent Test**: Execute repeated assignments using `memory.items[] := value` and verify that
a list is created on first use and grows in insertion order.

**Acceptance Scenarios**:

1. **Given** `memory.items` does not exist, **When** a method evaluates `memory.items[] := "a"`, **Then** `memory.items` becomes a list containing one element `"a"`.
2. **Given** `memory.items` already contains `["a"]`, **When** a method evaluates `memory.items[] := "b"`, **Then** `memory.items` becomes `["a", "b"]`.
3. **Given** repeated append operations, **When** values are appended in order, **Then** the resulting list preserves that order.

---

### User Story 2 - Append through dynamic target paths (Priority: P2)

A method author can append to a list selected by runtime keys while constructing nested parsed
structures.

**Why this priority**: YAML parsing often needs to append under keys discovered at runtime.

**Independent Test**: Execute assignments such as `memory.node[memory.key][] := memory.value` and
verify that the resolved target list is updated correctly.

**Acceptance Scenarios**:

1. **Given** `memory.key = "items"` and `memory.value = "x"`, **When** a method evaluates `memory.node[memory.key][] := memory.value`, **Then** `memory.node.items` becomes a list containing `"x"`.
2. **Given** the same target already exists as a list, **When** a second append occurs, **Then** the new value is added at the end.

---

### User Story 3 - Fail safely on non-list append targets (Priority: P3)

A runtime operator can rely on append syntax not corrupting existing scalar or map values.

**Why this priority**: Append introduces a new structural mutation path and must preserve memory
safety and predictable behavior.

**Independent Test**: Attempt append assignment into an existing non-list target and verify that
the runtime rejects the operation without partial mutation.

**Acceptance Scenarios**:

1. **Given** `memory.items = "not a list"`, **When** a method evaluates `memory.items[] := "x"`, **Then** the append fails and `memory.items` remains unchanged.
2. **Given** `memory.node[memory.key]` resolves through an invalid traversal path, **When** append is attempted, **Then** the operation fails safely and leaves all existing reachable values unchanged.

## Edge Cases *(mandatory)*

- What happens when the appended expression evaluates to a list or map value?
- What happens when the target path contains both computed indexing and dotted segments?
- What happens when an intermediate path segment is missing during append traversal?
- What happens when the target path resolves to a list element rather than a list container?
- What happens when append is attempted through `message...[]` or `context...[]`?

## Scope Boundaries *(mandatory)*

### In Scope

- Extending assignment syntax with trailing `[]` append semantics
- Creating a list automatically when the append target path does not yet exist
- Appending to existing lists in insertion order
- Supporting append through dynamic `memory...` paths
- Safe failure behavior when the resolved target is not a list

### Out of Scope

- Insert-at-index syntax
- Prepend syntax
- List removal or replacement operations
- Append targets rooted at `message` or `context`
- New list literal syntax

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: The assignment grammar MUST support a trailing empty bracket segment on the left-hand side, written as `memory.path[] := expression`.
- **FR-002**: Append syntax MUST be valid only for `memory...` assignment targets.
- **FR-003**: If the resolved append target path does not exist, the runtime MUST create a list at that path and append the evaluated right-hand-side value as the first element.
- **FR-004**: If the resolved append target path already exists as a list, the runtime MUST append the evaluated right-hand-side value to the end of that list.
- **FR-005**: If the resolved append target path exists but is not a list, the assignment MUST fail safely and leave the existing value unchanged.
- **FR-006**: Append syntax MUST support the same dynamic path resolution rules used by computed `memory...` targets, including paths such as `memory.node[memory.key][]`.
- **FR-007**: Append evaluation MUST preserve insertion order.
- **FR-008**: If append traversal requires intermediate map segments that do not exist, the runtime MUST auto-create those map segments using the same `memory...` write traversal rules as computed assignment.
- **FR-009**: Append attempts through `message` or `context` roots MUST be rejected.
- **FR-010**: Assignments that do not use trailing `[]` MUST continue to behave exactly as they do before this enhancement.
- **FR-011**: Append failure MUST NOT partially mutate the target path or any existing list contents.

### Key Entities *(include if feature involves data)*

- **Append Target**: A writable `memory...` path ending in `[]`, indicating append-to-list semantics.
- **Implicit List Creation**: The runtime behavior of creating a list automatically when a valid append target path does not yet exist.
- **Append Operation**: One atomic evaluation of the right-hand-side expression followed by list append or safe failure.

## Assumptions & Dependencies *(mandatory)*

- **Assumptions**:
  - Append reuses normal expression evaluation for the right-hand-side value.
  - Append shares path traversal and dynamic indexing rules with computed `memory...` assignment targets.
  - Appending a value transfers or copies ownership according to normal assignment semantics.
- **Dependencies**:
  - Assignment parser and evaluator
  - `ar_data_t` list support
  - Dynamic path indexing support for nested and computed targets
- **Open Questions**:
  - Whether a future release should add explicit list constructors or insert-by-index operations

## Documentation & Compatibility Impact *(mandatory)*

- **Affected Documentation**: `SPEC.md`, assignment grammar documentation, parser/evaluator module docs, and method examples that build structured data incrementally.
- **Affected Runtime Contracts**: Assignment syntax and memory mutation semantics.
- **Compatibility Notes**: Additive feature. Existing assignments remain unchanged unless they intentionally use trailing `[]`.

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: In the documented append acceptance fixture set, `100%` of valid append assignments create or extend the expected list in insertion order.
- **SC-002**: In the documented dynamic-append fixture set, `100%` of valid append assignments through computed paths mutate the expected resolved target path.
- **SC-003**: In the invalid-append fixture set, `100%` of append attempts against non-list or non-writable targets fail safely without partial mutation.
