# workflow-coordinator-1.0.0

## Overview

`workflow-coordinator` is the boot-time orchestrator for the bundled workflow demo. It accepts the
startup `start` message from `bootstrap`, spawns the supporting `workflow-definition` and
`workflow-reporter` agents, waits for definition readiness, and then either:

- records a successful handoff and initializes `workflow-item`, which emits progress and final
  summary events through `workflow-reporter`, or
- reports a startup dependency/definition failure without creating a fake work item.

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
  message("action") = "start" or
  message("action") = "definition_ready" or
  message("action") = "definition_error"

REQUIRES_START_MESSAGE_IS_COMPLETE:
  message("action") = "start" =>
    final_memory("self") > 0 and
    not (message("definition_method_name") = "") and
    not (message("definition_method_version") = "") and
    not (message("definition_path") = "") and
    not (message("reporter_method_name") = "") and
    not (message("reporter_method_version") = "") and
    not (message("item_id") = "") and
    not (message("title") = "") and
    not (message("priority") = "") and
    not (message("owner") = "") and
    not (message("review_status") = "")

REQUIRES_READY_MESSAGE_IS_COMPLETE:
  message("action") = "definition_ready" =>
    not (message("workflow_name") = "") and
    not (message("initial_stage") = "")

REQUIRES_ERROR_MESSAGE_IS_COMPLETE:
  message("action") = "definition_error" =>
    not (message("reason") = "")

ENSURES_START_SPAWNS_SUPPORT_AGENTS:
  message("action") = "start" =>
    final_memory("definition_agent_id") > 0 and
    final_memory("reporter_agent_id") > 0

ENSURES_START_PREPARES_THE_DEFINITION:
  message("action") = "start" =>
    final_memory("prepare_sent") = 1 and
    final_memory("run_status") = "waiting_for_definition"

ENSURES_READY_ACTIVATES_THE_RUN:
  message("action") = "definition_ready" =>
    final_memory("run_status") = "active" and
    final_memory("item_agent_id") > 0 and
    final_memory("initialize_sent") = 1

ENSURES_ERROR_FAILS_WITHOUT_ITEM_CREATION:
  message("action") = "definition_error" =>
    final_memory("run_status") = "startup_failed" and
    final_memory("item_agent_id") = 0 and
    final_memory("startup_sent") = 1
```

## Inputs

When the coordinator agent is created, the agency sets `memory.self` to the coordinator agent ID.

### `action=start`

Expected fields:
- `definition_method_name`
- `definition_method_version`
- `definition_path`
- `reporter_method_name`
- `reporter_method_version`
- `item_id`
- `title`
- `priority`
- `owner`
- `review_status`

On `start`, the coordinator:
- stores the workflow startup metadata in memory
- spawns `workflow-definition` and `workflow-reporter`
- sends `prepare_definition` to the definition agent
- moves to `run_status = "waiting_for_definition"`

### `action=definition_ready`

Expected fields:
- `workflow_name`
- `initial_stage`
- optional `complete_trace`

On `definition_ready`, the coordinator:
- records the active workflow metadata
- preserves the startup `complete_trace` marker from `workflow-definition`
- marks the run `active`
- spawns `workflow-item`
- sends an `initialize` message to the item with `reply_to=<coordinator_id>`; the system supplies the item's `self` field when the message is delivered

### `action=definition_error`

Expected fields:
- `reason`
- optional `failure_category`
- optional `complete_trace`

On `definition_error`, the coordinator:
- marks the run `startup_failed`
- preserves the startup failure reason/category
- preserves any startup `complete_trace` marker from `workflow-definition`
- sends a `startup_failure` message to `workflow-reporter`
- leaves `item_agent_id = 0`

## Current Implementation Notes

The current implementation runs the bundled workflow through the full item lifecycle:

- it stages definition readiness before item creation
- it suppresses item creation on startup failure
- its success path spawns `workflow-item` and sends an `initialize` message
- it uses the agency-managed `memory.self` value to initialize the coordinator's own ID and `reply_to` only for definition replies
- final visible summaries come from `workflow-item` after `workflow-definition` returns a transition decision
- executable behavior is validated through visible log output rather than by inspecting internal
  scheduler details

## Method Code

```agerun
memory.is_start := if(message.action = "start", 1, 0)
memory.is_definition_ready := if(message.action = "definition_ready", 1, 0)
memory.is_definition_error := if(message.action = "definition_error", 1, 0)
memory.prepare_action := "prepare_definition"
memory.initialize_action := "initialize"
memory.startup_action := "startup_failure"
memory.item_method_version := "1.0.0"
memory.initialize_workflow_name := ""
memory.initialize_initial_stage := ""
memory.startup_failure_reason := ""
memory.failure_category := ""
memory.complete_trace := "none"
memory.item_agent_id := 0
memory.should_launch := memory.is_start
memory.self_agent_id := if(memory.should_launch = 1, memory.self, memory.self_agent_id)
memory.definition_method_name := if(memory.should_launch = 1, message.definition_method_name, memory.definition_method_name)
memory.definition_method_version := if(memory.should_launch = 1, message.definition_method_version, memory.definition_method_version)
memory.reporter_method_name := if(memory.should_launch = 1, message.reporter_method_name, memory.reporter_method_name)
memory.reporter_method_version := if(memory.should_launch = 1, message.reporter_method_version, memory.reporter_method_version)
memory.definition_path := if(memory.should_launch = 1, message.definition_path, memory.definition_path)
memory.item_id := if(memory.should_launch = 1, message.item_id, memory.item_id)
memory.title := if(memory.should_launch = 1, message.title, memory.title)
memory.priority := if(memory.should_launch = 1, message.priority, memory.priority)
memory.owner := if(memory.should_launch = 1, message.owner, memory.owner)
memory.review_status := if(memory.should_launch = 1, message.review_status, memory.review_status)
memory.definition_spawn_method := if(memory.should_launch = 1, memory.definition_method_name, 0)
memory.reporter_spawn_method := if(memory.should_launch = 1, memory.reporter_method_name, 0)
memory.definition_spawn_result := spawn(memory.definition_spawn_method, memory.definition_method_version, context)
memory.reporter_spawn_result := spawn(memory.reporter_spawn_method, memory.reporter_method_version, context)
memory.definition_agent_id := if(memory.should_launch = 1, memory.definition_spawn_result, memory.definition_agent_id)
memory.reporter_agent_id := if(memory.should_launch = 1, memory.reporter_spawn_result, memory.reporter_agent_id)
memory.run_status := if(memory.should_launch = 1, "waiting_for_definition", memory.run_status)
memory.stage := ""
memory.reply_to := memory.self_agent_id
memory.prepare_input := build("action={prepare_action} definition_path={definition_path} stage={stage} review_status={review_status} reply_to={reply_to}", memory)
memory.prepare_payload := parse("action={action} definition_path={definition_path} stage={stage} review_status={review_status} reply_to={reply_to}", memory.prepare_input)
memory.prepare_target := memory.definition_agent_id * memory.should_launch
memory.prepare_sent := send(memory.prepare_target, memory.prepare_payload)
memory.demo_status := if(memory.should_launch = 1, "definition_preparation_started", memory.demo_status)
memory.initialize_workflow_name := if(memory.is_definition_ready = 1, message.workflow_name, memory.initialize_workflow_name)
memory.initialize_initial_stage := if(memory.is_definition_ready = 1, message.initial_stage, memory.initialize_initial_stage)
memory.complete_trace := if(memory.is_definition_ready = 1, message.complete_trace, memory.complete_trace)
memory.item_spawn_method := if(memory.is_definition_ready = 1, "workflow-item", 0)
memory.item_spawn_result := spawn(memory.item_spawn_method, memory.item_method_version, context)
memory.item_agent_id := if(memory.is_definition_ready = 1, memory.item_spawn_result, memory.item_agent_id)
memory.run_status := if(memory.is_definition_ready = 1, "active", memory.run_status)
memory.initialize_input := build("action={initialize_action} reply_to={self_agent_id} workflow_name={initialize_workflow_name} item_id={item_id} title={title} priority={priority} owner={owner} review_status={review_status} definition_agent_id={definition_agent_id} reporter_agent_id={reporter_agent_id} initial_stage={initialize_initial_stage}", memory)
memory.initialize_payload := parse("action={action} reply_to={reply_to} workflow_name={workflow_name} item_id={item_id} title={title} priority={priority} owner={owner} review_status={review_status} definition_agent_id={definition_agent_id} reporter_agent_id={reporter_agent_id} initial_stage={initial_stage}", memory.initialize_input)
memory.initialize_sent := send(memory.item_agent_id * memory.is_definition_ready, memory.initialize_payload)
memory.demo_status := if(memory.is_definition_ready = 1, "workflow_item_initialized", memory.demo_status)
memory.run_status := if(memory.is_definition_error = 1, "startup_failed", memory.run_status)
memory.startup_failure_reason := if(memory.is_definition_error = 1, message.reason, memory.startup_failure_reason)
memory.failure_category := if(memory.is_definition_error = 1, message.failure_category, memory.failure_category)
memory.complete_trace := if(memory.is_definition_error = 1, message.complete_trace, memory.complete_trace)
memory.startup_input := build("action={startup_action} reason={startup_failure_reason} failure_category={failure_category} complete_trace={complete_trace}", memory)
memory.startup_payload := parse("action={action} reason={reason} failure_category={failure_category} complete_trace={complete_trace}", memory.startup_input)
memory.startup_target := memory.reporter_agent_id * memory.is_definition_error
memory.startup_sent := send(memory.startup_target, memory.startup_payload)
memory.demo_status := if(memory.is_definition_error = 1, message.reason, memory.demo_status)
```

## Testing

Validated by `methods/workflow_coordinator_tests.c`.

The tests verify that the coordinator:
- waits for `definition_ready` before spawning `workflow-item`
- sends startup work to `workflow-definition`
- transitions to `run_status = "active"` on `definition_ready`
- transitions to `run_status = "startup_failed"` on `definition_error`
- skips item creation on startup failure
- produces visible reporter/log output for both success and failure paths
- runs the real local completion matrix through coordinator, definition, item, and reporter for
  `review_status = approved`, `pending`, and `blocked`
