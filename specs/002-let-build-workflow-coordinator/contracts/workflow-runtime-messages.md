# Contract: Workflow Runtime Messages

## Purpose

Define the message protocol exchanged among the boot-launched workflow methods.

## Participants

- `bootstrap`
- `workflow-coordinator`
- `workflow-item`
- `workflow-definition`
- file delegate (`-100` per runtime delegate contract)
- `workflow-reporter`

## 1. Bootstrap -> Workflow Coordinator

### Action: `start`

Used by `bootstrap` to start a fresh workflow demo run.

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

**Expected behavior**:
- coordinator spawns the generic workflow-definition, reporter, and item agents
- coordinator passes the YAML definition path into the definition agent startup flow
- coordinator seeds the item agent with the bundled demo data

## 2. Workflow Coordinator -> Workflow Item

### Action: `initialize`

Seeds the item agent with its initial state and the spawned collaborator agent IDs.

**Required fields**:
- `action = initialize`
- `workflow_name`
- `item_id`
- `title`
- `priority`
- `owner`
- `review_status`
- `definition_agent_id`
- `reporter_agent_id`

**Expected behavior**:
- item stores initial state
- item emits an initial progress event
- item begins asking the definition agent for transition decisions

## 3. Workflow Definition -> File Delegate

### Action: `read`

Used by the generic workflow-definition agent to read the YAML workflow definition file.

**Required fields**:
- `action = read`
- `path`

**Expected behavior**:
- the file delegate returns a success/error response containing YAML content or an error message

## 4. Workflow Item -> Workflow Definition

### Action: `evaluate_transition`

Requests a decision for the current item stage.

**Required fields**:
- `action = evaluate_transition`
- `sender` (item agent ID)
- `workflow_name`
- `stage`
- `item_id`
- `title`
- `priority`
- `owner`
- `review_status`
- `transition_count`

**Expected behavior**:
- definition agent evaluates the transition-attached validation clauses
- definition agent replies to `sender` with a `transition_decision`

## 5. Workflow Definition -> Workflow Item

### Action: `transition_decision`

Returns the evaluated outcome for the attempted transition.

**Required fields**:
- `action = transition_decision`
- `workflow_name`
- `from_stage`
- `outcome`
- `next_stage`
- `status`
- `validation_clause`
- `terminal_outcome`
- `note`

**Rules**:
- `outcome` is one of `advance`, `stay`, `reject`
- `terminal_outcome` is empty unless the item has reached a terminal outcome

**Expected behavior**:
- item updates its memory if the decision is valid
- item emits a progress event or final summary event
- item stops automatic progression when `terminal_outcome` is populated

## 6. Workflow Item -> Workflow Reporter

### Action: `progress`

Reports a non-terminal lifecycle checkpoint.

**Required fields**:
- `action = progress`
- `workflow_name`
- `item_id`
- `stage`
- `status`
- `owner`
- `transition_count`
- `terminal_outcome`
- `text`

### Action: `summary`

Reports the final lifecycle summary.

**Required fields**:
- `action = summary`
- `workflow_name`
- `item_id`
- `stage`
- `status`
- `owner`
- `transition_count`
- `terminal_outcome`
- `text`

**Expected behavior**:
- reporter emits a human-readable log message through the existing AgeRun log delegate path

## 7. Workflow Definition Describe Contract (tests/docs)

### Action: `describe`

Used by tests or documentation-oriented fixtures to confirm that the generic workflow-definition
agent exposes the required schema elements for the active YAML definition file.

**Required request fields**:
- `action = describe`
- `sender`

**Required response fields**:
- `action = describe_result`
- `workflow_name`
- `workflow_version`
- `initial_stage`
- `terminal_completed`
- `terminal_rejected`
- `item_fields`
- `stages`
- `transitions`
- `validation_clauses`

## Notes

- Message values should remain compatible with the existing `parse(...)` / `build(...)` method
  idioms used throughout AgeRun
- Tests may replace the default YAML definition file with an alternate YAML definition file as long
  as the same runtime message contract is satisfied
- This protocol intentionally avoids introducing new runtime instructions, shell behavior, or new C
  feature logic
