# Spec Implement: Workflow Coordinator

Implement `002-let-build-workflow-coordinator` end-to-end.

## Goals
- Finish all tasks in `specs/002-let-build-workflow-coordinator/tasks.md`
- Keep TDD discipline with small green slices
- Preserve clarified `complete(...)` behavior and startup-failure semantics

## Checklist
- [x] workflow-definition prepare success path green
- [x] workflow-definition invalid/error path green
- [x] workflow-definition transition decision path green
- [x] workflow-coordinator green
- [x] workflow-item green
- [x] workflow-reporter green
- [x] bootstrap green
- [x] executable tests green
- [x] docs/contracts synced
- [x] sanitizer/build/check-logs green

## Verification
- `make workflow_definition_tests 2>&1` ✅
- `make workflow_reporter_tests 2>&1` ✅
- `make workflow_item_tests 2>&1` ✅
- `make workflow_coordinator_tests 2>&1` ✅
- `make bootstrap_tests 2>&1` ✅
- `make ar_executable_tests 2>&1` ✅
- `make check-docs 2>&1` ✅
- `make sanitize-tests 2>&1` ✅
- `make clean build 2>&1` ✅
- `make check-logs` ✅

## Notes
- Narrowing aggressively: fix one failing method/test slice at a time.
- Known root causes already found: direct test-created agents need non-NULL context; method-level nested message-map construction is fragile.
- Converted workflow-definition and workflow-item message sends to build+parse payload creation.
- Reporter log assertions had to move after fixture destruction because log writes flush on destroy.
- Fixed the coordinator test loop by replacing `while (process_next_message)` drains with bounded step-by-step message processing.
- Found an actual coordinator bug rather than an interpreter/parser-only issue: `prepare_definition` was being re-sent on every coordinator message because the send was not gated by `memory.should_launch`; that extra queued message was what kept stealing scheduler turns.
- Bootstrap no longer segfaults; it now uses build+parse start payload creation and emits the intake log line directly.
- `ar_executable_tests.c` is now updated to the workflow demo world: 14 loaded methods, workflow demo activity, workflow log output, and 14-method methodology persistence.
- Updated user-facing docs in `README.md`, `methods/README.md`, and `methods/bootstrap.md` to describe the workflow demo instead of the older chat-session bootstrap behavior.
- Synced the workflow contracts/data model/research notes to the currently implemented runtime shape and added log-whitelist coverage for expected startup dependency failures.
