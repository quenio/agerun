# workflow-definition-1.0.0

## Overview

`workflow-definition` is the workflow metadata and decision method for the bundled workflow demo.
It recognizes the supported workflow definition paths, performs the startup `complete(...)` dependency
probe, and evaluates review-stage transition decisions by normalizing generated `outcome` and
`reason` values.

The current implementation is intentionally parse-friendly and path-based: it recognizes
`workflows/default.workflow`, `workflows/test.workflow`, and an invalid-schema fixture
path used by tests.

## ATN Specification

This ATN specification uses only the probeable agent-state constants requested by the workflow
contract: `initial_memory`, `final_memory`, `message`, and `context`.

```haskell
ProbeValue :: String | Natural
ProbeMap :: String -> ProbeValue

initial_memory: ProbeMap
final_memory: ProbeMap
message: ProbeMap
context: ProbeMap

REQUIRES_SUPPORTED_MESSAGE_ACTION:
  message("action") = "prepare_definition" or
  message("action") = "evaluate_transition" or
  message("action") = "describe"

REQUIRES_PREPARE_MESSAGE_IS_COMPLETE:
  message("action") = "prepare_definition" =>
    message("reply_to") > 0 and
    not (message("definition_path") = "") and
    not (message("stage") = "") and
    not (message("review_status") = "")

REQUIRES_EVALUATE_MESSAGE_IS_COMPLETE:
  message("action") = "evaluate_transition" =>
    message("reply_to") > 0 and
    not (message("workflow_name") = "") and
    not (message("stage") = "") and
    not (message("item_id") = "") and
    not (message("title") = "") and
    not (message("priority") = "") and
    not (message("owner") = "") and
    not (message("review_status") = "")

REQUIRES_DESCRIBE_HAS_A_REPLY_TARGET:
  message("action") = "describe" => message("reply_to") > 0

ENSURES_KNOWN_READY_DEFINITION_BECOMES_READY:
  message("action") = "prepare_definition" and
  (message("definition_path") = "workflows/default.workflow" or
   message("definition_path") = "workflows/test.workflow") and
  final_memory("probe_ok") = 1 =>
    final_memory("ready_sent") = 1 and
    final_memory("last_reply_action") = "definition_ready"

ENSURES_INVALID_OR_UNKNOWN_DEFINITION_IS_REJECTED:
  message("action") = "prepare_definition" and
  not (message("definition_path") = "workflows/default.workflow") and
  not (message("definition_path") = "workflows/test.workflow") =>
    final_memory("error_sent") = 1 and
    final_memory("error_reason") = "invalid_definition_schema" and
    final_memory("last_reply_action") = "definition_error"

ENSURES_STARTUP_PROBE_FAILURE_IS_REPORTED:
  message("action") = "prepare_definition" and
  (message("definition_path") = "workflows/default.workflow" or
   message("definition_path") = "workflows/test.workflow") and
  final_memory("probe_ok") = 0 =>
    final_memory("error_sent") = 1 and
    final_memory("error_reason") = "startup_dependency_unavailable" and
    final_memory("last_reply_action") = "definition_error"

ENSURES_EVALUATION_PRODUCES_A_DECISION:
  message("action") = "evaluate_transition" => final_memory("transition_sent") = 1

ENSURES_TRANSITION_FAILURE_BECOMES_RETRYABLE_STAY:
  message("action") = "evaluate_transition" and final_memory("probe_ok") = 0 =>
    final_memory("transition_outcome") = "stay" and
    final_memory("transition_reason") = "complete_transition_failed" and
    final_memory("retryable") = 1 and
    final_memory("terminal_outcome") = ""

ENSURES_REJECT_DECISION_REJECTS_THE_ITEM:
  message("action") = "evaluate_transition" and final_memory("transition_outcome") = "reject" =>
    final_memory("terminal_outcome") = "rejected"

ENSURES_REVIEW_ADVANCE_COMPLETES_THE_ITEM:
  message("action") = "evaluate_transition" and
  final_memory("transition_outcome") = "advance" and
  final_memory("next_stage") = "completion" =>
    final_memory("terminal_outcome") = "completed"

ENSURES_DESCRIBE_RETURNS_A_DESCRIPTION:
  message("action") = "describe" => final_memory("describe_sent") = 1
```

## Inputs

### `action=prepare_definition`

Expected fields:
- `definition_path`
- `stage`
- `review_status`
- `reply_to`

Behavior:
- resolves known workflow definitions by path
- sets workflow metadata such as `workflow_name`, `initial_stage`, and terminal outcomes
- runs `complete("Workflow dependency probe outcome={outcome} reason={reason}.")`
- uses the boolean return value as the startup gate: success allows `definition_ready`, failure produces `definition_error`
- treats generated `reason` as diagnostic context (`last_reason` on success, `startup_dependency_unavailable` on failure)
- does not currently use generated `outcome` to change startup behavior
- emits a highlighted `complete_trace` marker in startup replies so downstream workflow logs can
  surface searchable `COMPLETE_TRACE[...]` fragments

### `action=evaluate_transition`

Expected fields:
- `reply_to`
- `workflow_name`
- `stage`
- `item_id`
- `title`
- `priority`
- `owner`
- `review_status`
- `transition_count`

Behavior:
- runs `complete("Workflow transition decision outcome={outcome} reason={reason}.")`
- copies generated `outcome` into `transition_outcome` and generated `reason` into `transition_reason`
- uses `transition_outcome` to choose `advance`, `stay`, or `reject`, which then determines
  `next_stage`, `status`, and `terminal_outcome`
- uses `transition_reason` as the explanation propagated in the outgoing `transition_decision`
  message
- emits a highlighted `complete_trace` marker in the outgoing decision so downstream workflow logs
  can surface searchable `COMPLETE_TRACE[...]` fragments
- converts completion/runtime failure into retryable `stay`
- returns a `transition_decision` message with `next_stage`, `status`, `reason`, `retryable`, and
  `terminal_outcome`

### `action=describe`

Returns a description payload containing:
- `workflow_name`
- `workflow_version`
- `initial_stage`
- `terminal_completed`
- `terminal_rejected`
- `requires_local_completion`
- `item_fields`
- `stages`
- `transitions`
- `validation_clauses`

## Current Supported Definitions

### `workflows/default.workflow`
- `workflow_name = default_workflow`
- `validation_clause = review_gate`
- normal bundled executable demo path

### `workflows/test.workflow`
- `workflow_name = test_workflow`
- `validation_clause = test_gate`
- deterministic alternate-definition test path

### Invalid path handling
Unknown definitions and `invalid.workflow` are rejected with:
- `reason = invalid_definition_schema`

Startup dependency probe failures are reported with:
- `reason = startup_dependency_unavailable`

Transition evaluation failures are normalized to:
- `outcome = stay`
- `reason = complete_transition_failed`
- `retryable = 1`

## `complete(...)` Placeholder Usage Summary

The method currently uses the generated placeholders differently in its two `complete(...)` calls:

### Startup dependency probe
- `probe_ok` is the real control signal.
- Generated `reason` is kept only as diagnostic context via `last_reason` when the probe succeeds.
- Generated `outcome` is not currently used to affect startup behavior.
- Successful startup replies now also carry
  `COMPLETE_TRACE[phase=startup|outcome=...|reason=...]` for searchable log output.
- Probe failure is surfaced externally as `definition_error` with
  `reason = startup_dependency_unavailable` and a failure marker
  `COMPLETE_TRACE[phase=startup|status=failure]`.

### Transition decision evaluation
- Generated `outcome` becomes `transition_outcome` and drives the workflow branch:
  - `advance` moves to the next stage
  - `reject` produces `terminal_outcome = rejected`
  - `stay` keeps the current stage
- Generated `reason` becomes `transition_reason` and is forwarded in the
  `transition_decision` message so downstream methods can explain the decision in progress/summary
  logs.
- Outgoing decisions now also carry
  `COMPLETE_TRACE[phase=transition|outcome=...|reason=...]` so reporter output can be searched for
  the exact completion-derived values.
- If `complete(...)` fails, the method preserves workflow continuity by emitting a retryable
  `stay` decision with `reason = complete_transition_failed`.

## Method Code

```agerun
memory.is_prepare := if(message.action = "prepare_definition", 1, 0)
memory.is_evaluate := if(message.action = "evaluate_transition", 1, 0)
memory.is_describe := if(message.action = "describe", 1, 0)
memory.reply_to_id := if(message.reply_to > 0, message.reply_to, 0)
memory.from_stage := message.stage
memory.workflow_name := ""
memory.workflow_version := ""
memory.initial_stage := ""
memory.terminal_completed := ""
memory.terminal_rejected := ""
memory.requires_local_completion := ""
memory.item_fields := ""
memory.stages := ""
memory.validation_clause := ""
memory.file_status := ""
memory.dependency_status := ""
memory.error_reason := ""
memory.last_reason := ""
memory.last_reply_action := ""
memory.error_category := ""
memory.startup_complete_trace := "none"
memory.transition_complete_trace := "none"
memory.transition_outcome := ""
memory.transition_reason := ""
memory.next_stage := ""
memory.transition_status := ""
memory.terminal_outcome := ""
memory.definition_path := if(memory.is_prepare = 1, message.definition_path, memory.definition_path)
memory.is_default_definition := if(memory.definition_path = "workflows/default.workflow", 1, 0)
memory.is_test_definition := if(memory.definition_path = "workflows/test.workflow", 1, 0)
memory.is_known_definition := memory.is_default_definition + memory.is_test_definition
memory.is_invalid_definition := if(memory.definition_path = "invalid.workflow", 1, 0)
memory.workflow_name := if(memory.is_default_definition = 1, "default_workflow", memory.workflow_name)
memory.workflow_name := if(memory.is_test_definition = 1, "test_workflow", memory.workflow_name)
memory.workflow_version := if(memory.is_known_definition > 0, "1.0.0", memory.workflow_version)
memory.initial_stage := if(memory.is_known_definition > 0, "intake", memory.initial_stage)
memory.terminal_completed := if(memory.is_known_definition > 0, "completed", memory.terminal_completed)
memory.terminal_rejected := if(memory.is_known_definition > 0, "rejected", memory.terminal_rejected)
memory.requires_local_completion := if(memory.is_known_definition > 0, "1", memory.requires_local_completion)
memory.item_fields := if(memory.is_known_definition > 0, "item_id|title|priority|owner|review_status", memory.item_fields)
memory.stages := if(memory.is_known_definition > 0, "intake|triage|active|review|completion", memory.stages)
memory.validation_clause := if(memory.is_default_definition = 1, "review_gate", memory.validation_clause)
memory.validation_clause := if(memory.is_test_definition = 1, "test_gate", memory.validation_clause)
memory.file_status := if(memory.is_prepare = 1, "loaded", memory.file_status)
memory.probe_ok := complete("Workflow dependency probe outcome={outcome} reason={reason}.")
memory.dependency_status := if(memory.probe_ok = 1, "ready", memory.dependency_status)
memory.unknown_definition_flag := if(memory.is_known_definition = 0, 1, 0)
memory.probe_failed_flag := if(memory.probe_ok = 0, 1, 0)
memory.probe_failed_flag := if(memory.is_known_definition = 0, 0, memory.probe_failed_flag)
memory.probe_failed_flag := if(memory.is_prepare = 0, 0, memory.probe_failed_flag)
memory.ready_flag := memory.is_prepare * memory.is_known_definition
memory.ready_flag := memory.ready_flag * memory.probe_ok
memory.error_flag := if(memory.is_invalid_definition = 1, 1, 0)
memory.error_flag := if(memory.unknown_definition_flag = 1, 1, memory.error_flag)
memory.error_flag := if(memory.probe_failed_flag = 1, 1, memory.error_flag)
memory.error_flag := if(memory.is_prepare = 0, 0, memory.error_flag)
memory.error_reason := if(memory.is_invalid_definition = 1, "invalid_definition_schema", memory.error_reason)
memory.error_reason := if(memory.unknown_definition_flag = 1, "invalid_definition_schema", memory.error_reason)
memory.error_reason := if(memory.probe_failed_flag = 1, "startup_dependency_unavailable", memory.error_reason)
memory.startup_complete_trace_input := build("COMPLETE_TRACE[phase=startup|outcome={outcome}|reason={reason}]", memory)
memory.startup_complete_trace := if(memory.probe_ok = 1, memory.startup_complete_trace_input, memory.startup_complete_trace)
memory.startup_complete_trace := if(memory.probe_failed_flag = 1, "COMPLETE_TRACE[phase=startup|status=failure]", memory.startup_complete_trace)
memory.last_reason := if(memory.ready_flag = 1, memory.reason, memory.last_reason)
memory.last_reason := if(memory.error_flag = 1, memory.error_reason, memory.last_reason)
memory.ready_input := build("action=definition_ready workflow_name={workflow_name} workflow_version={workflow_version} initial_stage={initial_stage} requires_local_completion={requires_local_completion} complete_trace={startup_complete_trace}", memory)
memory.ready_payload := parse("action={action} workflow_name={workflow_name} workflow_version={workflow_version} initial_stage={initial_stage} requires_local_completion={requires_local_completion} complete_trace={complete_trace}", memory.ready_input)
memory.ready_sent := send(memory.reply_to_id * memory.ready_flag, memory.ready_payload)
memory.last_reply_action := if(memory.ready_flag = 1, "definition_ready", memory.last_reply_action)
memory.error_input := build("action=definition_error reason={error_reason} complete_trace={startup_complete_trace}", memory)
memory.error_payload := parse("action={action} reason={reason} complete_trace={complete_trace}", memory.error_input)
memory.error_sent := send(memory.reply_to_id * memory.error_flag, memory.error_payload)
memory.last_reply_action := if(memory.error_flag = 1, "definition_error", memory.last_reply_action)
memory.transition_ok := complete("Workflow transition decision outcome={outcome} reason={reason}.")
memory.transition_outcome := if(memory.transition_ok = 1, memory.outcome, memory.transition_outcome)
memory.transition_outcome := if(memory.transition_ok = 0, "stay", memory.transition_outcome)
memory.transition_reason := if(memory.transition_ok = 1, memory.reason, memory.transition_reason)
memory.transition_reason := if(memory.transition_ok = 0, "complete_transition_failed", memory.transition_reason)
memory.next_stage := if(message.stage = "intake", "triage", memory.next_stage)
memory.next_stage := if(message.stage = "triage", "active", memory.next_stage)
memory.next_stage := if(message.stage = "active", "review", memory.next_stage)
memory.next_stage := if(message.stage = "review", "completion", memory.next_stage)
memory.next_stage := if(memory.transition_outcome = "stay", message.stage, memory.next_stage)
memory.next_stage := if(memory.transition_outcome = "reject", message.stage, memory.next_stage)
memory.transition_status := if(memory.transition_outcome = "advance", memory.next_stage, memory.transition_status)
memory.transition_status := if(memory.transition_outcome = "stay", "review_waiting", memory.transition_status)
memory.transition_status := if(memory.transition_outcome = "reject", "rejected", memory.transition_status)
memory.retryable := if(memory.transition_ok = 0, 1, 0)
memory.retryable := if(memory.transition_outcome = "stay", memory.retryable, 0)
memory.review_terminal := if(message.stage = "review", "completed", "")
memory.terminal_outcome := if(memory.transition_outcome = "advance", memory.review_terminal, memory.terminal_outcome)
memory.terminal_outcome := if(memory.transition_outcome = "reject", "rejected", memory.terminal_outcome)
memory.terminal_outcome := if(memory.transition_outcome = "stay", "", memory.terminal_outcome)
memory.transition_complete_trace_input := build("COMPLETE_TRACE[phase=transition|outcome={transition_outcome}|reason={transition_reason}]", memory)
memory.transition_complete_trace := if(memory.is_evaluate = 1, memory.transition_complete_trace_input, memory.transition_complete_trace)
memory.transition_input := build("action=transition_decision workflow_name={workflow_name} from_stage={from_stage} outcome={transition_outcome} next_stage={next_stage} status={transition_status} validation_clause={validation_clause} reason={transition_reason} retryable={retryable} terminal_outcome={terminal_outcome} note={transition_reason} complete_trace={transition_complete_trace}", memory)
memory.transition_payload := parse("action={action} workflow_name={workflow_name} from_stage={from_stage} outcome={outcome} next_stage={next_stage} status={status} validation_clause={validation_clause} reason={reason} retryable={retryable} terminal_outcome={terminal_outcome} note={note} complete_trace={complete_trace}", memory.transition_input)
memory.transition_sent := send(memory.reply_to_id * memory.is_evaluate, memory.transition_payload)
memory.describe_input := build("action=describe_result workflow_name={workflow_name} workflow_version={workflow_version} initial_stage={initial_stage} terminal_completed={terminal_completed} terminal_rejected={terminal_rejected} requires_local_completion={requires_local_completion} item_fields={item_fields} stages={stages} transitions={validation_clause} validation_clauses={validation_clause}", memory)
memory.describe_payload := parse("action={action} workflow_name={workflow_name} workflow_version={workflow_version} initial_stage={initial_stage} terminal_completed={terminal_completed} terminal_rejected={terminal_rejected} requires_local_completion={requires_local_completion} item_fields={item_fields} stages={stages} transitions={transitions} validation_clauses={validation_clauses}", memory.describe_input)
memory.describe_sent := send(memory.reply_to_id * memory.is_describe, memory.describe_payload)
```

## Testing

Validated by `methods/workflow_definition_tests.c`.

The tests cover:
- supported default/test workflow path loading
- invalid schema rejection
- startup dependency probe success/failure
- `describe` responses
- normalized `transition_decision` replies for `advance`, `stay`, `reject`
- retryable `stay` conversion when `complete(...)` fails
