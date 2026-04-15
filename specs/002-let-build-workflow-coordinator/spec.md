# Feature Specification: Workflow Coordinator

**Feature Branch**: `002-let-build-workflow-coordinator`
**Created**: 2026-04-15
**Status**: Draft
**Input**: User description: "Let's build the workflow coordinator."

## Clarifications

### Session 2026-04-15

- Q: What should happen when persisted workflow agents are restored? → A: Resume restored state and skip creating a new demo.
- Q: Should the bundled workflow allow reopening/rework after review, or stay a single-pass lifecycle? → A: Single-pass lifecycle only.
- Q: What terminal outcomes should the bundled example support? → A: Both completed and rejected.
- Q: Which terminal outcome should the default fresh-run demo actually show? → A: Validation-driven outcome.
- Q: How generic should the coordinator be about workflow definitions? → A: Read the workflow definition from a workflow definition file; tests define a test workflow.
- Q: What kind of schema should the workflow definition file use? → A: Declarative stages, explicit transitions, and validation rules.
- Q: What top-level elements should every workflow definition include? → A: Metadata, item field schema, stages, transitions, validation clauses, and terminal outcomes.
- Q: Where should validation clauses attach in the workflow definition? → A: Attach validations to transitions only.
- Q: What outcomes should a transition-level validation be allowed to produce? → A: advance, stay, or reject.
- Q: Where should the feature logic live? → A: Implement the coordinator entirely as AgeRun methods launched by the executable's boot method, not as new C feature logic.
- Q: What kind of workflow definition file should be used? → A: A YAML file read by agents via the file delegate, not a method file.

## User Scenarios & Testing *(mandatory)*

### User Story 1 - Launch a Useful Demo Automatically (Priority: P1)

A user runs the main AgeRun executable and immediately sees a complete, useful workflow example
start and finish without needing the interactive shell. The example demonstrates how AgeRun can
coordinate a generic work item from creation through a terminal outcome of either completion or
rejection.

**Why this priority**: The feature only delivers value as a bundled example if it launches from the
normal executable startup flow and demonstrates useful behavior immediately.

**Independent Test**: Start the executable in a clean runtime state and confirm it loads the
bundled workflow definition file, launches the workflow coordinator automatically, advances one demo
work item through the expected lifecycle, and emits human-readable progress updates plus a final
result.

**Acceptance Scenarios**:

1. **Given** no persisted agents are restored at startup, **When** the main executable starts,
   **Then** the boot flow launches the workflow coordinator demo automatically without requiring
   shell input.
2. **Given** the workflow coordinator demo launches on startup, **When** the run completes,
   **Then** the user can observe a complete end-to-end workflow from item creation through a
   validation-driven terminal outcome and final summary in the normal executable output and logs.
3. **Given** persisted workflow agents are restored at startup, **When** the executable starts,
   **Then** the system resumes the restored workflow state and does not create a duplicate fresh
   demo item on top of that restored state.

---

### User Story 2 - Track a Generic Work Item Through Stages (Priority: P2)

A user can treat the bundled example as a reusable template for any staged work process by seeing a
single work item move through a single-pass sequence of neutral stages such as intake, triage,
active work, review, and completion while preserving ownership, priority, and status information.

**Why this priority**: The app is only broadly useful if the example is generic enough to adapt to
many real workflows instead of being tied to one business domain.

**Independent Test**: Run the coordinator against one demo work item and confirm that each stage
change updates the item's visible state consistently, including its identity, priority, assigned
owner, and current status. Validation runs may substitute a test workflow definition file to prove
alternative stage rules and rejection behavior.

**Acceptance Scenarios**:

1. **Given** a new work item enters the coordinator, **When** it is accepted for processing,
   **Then** the system records a stable item identity, title, priority, current stage, and current
   status.
2. **Given** a work item is ready to advance, **When** the coordinator routes it to the next stage,
   **Then** the item reflects the new stage without losing previously established details such as
   priority and ownership.
3. **Given** a work item receives an invalid or out-of-order update, **When** the coordinator
   evaluates that update, **Then** the item remains in its last valid state and the user receives a
   clear explanation of the rejected action.

---

### User Story 3 - Review Progress and Final Outcome (Priority: P3)

A user can understand what happened during the workflow run by reading concise progress messages and
one final summary that explains the item's lifecycle, final status, and key state at completion.

**Why this priority**: A reusable example is far more valuable when users can understand the
pattern from observable output rather than by inspecting internal runtime state.

**Independent Test**: Run the coordinator and confirm that lifecycle checkpoints and the final
summary are observable in normal runtime output, and that the final summary alone is sufficient to
identify the item's terminal state.

**Acceptance Scenarios**:

1. **Given** the coordinator is processing a work item, **When** the item changes stage,
   **Then** the runtime emits a human-readable progress update describing the new lifecycle state.
2. **Given** the coordinator finishes the workflow, **When** the final summary is emitted,
   **Then** the summary identifies the item, its final stage, final status, assigned owner, and the
   number of stage changes completed.

## Edge Cases *(mandatory)*

- What happens when the executable receives the boot trigger more than once during the same run?
- What happens when a workflow definition file is missing required top-level elements such as metadata, item field schema, stages, transitions, validation clauses, or terminal outcomes?
- What happens when a workflow update omits required item details such as item identity or title?
- What happens when a stage advance is requested before prerequisite stages have completed?
- What happens when a rework or reopen request is received after the item has already entered review
  or reached a terminal outcome in the bundled single-pass lifecycle?
- What happens when a duplicate completion or duplicate assignment update is received?
- Restored persisted workflow agents resume from their last valid state and do not trigger a new
  fresh demo run on top of that restored state.
- What happens when workflow step validation determines that the item must be rejected rather than
  completed?
- What happens when the coordinator cannot produce a valid summary for the current state?
- What happens when a user-visible progress update cannot be delivered to the normal runtime log
  path?

## Scope Boundaries *(mandatory)*

### In Scope

- A bundled, boot-launched workflow coordinator example that runs from the main executable's boot
  method rather than from the interactive shell
- Implementing the coordinator behavior as AgeRun methods rather than as new C feature logic
- Reading a YAML workflow definition file via the file delegate, with the file declaring metadata, item field schema, stages, explicit transitions, validation clauses, and terminal outcomes
- A generic staged-workflow example centered on one reusable work-item lifecycle
- Automatic creation, routing, progression, and terminal resolution of a demo work item during a
  fresh run
- Human-readable progress messages and final summaries that show how the workflow behaves
- Protection against invalid or out-of-order updates corrupting workflow state
- Preservation of existing startup behavior that skips fresh boot creation when persisted agents are
  restored
- Documentation that explains how the bundled example can be adapted to other staged workflows

### Out of Scope

- A visual workflow designer or graphical interface
- Arbitrary user-authored workflow editing during runtime
- Network integrations, email delivery, or third-party service hooks
- Multi-user authentication, permissions, or approval chains
- Interactive shell-driven control of the workflow demo as the primary experience
- Domain-specific business rules for incidents, games, e-commerce, or helpdesk systems

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: The system MUST provide a bundled workflow coordinator example that demonstrates a
  generic staged workflow useful to AgeRun users beyond one specific business domain.
- **FR-0010**: The workflow coordinator feature logic MUST be implemented as AgeRun methods launched
  by the executable's boot method rather than as new C feature logic.
- **FR-001a**: The workflow coordinator MUST read its workflow definition from a YAML workflow
  definition file via the file delegate.
- **FR-001aa**: The workflow definition schema MUST be declarative and MUST describe stages,
  explicit transitions, and validation rules for workflow progression.
- **FR-001ab**: Every YAML workflow definition file MUST include metadata, an item field schema,
  stages, transitions, validation clauses, and terminal outcomes.
- **FR-001ac**: Validation clauses in the workflow definition MUST attach to transitions rather than
  existing only as stage-level or global rules.
- **FR-001b**: The bundled example MUST include one default YAML workflow definition file used for
  fresh executable demo runs.
- **FR-002**: On a fresh executable run, the system MUST launch the workflow coordinator through the
  normal boot flow without requiring shell interaction.
- **FR-003**: The bundled example MUST create exactly one demo work item during a fresh boot-driven
  run and process it from initial intake to a terminal outcome within that run.
- **FR-003a**: The bundled example MUST support both successful completion and explicit rejection as
  valid terminal outcomes for a work item.
- **FR-003b**: The terminal outcome for a workflow run MUST be determined by workflow step
  validation results rather than by a hard-coded default outcome.
- **FR-003c**: A transition-level validation rule MUST be able to produce exactly one of these
  outcomes: advance, stay, or reject.
- **FR-004**: The demo work item MUST maintain visible state including an item identifier, title,
  priority, current stage, current status, assigned owner, and final summary.
- **FR-005**: The workflow coordinator MUST support an ordered generic single-pass lifecycle that
  covers intake, triage, active work, review, and completion.
- **FR-005a**: In the bundled example, the lifecycle MUST be monotonic; once a work item advances
  past a stage, it does not reopen or loop back to an earlier stage.
- **FR-006**: The system MUST preserve established work-item state across valid stage changes unless
  a later valid update explicitly changes that field.
- **FR-007**: The system MUST reject invalid, duplicate, or out-of-order lifecycle updates without
  corrupting the last valid work-item state.
- **FR-008**: The system MUST emit human-readable progress updates for meaningful lifecycle changes
  during the demo run.
- **FR-009**: The system MUST emit a final human-readable summary that identifies the work item and
  its terminal state without requiring users to inspect internal runtime memory.
- **FR-010**: When persisted workflow agents are restored at startup, the system MUST resume the
  restored workflow state and MUST avoid creating a duplicate fresh workflow demo on top of that
  restored runtime state.
- **FR-011**: The bundled example MUST use neutral workflow language so AgeRun users can adapt the
  pattern to many domains.
- **FR-012**: The feature MUST include documentation that explains the workflow lifecycle, expected
  observable output, and how the example serves as a reusable AgeRun pattern.
- **FR-012a**: Validation and acceptance testing MUST be able to supply alternate YAML test
  workflow definition files so workflow-specific step validation and rejection behavior can be
  verified.

### Key Entities *(include if feature involves data)*

- **Workflow Definition File**: A YAML file containing the declarative metadata, item field
  schema, stages, explicit transitions, validation clauses, and terminal outcomes that the workflow
  coordinator uses to process work items.
- **Workflow Metadata**: High-level identifying information about the workflow definition that lets
  users and tests distinguish one workflow from another.
- **File Delegate Request**: A message sent by a workflow agent to the file delegate so the agent
  can read the YAML workflow definition file content.
- **Item Field Schema**: The declared set of workflow-item fields and required attributes that a
  work item must satisfy as it moves through the workflow.
- **Validation Clause**: A declarative rule attached to a transition in the workflow definition
  that determines whether the attempted progression may proceed and what outcome is produced.
- **Validation Outcome**: The result of evaluating a transition-level validation clause; in this
  feature the allowed outcomes are advance, stay, or reject.
- **Workflow Item**: A generic unit of work that moves through a staged lifecycle and exposes a
  stable identity, title, priority, owner, current stage, current status, and final outcome.
- **Workflow Stage**: A named lifecycle step that represents where the item currently sits in the
  overall process and determines which next transitions are valid in the bundled single-pass
  lifecycle.
- **Workflow Update**: A lifecycle event that creates, assigns, advances, reviews, completes, or
  rejects work on an item.
- **Step Validation**: A decision point that evaluates whether the workflow item satisfies the
  conditions needed to advance or must be rejected.
- **Terminal Outcome**: The final resolved state of a workflow item, determined by workflow step
  validation and expressed in the bundled example as either completed or rejected.
- **Workflow Summary**: A human-readable snapshot of the item lifecycle that captures the most
  important state for users evaluating the run.

## Assumptions & Dependencies *(mandatory)*

- **Assumptions**:
  - The bundled example will demonstrate one primary work item on a fresh run so the output remains
    concise and easy to follow.
  - The default fresh-run demo uses a bundled YAML workflow definition file, while validation may
    use a separate YAML test workflow definition file.
  - Workflow definition files are declarative YAML rather than imperative code and provide explicit
    metadata, field schema, transition, transition-level validation, and terminal-outcome data.
  - A neutral single-pass lifecycle of intake -> triage -> active work -> review -> completion is a
    reasonable default for a generic workflow example.
  - The specific terminal outcome of a run is determined by workflow step validation rather than by
    a fixed always-complete or always-reject demo script.
  - Normal executable output and log output are the primary ways users will observe the example.
  - Existing AgeRun startup and persistence behavior remain the baseline and should be extended, not
    replaced.
  - Restored workflow agents continue from persisted state instead of being replaced by a fresh demo.
- **External Dependencies**: No new external services are required; the feature depends on existing
  AgeRun executable startup, logging, persistence, method-loading behavior, the existing file
  delegate path for file reads, and availability of the selected YAML workflow definition file.
- **Open Questions**: None.

## Documentation & Compatibility Impact *(mandatory)*

- **Affected Documentation**: README.md, methods/README.md, workflow coordinator method
  documentation, bootstrap/startup demo documentation, CHANGELOG.md, and any relevant KB articles
  if new reusable patterns are discovered.
- **Affected Runtime Contracts**: Boot-driven executable startup behavior, YAML
  workflow-definition-file reads through the file delegate, bundled method inventory, restored
  workflow-agent startup/resume behavior, and observable runtime log output for the demo.
- **Compatibility Notes**: Intended to be backward-compatible with existing startup and persistence
  behavior; the new example replaces the current bundled demo scenario but should not require shell
  usage or a data migration.

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: On a fresh executable run, users can observe the workflow coordinator start
  automatically from the boot method and reach a validation-driven completed or rejected terminal
  outcome without entering shell input.
- **SC-002**: The demo produces at least four distinct user-visible lifecycle checkpoints before the
  final summary, covering creation, triage, active work, and completion.
- **SC-003**: In validation runs, an invalid or duplicate lifecycle update leaves the item in its
  last valid state while also producing an explicit rejection message.
- **SC-003a**: Validation runs can substitute a test workflow definition file with different
  validation clauses and terminal-outcome rules without changing the coordinator's core role.
- **SC-003b**: Validation runs can demonstrate each supported validation outcome—advance, stay, and
  reject—using workflow-definition-driven behavior rather than coordinator-specific branching.
- **SC-004**: A single final summary is sufficient to identify the work item, its final stage,
  final status, assigned owner, and how many lifecycle transitions occurred.
