# Feature Specification: Executable Persistence Disable Option

**Feature Branch**: `010-command-line-option`
**Created**: 2026-04-22
**Status**: Implemented
**Input**: User description: "command line option in executable to disable loading/saving of methology/agency"

## User Scenarios & Testing *(mandatory)*

### User Story 1 - Start without restored runtime state (Priority: P1)

A runtime operator can launch the main AgeRun executable in a mode that ignores previously persisted
methods and agents so the session starts fresh even when persisted runtime files already exist.

**Why this priority**: The primary user value is being able to run a clean session on demand without
manually deleting persisted files or altering the normal runtime configuration.

**Independent Test**: Launch the executable with persisted method and agent state already present and
verify that startup does not restore either persisted store and instead follows the fresh-start boot
path.

**Acceptance Scenarios**:

1. **Given** persisted method state and persisted agent state already exist, **When** the operator
   starts the executable with the persistence-disable option, **Then** the runtime does not restore
   either persisted store during startup.
2. **Given** persisted runtime state already exists, **When** the operator starts the executable with
   the persistence-disable option, **Then** startup behaves like a fresh session instead of a
   restored session.

---

### User Story 2 - Exit without writing persistence artifacts (Priority: P2)

A runtime operator can use the same mode to prevent the executable from saving method state or agent
state back to disk when the run finishes.

**Why this priority**: A clean run is incomplete if shutdown writes new persisted state and silently
changes the next startup. The operator needs a fully non-persistent session for testing and
experimentation.

**Independent Test**: Run the executable with the persistence-disable option, allow startup and
shutdown to complete, and verify that the run does not create or update persisted method or agent
state.

**Acceptance Scenarios**:

1. **Given** the executable is running with the persistence-disable option, **When** shutdown
   completes, **Then** the runtime does not save persisted method state for that run.
2. **Given** the executable is running with the persistence-disable option, **When** shutdown
   completes, **Then** the runtime does not save persisted agent state for that run.

---

### User Story 3 - Understand when persistence is disabled or unchanged (Priority: P3)

A runtime operator can tell from executable output whether persistence was intentionally disabled or
whether the executable is still using the normal load/save behavior.

**Why this priority**: Operators need confidence about which startup mode they actually used,
especially when troubleshooting interactions between fresh runs, restored state, and other startup
options.

**Independent Test**: Compare one executable launch with the persistence-disable option and one
without it, confirming that the observed output clearly distinguishes disabled persistence from the
default behavior.

**Acceptance Scenarios**:

1. **Given** the operator starts the executable with the persistence-disable option, **When** startup
   begins, **Then** the executable reports that persisted state loading and saving are disabled for
   the current run.
2. **Given** the operator starts the executable without the persistence-disable option, **When** the
   executable runs normally, **Then** the current default persistence behavior remains unchanged.

## Edge Cases *(mandatory)*

- If persisted method state, persisted agent state, or both exist before startup, the
  persistence-disable option causes the executable to skip loading all of them for that run.
- If persisted files are missing, invalid, or corrupted, enabling the persistence-disable option
  suppresses those load paths instead of surfacing load-related errors for ignored files.
- If the persistence-disable option is used together with other startup options that apply only to
  fresh sessions, the executable evaluates those options within the fresh-start path rather than the
  restored-state path.
- If persisted files already exist before a non-persistent run, the executable leaves those files in
  place and does not treat them as newly restored or newly rewritten state.

## Scope Boundaries *(mandatory)*

### In Scope

- Adding an optional startup control to the main AgeRun executable that disables automatic loading
  and saving of persisted method state and persisted agent state for a single run
- Treating an option-enabled launch as a fresh runtime startup even when persisted runtime files are
  present
- Making the disabled-persistence mode observable in executable output
- Preserving existing executable behavior when the option is not used
- Updating executable-facing documentation to explain the new behavior and its interaction with
  persisted state

### Out of Scope

- Changing the persistence file formats or persistence data model
- Removing persistence support from the AgeRun runtime entirely
- Adding separate partial controls for methods-only or agents-only persistence
- Changing persistence behavior for executables other than the main `agerun` runtime
- Altering how persisted state is produced or consumed when the option is not used

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: The main AgeRun executable MUST support an optional startup parameter that disables
  automatic persistence loading and saving for the current run.
- **FR-002**: When the persistence-disable option is enabled, the executable MUST NOT load persisted
  method state during startup.
- **FR-003**: When the persistence-disable option is enabled, the executable MUST NOT load persisted
  agent state during startup.
- **FR-004**: When the persistence-disable option is enabled, the executable MUST follow the
  fresh-start startup path for the current run rather than the restored-state path.
- **FR-005**: When the persistence-disable option is enabled, the executable MUST NOT save persisted
  method state during shutdown.
- **FR-006**: When the persistence-disable option is enabled, the executable MUST NOT save persisted
  agent state during shutdown.
- **FR-007**: When the persistence-disable option is enabled, the executable MUST make it observable
  to the operator that persisted loading and saving were skipped for the current run.
- **FR-008**: The persistence-disable option MUST apply consistently whether persisted method state,
  persisted agent state, both, or neither are present before startup.
- **FR-009**: Existing executable behavior without the persistence-disable option MUST remain
  unchanged.
- **FR-010**: Documentation for executable startup behavior MUST describe the option, its fresh-run
  semantics, and its effect on persisted method and agent state.

### Key Entities *(include if feature involves data)*

- **Persistence-Disabled Run**: A single executable session in which the operator has requested that
  persisted method state and persisted agent state be ignored on startup and not written on
  shutdown.
- **Persisted Method State**: Previously saved runtime method information that the executable would
  normally restore and save across sessions.
- **Persisted Agent State**: Previously saved runtime agent information that the executable would
  normally restore and save across sessions.
- **Fresh-Start Runtime Path**: The executable startup path used when no persisted agent state is
  restored and a new boot flow is created for the session.

## Assumptions & Dependencies *(mandatory)*

- **Assumptions**:
  - A single executable option disables both persisted method handling and persisted agent handling
    for the run.
  - The option applies only to the main `agerun` executable, not to other binaries or lower-level
    APIs.
  - Persisted files that already exist before the run are left untouched rather than deleted.
  - Operators use this mode primarily for clean testing, temporary experiments, and startup control
    without editing or removing runtime files.
- **External Dependencies**:
  - Main executable startup and shutdown flow
  - Existing persisted method and agent state handling
  - Executable-facing user documentation
- **Open Questions**: None

## Documentation & Compatibility Impact *(mandatory)*

- **Affected Documentation**: `README.md`, `SPEC.md`, `modules/ar_executable.md`, and executable
  startup/behavior documentation that currently describes automatic loading and saving of persisted
  state.
- **Affected Runtime Contracts**: Main executable CLI behavior, fresh-start versus restored-start
  behavior, and operator-visible persistence semantics.
- **Compatibility Notes**: Backward-compatible additive change. Existing executable launches without
  the option continue to use the current persistence behavior.

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: In executable startup tests with persisted method and agent state present, `100%` of
  runs that use the persistence-disable option start without restoring persisted state.
- **SC-002**: In executable shutdown tests, `100%` of runs that use the persistence-disable option
  complete without writing new persisted method or agent state for that run.
- **SC-003**: In regression tests without the new option, `100%` of launches preserve the current
  persistence-enabled behavior.
- **SC-004**: In operator-visible output checks, `100%` of option-enabled runs clearly indicate that
  persistence loading and saving were disabled for the current session.
