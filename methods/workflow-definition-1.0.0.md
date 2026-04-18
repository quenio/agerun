# workflow-definition-1.0.0

## Overview

`workflow-definition` is the workflow metadata and decision method for the bundled workflow demo.
It recognizes the supported YAML definition paths, performs the startup `complete(...)` dependency
probe, and evaluates review-stage transition decisions by normalizing generated `outcome` and
`reason` values.

The current implementation is intentionally parse-friendly and path-based: it recognizes
`workflows/default-workflow.yaml`, `workflows/test-workflow.yaml`, and an invalid-schema fixture
path used by tests.

## ATN Specification

The ATN below specifies the observable contract of the definition method: accepted requests,
recognized definition categories, and the required normalization of startup and transition results.

```haskell
prepare_requested: Boolean
evaluate_requested: Boolean
describe_requested: Boolean

known_definition_path: Boolean
invalid_definition_path: Boolean
startup_probe_succeeded: Boolean
transition_probe_succeeded: Boolean

definition_ready_sent: Boolean
definition_error_sent: Boolean
transition_decision_sent: Boolean
describe_result_sent: Boolean

error_reason: String
outcome: String
next_stage: String
terminal_outcome: String
retryable: Boolean

READY_AND_ERROR_ARE_MUTUALLY_EXCLUSIVE:
  not (definition_ready_sent and definition_error_sent)

KNOWN_DEFINITION_WITH_READY_DEPENDENCIES_BECOMES_READY:
  prepare_requested and known_definition_path and startup_probe_succeeded => definition_ready_sent

INVALID_DEFINITION_IS_REJECTED:
  prepare_requested and invalid_definition_path =>
    definition_error_sent and error_reason = "invalid_definition_schema"

UNKNOWN_DEFINITION_IS_REJECTED:
  prepare_requested and not known_definition_path and not invalid_definition_path =>
    definition_error_sent and error_reason = "invalid_definition_schema"

STARTUP_PROBE_FAILURE_IS_REPORTED:
  prepare_requested and known_definition_path and not startup_probe_succeeded =>
    definition_error_sent and error_reason = "startup_dependency_unavailable"

EVALUATION_PRODUCES_A_DECISION:
  evaluate_requested => transition_decision_sent

TRANSITION_FAILURE_BECOMES_RETRYABLE_STAY:
  evaluate_requested and not transition_probe_succeeded =>
    outcome = "stay" and retryable and terminal_outcome = ""

REJECT_DECISION_REJECTS_THE_ITEM:
  evaluate_requested and outcome = "reject" => terminal_outcome = "rejected"

REVIEW_ADVANCE_COMPLETES_THE_ITEM:
  evaluate_requested and outcome = "advance" and next_stage = "completion" =>
    terminal_outcome = "completed"

DESCRIBE_REQUEST_RETURNS_A_DESCRIPTION:
  describe_requested => describe_result_sent
```

## Inputs

### `action=prepare_definition`

Expected fields:
- `definition_path`
- `stage`
- `review_status`
- `sender`

Behavior:
- resolves known workflow definitions by path
- sets workflow metadata such as `workflow_name`, `initial_stage`, and terminal outcomes
- runs `complete("Workflow dependency probe outcome={outcome} reason={reason}.")`
- replies with `definition_ready` or `definition_error`

### `action=evaluate_transition`

Expected fields:
- `sender`
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
- normalizes the result into `advance`, `stay`, or `reject`
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

### `workflows/default-workflow.yaml`
- `workflow_name = default_workflow`
- `validation_clause = review_gate`
- normal bundled executable demo path

### `workflows/test-workflow.yaml`
- `workflow_name = test_workflow`
- `validation_clause = test_gate`
- deterministic alternate-definition test path

### Invalid path handling
Unknown definitions and `invalid-workflow.yaml` are rejected with:
- `reason = invalid_definition_schema`

Startup dependency probe failures are reported with:
- `reason = startup_dependency_unavailable`

Transition evaluation failures are normalized to:
- `outcome = stay`
- `reason = complete_transition_failed`
- `retryable = 1`

## Method Code

```agerun
memory.is_prepare := if(message.action = "prepare_definition", 1, 0)
memory.is_evaluate := if(message.action = "evaluate_transition", 1, 0)
memory.is_describe := if(message.action = "describe", 1, 0)
memory.sender_id := if(message.sender > 0, message.sender, 0)
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
memory.transition_outcome := ""
memory.transition_reason := ""
memory.next_stage := ""
memory.transition_status := ""
memory.terminal_outcome := ""
memory.definition_path := if(memory.is_prepare = 1, message.definition_path, memory.definition_path)
memory.is_default_definition := if(memory.definition_path = "workflows/default-workflow.yaml", 1, 0)
memory.is_test_definition := if(memory.definition_path = "workflows/test-workflow.yaml", 1, 0)
memory.is_known_definition := memory.is_default_definition + memory.is_test_definition
memory.is_invalid_definition := if(memory.definition_path = "invalid-workflow.yaml", 1, 0)
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
memory.last_reason := if(memory.ready_flag = 1, memory.reason, memory.last_reason)
memory.last_reason := if(memory.error_flag = 1, memory.error_reason, memory.last_reason)
memory.ready_input := build("action=definition_ready workflow_name={workflow_name} workflow_version={workflow_version} initial_stage={initial_stage} requires_local_completion={requires_local_completion}", memory)
memory.ready_payload := parse("action={action} workflow_name={workflow_name} workflow_version={workflow_version} initial_stage={initial_stage} requires_local_completion={requires_local_completion}", memory.ready_input)
memory.ready_sent := send(memory.sender_id * memory.ready_flag, memory.ready_payload)
memory.last_reply_action := if(memory.ready_flag = 1, "definition_ready", memory.last_reply_action)
memory.error_input := build("action=definition_error reason={error_reason}", memory)
memory.error_payload := parse("action={action} reason={reason}", memory.error_input)
memory.error_sent := send(memory.sender_id * memory.error_flag, memory.error_payload)
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
memory.transition_input := build("action=transition_decision workflow_name={workflow_name} from_stage={from_stage} outcome={transition_outcome} next_stage={next_stage} status={transition_status} validation_clause={validation_clause} reason={transition_reason} retryable={retryable} terminal_outcome={terminal_outcome} note={transition_reason}", memory)
memory.transition_payload := parse("action={action} workflow_name={workflow_name} from_stage={from_stage} outcome={outcome} next_stage={next_stage} status={status} validation_clause={validation_clause} reason={reason} retryable={retryable} terminal_outcome={terminal_outcome} note={note}", memory.transition_input)
memory.transition_sent := send(memory.sender_id * memory.is_evaluate, memory.transition_payload)
memory.describe_input := build("action=describe_result workflow_name={workflow_name} workflow_version={workflow_version} initial_stage={initial_stage} terminal_completed={terminal_completed} terminal_rejected={terminal_rejected} requires_local_completion={requires_local_completion} item_fields={item_fields} stages={stages} transitions={validation_clause} validation_clauses={validation_clause}", memory)
memory.describe_payload := parse("action={action} workflow_name={workflow_name} workflow_version={workflow_version} initial_stage={initial_stage} terminal_completed={terminal_completed} terminal_rejected={terminal_rejected} requires_local_completion={requires_local_completion} item_fields={item_fields} stages={stages} transitions={transitions} validation_clauses={validation_clauses}", memory.describe_input)
memory.describe_sent := send(memory.sender_id * memory.is_describe, memory.describe_payload)
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
