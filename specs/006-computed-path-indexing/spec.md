# Feature Specification: Computed Path Indexing

**Feature Branch**: `006-computed-path-indexing`
**Created**: 2026-04-18
**Status**: Draft
**Input**: Enhancement proposal: "extend AgeRun access paths and assignment targets with computed indexing so methods can read and write structured data using runtime keys and indexes"

## Clarifications

### Session 2026-04-18

- Q: Which roots should support computed indexing? → A: `message`, `memory`, and `context` for reads; `memory` only for writes.
- Q: What syntax should computed indexing use? → A: Use square brackets, such as `memory[key]`, `memory.node[key]`, and `memory.list[index]`.
- Q: What types may index expressions resolve to? → A: String for map access and integer for list access.
- Q: Should computed indexing be allowed after dotted segments? → A: Yes.
- Q: Should computed indexing be allowed on the left-hand side of assignment? → A: Yes, but only for `memory...` targets.
- Q: What should happen when the resolved path is invalid for the container type? → A: The read follows existing missing-path semantics, and writes fail safely without partial mutation.
- Q: Should missing intermediate map segments be auto-created during writes? → A: Yes, for `memory...` assignment traversal.

## User Scenarios & Testing *(mandatory)*

### User Story 1 - Read map values using runtime keys (Priority: P1)

A method author can read structured data from `message`, `memory`, or `context` using keys that are
known only at runtime.

**Why this priority**: YAML parsing needs runtime-selected keys, not only statically named fields.

**Independent Test**: Evaluate expressions such as `memory[memory.key]` and `message[memory.field]`
and verify that the resolved values match the referenced runtime key.

**Acceptance Scenarios**:

1. **Given** `memory.key = "name"` and `memory.name = "Alice"`, **When** a method evaluates `memory[memory.key]`, **Then** the result is `"Alice"`.
2. **Given** `message.field_name = "status"` and `message.status = "ready"`, **When** a method evaluates `message[message.field_name]`, **Then** the result is `"ready"`.
3. **Given** `context.lookup = "mode"` and `context.mode = "strict"`, **When** a method evaluates `context[context.lookup]`, **Then** the result is `"strict"`.

---

### User Story 2 - Read list values using runtime indexes (Priority: P2)

A method author can address list elements dynamically using integer expressions.

**Why this priority**: Stack-like parser state and container traversal require index-based access.

**Independent Test**: Evaluate expressions such as `memory.stack[memory.depth]` and verify that the
correct list element is returned.

**Acceptance Scenarios**:

1. **Given** `memory.depth = 1` and `memory.stack = ["root", "child"]`, **When** a method evaluates `memory.stack[memory.depth]`, **Then** the result is `"child"`.
2. **Given** `memory.depth = 0`, **When** the same expression is evaluated, **Then** the result is the first list element.

---

### User Story 3 - Write memory values using runtime keys and indexes (Priority: P3)

A method author can assign into structured memory using keys and indexes that are only known while
executing the method.

**Why this priority**: Dynamic map construction is required to build parsed structures from YAML.

**Independent Test**: Execute assignments such as `memory.node[memory.key] := memory.value` and
verify that the resolved memory path is updated correctly.

**Acceptance Scenarios**:

1. **Given** `memory.key = "age"` and `memory.value = 30`, **When** a method evaluates `memory.node[memory.key] := memory.value`, **Then** `memory.node.age` is set to `30`.
2. **Given** a missing intermediate path, **When** a method evaluates a nested computed assignment that first resolves a section key and then resolves a field key under that section, **Then** missing intermediate map segments are created as maps during assignment traversal.
3. **Given** a computed write target that resolves through `context` or `message`, **When** assignment is attempted, **Then** the assignment is rejected because only `memory...` paths are writable.

## Edge Cases *(mandatory)*

- What happens when an index expression for map access resolves to a non-string value?
- What happens when an index expression for list access resolves to a non-integer value?
- What happens when a list index is negative or out of range?
- What happens when a map-style index is applied to a list value or a list-style index is applied to a map value?
- What happens when a computed write target tries to traverse through an existing scalar value?
- What happens when the same path mixes dotted and computed segments repeatedly?

## Scope Boundaries *(mandatory)*

### In Scope

- Extending access paths with computed indexing using square brackets
- Supporting computed reads from `message`, `memory`, and `context`
- Supporting computed writes to `memory...` targets only
- Supporting both map-key and list-index access using evaluated expressions
- Auto-creating missing intermediate map segments during computed `memory...` writes
- Preserving existing semantics for dotted access without computed indexing

### Out of Scope

- General expression indexing on arbitrary scalar values
- Writing through `message` or `context`
- Implicit list growth by numeric index assignment
- Slice syntax, range syntax, or negative-index shorthand
- Insert, delete, or reorder operations on lists

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: The expression grammar MUST support computed indexing segments written as `[expression]` after any `message`, `memory`, or `context` access segment.
- **FR-002**: Computed indexing MUST be allowed after a root segment and after dotted segments.
- **FR-003**: When a computed indexing segment is applied to a map, the index expression MUST resolve to a string key.
- **FR-004**: When a computed indexing segment is applied to a list, the index expression MUST resolve to an integer index.
- **FR-005**: Read access using computed indexing MUST preserve the existing missing-path contract already used by normal access paths.
- **FR-006**: Assignment targets MAY use computed indexing segments, but only on `memory...` paths.
- **FR-007**: During a computed `memory...` write, missing intermediate path segments that must be maps for traversal MUST be auto-created as maps.
- **FR-008**: A computed write MUST fail safely if traversal reaches an existing value whose type cannot support the requested next step.
- **FR-009**: A computed write MUST fail safely if a list index is invalid or if the index type does not match the container type.
- **FR-010**: A computed write MUST NOT permit assignment through `message` or `context` roots.
- **FR-011**: Access paths that do not use computed indexing MUST continue to behave exactly as they do before this enhancement.
- **FR-012**: Computed indexing MUST support nesting, including combinations that apply one computed lookup under another computed lookup, and paths such as `memory.stack[memory.depth]`.

### Key Entities *(include if feature involves data)*

- **Computed Index Segment**: A square-bracket path segment whose selector is determined by evaluating an expression.
- **Dynamic Map Key**: A string result used to access or assign a map entry.
- **Dynamic List Index**: An integer result used to access a list element.
- **Computed Memory Target**: A writable `memory...` path that includes one or more computed index segments.

## Assumptions & Dependencies *(mandatory)*

- **Assumptions**:
  - Existing AgeRun access semantics for missing fields remain the baseline for computed reads.
  - Type checking for computed indexing is based on the runtime type of the container being traversed.
  - Auto-creation during writes is limited to intermediate map segments required for path traversal.
- **Dependencies**:
  - Expression parser and evaluator
  - Memory-path parser and assignment target resolution
  - `ar_data_t` map/list access helpers
- **Open Questions**:
  - Whether a future release should support explicit numeric assignment that grows lists by index

## Documentation & Compatibility Impact *(mandatory)*

- **Affected Documentation**: `SPEC.md`, grammar/reference docs for access paths and assignment targets, parser/evaluator module docs, and method examples using dynamic structure access.
- **Affected Runtime Contracts**: Access-path grammar, expression evaluation, and assignment target resolution.
- **Compatibility Notes**: Additive feature. Existing dotted access and static assignments remain unchanged.

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: In the documented dynamic-map fixture set, `100%` of valid computed reads return the expected values.
- **SC-002**: In the documented dynamic-list fixture set, `100%` of valid computed index reads return the expected list element.
- **SC-003**: In the documented computed-write fixture set, `100%` of valid `memory...` writes mutate the expected resolved target path.
- **SC-004**: In the invalid-computed-path fixture set, `100%` of invalid writes fail safely without partial mutation or runtime instability.
