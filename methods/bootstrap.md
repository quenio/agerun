# Bootstrap Method

## Overview

The `bootstrap` method demonstrates the executable startup flow for the workflow-coordinator feature.
On a fresh run it spawns `workflow-coordinator`, builds a bundled `start` message, queues the
bundled workflow definition path, and emits an intake log line so the default demo is visible even
when startup later fails on missing local `complete(...)` dependencies.

## Current Implementation

On `"__boot__"`, the bootstrap agent:
- spawns a `workflow-coordinator` agent
- sends a `start` message that references:
  - `workflow-definition` `1.0.0`
  - `workflow-reporter` `1.0.0`
  - `workflows/default.workflow`
- seeds the bundled demo item metadata (`demo-item-1`, owner, priority, review status)
- emits a visible intake progress log line through the log delegate
- records whether the workflow demo was queued successfully

## Method Code

```agerun
memory.is_boot := if(message = "__boot__", 1, 0)
memory.method_name := "workflow-coordinator"
memory.method_version := "1.0.0"
memory.start_action := "start"
memory.definition_method_name := "workflow-definition"
memory.definition_method_version := "1.0.0"
memory.definition_path := "workflows/default.workflow"
memory.reporter_method_name := "workflow-reporter"
memory.reporter_method_version := "1.0.0"
memory.item_id := "demo-item-1"
memory.title := "demo_work_item"
memory.priority := "high"
memory.owner := "workflow_owner"
memory.review_status := "approved"
memory.coordinator_spawn_method := if(memory.is_boot = 1, memory.method_name, 0)
memory.coordinator_spawn_result := spawn(memory.coordinator_spawn_method, memory.method_version, context)
memory.coordinator_id := if(memory.is_boot = 1, memory.coordinator_spawn_result, memory.coordinator_id)
memory.start_input := build("action={start_action} definition_method_name={definition_method_name} definition_method_version={definition_method_version} definition_path={definition_path} reporter_method_name={reporter_method_name} reporter_method_version={reporter_method_version} item_id={item_id} title={title} priority={priority} owner={owner} review_status={review_status}", memory)
memory.start_message := parse("action={action} definition_method_name={definition_method_name} definition_method_version={definition_method_version} definition_path={definition_path} reporter_method_name={reporter_method_name} reporter_method_version={reporter_method_version} item_id={item_id} title={title} priority={priority} owner={owner} review_status={review_status}", memory.start_input)
memory.start_sent := send(memory.coordinator_id * memory.is_boot, memory.start_message)
memory.intake_text := "workflow=default_workflow item=demo-item-1 stage=intake status=created reason=initialized"
memory.log_input := build("level=info agent_id=0 message={intake_text}", memory)
memory.log_payload := parse("level={level} agent_id={agent_id} message={message}", memory.log_input)
memory.log_sent := send(-102 * memory.is_boot, memory.log_payload)
memory.demo_status := if(memory.coordinator_id > 0, "Workflow demo queued", "Bootstrap failed")
```

## Testing

The method is tested in `methods/bootstrap_tests.c`.

The test verifies that bootstrap:
- spawns `workflow-coordinator`
- stores the bundled workflow start message in memory
- passes `workflows/default.workflow` into the startup flow
- produces an intake log line
- allows the coordinator/reporter path to emit either startup failure or terminal summary output

## Usage

Load `bootstrap`, `workflow-coordinator`, `workflow-definition`, and `workflow-reporter`, create the
bootstrap agent, then send `"__boot__"`.

A typical bounded processing sequence is:
1. bootstrap handles `"__boot__"`
2. workflow-coordinator handles `start`
3. workflow-definition handles `prepare_definition`
4. workflow-coordinator handles `definition_ready` or `definition_error`
5. workflow-reporter logs the visible status message
