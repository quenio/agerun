# Data Model: Workflow Coordinator

This feature is implemented as cooperating AgeRun methods. The "entities" below are therefore
runtime agents, agent memory maps, and message contracts rather than new C structs.

## 1. Bootstrap Workflow Demo Context

### Description
The startup context carried by `bootstrap-1.0.0.method` when it launches the workflow coordinator on
fresh executable runs.

### Key Attributes
- `coordinator_method_name`: method name for the coordinator asset
- `coordinator_method_version`: version for the coordinator asset
- `definition_method_name`: method name for the generic workflow-definition asset
- `definition_method_version`: version for the generic workflow-definition asset
- `definition_path`: relative path to the bundled default YAML workflow definition file
- `reporter_method_name`: method name for the workflow reporter asset
- `reporter_method_version`: version for the workflow reporter asset
- `item_id`: bundled demo item identifier
- `title`: bundled demo item title
- `priority`: bundled demo item priority
- `owner`: bundled demo item owner
- `review_status`: bundled demo review result used by transition validation

### Validation Rules
- The context only launches the workflow demo on fresh boot runs
- The method/version pairs refer to method assets already loaded by the runtime
- Context values are sufficient to seed exactly one bundled demo work item

## 2. Workflow Coordinator Agent

### Description
A boot-launched orchestrator agent that creates the definition, reporter, and item agents for one
workflow run, seeds the initial work item, and records the demo run status.

### Key Attributes
- `definition_agent_id`: spawned agent ID for the active workflow definition
- `reporter_agent_id`: spawned agent ID for the active workflow reporter
- `item_agent_id`: spawned agent ID for the active workflow item
- `definition_method_name`: active workflow definition method name
- `definition_method_version`: active workflow definition method version
- `run_status`: `starting`, `active`, or `complete`
- `demo_status`: human-readable summary of launch success/failure

### Validation Rules
- Exactly one default workflow definition agent is spawned per fresh demo run
- Exactly one workflow item agent is spawned per fresh demo run
- The coordinator does not create duplicate agents when persisted workflow agents were restored
- The coordinator reaches `complete` only after the item reaches a terminal outcome or a launch
  failure is reported

## 3. Workflow Definition File

### Description
A YAML file read through the file delegate that declares the workflow metadata, item field schema,
stages, explicit transitions, validation clauses, and terminal outcomes.

### Key Attributes
- `path`: relative file path used in file-delegate read requests
- `workflow_name`: logical identifier for the workflow
- `workflow_version`: definition version visible to the coordinator/tests
- `initial_stage`: first stage for new items
- `terminal_completed`: canonical completed terminal outcome name
- `terminal_rejected`: canonical rejected terminal outcome name

### Validation Rules
- The file is YAML, not a method asset
- The file is readable through the file delegate within the allowed path
- The logical schema represented by the file includes metadata, item field schema, stages,
  transitions, validation clauses, and terminal outcomes
- Validation clauses attach to transitions only

## 4. Workflow Definition Agent

### Description
A generic agent implementing the reusable `workflow-definition` method. It reads the active YAML
workflow definition through the file delegate, stores the parsed definition state it needs, exposes
workflow metadata, and evaluates requested transitions for the current item state.

### Key Attributes
- `definition_path`: relative path to the active YAML workflow definition file
- `file_status`: `unread`, `loaded`, or `error`
- `workflow_name`: logical identifier for the workflow
- `workflow_version`: definition version visible to the coordinator/tests
- `initial_stage`: first stage for new items
- `terminal_completed`: canonical completed terminal outcome name
- `terminal_rejected`: canonical rejected terminal outcome name
- `last_read_status`: last file-delegate read result

### Validation Rules
- The definition logic is implemented by a method, but its schema source is a YAML file
- The definition agent must load the YAML file before serving `describe` or `evaluate_transition`
- The logical schema represented by the file includes metadata, item field schema, stages,
  transitions, validation clauses, and terminal outcomes
- Validation clauses attach to transitions only
- Evaluation responses produce only `advance`, `stay`, or `reject`

### Protocol Operations
- `load_definition`: request the YAML file content through the file delegate and store the parsed
  values needed for later evaluation
- `describe`: returns the workflow metadata/schema summary needed by tests and documentation
- `evaluate_transition`: returns the decision for one attempted stage transition

## 5. Workflow Item Agent

### Description
A stateful per-item agent that owns the work item memory, requests transition decisions from the
workflow definition agent, applies valid decisions, and sends progress/final-summary events to the
reporter.

### Key Attributes
- `item_id`: stable work-item identifier
- `title`: item title
- `priority`: item priority
- `owner`: assigned owner token
- `review_status`: review result token used by workflow validation
- `current_stage`: current lifecycle stage
- `current_status`: current item status label
- `transition_count`: number of accepted stage transitions
- `terminal_outcome`: empty until terminal, then `completed` or `rejected`
- `definition_agent_id`: agent ID used for transition evaluation
- `reporter_agent_id`: agent ID used for progress/final-summary logging
- `last_validation_outcome`: last decision result (`advance`, `stay`, `reject`)
- `last_validation_clause`: last validation clause name reported by the definition
- `summary`: final human-readable workflow summary

### Validation Rules
- The item owns its own state and does not rely on bootstrap memory after initialization
- `current_stage` always matches the last accepted progression decision
- `transition_count` increments only on `advance`
- `terminal_outcome` is set only once and ends further automatic progression
- Invalid or out-of-order updates leave the previously valid item state unchanged

### State Transitions
- `created -> initialized`: item receives its first coordinator message and stores seed data
- `initialized -> progressing`: item starts requesting transition evaluations
- `progressing -> progressing`: item accepts an `advance` outcome and moves to the next stage
- `progressing -> waiting`: item receives a `stay` outcome and remains on the same stage
- `progressing -> completed`: item receives a terminal `completed` decision
- `progressing -> rejected`: item receives a terminal `rejected` decision
- `waiting -> progressing`: a later valid transition evaluation allows advancement

## 6. Workflow Reporter Agent

### Description
A reporting agent that turns item progress/final-summary events into user-visible log output through
AgeRun's existing logging delegate path.

### Key Attributes
- `workflow_name`: logical workflow identifier for emitted messages
- `last_event_type`: `progress` or `summary`
- `last_item_id`: last reported item identifier
- `last_message`: last emitted human-readable message

### Validation Rules
- The reporter does not own workflow progression logic
- Every emitted message includes enough information to identify the workflow item and lifecycle
  checkpoint being reported
- The reporter uses the existing runtime logging path rather than inventing a new transport

## 7. File Delegate Read Message

### Description
The structured request/response pair used by the workflow definition agent to read the YAML
workflow definition file.

### Required Request Fields
- `action`: `read`
- `path`: relative path to the YAML workflow definition file

### Required Response Fields
- `status`: `success` or `error`
- `content`: YAML file content when `status = success`
- `message`: error text when `status = error`

### Validation Rules
- The path is relative and stays within the delegate's allowed root
- The definition agent handles read errors without corrupting later workflow state

## 8. Transition Decision Message

### Description
The structured response sent by the workflow definition agent after evaluating one requested
transition.

### Required Fields
- `action`: `transition_decision`
- `workflow_name`: logical workflow identifier
- `from_stage`: source stage that was evaluated
- `outcome`: `advance`, `stay`, or `reject`
- `next_stage`: next stage when `outcome = advance`, otherwise the current stage or terminal label
- `status`: status label to store on the item
- `validation_clause`: name of the clause that decided the transition
- `terminal_outcome`: empty for non-terminal decisions, otherwise `completed` or `rejected`
- `note`: human-readable progress/rejection text

### Validation Rules
- `outcome` is always one of the three clarified values
- `terminal_outcome` is populated only when the item should stop progressing
- `validation_clause` is always present so tests can verify which rule fired

## 9. Workflow Progress Event

### Description
The structured event the workflow item sends to the reporter whenever the item is initialized,
advances, stays, or reaches a terminal outcome.

### Required Fields
- `action`: `progress` or `summary`
- `workflow_name`: logical workflow identifier
- `item_id`: work-item identifier
- `stage`: current stage or terminal label
- `status`: current status label
- `owner`: current owner value
- `transition_count`: current transition count
- `terminal_outcome`: empty for non-terminal progress, otherwise terminal value
- `text`: human-readable message for the reporter to log

### Validation Rules
- The event payload contains enough data for the reporter to emit useful logs without re-reading the
  item memory directly
- `summary` events include the final terminal outcome and final transition count

## Relationships

- One **Bootstrap Workflow Demo Context** starts one **Workflow Coordinator Agent** on a fresh run
- One **Workflow Coordinator Agent** spawns one **Workflow Definition Agent**
- One **Workflow Coordinator Agent** passes one **Workflow Definition File** path into that agent
- One **Workflow Definition Agent** reads one **Workflow Definition File** through many **File Delegate Read Messages** over time as needed
- One **Workflow Coordinator Agent** spawns one **Workflow Reporter Agent**
- One **Workflow Coordinator Agent** spawns one **Workflow Item Agent** for the bundled demo item
- One **Workflow Item Agent** sends many transition-evaluation requests to one **Workflow Definition Agent**
- One **Workflow Definition Agent** returns many **Transition Decision Messages** over time
- One **Workflow Item Agent** sends many **Workflow Progress Events** to one **Workflow Reporter Agent**
- One **Workflow Reporter Agent** emits many log messages during one workflow run
