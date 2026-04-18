# Contract: Workflow Runtime Messages

## Purpose

Define the currently implemented message protocol among the workflow bootstrap methods.

## Participants

- `bootstrap`
- `workflow-coordinator`
- `workflow-definition`
- `workflow-reporter`
- log delegate (`-102`)

## 1. Bootstrap -> Workflow Coordinator

### Action: `start`

Used by `bootstrap` to start the bundled workflow demo.

**Required fields**:
- `action = start`
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
- `sender`

**Implemented behavior**:
- coordinator spawns `workflow-definition`
- coordinator spawns `workflow-reporter`
- coordinator sends `prepare_definition`
- bootstrap separately emits an intake log line through the log delegate

## 2. Workflow Coordinator -> Workflow Definition

### Action: `prepare_definition`

**Required fields**:
- `action = prepare_definition`
- `definition_path`
- `stage`
- `review_status`
- `sender`

**Implemented behavior**:
- definition resolves the supported workflow by path
- definition runs a startup `complete(...)` probe
- definition replies with `definition_ready` or `definition_error`

## 3. Workflow Definition -> Workflow Coordinator

### Action: `definition_ready`

**Required fields**:
- `action = definition_ready`
- `workflow_name`
- `workflow_version`
- `initial_stage`
- `requires_local_completion`

### Action: `definition_error`

**Required fields**:
- `action = definition_error`
- `reason`

**Implemented behavior**:
- on `definition_ready`, coordinator moves to `run_status = active`
- on `definition_error`, coordinator moves to `run_status = startup_failed`

## 4. Workflow Definition Transition API

### Action: `evaluate_transition`

**Required fields**:
- `action = evaluate_transition`
- `sender`
- `workflow_name`
- `stage`
- `item_id`
- `title`
- `priority`
- `owner`
- `review_status`
- `transition_count`

### Action: `transition_decision`

**Required fields**:
- `action = transition_decision`
- `workflow_name`
- `from_stage`
- `outcome`
- `next_stage`
- `status`
- `validation_clause`
- `reason`
- `retryable`
- `terminal_outcome`
- `note`

**Implemented rules**:
- `outcome` is one of `advance`, `stay`, `reject`
- failed completion evaluation becomes `stay` with `retryable = 1`
- reject decisions produce `terminal_outcome = rejected`
- successful review-stage advancement produces `terminal_outcome = completed`

## 5. Workflow Reporter Inputs

### Action: `progress`

**Required fields**:
- `action = progress`
- `workflow_name`
- `item_id`
- `stage`
- `status`
- `owner`
- `transition_count`
- `terminal_outcome`
- `reason`
- `text`

### Action: `summary`

**Required fields**:
- `action = summary`
- `workflow_name`
- `item_id`
- `stage`
- `status`
- `owner`
- `transition_count`
- `terminal_outcome`
- `reason`
- `text`

### Action: `startup_failure`

**Required fields**:
- `action = startup_failure`
- `reason`
- `failure_category`

**Implemented behavior**:
- reporter converts these messages into log-delegate messages
- tests verify visible output from `agerun.log`

## 6. Coordinator Success/Failure Handoff

The current coordinator implementation uses these simplified paths:
- success path:
  - accepts `definition_ready`
  - records a non-zero `item_agent_id`
  - sends a `summary` message directly to `workflow-reporter`
- startup failure path:
  - accepts `definition_error`
  - sends `startup_failure` directly to `workflow-reporter`

## Notes

- This contract reflects the current implementation, which is narrower than the original full
  orchestration plan.
- The current coordinator success path does not rely on a real workflow-item agent to generate the
  final summary during executable startup tests.
- Visible output is still emitted through the existing log-delegate path and remains the externally
  testable behavior.
