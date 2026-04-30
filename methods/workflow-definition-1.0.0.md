# workflow-definition-1.0.0

## Overview

`workflow-definition` is the workflow metadata and decision method for the bundled workflow demo.
It reads a flat workflow definition through the built-in file delegate, validates required metadata
and transition records, performs the startup `complete(...)` dependency probe, and evaluates every
configured stage transition by normalizing generated `outcome` and `reason` values.

The current implementation keeps the parse-friendly flat `.workflow` format. The workflow file is
the authority for stage order, terminal outcomes, and transition prompts; `complete(...)` can choose
`advance`, `stay`, or `reject`, but cannot invent arbitrary next-stage names.

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

ENSURES_VALID_READY_DEFINITION_BECOMES_READY:
  message("action") = "prepare_definition" and
  final_memory("schema_ok") = 1 and
  final_memory("probe_ok") = 1 =>
    final_memory("ready_sent") = 1

ENSURES_INVALID_DEFINITION_IS_REJECTED:
  message("action") = "prepare_definition" and
  final_memory("schema_ok") = 0 =>
    final_memory("error_sent") = 1 and
    final_memory("error_reason") = "invalid_definition_schema"

ENSURES_STARTUP_PROBE_FAILURE_IS_REPORTED:
  message("action") = "prepare_definition" and
  final_memory("schema_ok") = 1 and
  final_memory("probe_ok") = 0 =>
    final_memory("error_sent") = 1 and
    final_memory("error_reason") = "startup_dependency_unavailable"

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

ENSURES_TERMINAL_ADVANCE_COMPLETES_THE_ITEM:
  message("action") = "evaluate_transition" and
  final_memory("transition_outcome") = "advance" and
  final_memory("next_stage") = final_memory("transition_4_to") =>
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
- sends an asynchronous file read request to delegate `-100`
- parses the flat workflow record after the file response arrives
- validates required metadata and the configured transition records
- runs `complete("Workflow dependency probe outcome={outcome} reason={reason}.")`
- checks the returned completion map for substituted `outcome`/`reason` values as the startup gate: success allows `definition_ready`, failure produces `definition_error`
- treats returned `reason` as diagnostic context (`last_reason` on success, `startup_dependency_unavailable` on failure)
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
- selects the configured transition whose `from` stage matches the current item stage
- builds a transition context map from the current workflow item fields and the canonical
  definition-backed workflow name, then passes it to the configured transition prompt
- asks `complete(...)` to evaluate the current item and generate `outcome`/`reason` from that context
- copies returned `outcome` into `transition_outcome` and returned `reason` into `transition_reason`
- validates generated `outcome`; `advance` may only move to the parsed transition's configured
  `to` stage, `stay` keeps the current stage, and `reject` sets the rejected terminal outcome
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
- `transition_count`

## Workflow Definition Schema

The parser expects a flat key-value record with workflow metadata plus four explicit transition
records:
- `transition_count=4`
- `transition_1_from=intake transition_1_to=triage transition_1_prompt=...`
- `transition_2_from=triage transition_2_to=active transition_2_prompt=...`
- `transition_3_from=active transition_3_to=review transition_3_prompt=...`
- `transition_4_from=review transition_4_to=completion transition_4_prompt=...`

Missing or invalid required metadata, transition counts, transition endpoints, or prompts are
rejected with:
- `reason = invalid_definition_schema`

Startup dependency probe failures are reported with:
- `reason = startup_dependency_unavailable`

Transition evaluation failures are normalized to:
- `outcome = stay`
- `reason = complete_transition_failed`
- `retryable = 1`

## `complete(...)` Placeholder Usage Summary

The method passes explicit values maps to its `complete(...)` calls so generated fields are returned
in isolated result maps and completion success is detected without relying on model-controllable
placeholder text:

### Startup dependency probe
- `probe_ok` is derived from the copied `complete_present` marker plus generated `outcome`/`reason`
  fields after build/parse normalization.
- Returned `startup_complete.reason` is copied to `startup_complete_reason` and kept only as
  diagnostic context via `last_reason` when the probe succeeds.
- Returned `startup_complete.outcome` is copied to `startup_complete_outcome` and is not currently
  used to affect startup behavior beyond detecting success.
- Successful startup replies now also carry
  `COMPLETE_TRACE[phase=startup|outcome=...|reason=...]` for searchable log output.
- Probe failure is surfaced externally as `definition_error` with
  `reason = startup_dependency_unavailable` and a failure marker
  `COMPLETE_TRACE[phase=startup|status=failure]`.

### Transition decision evaluation
- The selected transition prompt includes the current workflow item context (`workflow_name`, `stage`,
  `item_id`, `title`, `priority`, `owner`, `review_status`, and `transition_count`) as provided
  values so `complete(...)` evaluates the item instead of returning a canned decision.
- `workflow_name` remains canonical definition metadata; `evaluate_transition` messages do not
  overwrite it from caller-supplied input.
- Returned `transition_complete.outcome` becomes `transition_outcome` and drives the workflow branch:
  - `advance` moves to the transition's configured `to` stage
  - `reject` produces the parsed rejected terminal outcome
  - `stay` keeps the current stage and is retryable
- Returned `transition_complete.reason` becomes `transition_reason` and is forwarded in the
  `transition_decision` message so downstream methods can explain the decision in progress/summary
  logs.
- Outgoing decisions now also carry
  `COMPLETE_TRACE[phase=transition|outcome=...|reason=...]` so reporter output can be searched for
  the exact completion-derived values.
- `transition_ok` is derived from the copied `complete_present` marker plus generated
  `outcome`/`reason` fields after build/parse normalization.
- If `complete(...)` fails, the method preserves workflow continuity by emitting a retryable
  `stay` decision with `reason = complete_transition_failed`.

## Method Code

```agerun
memory.is_prepare := if(message.action = "prepare_definition", 1, 0)
memory.is_file_response := if(message.action = "definition_file", 1, 0)
memory.is_evaluate := if(message.action = "evaluate_transition", 1, 0)
memory.is_describe := if(message.action = "describe", 1, 0)
memory.self_agent_id := if(memory.is_prepare = 1, memory.self, memory.self_agent_id)
memory.reply_to_id := if(message.reply_to > 0, message.reply_to, memory.reply_to_id)
memory.definition_path := if(memory.is_prepare = 1, message.definition_path, memory.definition_path)
memory.workflow_name := if(memory.is_prepare = 1, "", memory.workflow_name)
memory.workflow_version := if(memory.is_prepare = 1, "", memory.workflow_version)
memory.initial_stage := if(memory.is_prepare = 1, "", memory.initial_stage)
memory.terminal_completed := if(memory.is_prepare = 1, "", memory.terminal_completed)
memory.terminal_rejected := if(memory.is_prepare = 1, "", memory.terminal_rejected)
memory.requires_local_completion := if(memory.is_prepare = 1, "", memory.requires_local_completion)
memory.item_fields := if(memory.is_prepare = 1, "", memory.item_fields)
memory.stages := if(memory.is_prepare = 1, "", memory.stages)
memory.transition_count := if(memory.is_prepare = 1, 0, memory.transition_count)
memory.transition_1_from := if(memory.is_prepare = 1, "", memory.transition_1_from)
memory.transition_1_to := if(memory.is_prepare = 1, "", memory.transition_1_to)
memory.transition_1_prompt := if(memory.is_prepare = 1, "", memory.transition_1_prompt)
memory.transition_2_from := if(memory.is_prepare = 1, "", memory.transition_2_from)
memory.transition_2_to := if(memory.is_prepare = 1, "", memory.transition_2_to)
memory.transition_2_prompt := if(memory.is_prepare = 1, "", memory.transition_2_prompt)
memory.transition_3_from := if(memory.is_prepare = 1, "", memory.transition_3_from)
memory.transition_3_to := if(memory.is_prepare = 1, "", memory.transition_3_to)
memory.transition_3_prompt := if(memory.is_prepare = 1, "", memory.transition_3_prompt)
memory.transition_4_from := if(memory.is_prepare = 1, "", memory.transition_4_from)
memory.transition_4_to := if(memory.is_prepare = 1, "", memory.transition_4_to)
memory.transition_4_prompt := if(memory.is_prepare = 1, "", memory.transition_4_prompt)
memory.file_status := if(memory.is_prepare = 1, "requested", memory.file_status)
memory.dependency_status := if(memory.is_prepare = 1, "", memory.dependency_status)
memory.last_reason := if(memory.is_prepare = 1, "", memory.last_reason)
memory.error_reason := ""
memory.error_category := ""
memory.startup_complete_trace := "none"
memory.startup_complete_outcome := ""
memory.startup_complete_reason := ""
memory.transition_complete_trace := "none"
memory.file_response_action := "definition_file"
memory.file_request_input := build("action=read path={definition_path} response_action={file_response_action} reply_to={self_agent_id}", memory)
memory.file_request_payload := parse("action={action} path={path} response_action={response_action} reply_to={reply_to}", memory.file_request_input)
memory.file_request_sent := send(-100 * memory.is_prepare, memory.file_request_payload)
memory.file_success_value := if(message.status = "success", 1, 0)
memory.file_success := memory.is_file_response * memory.file_success_value
memory.definition_content := if(memory.file_success = 1, message.content, "")
memory.definition_record := parse("workflow_name={workflow_name} workflow_version={workflow_version} initial_stage={initial_stage} terminal_completed={terminal_completed} terminal_rejected={terminal_rejected} requires_local_completion={requires_local_completion} item_fields={item_fields} stages={stages} transition_count={transition_count} transition_1_from={transition_1_from} transition_1_to={transition_1_to} transition_1_prompt={transition_1_prompt} transition_2_from={transition_2_from} transition_2_to={transition_2_to} transition_2_prompt={transition_2_prompt} transition_3_from={transition_3_from} transition_3_to={transition_3_to} transition_3_prompt={transition_3_prompt} transition_4_from={transition_4_from} transition_4_to={transition_4_to} transition_4_prompt={transition_4_prompt}", memory.definition_content)
memory.file_status := if(memory.file_success = 1, "loaded", memory.file_status)
memory.workflow_name := if(memory.file_success = 1, memory.definition_record.workflow_name, memory.workflow_name)
memory.workflow_version := if(memory.file_success = 1, memory.definition_record.workflow_version, memory.workflow_version)
memory.initial_stage := if(memory.file_success = 1, memory.definition_record.initial_stage, memory.initial_stage)
memory.terminal_completed := if(memory.file_success = 1, memory.definition_record.terminal_completed, memory.terminal_completed)
memory.terminal_rejected := if(memory.file_success = 1, memory.definition_record.terminal_rejected, memory.terminal_rejected)
memory.requires_local_completion := if(memory.file_success = 1, memory.definition_record.requires_local_completion, memory.requires_local_completion)
memory.item_fields := if(memory.file_success = 1, memory.definition_record.item_fields, memory.item_fields)
memory.stages := if(memory.file_success = 1, memory.definition_record.stages, memory.stages)
memory.transition_count := if(memory.file_success = 1, memory.definition_record.transition_count, memory.transition_count)
memory.transition_1_from := if(memory.file_success = 1, memory.definition_record.transition_1_from, memory.transition_1_from)
memory.transition_1_to := if(memory.file_success = 1, memory.definition_record.transition_1_to, memory.transition_1_to)
memory.transition_1_prompt := if(memory.file_success = 1, memory.definition_record.transition_1_prompt, memory.transition_1_prompt)
memory.transition_2_from := if(memory.file_success = 1, memory.definition_record.transition_2_from, memory.transition_2_from)
memory.transition_2_to := if(memory.file_success = 1, memory.definition_record.transition_2_to, memory.transition_2_to)
memory.transition_2_prompt := if(memory.file_success = 1, memory.definition_record.transition_2_prompt, memory.transition_2_prompt)
memory.transition_3_from := if(memory.file_success = 1, memory.definition_record.transition_3_from, memory.transition_3_from)
memory.transition_3_to := if(memory.file_success = 1, memory.definition_record.transition_3_to, memory.transition_3_to)
memory.transition_3_prompt := if(memory.file_success = 1, memory.definition_record.transition_3_prompt, memory.transition_3_prompt)
memory.transition_4_from := if(memory.file_success = 1, memory.definition_record.transition_4_from, memory.transition_4_from)
memory.transition_4_to := if(memory.file_success = 1, memory.definition_record.transition_4_to, memory.transition_4_to)
memory.transition_4_prompt := if(memory.file_success = 1, memory.definition_record.transition_4_prompt, memory.transition_4_prompt)
memory.has_workflow_name := if(memory.workflow_name = "", 0, 1)
memory.has_workflow_version := if(memory.workflow_version = "", 0, 1)
memory.has_initial_stage := if(memory.initial_stage = "", 0, 1)
memory.has_terminal_completed := if(memory.terminal_completed = "", 0, 1)
memory.has_terminal_rejected := if(memory.terminal_rejected = "", 0, 1)
memory.has_item_fields := if(memory.item_fields = "", 0, 1)
memory.has_stages := if(memory.stages = "", 0, 1)
memory.has_transition_count := if(memory.transition_count = 4, 1, 0)
memory.has_transition_1_from := if(memory.transition_1_from = "", 0, 1)
memory.has_transition_1_to := if(memory.transition_1_to = "", 0, 1)
memory.has_transition_1_prompt := if(memory.transition_1_prompt = "", 0, 1)
memory.has_transition_1 := memory.has_transition_1_from * memory.has_transition_1_to
memory.has_transition_1 := memory.has_transition_1 * memory.has_transition_1_prompt
memory.has_transition_2_from := if(memory.transition_2_from = "", 0, 1)
memory.has_transition_2_to := if(memory.transition_2_to = "", 0, 1)
memory.has_transition_2_prompt := if(memory.transition_2_prompt = "", 0, 1)
memory.has_transition_2 := memory.has_transition_2_from * memory.has_transition_2_to
memory.has_transition_2 := memory.has_transition_2 * memory.has_transition_2_prompt
memory.has_transition_3_from := if(memory.transition_3_from = "", 0, 1)
memory.has_transition_3_to := if(memory.transition_3_to = "", 0, 1)
memory.has_transition_3_prompt := if(memory.transition_3_prompt = "", 0, 1)
memory.has_transition_3 := memory.has_transition_3_from * memory.has_transition_3_to
memory.has_transition_3 := memory.has_transition_3 * memory.has_transition_3_prompt
memory.has_transition_4_from := if(memory.transition_4_from = "", 0, 1)
memory.has_transition_4_to := if(memory.transition_4_to = "", 0, 1)
memory.has_transition_4_prompt := if(memory.transition_4_prompt = "", 0, 1)
memory.has_transition_4 := memory.has_transition_4_from * memory.has_transition_4_to
memory.has_transition_4 := memory.has_transition_4 * memory.has_transition_4_prompt
memory.metadata_ok := memory.has_workflow_name * memory.has_workflow_version
memory.metadata_ok := memory.metadata_ok * memory.has_initial_stage
memory.metadata_ok := memory.metadata_ok * memory.has_terminal_completed
memory.metadata_ok := memory.metadata_ok * memory.has_terminal_rejected
memory.metadata_ok := memory.metadata_ok * memory.has_item_fields
memory.metadata_ok := memory.metadata_ok * memory.has_stages
memory.transitions_ok := memory.has_transition_count * memory.has_transition_1
memory.transitions_ok := memory.transitions_ok * memory.has_transition_2
memory.transitions_ok := memory.transitions_ok * memory.has_transition_3
memory.transitions_ok := memory.transitions_ok * memory.has_transition_4
memory.schema_ok := memory.file_success * memory.metadata_ok
memory.schema_ok := memory.schema_ok * memory.transitions_ok
memory.complete_presence_input := "complete_present=1"
memory.complete_presence := parse("complete_present={complete_present}", memory.complete_presence_input)
memory.startup_result := complete("Answer with outcome ready and reason ok. The dependency probe result is {outcome}. The short reason is {reason}.", memory.complete_presence)
memory.probe_check_input := build("complete_present={complete_present} outcome={outcome} reason={reason}", memory.startup_result)
memory.probe_check := parse("complete_present={complete_present} outcome={outcome} reason={reason}", memory.probe_check_input)
memory.probe_has_marker := if(memory.probe_check.complete_present = 1, 1, 0)
memory.probe_has_outcome := if(memory.probe_check.outcome = "{outcome}", 0, 1)
memory.probe_has_reason := if(memory.probe_check.reason = "{reason}", 0, 1)
memory.probe_ok := memory.probe_has_marker * memory.probe_has_outcome
memory.probe_ok := memory.probe_ok * memory.probe_has_reason
memory.startup_complete_outcome := if(memory.probe_ok = 1, memory.probe_check.outcome, memory.startup_complete_outcome)
memory.startup_complete_reason := if(memory.probe_ok = 1, memory.probe_check.reason, memory.startup_complete_reason)
memory.dependency_status := if(memory.probe_ok = 1, "ready", memory.dependency_status)
memory.ready_flag := memory.schema_ok * memory.probe_ok
memory.probe_failed_value := if(memory.probe_ok = 1, 0, 1)
memory.probe_failed_flag := memory.schema_ok * memory.probe_failed_value
memory.file_response_flag := memory.is_file_response
memory.not_ready_flag := if(memory.ready_flag = 1, 0, 1)
memory.error_flag := memory.file_response_flag * memory.not_ready_flag
memory.error_reason := if(message.action = "definition_file", "invalid_definition_schema", memory.error_reason)
memory.error_reason := if(memory.dependency_status = "ready", memory.error_reason, "startup_dependency_unavailable")
memory.error_category := if(memory.dependency_status = "ready", "", "runtime_unavailable")
memory.startup_complete_trace_input := build("COMPLETE_TRACE[phase=startup|outcome={startup_complete_outcome}|reason={startup_complete_reason}]", memory)
memory.startup_complete_trace := if(memory.probe_ok = 1, memory.startup_complete_trace_input, memory.startup_complete_trace)
memory.startup_complete_trace := if(memory.probe_failed_flag = 1, "COMPLETE_TRACE[phase=startup|status=failure]", memory.startup_complete_trace)
memory.last_reason := if(memory.ready_flag = 1, memory.startup_complete_reason, memory.last_reason)
memory.last_reason := if(memory.error_flag = 1, memory.error_reason, memory.last_reason)
memory.ready_input := build("action=definition_ready workflow_name={workflow_name} workflow_version={workflow_version} initial_stage={initial_stage} terminal_completed={terminal_completed} terminal_rejected={terminal_rejected} requires_local_completion={requires_local_completion} item_fields={item_fields} transition_count={transition_count} complete_trace={startup_complete_trace}", memory)
memory.ready_payload := parse("action={action} workflow_name={workflow_name} workflow_version={workflow_version} initial_stage={initial_stage} terminal_completed={terminal_completed} terminal_rejected={terminal_rejected} requires_local_completion={requires_local_completion} item_fields={item_fields} transition_count={transition_count} complete_trace={complete_trace}", memory.ready_input)
memory.ready_sent := send(memory.reply_to_id * memory.ready_flag, memory.ready_payload)
memory.error_input := build("action=definition_error reason={error_reason} failure_category={error_category} complete_trace={startup_complete_trace}", memory)
memory.error_payload := parse("action={action} reason={reason} failure_category={failure_category} complete_trace={complete_trace}", memory.error_input)
memory.error_target := memory.reply_to_id * memory.error_flag
memory.error_sent := send(memory.error_target, memory.error_payload)
memory.from_stage := if(memory.is_evaluate = 1, message.stage, "")
memory.item_id := if(memory.is_evaluate = 1, message.item_id, "")
memory.title := if(memory.is_evaluate = 1, message.title, "")
memory.priority := if(memory.is_evaluate = 1, message.priority, "")
memory.owner := if(memory.is_evaluate = 1, message.owner, "")
memory.review_status := if(memory.is_evaluate = 1, message.review_status, "")
memory.request_transition_count := if(memory.is_evaluate = 1, message.transition_count, 0)
memory.transition_prompt := ""
memory.configured_to_stage := ""
memory.transition_prompt := if(memory.from_stage = memory.transition_1_from, memory.transition_1_prompt, memory.transition_prompt)
memory.configured_to_stage := if(memory.from_stage = memory.transition_1_from, memory.transition_1_to, memory.configured_to_stage)
memory.transition_prompt := if(memory.from_stage = memory.transition_2_from, memory.transition_2_prompt, memory.transition_prompt)
memory.configured_to_stage := if(memory.from_stage = memory.transition_2_from, memory.transition_2_to, memory.configured_to_stage)
memory.transition_prompt := if(memory.from_stage = memory.transition_3_from, memory.transition_3_prompt, memory.transition_prompt)
memory.configured_to_stage := if(memory.from_stage = memory.transition_3_from, memory.transition_3_to, memory.configured_to_stage)
memory.transition_prompt := if(memory.from_stage = memory.transition_4_from, memory.transition_4_prompt, memory.transition_prompt)
memory.configured_to_stage := if(memory.from_stage = memory.transition_4_from, memory.transition_4_to, memory.configured_to_stage)
memory.has_configured_transition := if(memory.transition_prompt = "", 0, 1)
memory.has_configured_to_stage := if(memory.configured_to_stage = "", 0, 1)
memory.has_configured_transition := memory.has_configured_transition * memory.has_configured_to_stage
memory.transition_context_input := build("complete_present=1 workflow_name={workflow_name} stage={from_stage} item_id={item_id} title={title} priority={priority} owner={owner} review_status={review_status} transition_count={request_transition_count}", memory)
memory.transition_context := parse("complete_present={complete_present} workflow_name={workflow_name} stage={stage} item_id={item_id} title={title} priority={priority} owner={owner} review_status={review_status} transition_count={transition_count}", memory.transition_context_input)
memory.transition_result := complete(memory.transition_prompt, memory.transition_context)
memory.transition_check_input := build("complete_present={complete_present} outcome={outcome} reason={reason}", memory.transition_result)
memory.transition_check := parse("complete_present={complete_present} outcome={outcome} reason={reason}", memory.transition_check_input)
memory.transition_has_marker := if(memory.transition_check.complete_present = 1, 1, 0)
memory.transition_has_outcome := if(memory.transition_check.outcome = "{outcome}", 0, 1)
memory.transition_has_reason := if(memory.transition_check.reason = "{reason}", 0, 1)
memory.transition_outcome_allowed := if(memory.transition_check.outcome = "advance", 1, 0)
memory.transition_outcome_allowed := if(memory.transition_check.outcome = "stay", 1, memory.transition_outcome_allowed)
memory.transition_outcome_allowed := if(memory.transition_check.outcome = "reject", 1, memory.transition_outcome_allowed)
memory.transition_ok := memory.transition_has_marker * memory.transition_has_outcome
memory.transition_ok := memory.transition_ok * memory.transition_has_reason
memory.transition_ok := memory.transition_ok * memory.transition_outcome_allowed
memory.transition_ok := memory.transition_ok * memory.has_configured_transition
memory.transition_ok := memory.transition_ok * memory.is_evaluate
memory.transition_outcome := if(memory.transition_ok = 1, memory.transition_check.outcome, "stay")
memory.transition_reason := if(memory.transition_ok = 1, memory.transition_check.reason, "complete_transition_failed")
memory.next_stage := if(memory.transition_outcome = "advance", memory.configured_to_stage, memory.from_stage)
memory.next_stage := if(memory.transition_outcome = "stay", memory.from_stage, memory.next_stage)
memory.next_stage := if(memory.transition_outcome = "reject", memory.from_stage, memory.next_stage)
memory.stay_status := build("{from_stage}_waiting", memory)
memory.transition_status := if(memory.transition_outcome = "advance", memory.next_stage, "")
memory.transition_status := if(memory.transition_outcome = "stay", memory.stay_status, memory.transition_status)
memory.transition_status := if(memory.transition_outcome = "reject", memory.terminal_rejected, memory.transition_status)
memory.retryable := if(memory.transition_outcome = "stay", 1, 0)
memory.terminal_outcome := if(memory.transition_outcome = "reject", memory.terminal_rejected, "")
memory.is_completed_stage := if(memory.next_stage = memory.transition_4_to, 1, 0)
memory.completed_terminal_value := if(memory.is_completed_stage = 1, memory.terminal_completed, "")
memory.terminal_outcome := if(memory.transition_outcome = "advance", memory.completed_terminal_value, memory.terminal_outcome)
memory.transition_complete_trace_input := build("COMPLETE_TRACE[phase=transition|outcome={transition_outcome}|reason={transition_reason}]", memory)
memory.transition_complete_trace := if(memory.is_evaluate = 1, memory.transition_complete_trace_input, memory.transition_complete_trace)
memory.transition_input := build("action=transition_decision workflow_name={workflow_name} from_stage={from_stage} outcome={transition_outcome} next_stage={next_stage} status={transition_status} reason={transition_reason} retryable={retryable} terminal_outcome={terminal_outcome} note={transition_reason} complete_trace={transition_complete_trace}", memory)
memory.transition_payload := parse("action={action} workflow_name={workflow_name} from_stage={from_stage} outcome={outcome} next_stage={next_stage} status={status} reason={reason} retryable={retryable} terminal_outcome={terminal_outcome} note={note} complete_trace={complete_trace}", memory.transition_input)
memory.transition_sent := send(memory.reply_to_id * memory.is_evaluate, memory.transition_payload)
memory.transitions_description := build("{transition_1_from}->{transition_1_to}|{transition_2_from}->{transition_2_to}|{transition_3_from}->{transition_3_to}|{transition_4_from}->{transition_4_to}", memory)
memory.describe_input := build("action=describe_result workflow_name={workflow_name} workflow_version={workflow_version} initial_stage={initial_stage} terminal_completed={terminal_completed} terminal_rejected={terminal_rejected} requires_local_completion={requires_local_completion} item_fields={item_fields} stages={stages} transition_count={transition_count} transitions={transitions_description}", memory)
memory.describe_payload := parse("action={action} workflow_name={workflow_name} workflow_version={workflow_version} initial_stage={initial_stage} terminal_completed={terminal_completed} terminal_rejected={terminal_rejected} requires_local_completion={requires_local_completion} item_fields={item_fields} stages={stages} transition_count={transition_count} transitions={transitions}", memory.describe_input)
memory.describe_sent := send(memory.reply_to_id * memory.is_describe, memory.describe_payload)

```

## Testing

Validated by `methods/workflow_definition_tests.c`.

The tests cover:
- supported default/test workflow path loading
- invalid schema rejection
- malformed transition metadata rejection
- startup dependency probe success/failure
- `describe` responses
- configured transition prompt usage for intake, triage, active, and review
- normalized `transition_decision` replies for `advance`, `stay`, `reject`
- retryable `stay` conversion when `complete(...)` fails
