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

- [x] T001 Run baseline validation for `modules/ar_system_tests.c`, `modules/ar_methodology_tests.c`, `modules/ar_delegate_tests.c`, and `methods/chat_session_tests.c` with `make ar_system_tests 2>&1`, `make ar_methodology_tests 2>&1`, `make ar_delegate_tests 2>&1`, and `make chat_session_tests 2>&1`
- [x] T002 [P] Reconcile feature scope across `specs/001-command-line-shell/spec.md`, `specs/001-command-line-shell/plan.md`, `specs/001-command-line-shell/research.md`, `specs/001-command-line-shell/data-model.md`, `specs/001-command-line-shell/contracts/arsh-cli.md`, and `specs/001-command-line-shell/contracts/shell-session-protocol.md`
- [x] T003 [P] Confirm documentation sync targets in `README.md`, `SPEC.md`, `modules/ar_shell.md`, `modules/ar_shell_session.md`, `modules/ar_shell_delegate.md`, `.specify/memory/pi-agent.md`, and `specs/001-command-line-shell/quickstart.md`

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Create failing validation and shared scaffolding required by every shell story.

**⚠️ CRITICAL**: No user story implementation starts before this phase is complete.

- [x] T004 Add failing shell executable and transport tests in new `modules/ar_shell_tests.c`, new `modules/ar_shell_delegate_tests.c`, and `modules/ar_methodology_tests.c`
- [x] T005 [P] Add failing shell session mediation and shell method tests in new `modules/ar_shell_session_tests.c`, new `methods/shell_tests.c`, and `modules/ar_system_tests.c`
- [x] T006 [P] Create compile-only shell scaffolding in new `modules/ar_shell.h`, new `modules/ar_shell_delegate.h`, new `modules/ar_shell_session.h`, new `methods/shell-1.0.0.method`, and Makefile so the RED phase fails on assertions instead of compilation
- [x] T007 Confirm the RED phase for `modules/ar_shell_tests.c`, `modules/ar_shell_delegate_tests.c`, `modules/ar_shell_session_tests.c`, and `methods/shell_tests.c` with `make ar_shell_tests 2>&1`, `make ar_shell_delegate_tests 2>&1`, `make ar_shell_session_tests 2>&1`, and `make shell_tests 2>&1`

**Checkpoint**: Failing validation exists for shell executable startup, shell-session mediation, and built-in shell method behavior.

---

## Phase 3: User Story 1 - Send Terminal Input Into AgeRun (Priority: P1) 🎯 MVP

**Goal**: Deliver the dedicated `arsh` executable implemented by `ar_shell` so it starts the shell, selects the shell acknowledgement mode via a CLI startup flag, creates the dedicated receiving agent from the built-in `shell` method, wraps each input line as `{text = input}`, forwards it into the runtime, and stays open for repeated input.

**Independent Test**: Start `arsh`, confirm startup creates the dedicated receiving agent, verify the requested startup mode becomes the active shell-session acknowledgement mode, enter one or more text lines, and verify the session-specific shell delegate forwards `{text = <exact input>}` envelopes while the session remains open.

### Validation for User Story 1

- [x] T008 [P] [US1] Run the targeted failing shell executable and transport tests in `modules/ar_shell_tests.c`, `modules/ar_shell_delegate_tests.c`, and `modules/ar_methodology_tests.c`

### Implementation for User Story 1

- [x] T009 [P] [US1] Define the shell executable module API and ownership documentation in new `modules/ar_shell.h` and new `modules/ar_shell.md`
- [x] T010 [P] [US1] Define the session-specific shell delegate API and ownership documentation in new `modules/ar_shell_delegate.h` and new `modules/ar_shell_delegate.md`
- [x] T011 [US1] Implement the `arsh` executable startup path, startup acknowledgement-mode flag parsing and propagation into `ar_shell_session`, shell-session creation, and dedicated receiving-agent creation in new `modules/ar_shell.c`, new `modules/ar_shell_tests.c`, and Makefile
- [x] T012 [US1] Register the built-in shell method asset for startup in new `methods/shell-1.0.0.method`, `modules/ar_methodology.c`, `modules/ar_methodology.h`, and `modules/ar_methodology_tests.c`
- [x] T013 [US1] Implement envelope construction, repeated stdin handling, and normal/verbose handoff acknowledgement behavior in new `modules/ar_shell_delegate.c` and new `modules/ar_shell_delegate_tests.c`
- [x] T014 [US1] Re-run validation for `modules/ar_shell_tests.c`, `modules/ar_shell_delegate_tests.c`, and `modules/ar_methodology_tests.c` with `make ar_shell_tests 2>&1`, `make ar_shell_delegate_tests 2>&1`, and `make ar_methodology_tests 2>&1` until User Story 1 passes
- [x] T015 [US1] Refactor shared shell startup helpers in `modules/ar_shell.c` and `modules/ar_shell_delegate.c` while preserving green User Story 1 tests

**Checkpoint**: `arsh` can start, create its receiving agent, forward wrapped input, and remain usable for repeated input.

---

## Phase 4: User Story 2 - Use the Built-in Shell Method to Launch and Message Agents (Priority: P2)

**Goal**: Let the built-in `shell` method interpret the restricted one-line syntax subset for `spawn(...)`, `send(...)`, `memory... := ...`, `memory... := spawn(...)`, and `memory... := send(...)`, while `ar_shell` manages shell sessions and `ar_shell_session` owns per-session state and lifecycle.

**Independent Test**: Start `arsh`, use the built-in shell method to run one valid spawn, one valid send, and one `memory... := ...` assignment, then enter one invalid shell-syntax line and verify the requested runtime action or session-state change succeeds while the invalid line reports an error without breaking the session.

### Validation for User Story 2

- [x] T016 [P] [US2] Run the targeted failing shell method, shell-session mediation, invalid-shell-syntax recovery, and system wiring tests in `methods/shell_tests.c`, `modules/ar_shell_session_tests.c`, `modules/ar_shell_tests.c`, and `modules/ar_system_tests.c`

### Implementation for User Story 2

- [x] T017 [P] [US2] Define the shell session state/lifecycle API, message protocol, and ownership documentation in new `modules/ar_shell_session.h` and new `modules/ar_shell_session.md`
- [x] T018 [US2] Implement `ar_shell_session` state ownership plus message-based ar_shell_session__store_value / ar_shell_session__load_value / ar_shell_session__return_loaded_value / ar_shell_session__report_operation_failure mediation in new `modules/ar_shell_session.c` and new `modules/ar_shell_session_tests.c`
- [x] T019 [US2] Implement shell-session management wiring between `ar_shell`, `ar_shell_session`, and the runtime in `modules/ar_shell.c`, `modules/ar_shell_session.c`, `modules/ar_system.c`, `modules/ar_system.h`, and `modules/ar_system_tests.c`
- [x] T020 [US2] Implement the restricted shell syntax, recoverable invalid-shell-syntax reporting, assignment redirection, and assigned `spawn`/`send` forms in new `methods/shell-1.0.0.method`, new `methods/shell-1.0.0.md`, and new `methods/shell_tests.c`
- [x] T021 [US2] Re-run validation for `modules/ar_shell_session_tests.c`, `methods/shell_tests.c`, `modules/ar_shell_tests.c`, and `modules/ar_system_tests.c` with `make ar_shell_session_tests 2>&1`, `make shell_tests 2>&1`, `make ar_shell_tests 2>&1`, and `make ar_system_tests 2>&1` until User Story 2 passes
- [ ] T022 [US2] Refactor shell/session message helpers in `modules/ar_shell.c`, `modules/ar_shell_session.c`, `modules/ar_system.c`, and `methods/shell-1.0.0.method` while preserving green User Story 2 tests

**Checkpoint**: The shell method can spawn agents, send messages, and store/reuse shell session values with `ar_shell` managing sessions and `ar_shell_session` owning per-session state.

---

## Phase 5: User Story 3 - Observe Replies in the Terminal Session (Priority: P3)

**Goal**: Display runtime replies asynchronously in the active terminal session, attribute them using only the runtime sender ID, discard delayed replies after EOF / Ctrl-D closes the session, and cleanly destroy the receiving agent when the shell exits.

**Independent Test**: Trigger a reply from a shell-driven interaction, continue entering input, and verify the later reply appears in the same terminal session with runtime-sender-ID-only attribution and without breaking continuity while the session remains open, then verify EOF / Ctrl-D closes immediately and suppresses later returned replies.

### Validation for User Story 3

- [ ] T023 [P] [US3] Run the targeted failing reply-display, runtime-sender-ID-only attribution, EOF / Ctrl-D shutdown, late-reply discard, and receiving-agent cleanup tests in `modules/ar_shell_delegate_tests.c`, `modules/ar_shell_tests.c`, and `modules/ar_system_tests.c`

### Implementation for User Story 3

- [ ] T024 [P] [US3] Define reply-display and sender-attribution behavior using only the runtime sender ID, plus EOF / Ctrl-D discard semantics, in `modules/ar_shell_delegate.md`, `specs/001-command-line-shell/contracts/arsh-cli.md`, and `specs/001-command-line-shell/contracts/shell-session-protocol.md`
- [ ] T025 [US3] Implement asynchronous returned-message callback routing from `ar_shell_delegate` into `ar_shell_session__render_output`, with display attribution limited to the runtime sender ID, in `modules/ar_shell_delegate.c`, `modules/ar_shell_session.c`, and `modules/ar_shell_delegate_tests.c`
- [ ] T026 [US3] Wire runtime replies back into the active shell executable flow in `modules/ar_shell.c`, `modules/ar_shell_session.c`, `modules/ar_system.c`, and `modules/ar_shell_tests.c`
- [ ] T027 [US3] Implement EOF / Ctrl-D-only shell shutdown, immediate session close, late-returned-message discard, and receiving-agent cleanup handling in `modules/ar_shell.c`, `modules/ar_shell_tests.c`, `modules/ar_system.c`, and `modules/ar_system_tests.c`
- [ ] T028 [US3] Re-run validation for `modules/ar_shell_delegate_tests.c`, `modules/ar_shell_tests.c`, and `modules/ar_system_tests.c` with `make ar_shell_delegate_tests 2>&1`, `make ar_shell_tests 2>&1`, and `make ar_system_tests 2>&1` until User Story 3 passes
- [ ] T029 [US3] Refactor returned-message callback routing and shutdown helpers in `modules/ar_shell.c`, `modules/ar_shell_delegate.c`, `modules/ar_shell_session.c`, and `modules/ar_system.c` while preserving green User Story 3 tests

**Checkpoint**: The shell shows runtime-sender-ID-attributed asynchronous replies while open and exits immediately on EOF / Ctrl-D with session-scoped cleanup and late-reply discard.

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
- Preserve the separation of concerns from `research.md`: session management in `ar_shell`, per-session state/lifecycle and runtime mediation in `ar_shell_session`, transport in `ar_shell_delegate`, and shell semantics in `methods/shell-1.0.0.method`
- Use ownership-prefixed names and heap-tracking macros in every new C implementation file
- Do not mark any task complete until the corresponding `make` output, file diff, or documentation update exists
