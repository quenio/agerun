# Feature Specification: Context Self ID

**Feature Branch**: `008-context-self-id`
**Created**: 2026-04-18
**Status**: Draft
**Input**: Enhancement proposal: "expose the current agent ID to methods as `context.self_id` so agents can send work to themselves and implement iterative parsing flows"

## Clarifications

### Session 2026-04-18

- Q: What name should expose the current agent identifier? → A: `context.self_id`.
- Q: Should `self_id` be available to every agent method execution? → A: Yes.
- Q: What type should `context.self_id` use? → A: Integer.
- Q: Should `self_id` be writable? → A: No. It is a runtime-supplied read-only context field.
- Q: Can user-provided agent context override `self_id`? → A: No. `self_id` is a reserved runtime field.
- Q: What is the main intended use? → A: Self-messaging via `send(context.self_id, message)` and other self-referential workflows.

## User Scenarios & Testing *(mandatory)*

### User Story 1 - Send work to self (Priority: P1)

A method author can queue follow-up work for the same agent without needing an externally provided
self identifier.

**Why this priority**: Iterative parsing and state-machine workflows depend on a stable way for an
agent to address itself.

**Independent Test**: Execute a method that evaluates `send(context.self_id, message)` and verify
that the message is enqueued for the same agent.

**Acceptance Scenarios**:

1. **Given** an active agent handling a message, **When** the method evaluates `send(context.self_id, memory.next_message)`, **Then** the runtime enqueues that message for the same agent.
2. **Given** a method uses `context.self_id` in later arithmetic or comparisons, **When** the method executes, **Then** the value behaves as a normal integer expression result.

---

### User Story 2 - Use self identity without extra user wiring (Priority: P2)

A method author can rely on `context.self_id` being present even when the user-provided context did
not include any self-reference.

**Why this priority**: Self-messaging should be a native runtime capability, not an external setup
requirement.

**Independent Test**: Create an agent with an empty or unrelated context, execute a method that
reads `context.self_id`, and verify that the runtime supplies the current agent ID.

**Acceptance Scenarios**:

1. **Given** an agent was created with no `self_id` field in its user context, **When** the method reads `context.self_id`, **Then** it receives the current agent ID.
2. **Given** the user context already contains a `self_id` key, **When** the method reads `context.self_id`, **Then** the runtime-supplied current agent ID takes precedence.

---

### User Story 3 - Preserve context read-only semantics (Priority: P3)

A runtime operator can rely on the new field not weakening the rule that `context` is read-only.

**Why this priority**: The enhancement must preserve one of the method language's core invariants.

**Independent Test**: Attempt to assign to `context.self_id` or use it as a writable path and
verify that the normal rejection of `context...` assignment still applies.

**Acceptance Scenarios**:

1. **Given** a method attempts `context.self_id := 7`, **When** the method is parsed or evaluated, **Then** the operation is rejected by the existing `memory...`-only write rule.
2. **Given** a method reads `context.self_id` and later updates `memory.self_id`, **When** the method executes, **Then** only `memory.self_id` changes and the runtime-provided `context.self_id` remains read-only.

## Edge Cases *(mandatory)*

- What happens when an agent is executing its first message immediately after creation?
- What happens when the method compares `context.self_id` to `message.sender` for self-sent messages?
- What happens when an agent persists memory but not context—does `self_id` stay runtime-derived on later executions?
- What happens when system or delegate messages are involved—does `context.self_id` still refer to the current agent?

## Scope Boundaries *(mandatory)*

### In Scope

- Exposing the current agent ID as `context.self_id`
- Making `self_id` available on every agent method execution
- Keeping `self_id` read-only as part of the runtime context view
- Reserving the `self_id` field name against user override in the runtime-visible context view
- Supporting self-messaging with existing `send(...)`

### Out of Scope

- Adding a new `self` keyword or `send(self, ...)` syntax
- Exposing delegate IDs or system IDs through the same field
- Making context writable
- Adding other runtime metadata fields unless specified separately

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: The runtime MUST expose the current agent's identifier to methods as `context.self_id`.
- **FR-002**: `context.self_id` MUST be available during every agent method execution.
- **FR-003**: `context.self_id` MUST be an integer value.
- **FR-004**: `context.self_id` MUST refer to the ID of the currently executing agent, not to the sender of the current message.
- **FR-005**: `context.self_id` MUST be read-only and MUST remain subject to the existing rule that only `memory...` paths are writable.
- **FR-006**: The runtime-visible value of `context.self_id` MUST take precedence over any user-provided context field with the same name.
- **FR-007**: `send(context.self_id, message)` MUST use the normal `send(...)` semantics for sending a message to the currently executing agent.
- **FR-008**: `context.self_id` MUST behave like a normal integer expression value in comparisons, arithmetic, and message construction.
- **FR-009**: The addition of `context.self_id` MUST NOT change the behavior of methods that do not read it.

### Key Entities *(include if feature involves data)*

- **Self ID**: The integer identifier of the currently executing agent, exposed through the runtime context as `context.self_id`.
- **Runtime Context View**: The effective read-only context visible to a method during execution, combining user-provided context with reserved runtime-supplied fields.

## Assumptions & Dependencies *(mandatory)*

- **Assumptions**:
  - AgeRun already knows the current agent ID during method execution.
  - `context` remains read-only in the language contract.
  - Existing `send(...)` semantics are sufficient once the current agent ID is available.
- **Dependencies**:
  - Frame/interpreter plumbing that already tracks the currently executing agent
  - Expression evaluation for `context...` access
  - `send(...)` routing semantics
- **Open Questions**:
  - Whether future runtime metadata should live under `context.*` or a dedicated reserved root

## Documentation & Compatibility Impact *(mandatory)*

- **Affected Documentation**: `SPEC.md`, method-language reference docs, interpreter/frame docs if they mention execution context, and examples showing iterative self-messaging.
- **Affected Runtime Contracts**: Runtime-visible context semantics.
- **Compatibility Notes**: Additive feature. Existing methods continue to behave unchanged unless they intentionally read `context.self_id`.

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: In the documented self-send fixture set, `100%` of `send(context.self_id, ...)` calls enqueue work for the currently executing agent.
- **SC-002**: In the documented runtime-context fixture set, `100%` of method executions expose the current agent ID through `context.self_id` even when the user-provided context omits or conflicts with that field.
- **SC-003**: In the read-only fixture set, `100%` of attempted writes to `context.self_id` are rejected under the existing `memory...`-only assignment rule.
