# Tasks: Workflow Coordinator

**Input**: Design documents from `specs/002-let-build-workflow-coordinator/`
**Prerequisites**: `plan.md` (required), `spec.md` (required), `research.md`, `data-model.md`, `contracts/`

**Tests**: Tests and validation tasks are REQUIRED for this feature because it changes boot-time executable behavior, method-driven workflow logic, YAML definition loading, message protocols, and persistence-sensitive startup behavior.

**Organization**: Tasks are grouped by user story so each story can be implemented, validated, and demonstrated independently.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no unmet dependencies)
- **[Story]**: Which user story this belongs to (`[US1]`, `[US2]`, `[US3]`)
- Every task includes exact repository paths
- Use `make` targets for all validation steps

## Phase 1: Setup (Shared Context)

**Purpose**: Establish baseline evidence and confirm the exact files, YAML assets, and contracts this feature must keep in sync.

- [ ] T001 Run baseline validation for `methods/bootstrap_tests.c`, `modules/ar_executable_tests.c`, `methods/chat_session_tests.c`, and `modules/ar_file_delegate_tests.c` with `make bootstrap_tests 2>&1`, `make ar_executable_tests 2>&1`, `make chat_session_tests 2>&1`, and `make ar_file_delegate_tests 2>&1`
- [ ] T002 [P] Reconcile workflow scope across `specs/002-let-build-workflow-coordinator/spec.md`, `specs/002-let-build-workflow-coordinator/plan.md`, `specs/002-let-build-workflow-coordinator/research.md`, `specs/002-let-build-workflow-coordinator/data-model.md`, `specs/002-let-build-workflow-coordinator/contracts/workflow-definition-schema.md`, and `specs/002-let-build-workflow-coordinator/contracts/workflow-runtime-messages.md`
- [ ] T003 [P] Confirm documentation sync targets in `README.md`, `methods/README.md`, `methods/bootstrap.md`, `specs/002-let-build-workflow-coordinator/quickstart.md`, `specs/002-let-build-workflow-coordinator/contracts/README.md`, and `.specify/memory/pi-agent.md`

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Create failing validation and shared scaffolding required by every workflow story.

**⚠️ CRITICAL**: No user story implementation starts before this phase is complete.

- [ ] T004 Add failing boot-demo and executable assertions in `methods/bootstrap_tests.c` and `modules/ar_executable_tests.c` for launching `workflow-coordinator` from `bootstrap` and reading `workflows/default-workflow.yaml`
- [ ] T005 [P] Add failing workflow-definition and workflow-item contract tests in new `methods/workflow_definition_tests.c` and new `methods/workflow_item_tests.c`
- [ ] T006 [P] Add failing workflow-coordinator and workflow-reporter behavior tests in new `methods/workflow_coordinator_tests.c` and new `methods/workflow_reporter_tests.c`
- [ ] T007 [P] Create placeholder method assets and docs in `methods/workflow-coordinator-1.0.0.method`, `methods/workflow-coordinator-1.0.0.md`, `methods/workflow-item-1.0.0.method`, `methods/workflow-item-1.0.0.md`, `methods/workflow-reporter-1.0.0.method`, `methods/workflow-reporter-1.0.0.md`, `methods/workflow-definition-1.0.0.method`, `methods/workflow-definition-1.0.0.md`, `workflows/default-workflow.yaml`, and `workflows/test-workflow.yaml` so the RED phase fails on assertions instead of compilation or missing files
- [ ] T008 Confirm the RED phase for `methods/workflow_definition_tests.c`, `methods/workflow_item_tests.c`, `methods/workflow_coordinator_tests.c`, `methods/workflow_reporter_tests.c`, `methods/bootstrap_tests.c`, and `modules/ar_executable_tests.c` with `make workflow_definition_tests 2>&1`, `make workflow_item_tests 2>&1`, `make workflow_coordinator_tests 2>&1`, `make workflow_reporter_tests 2>&1`, `make bootstrap_tests 2>&1`, and `make ar_executable_tests 2>&1`

**Checkpoint**: Failing validation exists for YAML definition loading, workflow state progression, boot-time orchestration, and reporting.

---

## Phase 3: User Story 1 - Launch a Useful Demo Automatically (Priority: P1) 🎯 MVP

**Goal**: Deliver a fresh executable run that uses `bootstrap` to start the workflow coordinator, reads the bundled YAML workflow definition, processes exactly one demo item, and reaches a validation-driven terminal outcome without shell input.

**Independent Test**: Run `make run-exec 2>&1` or the equivalent executable tests from a clean runtime state and verify the boot flow launches the workflow demo automatically, reads `workflows/default-workflow.yaml`, progresses one demo item, and avoids duplicate fresh demo creation when persisted workflow agents already exist.

### Validation for User Story 1

- [ ] T009 [P] [US1] Run the targeted failing boot/demo tests in `methods/bootstrap_tests.c`, `modules/ar_executable_tests.c`, and `methods/workflow_coordinator_tests.c`

### Implementation for User Story 1

- [ ] T010 [P] [US1] Author the bundled happy-path YAML definition in `workflows/default-workflow.yaml`
- [ ] T011 [P] [US1] Implement YAML file loading, `describe`, and happy-path `evaluate_transition` behavior in `methods/workflow-definition-1.0.0.method`, `methods/workflow-definition-1.0.0.md`, and `methods/workflow_definition_tests.c`
- [ ] T012 [P] [US1] Implement boot-time orchestration in `methods/workflow-coordinator-1.0.0.method`, `methods/workflow-coordinator-1.0.0.md`, and `methods/workflow_coordinator_tests.c`
- [ ] T013 [P] [US1] Implement demo item initialization and happy-path progression in `methods/workflow-item-1.0.0.method`, `methods/workflow-item-1.0.0.md`, and `methods/workflow_item_tests.c`
- [ ] T014 [P] [US1] Implement minimal progress/final-summary logging in `methods/workflow-reporter-1.0.0.method`, `methods/workflow-reporter-1.0.0.md`, and `methods/workflow_reporter_tests.c`
- [ ] T015 [US1] Update `methods/bootstrap-1.0.0.method`, `methods/bootstrap.md`, and `methods/bootstrap_tests.c` to spawn `workflow-coordinator` with `workflows/default-workflow.yaml` and skip duplicate fresh demo setup when restored workflow agents already exist
- [ ] T016 [US1] Re-run `make workflow_definition_tests 2>&1`, `make workflow_item_tests 2>&1`, `make workflow_coordinator_tests 2>&1`, `make workflow_reporter_tests 2>&1`, `make bootstrap_tests 2>&1`, and `make ar_executable_tests 2>&1` until User Story 1 passes
- [ ] T017 [US1] Refactor shared parse/build message helpers in `methods/workflow-definition-1.0.0.method`, `methods/workflow-coordinator-1.0.0.method`, and `methods/workflow-item-1.0.0.method` while preserving green User Story 1 tests

**Checkpoint**: A fresh boot run launches the workflow demo automatically, reads the bundled YAML definition, and reaches one terminal outcome without shell usage.

---

## Phase 4: User Story 2 - Track a Generic Work Item Through Stages (Priority: P2)

**Goal**: Make the workflow item and generic workflow-definition logic reusable across multiple YAML workflow definitions, while preserving stable item identity/state and correctly handling `advance`, `stay`, `reject`, invalid, duplicate, and out-of-order updates.

**Independent Test**: Run the workflow-definition and workflow-item tests with both `workflows/default-workflow.yaml` and `workflows/test-workflow.yaml` and verify the item preserves identity, title, priority, owner, current stage, and status while validation outcomes drive `advance`, `stay`, and `reject` behavior correctly.

### Validation for User Story 2

- [ ] T018 [P] [US2] Run the targeted failing state and validation tests in `methods/workflow_definition_tests.c`, `methods/workflow_item_tests.c`, and `methods/workflow_coordinator_tests.c`

### Implementation for User Story 2

- [ ] T019 [P] [US2] Author alternate validation coverage for `advance`, `stay`, and `reject` in `workflows/test-workflow.yaml`
- [ ] T020 [US2] Extend YAML parsing, transition-attached validation-clause evaluation, and `advance` / `stay` / `reject` decision output in `methods/workflow-definition-1.0.0.method`, `methods/workflow-definition-1.0.0.md`, and `methods/workflow_definition_tests.c`
- [ ] T021 [US2] Extend stable item-state handling, monotonic lifecycle enforcement, and invalid/duplicate/out-of-order update rejection in `methods/workflow-item-1.0.0.method`, `methods/workflow-item-1.0.0.md`, and `methods/workflow_item_tests.c`
- [ ] T022 [US2] Update definition-path handoff and item seed-field wiring in `methods/workflow-coordinator-1.0.0.method` and `methods/workflow_coordinator_tests.c` for alternate YAML workflow definitions
- [ ] T023 [US2] Re-run `make workflow_definition_tests 2>&1`, `make workflow_item_tests 2>&1`, and `make workflow_coordinator_tests 2>&1` until User Story 2 passes
- [ ] T024 [US2] Refactor YAML-read and transition-decision message shapes in `methods/workflow-definition-1.0.0.method` and `methods/workflow-item-1.0.0.method` while preserving green User Story 2 tests

**Checkpoint**: The workflow item remains generic and stateful across YAML definitions, and validation outcomes drive stage progression without corrupting existing item state.

---

## Phase 5: User Story 3 - Review Progress and Final Outcome (Priority: P3)

**Goal**: Emit clear human-readable lifecycle checkpoints and final summaries so users can understand the workflow run from normal executable output and logs alone.

**Independent Test**: Run the reporter, bootstrap, and executable tests and verify the runtime emits at least four visible lifecycle checkpoints plus a final summary that identifies the item, final stage, final status, assigned owner, terminal outcome, and transition count.

### Validation for User Story 3

- [ ] T025 [P] [US3] Run the targeted failing reporting/output tests in `methods/workflow_reporter_tests.c`, `methods/bootstrap_tests.c`, and `modules/ar_executable_tests.c`

### Implementation for User Story 3

- [ ] T026 [P] [US3] Extend human-readable progress and final-summary output in `methods/workflow-reporter-1.0.0.method`, `methods/workflow-reporter-1.0.0.md`, and `methods/workflow_reporter_tests.c`
- [ ] T027 [P] [US3] Extend progress-event and summary-event emission with transition counts and terminal outcomes in `methods/workflow-item-1.0.0.method` and `methods/workflow_item_tests.c`
- [ ] T028 [US3] Update `methods/bootstrap-1.0.0.method`, `methods/bootstrap.md`, `methods/bootstrap_tests.c`, and `modules/ar_executable_tests.c` to assert the new workflow progress output and final summary content for the boot demo
- [ ] T029 [US3] Re-run `make workflow_reporter_tests 2>&1`, `make workflow_item_tests 2>&1`, `make bootstrap_tests 2>&1`, and `make ar_executable_tests 2>&1` until User Story 3 passes
- [ ] T030 [US3] Refactor reporting text/build helpers in `methods/workflow-reporter-1.0.0.method` and `methods/workflow-item-1.0.0.method` while preserving green User Story 3 tests

**Checkpoint**: Users can understand the workflow run from the emitted progress checkpoints and final summary without inspecting internal memory.

---

## Final Phase: Polish & Cross-Cutting Concerns

**Purpose**: Finish documentation synchronization and repo-wide validation.

- [ ] T031 [P] Update workflow user-facing documentation in `README.md`, `methods/README.md`, `methods/bootstrap.md`, and `specs/002-let-build-workflow-coordinator/quickstart.md`
- [ ] T032 [P] Update workflow method documentation in `methods/workflow-coordinator-1.0.0.md`, `methods/workflow-item-1.0.0.md`, `methods/workflow-reporter-1.0.0.md`, and `methods/workflow-definition-1.0.0.md`
- [ ] T033 [P] Sync design artifacts in `specs/002-let-build-workflow-coordinator/contracts/README.md`, `specs/002-let-build-workflow-coordinator/contracts/workflow-definition-schema.md`, `specs/002-let-build-workflow-coordinator/contracts/workflow-runtime-messages.md`, `specs/002-let-build-workflow-coordinator/data-model.md`, and `.specify/memory/pi-agent.md` if implementation details changed the final runtime contract
- [ ] T034 [P] Update YAML definition comments/examples in `workflows/default-workflow.yaml` and `workflows/test-workflow.yaml` to match the implemented schema and test scenarios
- [ ] T035 Run targeted workflow regression with `make workflow_definition_tests 2>&1`, `make workflow_item_tests 2>&1`, `make workflow_coordinator_tests 2>&1`, `make workflow_reporter_tests 2>&1`, `make bootstrap_tests 2>&1`, and `make ar_executable_tests 2>&1`
- [ ] T036 Run documentation validation for `README.md`, `methods/README.md`, `methods/bootstrap.md`, `methods/workflow-coordinator-1.0.0.md`, `methods/workflow-item-1.0.0.md`, `methods/workflow-reporter-1.0.0.md`, `methods/workflow-definition-1.0.0.md`, and `specs/002-let-build-workflow-coordinator/quickstart.md` with `make check-docs`
- [ ] T037 Run sanitizer coverage for `methods/bootstrap_tests.c`, `methods/workflow_definition_tests.c`, `methods/workflow_item_tests.c`, `methods/workflow_coordinator_tests.c`, `methods/workflow_reporter_tests.c`, and `modules/ar_executable_tests.c` with `make sanitize-tests 2>&1`
- [ ] T038 Run final repo gates for `methods/bootstrap-1.0.0.method`, `methods/workflow-coordinator-1.0.0.method`, `methods/workflow-item-1.0.0.method`, `methods/workflow-reporter-1.0.0.method`, `methods/workflow-definition-1.0.0.method`, and `workflows/default-workflow.yaml` with `make clean build 2>&1` and then `make check-logs`
- [ ] T039 Update `CHANGELOG.md` for the workflow coordinator feature and confirm durable workflow context in `.specify/memory/pi-agent.md`

---

## Dependencies & Execution Order

### Phase Dependencies

- **Phase 1: Setup** starts immediately
- **Phase 2: Foundational** depends on Setup and establishes the RED phase for YAML definition loading, workflow state progression, boot-time orchestration, and reporting
- **Phase 3: User Story 1** depends on Foundational completion
- **Phase 4: User Story 2** depends on User Story 1 because reusable stage validation requires the boot-time workflow path, generic definition reader, and item lifecycle scaffolding to exist first
- **Phase 5: User Story 3** depends on User Story 2 because meaningful reporting depends on stable workflow progression and transition outcomes
- **Final Phase** depends on all desired stories being complete

### Within Each User Story

- Run the failing tests first
- Implement the minimal method/YAML changes that make the story pass
- Re-run targeted tests until green
- Refactor only after the story is green
- Synchronize docs/contracts after behavior stabilizes

### Parallel Opportunities

- **Setup**: `T002` and `T003` can run in parallel after `T001`
- **Foundational**: `T005`, `T006`, and `T007` can run in parallel after `T004`
- **User Story 1**: `T010`, `T011`, `T012`, `T013`, and `T014` can run in parallel after `T009` exposes the failures
- **User Story 2**: `T019` can run in parallel with `T020` planning once `T018` exposes the failing scenarios
- **User Story 3**: `T026` and `T027` can run in parallel after `T025`
- **Polish**: `T031`, `T032`, `T033`, and `T034` can run in parallel after all story behavior is stable

---

## Implementation Strategy

### MVP First (User Story 1 Only)

1. Complete Setup
2. Complete Foundational RED-phase work
3. Deliver User Story 1 (`bootstrap` launches the workflow demo, the definition YAML is read through the file delegate, and the demo reaches one terminal outcome)
4. Validate User Story 1 independently with the targeted `make` tests
5. Stop if the fresh-run boot demo is sufficient for the current milestone

### Incremental Delivery

1. Finish shared RED-phase and scaffolding once
2. Deliver User Story 1 for automatic boot-time launch
3. Deliver User Story 2 for reusable state tracking and alternate YAML validation outcomes
4. Deliver User Story 3 for user-visible lifecycle reporting and final summaries
5. Finish with documentation sync and full repository quality gates

---

## Notes

- Keep every RED phase at assertion level; use placeholder method/YAML assets only to avoid compilation failures
- Preserve the separation of concerns from `research.md`: boot orchestration in `bootstrap` and `workflow-coordinator`, reusable state in `workflow-item`, YAML-backed validation in `workflow-definition`, and output in `workflow-reporter`
- Use parse/build-friendly message contracts because AgeRun methods cannot use map literals or nested function calls
- Do not mark any task complete until the corresponding `make` output, file diff, or documentation update exists
