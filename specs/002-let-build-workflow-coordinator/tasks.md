# Tasks: Workflow Coordinator

**Input**: Design documents from `specs/002-let-build-workflow-coordinator/`
**Prerequisites**: `plan.md` (required), `spec.md` (required), `research.md`, `data-model.md`, `contracts/`

**Tests**: Tests and validation tasks are REQUIRED for this feature because it changes boot-time executable behavior, method-driven workflow logic, workflow definition loading, `complete(...)`-driven transition decisions, reporting, persistence-sensitive startup behavior, and completion-runtime dependency handling.

**Organization**: Tasks are grouped by user story so each story can be implemented, validated, and demonstrated independently.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no unmet dependencies)
- **[Story]**: Which user story this belongs to (`[US1]`, `[US2]`, `[US3]`)
- Every task includes exact repository paths
- Use `make` targets for all validation steps

## Phase 1: Setup (Shared Context)

**Purpose**: Establish baseline evidence and confirm the exact workflow, completion, and documentation files this feature must keep in sync.

- [x] T001 Run baseline validation for `methods/bootstrap_tests.c`, `modules/ar_executable_tests.c`, `modules/ar_file_delegate_tests.c`, `modules/ar_complete_instruction_evaluator_tests.c`, and `modules/ar_local_completion_tests.c` with `make bootstrap_tests 2>&1`, `make ar_executable_tests 2>&1`, `make ar_file_delegate_tests 2>&1`, `make ar_complete_instruction_evaluator_tests 2>&1`, and `make ar_local_completion_tests 2>&1`
- [x] T002 [P] Reconcile `complete(...)`-driven workflow scope across `specs/002-let-build-workflow-coordinator/spec.md`, `specs/002-let-build-workflow-coordinator/plan.md`, `specs/002-let-build-workflow-coordinator/research.md`, `specs/002-let-build-workflow-coordinator/data-model.md`, `specs/002-let-build-workflow-coordinator/contracts/workflow-definition-schema.md`, and `specs/002-let-build-workflow-coordinator/contracts/workflow-runtime-messages.md`
- [x] T003 [P] Confirm documentation and contract sync targets in `README.md`, `methods/README.md`, `methods/bootstrap.md`, `specs/002-let-build-workflow-coordinator/quickstart.md`, `specs/002-let-build-workflow-coordinator/contracts/README.md`, and `.specify/memory/pi-agent.md`

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Create the failing validation and shared scaffolding that every workflow story depends on.

**⚠️ CRITICAL**: No user story implementation starts before this phase is complete.

- [x] T004 Add failing boot-demo and executable assertions in `methods/bootstrap_tests.c` and `modules/ar_executable_tests.c` for staged definition preparation, startup dependency failure when local `complete(...)` is unavailable, and duplicate boot-trigger suppression
- [x] T005 [P] Add failing workflow-definition contract tests in `methods/workflow_definition_tests.c` for method-friendly workflow definition extraction, invalid-schema rejection, required `{outcome}` / `{reason}` templates, `prepare_definition`, `probe_dependencies`, `definition_ready`, and `definition_error`
- [x] T006 [P] Add failing workflow-item and workflow-reporter behavior tests in `methods/workflow_item_tests.c` and `methods/workflow_reporter_tests.c` for bundled-default terminal progression, retryable `stay`, reason propagation, startup dependency failures without fake work-item summary fields, fallback summaries, and reporting-error preservation
- [x] T007 [P] Add failing workflow-coordinator behavior tests in `methods/workflow_coordinator_tests.c` for waiting on definition readiness before item creation, alternate definition-path handoff, and startup-failure reporting
- [x] T008 [P] Create placeholder method assets and docs in `methods/workflow-coordinator-1.0.0.method`, `methods/workflow-coordinator-1.0.0.md`, `methods/workflow-item-1.0.0.method`, `methods/workflow-item-1.0.0.md`, `methods/workflow-reporter-1.0.0.method`, `methods/workflow-reporter-1.0.0.md`, `methods/workflow-definition-1.0.0.method`, and `methods/workflow-definition-1.0.0.md` so the RED phase fails on assertions instead of missing assets
- [x] T009 [P] Create placeholder parse-friendly workflow assets in `workflows/default.workflow` and `workflows/test.workflow` with explicit transition blocks and `complete(...)` decision templates using `{outcome}` and `{reason}`
- [x] T010 Confirm the RED phase for `methods/workflow_definition_tests.c`, `methods/workflow_item_tests.c`, `methods/workflow_coordinator_tests.c`, `methods/workflow_reporter_tests.c`, `methods/bootstrap_tests.c`, and `modules/ar_executable_tests.c` with `make workflow_definition_tests 2>&1`, `make workflow_item_tests 2>&1`, `make workflow_coordinator_tests 2>&1`, `make workflow_reporter_tests 2>&1`, `make bootstrap_tests 2>&1`, and `make ar_executable_tests 2>&1`

**Checkpoint**: Failing validation exists for workflow definition loading, startup dependency probing, `complete(...)` decision handling, workflow state progression, boot-time orchestration, and reporting.

---

## Phase 3: User Story 1 - Launch a Useful Demo Automatically (Priority: P1) 🎯 MVP

**Goal**: Deliver a fresh executable run that launches the workflow coordinator from `bootstrap`, validates the bundled workflow definition and required local `complete(...)` dependency, then either (a) starts exactly one demo item, evaluates the bundled default `complete(...)` transitions, and reaches a completed or rejected terminal outcome, or (b) emits a clear startup dependency failure before item creation.

**Independent Test**: From a clean runtime state, run `make workflow_definition_tests 2>&1`, `make workflow_item_tests 2>&1`, `make workflow_reporter_tests 2>&1`, `make bootstrap_tests 2>&1`, and `make ar_executable_tests 2>&1` to verify the boot flow launches the workflow demo automatically, reads `workflows/default.workflow`, waits for `definition_ready` before spawning the item, suppresses duplicate boot triggers, skips duplicate fresh setup when persisted agents exist, reaches a bundled default completed or rejected terminal outcome when the local `complete(...)` runtime/model is available, and reports startup dependency failure cleanly without fake work-item summary fields when that dependency is unavailable.

### Validation for User Story 1

- [x] T011 [P] [US1] Run the targeted failing boot/demo tests in `methods/bootstrap_tests.c`, `modules/ar_executable_tests.c`, `methods/workflow_coordinator_tests.c`, `methods/workflow_definition_tests.c`, `methods/workflow_item_tests.c`, and `methods/workflow_reporter_tests.c`

### Implementation for User Story 1

- [x] T012 [P] [US1] Author the bundled default workflow in `workflows/default.workflow` with parse-friendly metadata, `requires_local_completion`, explicit transition blocks, and decision templates containing `{outcome}` and `{reason}`
- [x] T013 [P] [US1] Implement `load_definition`, `probe_dependencies`, `describe`, bundled-default transition-template parsing, bundled-default `complete(...)` outcome/reason normalization, startup dependency failure mapping, and invalid-definition rejection in `methods/workflow-definition-1.0.0.method`, `methods/workflow-definition-1.0.0.md`, and `methods/workflow_definition_tests.c`
- [x] T014 [P] [US1] Implement staged launch, `prepare_definition` / `definition_ready` / `definition_error` handling, and same-runtime duplicate-start suppression in `methods/workflow-coordinator-1.0.0.method`, `methods/workflow-coordinator-1.0.0.md`, and `methods/workflow_coordinator_tests.c`
- [x] T015 [P] [US1] Implement startup item seeding, bundled-default terminal-path state progression, and successful-run summary handoff in `methods/workflow-item-1.0.0.method`, plus startup dependency failure reporting and no-item failure messaging in `methods/workflow-reporter-1.0.0.method`, with matching coverage in `methods/workflow_item_tests.c` and `methods/workflow_reporter_tests.c`
- [x] T016 [US1] Update `methods/bootstrap-1.0.0.method`, `methods/bootstrap.md`, `methods/bootstrap_tests.c`, and `modules/ar_executable_tests.c` to spawn `workflow-coordinator`, wait for readiness, ignore repeated boot-trigger messages in one runtime, skip duplicate fresh demo setup when restored workflow agents already exist, and assert startup dependency failures are reported without fake work-item summary fields
- [x] T017 [US1] Re-run `make workflow_definition_tests 2>&1`, `make workflow_item_tests 2>&1`, `make workflow_coordinator_tests 2>&1`, `make workflow_reporter_tests 2>&1`, `make bootstrap_tests 2>&1`, and `make ar_executable_tests 2>&1` until User Story 1 passes
- [x] T018 [US1] Refactor shared definition-preparation and startup-failure message helpers in `methods/workflow-definition-1.0.0.method`, `methods/workflow-coordinator-1.0.0.method`, and `methods/bootstrap-1.0.0.method` while preserving green User Story 1 tests

**Checkpoint**: A fresh boot run launches the workflow demo automatically, reaches a bundled-default terminal outcome in supported environments, handles startup dependency failures clearly without fake work-item summaries, and never creates duplicate boot-time workflow agents.

---

## Phase 4: User Story 2 - Track a Generic Work Item Through Stages (Priority: P2)

**Goal**: Extend the workflow item and workflow-definition logic beyond the MVP bundled-default path so they are reusable across multiple workflow definitions while preserving stable item identity/state and correctly handling `advance`, `stay`, `reject`, retryable `stay` on `complete(...)` failure, and invalid/duplicate/out-of-order updates.

**Independent Test**: Run `make workflow_definition_tests 2>&1`, `make workflow_item_tests 2>&1`, and `make workflow_coordinator_tests 2>&1` with deterministic `AGERUN_COMPLETE_RUNNER` responses so both `workflows/default.workflow` and `workflows/test.workflow` demonstrate `advance`, `stay`, `reject`, reason propagation, and retryable `stay` without corrupting item state.

### Validation for User Story 2

- [x] T019 [P] [US2] Run the targeted failing state and decision tests in `methods/workflow_definition_tests.c`, `methods/workflow_item_tests.c`, and `methods/workflow_coordinator_tests.c` for `advance`, `stay`, `reject`, retryable `stay`, and alternate workflow definitions
- [x] T020 [P] [US2] Add or confirm failing fake-runner-driven fixture coverage in `methods/workflow_definition_tests.c` and `methods/workflow_item_tests.c` for exact `outcome` / `reason` behavior using `AGERUN_COMPLETE_RUNNER`

### Implementation for User Story 2

- [x] T021 [P] [US2] Author deterministic alternate workflow scenarios in `workflows/test.workflow` with transition templates using `{outcome}` / `{reason}`, startup-probe variants, and explicit `advance` / `stay` / `reject` coverage
- [x] T022 [US2] Extend `methods/workflow-definition-1.0.0.method`, `methods/workflow-definition-1.0.0.md`, and `methods/workflow_definition_tests.c` to generalize transition-template parsing, `complete(...)` evaluation, generated `outcome` / `reason` normalization, and retryable-`stay` conversion across alternate workflow definitions beyond the bundled default path
- [x] T023 [US2] Extend `methods/workflow-item-1.0.0.method`, `methods/workflow-item-1.0.0.md`, and `methods/workflow_item_tests.c` for reusable stable item-state handling across alternate workflows, monotonic lifecycle enforcement, invalid/duplicate/out-of-order update rejection, retryable `stay`, and last-reason tracking
- [x] T024 [US2] Update `methods/workflow-coordinator-1.0.0.method` and `methods/workflow_coordinator_tests.c` for alternate definition-path handoff and deterministic fake-runner-driven evaluation flows
- [x] T025 [US2] Re-run `make workflow_definition_tests 2>&1`, `make workflow_item_tests 2>&1`, `make workflow_coordinator_tests 2>&1`, `make ar_complete_instruction_evaluator_tests 2>&1`, and `make ar_local_completion_tests 2>&1` until User Story 2 passes
- [x] T026 [US2] Refactor `complete(...)` decision parsing and retryable-`stay` helper flows in `methods/workflow-definition-1.0.0.method` and `methods/workflow-item-1.0.0.method` while preserving green User Story 2 tests

**Checkpoint**: The workflow item remains generic and stateful across workflow definitions, and `complete(...)`-driven transition outcomes advance the lifecycle without corrupting existing item state.

---

## Phase 5: User Story 3 - Review Progress and Final Outcome (Priority: P3)

**Goal**: Emit clear human-readable lifecycle checkpoints, retryable error messages, startup dependency failures, and final summaries so users can understand the workflow run from normal executable output and logs alone, including relevant reasons.

**Independent Test**: Run `make workflow_reporter_tests 2>&1`, `make workflow_item_tests 2>&1`, `make bootstrap_tests 2>&1`, and `make ar_executable_tests 2>&1` to verify the runtime emits at least four visible lifecycle checkpoints plus a reason-bearing final summary when a demo item exists, and also surfaces retryable `stay`, startup dependency failure, and fallback-summary cases correctly without fake work-item summary fields on pre-item failure.

### Validation for User Story 3

- [x] T027 [P] [US3] Run the targeted failing reporting/output tests in `methods/workflow_reporter_tests.c`, `methods/workflow_item_tests.c`, `methods/bootstrap_tests.c`, and `modules/ar_executable_tests.c` for reason-bearing progress, startup dependency failure without fake work-item summary fields, and fallback summaries

### Implementation for User Story 3

- [x] T028 [P] [US3] Extend human-readable progress, retryable error, startup failure, and final-summary output in `methods/workflow-reporter-1.0.0.method`, `methods/workflow-reporter-1.0.0.md`, and `methods/workflow_reporter_tests.c` so visible messages surface relevant `reason` values and startup dependency failures never pretend a work item exists
- [x] T029 [P] [US3] Extend progress-event and summary-event emission in `methods/workflow-item-1.0.0.method` and `methods/workflow_item_tests.c` to include reason text, transition counts, terminal outcomes, and fallback-summary data
- [x] T030 [US3] Update `methods/bootstrap-1.0.0.method`, `methods/bootstrap.md`, `methods/bootstrap_tests.c`, and `modules/ar_executable_tests.c` to assert visible reasons for progress, retryable `stay`, startup dependency failure without fake work-item summary fields, and final summaries for the boot demo
- [x] T031 [US3] Re-run `make workflow_reporter_tests 2>&1`, `make workflow_item_tests 2>&1`, `make bootstrap_tests 2>&1`, `make ar_executable_tests 2>&1`, and `make workflow_coordinator_tests 2>&1` until User Story 3 passes
- [x] T032 [US3] Refactor reporting text/build helpers in `methods/workflow-reporter-1.0.0.method`, `methods/workflow-item-1.0.0.method`, and `methods/bootstrap-1.0.0.method` while preserving green User Story 3 tests

**Checkpoint**: Users can understand the workflow run and any retryable/startup failures from emitted progress checkpoints and final summaries without inspecting internal memory.

---

## Final Phase: Polish & Cross-Cutting Concerns

**Purpose**: Finish documentation synchronization and repo-wide validation.

- [x] T033 [P] Update workflow user-facing documentation in `README.md`, `methods/README.md`, `methods/bootstrap.md`, and `specs/002-let-build-workflow-coordinator/quickstart.md` for `complete(...)`-driven workflow behavior and local dependency requirements
- [x] T034 [P] Update workflow method documentation in `methods/workflow-coordinator-1.0.0.md`, `methods/workflow-item-1.0.0.md`, `methods/workflow-reporter-1.0.0.md`, and `methods/workflow-definition-1.0.0.md` to match implemented startup probing, `complete(...)` templates, and reason-bearing output
- [x] T035 [P] Sync design artifacts in `specs/002-let-build-workflow-coordinator/contracts/README.md`, `specs/002-let-build-workflow-coordinator/contracts/workflow-definition-schema.md`, `specs/002-let-build-workflow-coordinator/contracts/workflow-runtime-messages.md`, `specs/002-let-build-workflow-coordinator/data-model.md`, `specs/002-let-build-workflow-coordinator/research.md`, and `.specify/memory/pi-agent.md` if implementation details changed the final runtime contract
- [x] T036 [P] Update workflow definition comments/examples in `workflows/default.workflow` and `workflows/test.workflow` to match the implemented parse-friendly schema, `complete(...)` decision templates, and deterministic fake-runner scenarios
- [x] T037 Run targeted workflow regression with `make workflow_definition_tests 2>&1`, `make workflow_item_tests 2>&1`, `make workflow_coordinator_tests 2>&1`, `make workflow_reporter_tests 2>&1`, `make bootstrap_tests 2>&1`, `make ar_executable_tests 2>&1`, `make ar_complete_instruction_evaluator_tests 2>&1`, and `make ar_local_completion_tests 2>&1`
- [x] T038 Run documentation validation for `README.md`, `methods/README.md`, `methods/bootstrap.md`, `methods/workflow-coordinator-1.0.0.md`, `methods/workflow-item-1.0.0.md`, `methods/workflow-reporter-1.0.0.md`, `methods/workflow-definition-1.0.0.md`, and `specs/002-let-build-workflow-coordinator/quickstart.md` with `make check-docs`
- [x] T039 Run sanitizer coverage for `methods/bootstrap_tests.c`, `methods/workflow_definition_tests.c`, `methods/workflow_item_tests.c`, `methods/workflow_coordinator_tests.c`, `methods/workflow_reporter_tests.c`, and `modules/ar_executable_tests.c` with `make sanitize-tests 2>&1`
- [x] T040 Run final repo gates for `methods/bootstrap-1.0.0.method`, `methods/workflow-coordinator-1.0.0.method`, `methods/workflow-item-1.0.0.method`, `methods/workflow-reporter-1.0.0.method`, `methods/workflow-definition-1.0.0.method`, `workflows/default.workflow`, and `workflows/test.workflow` with `make clean build 2>&1` and then `make check-logs`
- [x] T041 Update `CHANGELOG.md` for the workflow coordinator feature and confirm durable workflow context in `.specify/memory/pi-agent.md`

---

## Dependencies & Execution Order

### Phase Dependencies

- **Phase 1: Setup** starts immediately
- **Phase 2: Foundational** depends on Setup and establishes the RED phase for workflow definition loading, startup dependency probing, `complete(...)` decision handling, workflow state progression, and reporting
- **Phase 3: User Story 1** depends on Foundational completion
- **Phase 4: User Story 2** depends on User Story 1 because reusable transition evaluation requires the boot-time workflow path, staged definition readiness, and item lifecycle scaffolding to exist first
- **Phase 5: User Story 3** depends on User Story 2 because meaningful reason-bearing reporting depends on stable workflow progression and validated decision outcomes
- **Final Phase** depends on all desired stories being complete

### Within Each User Story

- Run the failing tests first
- Implement the minimal method/workflow definition changes that make the story pass
- Re-run targeted tests until green
- Refactor only after the story is green
- Synchronize docs/contracts after behavior stabilizes

### Parallel Opportunities

- **Setup**: `T002` and `T003` can run in parallel after `T001`
- **Foundational**: `T005`, `T006`, `T007`, `T008`, and `T009` can run in parallel after `T004`
- **User Story 1**: `T012`, `T013`, `T014`, and `T015` can run in parallel after `T011` exposes the failures and once `T012` establishes the bundled workflow definition shape
- **User Story 2**: `T021` can run in parallel with `T022` planning once `T019` and `T020` expose the deterministic failing scenarios
- **User Story 3**: `T028` and `T029` can run in parallel after `T027`
- **Polish**: `T033`, `T034`, `T035`, and `T036` can run in parallel after all story behavior is stable

---

## Implementation Strategy

### MVP First (User Story 1 Only)

1. Complete Setup
2. Complete Foundational RED-phase work
3. Deliver User Story 1 (`bootstrap` launches the workflow demo, the workflow definition is read through the file delegate, and the run either starts one item or fails clearly before item creation)
4. Validate User Story 1 independently with the targeted `make` tests
5. Stop if the fresh-run boot demo plus startup dependency handling is sufficient for the current milestone

### Incremental Delivery

1. Finish shared RED-phase and scaffolding once
2. Deliver User Story 1 for automatic boot-time launch and startup dependency handling
3. Deliver User Story 2 for reusable state tracking and `complete(...)`-driven transition outcomes
4. Deliver User Story 3 for user-visible lifecycle reporting and reason-bearing final summaries
5. Finish with documentation sync and full repository quality gates

---

## Notes

- Keep every RED phase at assertion level; use placeholder method/workflow assets only to avoid compilation failures
- Preserve the separation of concerns from `research.md`: boot orchestration in `bootstrap` and `workflow-coordinator`, reusable state in `workflow-item`, workflow definition-backed validation plus `complete(...)` prompting in `workflow-definition`, and output in `workflow-reporter`
- Use parse/build-friendly message contracts because AgeRun methods cannot use map literals or nested function calls
- Reuse the existing deterministic `AGERUN_COMPLETE_RUNNER` test pattern from `modules/ar_complete_instruction_evaluator_tests.c` and `modules/ar_instruction_evaluator_tests.c` instead of requiring the real local model/runtime in workflow tests
- Do not mark any task complete until the corresponding `make` output, file diff, or documentation update exists
