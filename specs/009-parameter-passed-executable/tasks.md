# Tasks: Executable Boot Method Override

**Input**: Design documents from `specs/009-parameter-passed-executable/`
**Prerequisites**: `plan.md` (required), `spec.md` (required), `research.md`, `data-model.md`, `contracts/`

**Tests**: Tests and validation tasks are REQUIRED for this feature because it changes executable
startup behavior, CLI input handling, Makefile launch forwarding, persistence-aware boot selection,
and operator-visible documentation.

**Organization**: Tasks are grouped by user story so each story can be implemented, validated, and
demonstrated independently.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no unmet dependencies)
- **[Story]**: Which user story this belongs to (`[US1]`, `[US2]`, `[US3]`)
- Include exact repository paths in every task
- Use `make` targets instead of direct compiler or binary execution

## Phase 1: Setup (Shared Context)

**Purpose**: Establish baseline evidence, confirm the exact CLI/Make contract, and record all files
that must stay synchronized.

- [x] T001 Run baseline validation for `modules/ar_executable_tests.c` with `make ar_executable_tests 2>&1`
- [x] T002 [P] Reconcile feature scope across `specs/009-parameter-passed-executable/spec.md`, `specs/009-parameter-passed-executable/plan.md`, `specs/009-parameter-passed-executable/research.md`, `specs/009-parameter-passed-executable/data-model.md`, `specs/009-parameter-passed-executable/quickstart.md`, `specs/009-parameter-passed-executable/contracts/executable-startup-cli.md`, and `specs/009-parameter-passed-executable/contracts/run-exec-target.md`
- [x] T003 [P] Confirm documentation and implementation sync targets in the top-level build file, `modules/ar_executable.c`, `modules/ar_executable.h`, `modules/ar_executable_fixture.c`, `modules/ar_executable_fixture.h`, `modules/ar_executable.md`, `README.md`, `SPEC.md`, and `.specify/memory/pi-agent.md`

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Create the RED-phase evidence and helper scaffolding shared by all startup-override
stories.

**⚠️ CRITICAL**: No user story implementation starts before this phase is complete.

- [x] T004 Add failing fresh-start override coverage in `modules/ar_executable_tests.c` for valid `--boot-method <name-version>` selection and standard `"__boot__"` queueing
- [x] T005 [P] Add failing no-override regression coverage in `modules/ar_executable_tests.c` for default `bootstrap` startup and unchanged `make run-exec` behavior
- [x] T006 [P] Add failing invalid-override and restored-startup skip coverage in `modules/ar_executable_tests.c` for malformed identifiers, unavailable methods, and observable override skipping
- [x] T007 [P] Add compile-safe override helper scaffolding in `modules/ar_executable.h`, `modules/ar_executable.c`, `modules/ar_executable_fixture.h`, and `modules/ar_executable_fixture.c` so RED fails on assertions instead of compilation
- [x] T008 Confirm the RED phase for `modules/ar_executable_tests.c`, `modules/ar_executable.c`, and `modules/ar_executable_fixture.c` with `make ar_executable_tests 2>&1`

**Checkpoint**: Failing validation exists for valid override selection, default-path preservation, and invalid/skip behavior.

---

## Phase 3: User Story 1 - Start with an alternate boot method (Priority: P1) 🎯 MVP

**Goal**: Allow a fresh `agerun` runtime to start with a requested boot method identifier such as
`echo-1.0.0` instead of the default `bootstrap` method.

**Independent Test**: Start the executable in a clean runtime state with `BOOT_METHOD=echo-1.0.0`
and verify through `make ar_executable_tests 2>&1` that the first created agent uses the requested
method and still receives the normal `"__boot__"` startup message.

### Validation for User Story 1

- [x] T009 [P] [US1] Run the targeted failing fresh-start override tests in `modules/ar_executable_tests.c` for `BOOT_METHOD=echo-1.0.0` selection and `"__boot__"` queueing

### Implementation for User Story 1

- [x] T010 [P] [US1] Implement override-aware launch helpers in `modules/ar_executable_fixture.h` and `modules/ar_executable_fixture.c` so tests can request `BOOT_METHOD=<name-version>` through `make run-exec`
- [x] T011 [P] [US1] Update `run-exec` argument forwarding in the top-level build file so `BOOT_METHOD=<name-version>` expands to `./agerun --boot-method <name-version>`
- [x] T012 [US1] Implement `--boot-method <name-version>` parsing, final-hyphen splitting, and fresh-start boot selection in `modules/ar_executable.c` and `modules/ar_executable.h`
- [x] T013 [US1] Re-run `make ar_executable_tests 2>&1` until the valid-override assertions in `modules/ar_executable_tests.c` pass
- [x] T014 [US1] Refactor shared boot-selection helpers in `modules/ar_executable.c` and `modules/ar_executable_fixture.c` while preserving green User Story 1 tests

**Checkpoint**: A fresh runtime can start from an alternate boot method identified with the `methods/` naming convention.

---

## Phase 4: User Story 2 - Preserve default startup when no override is provided (Priority: P2)

**Goal**: Keep the existing bootstrap startup path unchanged when no override parameter is supplied.

**Independent Test**: Start the executable in a clean runtime state without `BOOT_METHOD` and
verify through `make ar_executable_tests 2>&1` that it still creates the default `bootstrap` agent
and runs the current startup flow.

### Validation for User Story 2

- [x] T015 [P] [US2] Run the targeted failing no-override regression tests in `modules/ar_executable_tests.c` for default `bootstrap` startup and unchanged `make run-exec` behavior

### Implementation for User Story 2

- [x] T016 [US2] Preserve the no-override default branch and unchanged bootstrap selection/output in `modules/ar_executable.c`
- [x] T017 [P] [US2] Update optional-override usage/help text in the top-level build file, `README.md`, and `modules/ar_executable.md` so the default path remains explicit and additive
- [x] T018 [US2] Re-run `make ar_executable_tests 2>&1` until the no-override default-start assertions in `modules/ar_executable_tests.c` pass
- [x] T019 [US2] Refactor default-branch selection/output handling in `modules/ar_executable.c` while preserving green User Story 2 tests

**Checkpoint**: Existing users who do not supply an override still get the current bootstrap startup behavior.

---

## Phase 5: User Story 3 - Get clear behavior for invalid or non-applicable overrides (Priority: P3)

**Goal**: Make invalid override requests fail clearly and make restored-agent startup explicitly skip
fresh boot creation even when an override was requested.

**Independent Test**: Verify through `make ar_executable_tests 2>&1` that malformed or unavailable
boot identifiers fail without fallback and that restored-agent startup reports the override as
skipped instead of creating a second fresh boot agent.

### Validation for User Story 3

- [x] T020 [P] [US3] Run the targeted failing invalid-override and restored-startup tests in `modules/ar_executable_tests.c` for malformed identifiers, unavailable methods, and skip observability

### Implementation for User Story 3

- [x] T021 [P] [US3] Implement invalid-identifier rejection and no-fallback startup failure reporting in `modules/ar_executable.c` and `modules/ar_executable.h`
- [x] T022 [P] [US3] Implement restored-startup override passthrough helpers in `modules/ar_executable_fixture.h` and `modules/ar_executable_fixture.c`
- [x] T023 [US3] Implement restored-agent override-skip messaging and failure-path outcome reporting in `modules/ar_executable.c`
- [x] T024 [P] [US3] Update failure and restored-state documentation in `README.md`, `SPEC.md`, `modules/ar_executable.md`, `specs/009-parameter-passed-executable/quickstart.md`, `specs/009-parameter-passed-executable/contracts/executable-startup-cli.md`, and `specs/009-parameter-passed-executable/contracts/run-exec-target.md`
- [x] T025 [US3] Re-run `make ar_executable_tests 2>&1` until malformed-override, unavailable-method, and restored-state assertions in `modules/ar_executable_tests.c` pass
- [x] T026 [US3] Refactor override-validation and startup-reporting helpers in `modules/ar_executable.c` and `modules/ar_executable_fixture.c` while preserving green User Story 3 tests

**Checkpoint**: Invalid overrides fail clearly, and restored persisted agents take precedence with explicit operator-visible messaging.

---

## Final Phase: Polish & Cross-Cutting Concerns

**Purpose**: Finish plan-artifact synchronization and run repo-wide quality gates.

- [ ] T027 [P] Sync final planning artifacts in `specs/009-parameter-passed-executable/plan.md`, `specs/009-parameter-passed-executable/research.md`, `specs/009-parameter-passed-executable/data-model.md`, and `specs/009-parameter-passed-executable/contracts/README.md` if implementation details move from the planned contract
- [ ] T028 [P] Update durable workflow context in `.specify/memory/pi-agent.md` if the final executable/Make contract differs from the current planned note
- [x] T029 [P] Run documentation validation for `README.md`, `SPEC.md`, `modules/ar_executable.md`, `specs/009-parameter-passed-executable/quickstart.md`, `specs/009-parameter-passed-executable/contracts/executable-startup-cli.md`, and `specs/009-parameter-passed-executable/contracts/run-exec-target.md` with `make check-docs`
- [x] T030 [P] Run sanitizer coverage for `modules/ar_executable.c`, `modules/ar_executable_fixture.c`, and `modules/ar_executable_tests.c` with `make sanitize-tests 2>&1`
- [x] T031 Run final targeted regression for `modules/ar_executable_tests.c` with `make ar_executable_tests 2>&1`
- [x] T032 Run repo gates for the top-level build file, `modules/ar_executable.c`, `modules/ar_executable_fixture.c`, `README.md`, and `SPEC.md` with `make clean build 2>&1` and then `make check-logs`
- [x] T033 Update `CHANGELOG.md` with the executable boot-method override and `make run-exec` passthrough once implementation is ready for commit

---

## Dependencies & Execution Order

### Phase Dependencies

- **Phase 1: Setup** starts immediately
- **Phase 2: Foundational** depends on Setup and establishes the RED phase for all startup-override behaviors
- **Phase 3: User Story 1** depends on Foundational completion
- **Phase 4: User Story 2** depends on User Story 1 because the default path must be preserved after the override path exists
- **Phase 5: User Story 3** depends on User Story 1 because invalid and skip behavior builds on the same boot-selection path; it may begin after User Story 1 even if User Story 2 docs are still in progress when file overlap permits
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
- **User Story 1**: `T010` and `T011` can run in parallel before `T012`
- **User Story 2**: `T017` can run in parallel with `T016` once the no-override branch is understood
- **User Story 3**: `T021`, `T022`, and `T024` can run in parallel after `T020`
- **Polish**: `T027`, `T028`, `T029`, and `T030` can run in parallel when behavior is stable

---

## Implementation Strategy

### MVP First (User Story 1 Only)

1. Complete Setup
2. Complete Foundational RED-phase work
3. Deliver User Story 1 (`--boot-method <name-version>` plus `make run-exec BOOT_METHOD=...`)
4. Validate User Story 1 independently with `make ar_executable_tests 2>&1`
5. Stop if alternate fresh-start boot selection is sufficient for the current milestone

### Incremental Delivery

1. Finish shared RED-phase and helper scaffolding once
2. Deliver User Story 1 for alternate boot selection
3. Deliver User Story 2 for default-path preservation and additive usage text
4. Deliver User Story 3 for invalid-input failure handling and restored-state skip messaging
5. Finish with documentation sync, sanitizer coverage, and repo-wide build/log validation

---

## Notes

- Keep every RED phase at assertion level; use helper scaffolding only to avoid compilation failures
- Preserve Make-only execution discipline in tests and validation tasks
- Keep the external override contract aligned with the `methods/` filename-stem convention `<method-name>-<version>`
- Do not mark any task complete until the corresponding `make` output, file diff, or documentation update exists
