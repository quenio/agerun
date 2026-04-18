# Contract: Workflow Definition Schema

## Purpose

Describe the currently implemented definition-file contract used by the workflow coordinator feature.

## Representation Rule

The current implementation uses parse-friendly, flat definition files under `workflows/`.
Although the original design targeted YAML read through the file delegate, the implemented method
logic currently identifies supported definitions by `definition_path` and stores a small fixed set
of workflow metadata in method memory.

In practice, the implemented workflow-definition method recognizes these paths:
- `workflows/default-workflow.yaml`
- `workflows/test-workflow.yaml`
- `invalid-workflow.yaml` (treated as invalid schema fixture)

## Implemented Logical Elements

For recognized workflow definition paths, the workflow-definition method exposes these elements:

1. **Metadata**
   - `workflow_name`
   - `workflow_version`
   - `initial_stage`
   - `terminal_completed`
   - `terminal_rejected`
   - `requires_local_completion`

2. **Item Field Schema**
   - `item_fields`

3. **Stages**
   - `stages`

4. **Validation Clause Identity**
   - one clause name stored in `validation_clause`
   - `review_gate` for the bundled default definition
   - `test_gate` for the alternate test definition

5. **Transition Decision Template Contract**
   - startup probe uses `complete(...)` with `{outcome}` and `{reason}` placeholders
   - transition evaluation uses `complete(...)` with `{outcome}` and `{reason}` placeholders

## Supported Runtime Actions

### `action = prepare_definition`
Loads the logical definition identified by `definition_path` and runs the startup completion probe.

### `action = evaluate_transition`
Runs a completion-backed decision for the current stage and normalizes the result.

### `action = describe`
Returns the stored workflow metadata for tests and documentation.

## `describe` Response Fields

- `action = describe_result`
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

## `evaluate_transition` Response Fields

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

## Implemented Validation Rules

- recognized definition paths produce stored workflow metadata
- unknown or explicitly invalid paths produce `definition_error`
- startup readiness depends on a successful `complete(...)` probe
- transition decisions normalize success/failure into:
  - `advance`
  - `stay`
  - `reject`
- failed in-flight `complete(...)` transition evaluation maps to:
  - `outcome = stay`
  - `retryable = 1`
  - `reason = complete_transition_failed`
- startup readiness failure maps to:
  - `action = definition_error`
  - `reason = startup_dependency_unavailable`

## Notes

- This document reflects the current implementation rather than the earlier YAML/file-delegate plan.
- The `workflows/*.yaml` files remain the external definition identifiers used by tests and runtime
  startup, even though the current method implementation resolves them by path instead of parsing
  general YAML content.
- Deterministic tests use `AGERUN_COMPLETE_RUNNER` to control `outcome` / `reason` values.
