# Data Model: Workflow Coordinator

This document records the currently implemented runtime data model for the workflow coordinator
feature.

## 1. Bootstrap Demo Context

### Description
The startup context assembled by `bootstrap-1.0.0.method` before it queues the bundled workflow
start message.

### Key Attributes
- `method_name = workflow-coordinator`
- `method_version = 1.0.0`
- `definition_method_name = workflow-definition`
- `definition_method_version = 1.0.0`
- `definition_path = workflows/default-workflow.yaml`
- `reporter_method_name = workflow-reporter`
- `reporter_method_version = 1.0.0`
- `item_id = demo-item-1`
- `title = demo_work_item`
- `priority = high`
- `owner = workflow_owner`
- `review_status = approved`

## 2. Workflow Coordinator Agent Memory

### Description
The coordinator owns the boot-time orchestration state.

### Key Attributes
- `definition_agent_id`
- `reporter_agent_id`
- `item_agent_id`
- `run_status`
- `demo_status`
- `startup_failure_reason`
- `failure_category`
- `initialize_workflow_name`
- `initialize_initial_stage`

### Implemented States
- `waiting_for_definition`
- `active`
- `startup_failed`

## 3. Workflow Definition Agent Memory

### Description
The definition agent stores the recognized definition identity and normalized decision state.

### Key Attributes
- `definition_path`
- `workflow_name`
- `workflow_version`
- `initial_stage`
- `terminal_completed`
- `terminal_rejected`
- `requires_local_completion`
- `file_status`
- `dependency_status`
- `validation_clause`
- `last_reason`
- `last_reply_action`
- `transition_outcome`
- `transition_reason`
- `retryable`
- `terminal_outcome`

## 4. Workflow Item Agent Memory

### Description
The workflow-item method owns reusable lifecycle state for the method-level tests.

### Key Attributes
- `workflow_name`
- `item_id`
- `title`
- `priority`
- `owner`
- `review_status`
- `current_stage`
- `current_status`
- `transition_count`
- `terminal_outcome`
- `last_reason`
- `summary`
- `definition_agent_id`
- `reporter_agent_id`

## 5. Workflow Reporter Agent Memory

### Description
The reporter tracks the last emitted visible event.

### Key Attributes
- `last_event_type`
- `last_item_id`
- `last_reason`
- `last_message`
- `delivery_status`

## 6. Definition Files

### Description
The current implementation treats the files under `workflows/` as path-selected definition assets.

### Supported Paths
- `workflows/default-workflow.yaml`
- `workflows/test-workflow.yaml`
- `invalid-workflow.yaml` (invalid-schema fixture)

### Implemented Attributes Exposed by the Definition Agent
- `workflow_name`
- `workflow_version`
- `initial_stage`
- `terminal_completed`
- `terminal_rejected`
- `requires_local_completion`
- `item_fields`
- `stages`
- `validation_clause`

## 7. Visible Output Events

### Description
The externally visible runtime output is emitted through the existing log delegate.

### Event Shapes
- intake progress from bootstrap
- summary from coordinator success path
- startup failure from coordinator failure path
- progress/summary from workflow-item and workflow-reporter method tests

## Relationships

- one bootstrap demo context starts one coordinator agent
- one coordinator spawns one definition agent and one reporter agent
- one definition agent returns either `definition_ready` or `definition_error`
- one coordinator emits either a success summary handoff or a startup failure handoff
- one reporter emits visible log output

## Notes

- This reflects the current implementation, not the earlier broader design target.
- The workflow-item method remains implemented and tested independently even though the current
  executable startup path uses a simplified coordinator success handoff.
