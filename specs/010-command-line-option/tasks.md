# Tasks: Executable Persistence Disable Option

**Input**: Design documents from `specs/010-command-line-option/`
**Prerequisites**: `plan.md` (required), `spec.md` (required), `research.md`, `data-model.md`, `contracts/`

**Tests**: Tests and validation tasks are REQUIRED for this feature because it changes executable CLI
behavior, startup persistence handling, shutdown persistence handling, operator-visible output, and
repository launch behavior.

**Organization**: Tasks are grouped by user story so each story can be implemented, validated, and
demonstrated independently.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no unmet dependencies)
- **[Story]**: Which user story this belongs to (`[US1]`, `[US2]`, `[US3]`)
- Include exact repository paths in every task
- Use `make` targets instead of direct compiler or binary execution

## Phase 1: Setup (Shared Context)

**Purpose**: Establish baseline evidence, confirm the planned contracts, and identify all files that
must remain synchronized.

- [x] T001 Run baseline executable coverage with `make ar_executable_tests 2>&1`
- [x] T002 [P] Reconcile scope across `specs/010-command-line-option/spec.md`, `specs/010-command-line-option/plan.md`, `specs/010-command-line-option/research.md`, `specs/010-command-line-option/data-model.md`, `specs/010-command-line-option/quickstart.md`, `specs/010-command-line-option/contracts/executable-persistence-cli.md`, and `specs/010-command-line-option/contracts/run-exec-persistence-mode.md`
- [x] T003 [P] Confirm implementation and documentation sync targets in `modules/ar_executable.c`, `modules/ar_executable.md`, `modules/ar_executable_fixture.c`, `modules/ar_executable_fixture.h`, `modules/ar_executable_tests.c`, `README.md`, `SPEC.md`, `.specify/memory/pi-agent.md`, and the top-level build file

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Create shared RED-phase evidence and fixture support that every user story depends on.

**⚠️ CRITICAL**: No user story implementation starts before this phase is complete.

- [x] T004 Add failing startup no-persistence coverage in `modules/ar_executable_tests.c` for skipped persisted methodology loading and skipped persisted agency loading
- [x] T005 [P] Add failing shutdown no-persistence coverage in `modules/ar_executable_tests.c` for skipped persisted methodology saving and skipped persisted agency saving
- [x] T006 [P] Add failing observability and regression coverage in `modules/ar_executable_tests.c` for explicit no-persistence output, unchanged default behavior, and no-persistence interaction with boot overrides
- [x] T007 [P] Extend shared executable test launch support in `modules/ar_executable_fixture.h` and `modules/ar_executable_fixture.c` so tests can request no-persistence runs through `make run-exec`
- [x] T008 Confirm the RED phase with `make ar_executable_tests 2>&1` and capture that failures come from the new no-persistence assertions

**Checkpoint**: Failing validation exists for skipped startup loads, skipped shutdown saves, and operator-visible no-persistence behavior.

---

## Phase 3: User Story 1 - Start without restored runtime state (Priority: P1) 🎯 MVP

**Goal**: Let operators start `agerun` in a fresh, non-persistent mode that ignores previously
saved methodology and agency state.

**Independent Test**: With persisted `agerun.methodology` and `agerun.agency` present, run
`make ar_executable_tests 2>&1` and verify the no-persistence path skips both restores and follows
fresh-start startup behavior.

### Validation for User Story 1

- [x] T009 [P] [US1] Run the targeted failing startup no-persistence tests in `modules/ar_executable_tests.c`

### Implementation for User Story 1

- [x] T010 [P] [US1] Implement `--no-persistence` CLI parsing and execution-mode selection in `modules/ar_executable.c`
- [x] T011 [US1] Gate persisted methodology loading and persisted agency loading on the no-persistence mode in `modules/ar_executable.c`
- [x] T012 [P] [US1] Add repository launch passthrough for no-persistence runs in the top-level build file so `make run-exec NO_PERSISTENCE=1` forwards the executable flag
- [x] T013 [US1] Update executable fixture launch helpers in `modules/ar_executable_fixture.h` and `modules/ar_executable_fixture.c` to pass the no-persistence mode through the existing `make run-exec` path
- [x] T014 [US1] Re-run `make ar_executable_tests 2>&1` until the startup no-persistence assertions in `modules/ar_executable_tests.c` pass
- [x] T015 [US1] Refactor shared startup-mode helpers in `modules/ar_executable.c` and `modules/ar_executable_fixture.c` while preserving green User Story 1 tests

**Checkpoint**: A no-persistence run skips persisted restores and behaves like a fresh startup session.

---

## Phase 4: User Story 2 - Exit without writing persistence artifacts (Priority: P2)

**Goal**: Prevent a no-persistence run from saving methodology or agency state at shutdown.

**Independent Test**: Run `make ar_executable_tests 2>&1` with no-persistence mode enabled and
verify shutdown completes without creating or updating persisted methodology and agency files.

### Validation for User Story 2

- [x] T016 [P] [US2] Run the targeted failing shutdown no-persistence tests in `modules/ar_executable_tests.c`

### Implementation for User Story 2

- [x] T017 [US2] Gate persisted methodology saving and persisted agency saving on the no-persistence mode in `modules/ar_executable.c`
- [x] T018 [P] [US2] Extend shutdown assertions and persisted-file fixture checks in `modules/ar_executable_tests.c` to verify untouched files and no-save behavior
- [x] T019 [US2] Re-run `make ar_executable_tests 2>&1` until the shutdown no-persistence assertions in `modules/ar_executable_tests.c` pass
- [x] T020 [US2] Refactor no-persistence shutdown branching in `modules/ar_executable.c` while preserving green User Story 2 tests

**Checkpoint**: A no-persistence run exits cleanly without writing persisted methodology or agency state.

---

## Phase 5: User Story 3 - Understand when persistence is disabled or unchanged (Priority: P3)

**Goal**: Make it obvious when no-persistence mode is active while preserving existing behavior when
the option is omitted.

**Independent Test**: Compare option-enabled and default runs through `make ar_executable_tests 2>&1`
and verify the output distinguishes disabled persistence from normal behavior, while default
persistence-enabled runs remain unchanged.

### Validation for User Story 3

- [x] T021 [P] [US3] Run the targeted failing observability and default-behavior tests in `modules/ar_executable_tests.c`

### Implementation for User Story 3

- [x] T022 [US3] Add operator-visible no-persistence mode reporting and default-mode messaging in `modules/ar_executable.c`
- [x] T023 [P] [US3] Implement and verify no-persistence plus boot-override interaction coverage in `modules/ar_executable_tests.c`
- [x] T024 [P] [US3] Update operator-facing documentation in `README.md`, `SPEC.md`, and `modules/ar_executable.md` for `--no-persistence`, `NO_PERSISTENCE=1`, untouched persisted files, and fresh-start semantics
- [x] T025 [P] [US3] Update planning and contract docs in `specs/010-command-line-option/quickstart.md`, `specs/010-command-line-option/contracts/executable-persistence-cli.md`, `specs/010-command-line-option/contracts/run-exec-persistence-mode.md`, and `specs/010-command-line-option/contracts/README.md`
- [x] T026 [US3] Re-run `make ar_executable_tests 2>&1` until the no-persistence observability, default-regression, and boot-override interaction assertions in `modules/ar_executable_tests.c` pass
- [x] T027 [US3] Refactor no-persistence status reporting in `modules/ar_executable.c` while preserving green User Story 3 tests

**Checkpoint**: Operators can clearly tell when persistence is disabled, and default runs remain unchanged.

---

## Final Phase: Polish & Cross-Cutting Concerns

**Purpose**: Complete repo-wide validation and synchronization work.

- [x] T028 [P] Sync final planning artifacts in `specs/010-command-line-option/plan.md`, `specs/010-command-line-option/research.md`, and `specs/010-command-line-option/data-model.md` if implementation details shift from the planned contract
- [x] T029 [P] Update durable workflow context in `.specify/memory/pi-agent.md` if the final executable and repository launch contract differs from the planned note
- [x] T030 [P] Run documentation validation with `make check-docs` for `README.md`, `SPEC.md`, `modules/ar_executable.md`, `specs/010-command-line-option/quickstart.md`, and `specs/010-command-line-option/contracts/`
- [x] T031 [P] Run naming validation with `make check-naming 2>&1` for `modules/ar_executable.c`, `modules/ar_executable_fixture.c`, and `modules/ar_executable_tests.c`
- [x] T032 [P] Run sanitizer coverage with `make sanitize-tests 2>&1` for executable startup/shutdown persistence behavior
- [x] T033 Run final targeted regression with `make ar_executable_tests 2>&1`
- [x] T034 Run repo gates with `make clean build 2>&1` and then `make check-logs`
- [x] T035 Update `CHANGELOG.md` with the executable no-persistence option once implementation is ready for commit

---

## Post-Analysis Consistency Fixes

**Purpose**: Resolve read-only `/spec analyze` findings for executable no-persistence documentation,
contracts, and regression coverage without changing the core feature scope.

- [x] T036 Add executable regression coverage in `modules/ar_executable_tests.c` for rejecting
  unknown CLI arguments before runtime initialization
- [x] T037 Update `specs/010-command-line-option/spec.md` and
  `specs/010-command-line-option/quickstart.md` to reflect implemented status and present-tense
  operator guidance
- [x] T038 Reconcile the `make run-exec` no-persistence contract wording across
  `specs/010-command-line-option/contracts/run-exec-persistence-mode.md`,
  `specs/010-command-line-option/research.md`, and
  `specs/010-command-line-option/data-model.md` with the actual Make passthrough behavior
- [x] T039 Update `.specify/memory/pi-agent.md` so the durable feature note matches the final
  no-persistence contract wording
- [x] T040 Re-run `make ar_executable_tests 2>&1`, `make check-docs 2>&1`,
  `make check-naming 2>&1`, `make clean build 2>&1`, and `make check-logs` after the consistency
  fixes

---

## Dependencies & Execution Order

### Phase Dependencies

- **Phase 1: Setup** starts immediately
- **Phase 2: Foundational** depends on Setup and establishes the RED phase for all no-persistence behaviors
- **Phase 3: User Story 1** depends on Foundational completion
- **Phase 4: User Story 2** depends on User Story 1 because shutdown gating uses the same no-persistence execution mode
- **Phase 5: User Story 3** depends on User Story 1 and User Story 2 because it documents and validates the complete no-persistence contract
- **Final Phase** depends on all desired user stories being complete

### Within Each User Story

- Run the failing story-specific tests first
- Implement the minimal change that makes the story pass
- Re-run targeted tests until green
- Refactor only after the story is green
- Update story-specific documentation after behavior stabilizes

### Parallel Opportunities

- **Setup**: `T002` and `T003` can run in parallel after `T001`
- **Foundational**: `T005`, `T006`, and `T007` can run in parallel after the scope review in Phase 1
- **User Story 1**: `T010`, `T012`, and `T013` can run in parallel before `T011` integration and `T014`
- **User Story 2**: `T017` and `T018` can run in parallel after `T016`
- **User Story 3**: `T023`, `T024`, and `T025` can run in parallel after `T021`
- **Polish**: `T028`, `T029`, `T030`, `T031`, and `T032` can run in parallel when behavior is stable

---

## Implementation Strategy

### MVP First (User Story 1 Only)

1. Complete Setup
2. Complete Foundational RED-phase work
3. Deliver User Story 1 (`--no-persistence` plus `make run-exec NO_PERSISTENCE=1`)
4. Validate User Story 1 independently
5. Stop if clean startup without persisted restores is sufficient for the current milestone

### Incremental Delivery

1. Finish shared RED-phase and fixture scaffolding once
2. Deliver User Story 1 for no-persistence startup behavior
3. Deliver User Story 2 for no-persistence shutdown behavior
4. Deliver User Story 3 for operator-visible output, default regression safety, and documentation
5. Finish with repo-wide validation and synchronization work

---

## Notes

- Keep every RED phase at assertion level; use fixture/helper scaffolding only to avoid compilation failures
- Preserve Make-only execution discipline in tests and validation tasks
- Treat no-persistence mode as a runtime behavior switch, not a file-deletion feature
- Do not mark any task complete until the corresponding `make` output, file diff, or documentation update exists
