# Feature Specification: [FEATURE NAME]

**Feature Branch**: `[###-feature-name]`
**Created**: [DATE]
**Status**: Draft
**Input**: User description: "$ARGUMENTS"

## User Scenarios & Testing *(mandatory)*

<!--
  User stories MUST be prioritized user journeys ordered by importance.
  Each story MUST be independently testable and specific enough to support
  later TDD planning and acceptance validation.
-->

### User Story 1 - [Brief Title] (Priority: P1)

[Describe this user journey in plain language]

**Why this priority**: [Explain the value and why it has this priority level]

**Independent Test**: [Describe how this can be tested independently and what value it proves]

**Acceptance Scenarios**:

1. **Given** [initial state], **When** [action], **Then** [expected outcome]
2. **Given** [initial state], **When** [action], **Then** [expected outcome]

---

### User Story 2 - [Brief Title] (Priority: P2)

[Describe this user journey in plain language]

**Why this priority**: [Explain the value and why it has this priority level]

**Independent Test**: [Describe how this can be tested independently]

**Acceptance Scenarios**:

1. **Given** [initial state], **When** [action], **Then** [expected outcome]

---

### User Story 3 - [Brief Title] (Priority: P3)

[Describe this user journey in plain language]

**Why this priority**: [Explain the value and why it has this priority level]

**Independent Test**: [Describe how this can be tested independently]

**Acceptance Scenarios**:

1. **Given** [initial state], **When** [action], **Then** [expected outcome]

---

[Add more user stories as needed, each with an assigned priority]

## Edge Cases *(mandatory)*

- What happens when [boundary condition]?
- How does the system handle [error scenario]?
- What behavior is required if validation, persistence, or external dependencies fail?

## Scope Boundaries *(mandatory)*

### In Scope

- [Explicitly included behavior or constraint]

### Out of Scope

- [Explicitly excluded behavior or follow-up work]

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: System MUST [specific capability]
- **FR-002**: System MUST [specific capability]
- **FR-003**: Users MUST be able to [key interaction]
- **FR-004**: System MUST [data or state requirement]
- **FR-005**: System MUST [behavior or validation rule]

*Example of marking unclear requirements:*

- **FR-006**: System MUST authenticate users via [NEEDS CLARIFICATION: auth method not specified]
- **FR-007**: System MUST retain user data for [NEEDS CLARIFICATION: retention period not specified]

### Key Entities *(include if feature involves data)*

- **[Entity 1]**: [What it represents, key attributes without implementation]
- **[Entity 2]**: [What it represents, relationships to other entities]

## Assumptions & Dependencies *(mandatory)*

- **Assumptions**: [Reasonable defaults chosen while writing the spec]
- **External Dependencies**: [Required systems, files, services, or N/A]
- **Open Questions**: [Anything deferred to `/spec clarify` or `/spec plan`, or None]

## Documentation & Compatibility Impact *(mandatory)*

- **Affected Documentation**: [README.md, SPEC.md, module docs, KB articles, method docs, or N/A]
- **Affected Runtime Contracts**: [methods, modules, saved data, CLI behavior, or N/A]
- **Compatibility Notes**: [backward-compatible, breaking change, migration note, or N/A]

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: [Measurable user or system outcome]
- **SC-002**: [Measurable performance, reliability, or scale outcome]
- **SC-003**: [Observable completion metric]
- **SC-004**: [Business or maintenance impact metric]
