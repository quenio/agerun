# Tasks: Command-Line Shell

**Input**: Design documents from `specs/001-command-line-shell/`
**Prerequisites**: `plan.md` (required), `spec.md` (required), `research.md`, `data-model.md`, `contracts/`

**Tests**: Tests and validation tasks are REQUIRED for this feature because it changes executable behavior, runtime wiring, parsing, messaging, and ownership-sensitive session state.

**Organization**: Tasks are grouped by user story so each story can be implemented, validated, and demonstrated independently.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no unmet dependencies)
- **[Story]**: Which user story this task belongs to (`[US1]`, `[US2]`, `[US3]`)
- Every task includes exact repository paths
- Use `make` targets for all validation steps

## Phase 1: Setup (Shared Context)

**Purpose**: Establish baseline evidence and confirm the exact files/contracts this feature must keep in sync.

- [ ] T001 Run baseline validation for `modules/ar_system_tests.c`, `modules/ar_methodology_tests.c`, `modules/ar_delegate_tests.c`, and `methods/chat_session_tests.c` with `make ar_system_tests 2>&1`, `make ar_methodology_tests 2>&1`, `make ar_delegate_tests 2>&1`, and `make chat_session_tests 2>&1`
- [ ] T002 [P] Reconcile feature scope across `specs/001-command-line-shell/spec.md`, `specs/001-command-line-shell/plan.md`, `specs/001-command-line-shell/research.md`, `specs/001-command-line-shell/data-model.md`, `specs/001-command-line-shell/contracts/arsh-cli.md`, and `specs/001-command-line-shell/contracts/shell-session-protocol.md`
- [ ] T003 [P] Confirm documentation sync targets in `README.md`, `SPEC.md`, `modules/ar_shell.md`, `modules/ar_shell_session.md`, `modules/ar_shell_delegate.md`, `.specify/memory/pi-agent.md`, and `specs/001-command-line-shell/quickstart.md`

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Create failing validation and shared scaffolding required by every shell story.

**⚠️ CRITICAL**: No user story implementation starts before this phase is complete.

- [ ] T004 Add failing shell executable and transport tests in new `modules/ar_shell_tests.c`, new `modules/ar_shell_delegate_tests.c`, and `modules/ar_methodology_tests.c`
- [ ] T005 [P] Add failing shell session mediation and shell method tests in new `modules/ar_shell_session_tests.c`, new `methods/shell_tests.c`, and `modules/ar_system_tests.c`
- [ ] T006 [P] Create compile-only shell scaffolding in new `modules/ar_shell.h`, new `modules/ar_shell_delegate.h`, new `modules/ar_shell_session.h`, new `methods/shell-1.0.0.method`, and Makefile so the RED phase fails on assertions instead of compilation
- [ ] T007 Confirm the RED phase for `modules/ar_shell_tests.c`, `modules/ar_shell_delegate_tests.c`, `modules/ar_shell_session_tests.c`, and `methods/shell_tests.c` with `make ar_shell_tests 2>&1`, `make ar_shell_delegate_tests 2>&1`, `make ar_shell_session_tests 2>&1`, and `make shell_tests 2>&1`

**Checkpoint**: Failing validation exists for shell executable startup, shell-session mediation, and built-in shell method behavior.

---

## Phase 3: User Story 1 - Send Terminal Input Into AgeRun (Priority: P1) 🎯 MVP

**Goal**: Deliver the dedicated `arsh` executable implemented by `ar_shell` so it starts the shell, creates the dedicated receiving agent from the built-in `shell` method, wraps each input line as `{text = input}`, forwards it into the runtime, and stays open for repeated input.

**Independent Test**: Start `arsh`, confirm startup creates the dedicated receiving agent, enter one or more text lines, and verify the session-specific shell delegate forwards `{text = <exact input>}` envelopes while the session remains open.

### Validation for User Story 1

- [ ] T008 [P] [US1] Run the targeted failing shell executable and transport tests in `modules/ar_shell_tests.c`, `modules/ar_shell_delegate_tests.c`, and `modules/ar_methodology_tests.c`

### Implementation for User Story 1

- [ ] T009 [P] [US1] Define the shell executable module API and ownership documentation in new `modules/ar_shell.h` and new `modules/ar_shell.md`
- [ ] T010 [P] [US1] Define the session-specific shell delegate API and ownership documentation in new `modules/ar_shell_delegate.h` and new `modules/ar_shell_delegate.md`
- [ ] T011 [US1] Implement the `arsh` executable startup path, shell-session creation, and dedicated receiving-agent creation in new `modules/ar_shell.c`, new `modules/ar_shell_tests.c`, and Makefile
- [ ] T012 [US1] Register the built-in shell method asset for startup in new `methods/shell-1.0.0.method`, `modules/ar_methodology.c`, `modules/ar_methodology.h`, and `modules/ar_methodology_tests.c`
- [ ] T013 [US1] Implement envelope construction, repeated stdin handling, and handoff acknowledgement in new `modules/ar_shell_delegate.c` and new `modules/ar_shell_delegate_tests.c`
- [ ] T014 [US1] Re-run validation for `modules/ar_shell_tests.c`, `modules/ar_shell_delegate_tests.c`, and `modules/ar_methodology_tests.c` with `make ar_shell_tests 2>&1`, `make ar_shell_delegate_tests 2>&1`, and `make ar_methodology_tests 2>&1` until User Story 1 passes
- [ ] T015 [US1] Refactor shared shell startup helpers in `modules/ar_shell.c` and `modules/ar_shell_delegate.c` while preserving green User Story 1 tests

**Checkpoint**: `arsh` can start, create its receiving agent, forward wrapped input, and remain usable for repeated input.

---

## Phase 4: User Story 2 - Use the Built-in Shell Method to Launch and Message Agents (Priority: P2)

**Goal**: Let the built-in `shell` method interpret the restricted one-line syntax subset for `spawn(...)`, `send(...)`, `memory... := ...`, `memory... := spawn(...)`, and `memory... := send(...)`, while `ar_shell` owns shell session state and `ar_shell_session` mediates runtime access without directly handling the session map.

**Independent Test**: Start `arsh`, use the built-in shell method to run one valid spawn, one valid send, and one `memory... := ...` assignment, and verify the requested runtime action or session-state change succeeds without breaking the session.

### Validation for User Story 2

- [ ] T016 [P] [US2] Run the targeted failing shell method, shell-session mediation, and system wiring tests in `methods/shell_tests.c`, `modules/ar_shell_session_tests.c`, `modules/ar_shell_tests.c`, and `modules/ar_system_tests.c`

### Implementation for User Story 2

- [ ] T017 [P] [US2] Define the shell session mediation API, lifecycle rules, and ownership documentation in new `modules/ar_shell_session.h` and new `modules/ar_shell_session.md`
- [ ] T018 [US2] Implement `ar_shell_session` message-based `set`/`get`/`resolved`/`error` mediation in new `modules/ar_shell_session.c` and new `modules/ar_shell_session_tests.c`
- [ ] T019 [US2] Implement shell-session ownership and mediation wiring between `ar_shell`, `ar_shell_session`, and the runtime in `modules/ar_shell.c`, `modules/ar_shell_session.c`, `modules/ar_system.c`, `modules/ar_system.h`, and `modules/ar_system_tests.c`
- [ ] T020 [US2] Implement the restricted shell syntax, assignment redirection, and assigned `spawn`/`send` forms in new `methods/shell-1.0.0.method`, new `methods/shell-1.0.0.md`, and new `methods/shell_tests.c`
- [ ] T021 [US2] Re-run validation for `modules/ar_shell_session_tests.c`, `methods/shell_tests.c`, `modules/ar_shell_tests.c`, and `modules/ar_system_tests.c` with `make ar_shell_session_tests 2>&1`, `make shell_tests 2>&1`, `make ar_shell_tests 2>&1`, and `make ar_system_tests 2>&1` until User Story 2 passes
- [ ] T022 [US2] Refactor shell/session message helpers in `modules/ar_shell.c`, `modules/ar_shell_session.c`, `modules/ar_system.c`, and `methods/shell-1.0.0.method` while preserving green User Story 2 tests

**Checkpoint**: The shell method can spawn agents, send messages, and store/reuse shell session values through `ar_shell` ownership plus `ar_shell_session` mediation.

---

## Phase 5: User Story 3 - Observe Replies in the Terminal Session (Priority: P3)

**Goal**: Display runtime replies asynchronously in the active terminal session, keep sender attribution intact, and cleanly destroy the receiving agent when the shell exits.

**Independent Test**: Trigger a reply from a shell-driven interaction, continue entering input, and verify the later reply appears in the same terminal session with sender attribution and without breaking continuity.

### Validation for User Story 3

- [ ] T023 [P] [US3] Run the targeted failing reply-display and shutdown tests in `modules/ar_shell_delegate_tests.c`, `modules/ar_shell_tests.c`, and `modules/ar_system_tests.c`

### Implementation for User Story 3

- [ ] T024 [P] [US3] Define reply-display and sender-attribution behavior in `modules/ar_shell_delegate.md`, `specs/001-command-line-shell/contracts/arsh-cli.md`, and `specs/001-command-line-shell/contracts/shell-session-protocol.md`
- [ ] T025 [US3] Implement asynchronous reply polling, output-envelope unwrapping, and sender attribution in `modules/ar_shell_delegate.c` and `modules/ar_shell_delegate_tests.c`
- [ ] T026 [US3] Wire runtime replies back into the active shell executable flow in `modules/ar_shell.c`, `modules/ar_shell_session.c`, `modules/ar_system.c`, and `modules/ar_shell_tests.c`
- [ ] T027 [US3] Implement clean shell shutdown and receiving-agent cleanup handling in `modules/ar_shell.c`, `modules/ar_shell_tests.c`, `modules/ar_system.c`, and `modules/ar_system_tests.c`
- [ ] T028 [US3] Re-run validation for `modules/ar_shell_delegate_tests.c`, `modules/ar_shell_tests.c`, and `modules/ar_system_tests.c` with `make ar_shell_delegate_tests 2>&1`, `make ar_shell_tests 2>&1`, and `make ar_system_tests 2>&1` until User Story 3 passes
- [ ] T029 [US3] Refactor reply-routing and shutdown helpers in `modules/ar_shell.c`, `modules/ar_shell_delegate.c`, `modules/ar_shell_session.c`, and `modules/ar_system.c` while preserving green User Story 3 tests

**Checkpoint**: The shell shows attributed asynchronous replies and exits with session-scoped cleanup.

---

## Final Phase: Polish & Cross-Cutting Concerns

**Purpose**: Finish documentation synchronization and repo-wide validation.

- [ ] T030 [P] Update user-facing shell guidance in `README.md`, `SPEC.md`, and `specs/001-command-line-shell/quickstart.md`
- [ ] T031 [P] Update technical module and method documentation in `modules/ar_shell.md`, `modules/ar_shell_session.md`, `modules/ar_shell_delegate.md`, and `methods/shell-1.0.0.md`
- [ ] T032 [P] Sync shell contracts and workflow context in `specs/001-command-line-shell/contracts/README.md`, `specs/001-command-line-shell/contracts/arsh-cli.md`, `specs/001-command-line-shell/contracts/shell-session-protocol.md`, and `.specify/memory/pi-agent.md` if the active shell workflow context changed
- [ ] T033 Run full targeted regression for `modules/ar_shell_tests.c`, `modules/ar_shell_session_tests.c`, `modules/ar_shell_delegate_tests.c`, `methods/shell_tests.c`, `modules/ar_system_tests.c`, and `modules/ar_methodology_tests.c` with `make ar_shell_tests 2>&1`, `make ar_shell_session_tests 2>&1`, `make ar_shell_delegate_tests 2>&1`, `make shell_tests 2>&1`, `make ar_system_tests 2>&1`, and `make ar_methodology_tests 2>&1`
- [ ] T034 Run documentation validation for `README.md`, `SPEC.md`, `modules/ar_shell.md`, `modules/ar_shell_session.md`, `modules/ar_shell_delegate.md`, `methods/shell-1.0.0.md`, and `specs/001-command-line-shell/quickstart.md` with `make check-docs`
- [ ] T035 Run sanitizer coverage for `modules/ar_shell.c`, `modules/ar_shell_session.c`, `modules/ar_shell_delegate.c`, `modules/ar_system.c`, and `methods/shell_tests.c` with `make sanitize-tests 2>&1`
- [ ] T036 Run final repo gates for Makefile, `modules/ar_shell.c`, `modules/ar_shell_session.c`, `modules/ar_shell_delegate.c`, and `methods/shell-1.0.0.method` with `make clean build 2>&1` and then `make check-logs`

---

## Dependencies & Execution Order

### Phase Dependencies

- **Phase 1: Setup** starts immediately
- **Phase 2: Foundational** depends on Setup and establishes the RED phase for shell executable startup, shell-session mediation, and shell method behavior
- **Phase 3: User Story 1** depends on Foundational completion
- **Phase 4: User Story 2** depends on User Story 1 because shell interpretation requires the `arsh` executable, session-specific delegate, and receiving-agent startup path
- **Phase 5: User Story 3** depends on User Story 2 because asynchronous replies are most meaningful after shell-driven runtime interactions exist
- **Final Phase** depends on all desired stories being complete

### Within Each User Story

- Run the failing tests first
- Implement the minimal change that makes the story pass
- Re-run targeted tests until green
- Refactor only after the story is green
- Update documentation/contracts after behavior stabilizes

### Parallel Opportunities

- **Setup**: `T002` and `T003` can run in parallel after `T001`
- **Foundational**: `T005` and `T006` can run in parallel after `T004`
- **User Story 1**: `T009` and `T010` can proceed in parallel after `T008` exposes the failures
- **User Story 2**: `T017` can proceed in parallel with design work for `T020` once `T016` exposes the failures
- **User Story 3**: `T024` can proceed in parallel with implementation planning for `T025` after `T023`
- **Polish**: `T030`, `T031`, and `T032` can run in parallel after all story behavior is stable

---

## Implementation Strategy

### MVP First (User Story 1 Only)

1. Complete Setup
2. Complete Foundational RED-phase work
3. Deliver User Story 1 (`arsh` executable startup, receiving-agent creation, envelope transport, repeated input)
4. Validate User Story 1 independently with the targeted `make` tests
5. Stop if a transport-only MVP is sufficient for the current milestone

### Incremental Delivery

1. Finish shared RED-phase and scaffolding once
2. Deliver User Story 1 for interactive shell transport
3. Deliver User Story 2 for spawn/send/assignment behavior and session-state mediation
4. Deliver User Story 3 for asynchronous replies and shutdown cleanup
5. Finish with documentation sync and full repository quality gates

---

## Notes

- Keep every RED phase at assertion level; use scaffolding tasks only to avoid compilation failures
- Preserve the separation of concerns from `research.md`: executable/session ownership in `ar_shell`, runtime mediation in `ar_shell_session`, transport in `ar_shell_delegate`, and shell semantics in `methods/shell-1.0.0.method`
- Use ownership-prefixed names and heap-tracking macros in every new C implementation file
- Do not mark any task complete until the corresponding `make` output, file diff, or documentation update exists
