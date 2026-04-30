# Contract: Workflow Definition Schema

## Purpose

Describe the currently implemented definition-file contract used by the workflow coordinator feature.

## Representation Rule

The current implementation uses parse-friendly, flat definition files under `workflows/`.
`workflow-definition` reads the requested file through file delegate `-100`, parses the flat record,
validates required metadata and transition records, and stores the parsed definition in method
memory.

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

4. **Transition Topology**
   - `transition_count`
   - `transition_N_from`
   - `transition_N_to`
   - `transition_N_prompt`

5. **Transition Decision Prompt Contract**
   - startup probe uses `complete(...)` with `{outcome}` and `{reason}` placeholders
   - each transition evaluation uses that transition's configured prompt with `{outcome}` and
     `{reason}` placeholders

## Supported Runtime Actions

### `action = prepare_definition`
Reads the file identified by `definition_path`, parses and validates it, then runs the startup
completion probe.

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
- `transition_count`

## `evaluate_transition` Response Fields

- `action = transition_decision`
- `workflow_name`
- `from_stage`
- `outcome`
- `next_stage`
- `status`
- `reason`
- `retryable`
- `terminal_outcome`
- `note`

## Implemented Validation Rules

- readable, valid workflow definition files produce stored workflow metadata and transitions
- missing files or invalid schema records produce `definition_error`
- startup readiness depends on a successful `complete(...)` probe
- transition decisions normalize success/failure into:
  - `advance`
  - `stay`
  - `reject`
- `advance` may only move to the configured `to` stage for the current `from` stage
- failed in-flight `complete(...)` transition evaluation maps to:
  - `outcome = stay`
  - `retryable = 1`
  - `reason = complete_transition_failed`
- startup readiness failure maps to:
  - `action = definition_error`
  - `reason = startup_dependency_unavailable`

## Notes

- The `workflows/*.workflow` files are the source of workflow metadata, stage order, terminal
  outcomes, and transition prompt text.
- Deterministic tests use `AGERUN_COMPLETE_RUNNER` to control `outcome` / `reason` values.
