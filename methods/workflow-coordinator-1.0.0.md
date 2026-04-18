# workflow-coordinator-1.0.0

## Overview

`workflow-coordinator` is the boot-time orchestrator for the bundled workflow demo. It accepts the
startup `start` message from `bootstrap`, spawns the supporting `workflow-definition` and
`workflow-reporter` agents, waits for definition readiness, and then either:

- records a successful handoff and emits a final summary through `workflow-reporter`, or
- reports a startup dependency/definition failure without creating a fake work item.

## ATN Specification

The following ATN specification describes the observable contract of this method in terms of
accepted inputs and required coordinator state relationships. It is intentionally declarative: it
states what must be true of a valid coordinator state, not how the method computes that state.

```haskell
start_received: Boolean
ready_received: Boolean
error_received: Boolean

definition_agent_spawned: Boolean
reporter_agent_spawned: Boolean
item_agent_created: Boolean
prepare_definition_sent: Boolean
summary_sent: Boolean
startup_failure_sent: Boolean

run_status: String
startup_failure_reason: String

START_REQUIRES_SUPPORT_AGENTS:
  start_received => definition_agent_spawned and reporter_agent_spawned

START_QUEUES_DEFINITION_PREPARATION:
  start_received => prepare_definition_sent

START_ENTERS_WAITING_STATE:
  start_received => run_status = "waiting_for_definition"

READY_AND_ERROR_ARE_MUTUALLY_EXCLUSIVE:
  not (ready_received and error_received)

READINESS_REQUIRES_A_STARTED_RUN:
  ready_received => start_received

READINESS_ACTIVATES_THE_RUN:
  ready_received => run_status = "active" and item_agent_created and summary_sent

STARTUP_FAILURE_REQUIRES_A_STARTED_RUN:
  error_received => start_received

STARTUP_FAILURE_SUPPRESSES_ITEM_CREATION:
  error_received => run_status = "startup_failed" and not item_agent_created

STARTUP_FAILURE_IS_REPORTED:
  error_received => startup_failure_sent
```

## Inputs

### `action=start`

Expected fields:
- `sender`
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

On `definition_ready`, the coordinator:
- records the active workflow metadata
- marks the run `active`
- records a non-zero `item_agent_id`
- sends a `summary` message to `workflow-reporter`

### `action=definition_error`

Expected fields:
- `reason`
- optional `failure_category`

On `definition_error`, the coordinator:
- marks the run `startup_failed`
- preserves the startup failure reason/category
- sends a `startup_failure` message to `workflow-reporter`
- leaves `item_agent_id = 0`

## Current Implementation Notes

The current implementation is intentionally narrow and optimized for the bundled executable demo:

- it stages definition readiness before item creation
- it suppresses item creation on startup failure
- its success path currently sends a summary directly to `workflow-reporter`
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
memory.item_agent_id := 0
memory.should_launch := memory.is_start
memory.self_agent_id := if(memory.should_launch = 1, message.sender, memory.self_agent_id)
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
memory.sender := memory.self_agent_id
memory.prepare_input := build("action={prepare_action} definition_path={definition_path} stage={stage} review_status={review_status} sender={sender}", memory)
memory.prepare_payload := parse("action={action} definition_path={definition_path} stage={stage} review_status={review_status} sender={sender}", memory.prepare_input)
memory.prepare_target := memory.definition_agent_id * memory.should_launch
memory.prepare_sent := send(memory.prepare_target, memory.prepare_payload)
memory.demo_status := if(memory.should_launch = 1, "definition_preparation_started", memory.demo_status)
memory.initialize_workflow_name := if(memory.is_definition_ready = 1, message.workflow_name, memory.initialize_workflow_name)
memory.initialize_initial_stage := if(memory.is_definition_ready = 1, message.initial_stage, memory.initialize_initial_stage)
memory.item_agent_id := if(memory.is_definition_ready = 1, memory.reporter_agent_id + 1, memory.item_agent_id)
memory.run_status := if(memory.is_definition_ready = 1, "active", memory.run_status)
memory.summary_stage := if(memory.review_status = "approved", "completion", "review")
memory.summary_terminal := if(memory.review_status = "approved", "completed", "rejected")
memory.summary_reason := if(memory.review_status = "approved", "approved", "policy_rejected")
memory.summary_status := if(memory.review_status = "approved", "completed", "rejected")
memory.summary_text := build("workflow={initialize_workflow_name} item={item_id} stage={summary_stage} terminal={summary_terminal} reason={summary_reason}", memory)
memory.summary_input := build("action=summary workflow_name={initialize_workflow_name} item_id={item_id} stage={summary_stage} status={summary_status} owner={owner} transition_count=4 terminal_outcome={summary_terminal} reason={summary_reason} text={summary_text}", memory)
memory.summary_payload := parse("action={action} workflow_name={workflow_name} item_id={item_id} stage={stage} status={status} owner={owner} transition_count={transition_count} terminal_outcome={terminal_outcome} reason={reason} text={text}", memory.summary_input)
memory.summary_target := memory.reporter_agent_id * memory.is_definition_ready
memory.summary_sent := send(memory.summary_target, memory.summary_payload)
memory.demo_status := if(memory.is_definition_ready = 1, "workflow_item_initialized", memory.demo_status)
memory.run_status := if(memory.is_definition_error = 1, "startup_failed", memory.run_status)
memory.startup_failure_reason := if(memory.is_definition_error = 1, message.reason, memory.startup_failure_reason)
memory.failure_category := if(memory.is_definition_error = 1, message.failure_category, memory.failure_category)
memory.startup_input := build("action={startup_action} reason={startup_failure_reason} failure_category={failure_category}", memory)
memory.startup_payload := parse("action={action} reason={reason} failure_category={failure_category}", memory.startup_input)
memory.startup_target := memory.reporter_agent_id * memory.is_definition_error
memory.startup_sent := send(memory.startup_target, memory.startup_payload)
memory.demo_status := if(memory.is_definition_error = 1, message.reason, memory.demo_status)
```

## Testing

Validated by `methods/workflow_coordinator_tests.c`.

The tests verify that the coordinator:
- waits for `definition_ready` before recording item creation
- sends startup work to `workflow-definition`
- transitions to `run_status = "active"` on `definition_ready`
- transitions to `run_status = "startup_failed"` on `definition_error`
- skips item creation on startup failure
- produces visible reporter/log output for both success and failure paths
