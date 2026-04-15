# Contract: Workflow Definition Schema

## Purpose

Define the required logical schema for a workflow definition file in this feature.

## Representation Rule

For this feature, a workflow definition file is a YAML file read by workflow agents through the
existing file delegate. The file remains file-backed and swappable, and the coordinator behavior
stays in AgeRun methods rather than in new C feature logic.

## Required Logical Elements

Every workflow definition YAML file MUST represent these logical elements:

1. **Metadata**
   - workflow name
   - workflow version
   - initial stage
   - completed terminal outcome label
   - rejected terminal outcome label

2. **Item Field Schema**
   - declared item fields relevant to workflow validation
   - which fields are required before specific transitions may succeed

3. **Stages**
   - ordered single-pass lifecycle stages
   - at minimum the default bundled workflow covers intake, triage, active work, and review before
     terminal resolution

4. **Transitions**
   - explicit source-stage to attempted-next-stage relationships
   - no implicit stage loops in the bundled default workflow

5. **Validation Clauses**
   - attached to transitions only
   - each clause names the rule being evaluated
   - each clause produces one of the allowed outcomes: `advance`, `stay`, `reject`

6. **Terminal Outcomes**
   - `completed`
   - `rejected`

## Required Runtime Actions

A workflow-definition agent reading the YAML file MUST support these actions:

### `action = describe`
Returns a structured description of the definition so tests and documentation can verify the active
workflow identity and available lifecycle contract.

### `action = evaluate_transition`
Evaluates one attempted transition for the current item state and returns a decision.

## Required `describe` Response Fields

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

## Required `evaluate_transition` Response Fields

- `action = transition_decision`
- `workflow_name`
- `from_stage`
- `outcome`
- `next_stage`
- `status`
- `validation_clause`
- `terminal_outcome`
- `note`

## Validation Rules

- `outcome` MUST be exactly one of `advance`, `stay`, `reject`
- `terminal_outcome` MUST be empty for non-terminal decisions
- `terminal_outcome` MUST be `completed` or `rejected` for terminal decisions
- `validation_clause` MUST name the rule that produced the decision
- the bundled default definition MUST be deterministic for a given input item state
- tests MAY supply alternate YAML definition files as long as they satisfy this contract

## Notes

- This contract is intentionally methods-oriented because the feature logic is implemented entirely
  as AgeRun methods
- The YAML file is the on-disk source of the workflow definition; the generic workflow-definition
  method is the runtime reader/evaluator of that file
- Tests may replace the default YAML file with alternate YAML definitions as long as they satisfy
  the schema and runtime contracts
