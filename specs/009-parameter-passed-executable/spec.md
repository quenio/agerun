# Feature Specification: Executable Boot Method Override

**Feature Branch**: `009-parameter-passed-executable`
**Created**: 2026-04-18
**Status**: Implemented
**Input**: User description: "A parameter MAY be passed to the executable in order to override the boot method."

## User Scenarios & Testing *(mandatory)*

### User Story 1 - Start with an alternate boot method (Priority: P1)

A runtime operator can launch the main AgeRun executable with a startup parameter that selects a
non-default boot method for a fresh runtime session.

**Why this priority**: This is the core user value: the executable must support alternate startup
flows without requiring source changes or method file edits.

**Independent Test**: Start the executable in a clean runtime state with a valid boot-method
override and verify that the first created agent uses the requested method and receives the normal
boot startup message.

**Acceptance Scenarios**:

1. **Given** no persisted agents are available and the operator provides a valid boot-method
   override, **When** the executable starts, **Then** it creates the initial boot agent from the
   requested method instead of the default bootstrap method.
2. **Given** no persisted agents are available and the operator provides a valid boot-method
   override, **When** startup continues, **Then** the selected boot agent receives the same startup
   boot message that the default boot agent would have received.

---

### User Story 2 - Preserve default startup when no override is provided (Priority: P2)

A runtime operator who launches the executable without the optional parameter continues to receive
AgeRun's current default startup behavior.

**Why this priority**: The new capability must be additive. Existing workflows, documentation, and
automation should keep working when the operator does nothing new.

**Independent Test**: Start the executable in a clean runtime state without an override parameter
and verify that it still creates the default bootstrap agent and runs the existing startup flow.

**Acceptance Scenarios**:

1. **Given** no persisted agents are available and no override parameter is provided, **When** the
   executable starts, **Then** it creates the default bootstrap agent as it does today.
2. **Given** automation or users continue launching the executable without the new parameter,
   **When** startup completes, **Then** the observable startup behavior remains unchanged.

---

### User Story 3 - Get clear behavior for invalid or non-applicable overrides (Priority: P3)

A runtime operator can understand what happened when the override cannot be applied because the
requested method is unavailable or because the executable restored persisted agents instead of
starting fresh.

**Why this priority**: Startup overrides affect operator trust. Failures and skipped overrides must
be explicit so users do not misread the runtime state.

**Independent Test**: Verify one fresh-start run with an unavailable override and one restored-state
run with an override, confirming that each case reports the correct outcome.

**Acceptance Scenarios**:

1. **Given** no persisted agents are available and the operator provides an override for a method
   that is not available at startup, **When** the executable starts, **Then** startup fails with a
   clear error instead of silently falling back to the default boot method.
2. **Given** persisted agents are restored at startup and the operator also provides a boot-method
   override, **When** the executable starts, **Then** the executable keeps the restored runtime
   state and makes it clear that no fresh boot agent was created.

## Edge Cases *(mandatory)*

- If the override names a method that exists but is not semantically suitable as a boot method, the
  executable does not perform extra semantic screening beyond normal startup mechanics. For this
  feature, a method is considered boot-capable if it resolves successfully, the boot agent can be
  created, and the standard `"__boot__"` message can be queued.
- What happens when the override refers to a method identifier that is malformed, empty, or uses a
  version that does not resolve at startup?
- What happens when persisted agents are loaded successfully and an override is also supplied—how
  is it communicated that the override was not used?
- What happens when method loading succeeds but the selected override still cannot be instantiated
  as the initial boot agent?

## Scope Boundaries *(mandatory)*

### In Scope

- Allowing the main AgeRun executable to accept an optional startup parameter that overrides the
  default boot method on fresh startup
- Applying the override only to runs that would otherwise create a new default boot agent
- Preserving the standard startup boot message semantics for the selected boot method
- Reporting clear outcomes when the override is invalid, unavailable, or skipped because persisted
  agents were restored
- Documenting the new startup behavior and its interaction with persisted runtime state

### Out of Scope

- Changing the method language itself
- Defining new boot-message contents or a second startup protocol
- Replacing agents that were restored from persisted agency state
- Introducing multiple startup overrides or a broader executable configuration system
- Changing the behavior of the separate `arsh` executable

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: The main AgeRun executable MUST support an optional startup parameter that identifies
  an alternate boot method for fresh runtime startup.
- **FR-002**: When no override parameter is provided and no persisted agents are restored, the
  executable MUST continue using the current default boot method.
- **FR-003**: When a valid override parameter is provided and no persisted agents are restored, the
  executable MUST create the initial boot agent from the requested method instead of the default
  boot method.
- **FR-004**: When the override is applied, the selected boot agent MUST receive the same standard
  startup boot message used by the default boot method.
- **FR-005**: If the override identifies a method that is unavailable or cannot be instantiated at
  startup, the executable MUST fail clearly and MUST NOT silently fall back to the default boot
  method.
- **FR-006**: If persisted agents are restored at startup, the executable MUST preserve that
  restored runtime state and MUST NOT create a second fresh boot agent solely because an override
  parameter was supplied.
- **FR-007**: When an override parameter is supplied but not applied because persisted agents were
  restored, the executable MUST make that outcome observable to the operator.
- **FR-008**: The new override capability MUST NOT change method loading, message processing,
  persistence saving, or shutdown behavior apart from selecting the initial boot method on fresh
  startup.
- **FR-009**: Documentation for the executable startup flow MUST describe the optional override
  parameter, the default behavior when omitted, and the restored-state behavior when startup skips
  fresh boot creation.

### Key Entities *(include if feature involves data)*

- **Boot Method Override**: The optional executable startup input that identifies which method to
  use as the initial boot method for a fresh runtime session.
- **Default Boot Method**: The executable's existing startup method used when no override is
  supplied.
- **Fresh Startup**: An executable launch in which no persisted agents are restored, so the runtime
  must create an initial boot agent.
- **Restored Runtime State**: A startup path in which persisted agents are loaded and therefore no
  new boot agent is created.

## Assumptions & Dependencies *(mandatory)*

- **Assumptions**:
  - AgeRun already has a single default boot method used for fresh executable startup.
  - The override is optional and additive rather than a replacement for current startup behavior.
  - The executable continues using its existing persisted-agents rule: restored runtime state takes
    precedence over creating a new boot agent.
  - The selected override method is expected to participate in the same boot-message contract as the
    default boot method.
- **Dependencies**:
  - Main executable startup flow
  - Method discovery/loading at process startup
  - Initial boot-agent creation
  - Executable and README/SPEC startup documentation
- **Open Questions**: None

## Documentation & Compatibility Impact *(mandatory)*

- **Affected Documentation**: `README.md`, `SPEC.md`, `modules/ar_executable.md`, and executable
  startup tests/docs that describe default bootstrap behavior.
- **Affected Runtime Contracts**: Main executable startup behavior and CLI-facing startup contract.
- **Compatibility Notes**: Backward-compatible additive change. Existing executable invocations
  without the new parameter continue to use the default boot method.

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: In the fresh-start executable test set, `100%` of launches with a valid override start
  with the requested boot method instead of the default boot method.
- **SC-002**: In the default-start executable test set, `100%` of launches without an override
  preserve the current bootstrap startup behavior.
- **SC-003**: In the invalid-override test set, `100%` of launches with a malformed or unavailable
  override produce an explicit startup failure rather than silently falling back to the default boot
  method.
- **SC-004**: In the restored-state test set, `100%` of launches that restore persisted agents avoid
  creating a second fresh boot agent even when an override parameter is supplied.
