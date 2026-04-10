# Task

Investigate and implement a zero-copy forwarding path for `send(target, message)` so the currently processed message can be forwarded directly without going through `ar_data__claim_or_copy()`.

## Goals
- Preserve correctness and ownership safety while forwarding the active message.
- Improve the integration benchmark hot path in `modules/ar_integration_performance_tests.c`.

## Checklist
- [x] Analyze current forwarding path and identify why zero-copy is not happening.
- [x] Confirm which autoresearch ideas are already implemented.
- [x] Design the smallest safe ownership-transfer API needed for zero-copy forwarding.
- [x] Add focused tests that prove whole-message forwarding can avoid copying.
- [x] Implement the fast path for exact `send(..., message)` while preserving fallback behavior.
- [x] Run targeted tests and benchmark the change.

## Notes
- Confirmed `ar_map` dynamic resizing/open addressing is already implemented in `modules/ar_map.c`.
- Confirmed zero-copy forwarding is not implemented yet.
- Current hot path:
  - `modules/ar_system.c`: system takes ownership of dequeued message, executes method, then destroys processed message.
  - `modules/ar_expression_evaluator.zig`: `message` resolves through `ar_frame__get_message()`.
  - `modules/ar_send_instruction_evaluator.zig`: send currently obtains payload via `ar_data__claim_or_copy(message_result, ref_evaluator)`.
  - `modules/ar_data.c`: `ar_data__claim_or_copy()` returns the original only when unowned; otherwise it shallow-copies.
- Benchmark alignment is strong because the integration benchmark method is exactly `send(memory.next_id, message)`.
- Main design constraint: current queue send APIs assume fresh/unowned messages. A true zero-copy path needs atomic ownership transfer from the system-owned current message to the destination queue owner, without introducing an ownership gap.
- Chosen minimal design:
  - Add `ar_data__transfer_ownership(mut_data, ref_from_owner, ref_to_owner)` as the atomic primitive.
  - Thread the current message owner through execution context by extending `ar_interpreter__execute_method(...)` and `ar_frame__create/reset(...)` with a message-owner token plus `ar_frame__get_message_owner()`.
  - Add explicit queue-transfer APIs instead of changing existing send semantics: `ar_agent__send_from_owner(...)`, `ar_delegate__send_from_owner(...)`, and agency/delegation wrappers.
  - In `ar_send_instruction_evaluator.zig`, special-case exact whole-message forwarding only when AST is `message` with zero path components (`base == "message" && path_count == 0`).
  - Keep current `claim_or_copy` fallback for `message.field`, `memory.x`, literals, arithmetic, and any case where transfer is unavailable.
- Rejected design shortcut: making the frame/interpreter the owner of the active message directly would require const-hostile interface changes or ownership casts; carrying an owner token through the frame is cleaner.
- Planned proof tests:
  - system/integration-style test showing `send(next, message)` delivers the same pointer instance to the next queue;
  - negative/control test showing `send(next, message.field)` still uses copy semantics;
  - existing send evaluator behavior must remain unchanged for literals and memory references.
- Added RED-phase end-to-end tests in `modules/ar_system_tests.c`:
  - `test_message_forwarding__whole_message_reuses_pointer()`
  - `test_message_forwarding__message_field_still_copies()`
- Current implementation status:
  - Added atomic ownership transfer primitive: `ar_data__transfer_ownership(...)`.
  - Added frame message-owner plumbing via `ar_frame__set_message_owner()` and `ar_frame__get_message_owner()`.
  - Extended interpreter execution to pass the current message owner into the reusable frame.
  - Added explicit queue transfer APIs:
    - `ar_agent__send_from_owner(...)`
    - `ar_delegate__send_from_owner(...)`
    - `ar_agency__send_to_agent_from_owner(...)`
    - `ar_delegation__send_to_delegate_from_owner(...)`
  - Updated `modules/ar_send_instruction_evaluator.zig` to special-case exact `send(..., message)` forwarding:
    - zero-copy transfer is attempted when the frame has a current message owner;
    - fallback to existing `claim_or_copy` behavior remains in place for other expressions and for transfer failures.
- GREEN results after implementation:
  - `make ar_system_tests 2>&1` passes, including both new forwarding tests.
  - `make ar_send_instruction_evaluator_tests 2>&1` passes.
  - `make ar_interpreter_tests 2>&1` passes.
- Benchmark result:
  - Initialized autoresearch tracking in `autoresearch.jsonl`.
  - `bash autoresearch.sh` / `make ar_integration_performance_tests` reports `METRIC total_µs=1526`.
  - Compared to the prior ~`1936µs` integration result noted in `CHANGELOG.md`, this is an
    improvement of about `410µs` (~21%).
- Documentation/tracking updates:
  - Added a `2026-04-10` entry to `CHANGELOG.md`.
  - Updated `autoresearch.md` with the implemented experiment result.
  - Updated `autoresearch.ideas.md` to reflect that the pending ideas have been completed.
- Likely files involved: `modules/ar_send_instruction_evaluator.zig`, `modules/ar_data.h`, `modules/ar_data.c`, `modules/ar_agent.h`, `modules/ar_agent.c`, `modules/ar_delegate.h`, `modules/ar_delegate.c`, `modules/ar_agency.h`, `modules/ar_agency.c`, `modules/ar_delegation.h`, `modules/ar_delegation.c`, `modules/ar_frame.h`, `modules/ar_frame.c`, and `modules/ar_interpreter.h/c`.
