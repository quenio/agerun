# workflow-item-1.0.0

## Overview

`workflow-item` is the stateful per-item workflow method. It records bundled demo item metadata,
emits progress checkpoints through `workflow-reporter`, auto-advances through the early lifecycle
stages, asks `workflow-definition` for the review-stage decision, and then emits either a final
summary or a retryable progress update.

## ATN Specification

This ATN specification uses only the probeable agent-state constants requested by the workflow
contract: `initial_memory`, `final_memory`, `message`, and `context`.

```haskell
Memory
Message
Context

initial_memory: Memory
final_memory: Memory
message: Message
context: Context

PRECONDITION_SUPPORTED_MESSAGE_ACTION:
  message.action = "initialize" or
  message.action = "auto_progress" or
  message.action = "transition_decision"

PRECONDITION_INITIALIZE_MESSAGE_IS_COMPLETE:
  message.action = "initialize" =>
    message.sender > 0 and
    not (message.workflow_name = "") and
    not (message.item_id = "") and
    not (message.title = "") and
    not (message.priority = "") and
    not (message.owner = "") and
    not (message.review_status = "") and
    message.definition_agent_id > 0 and
    message.reporter_agent_id > 0 and
    not (message.initial_stage = "")

PRECONDITION_AUTO_PROGRESS_HAS_A_KNOWN_STAGE:
  message.action = "auto_progress" =>
    initial_memory.current_stage = "intake" or
    initial_memory.current_stage = "triage" or
    initial_memory.current_stage = "active" or
    initial_memory.current_stage = "review"

PRECONDITION_TRANSITION_DECISION_IS_COMPLETE:
  message.action = "transition_decision" =>
    (message.outcome = "advance" or
     message.outcome = "reject" or
     message.outcome = "stay") and
    not (message.reason = "")

POSTCONDITION_INITIALIZATION_RECORDS_THE_ITEM_IDENTITY:
  message.action = "initialize" =>
    final_memory.workflow_name = message.workflow_name and
    final_memory.item_id = message.item_id

POSTCONDITION_INITIALIZATION_EMITS_PROGRESS:
  message.action = "initialize" =>
    final_memory.current_stage = message.initial_stage and
    final_memory.current_status = "created" and
    final_memory.last_reason = "initialized" and
    final_memory.progress_sent = 1

POSTCONDITION_INTAKE_AUTO_PROGRESS_ADVANCES_TO_TRIAGE:
  message.action = "auto_progress" and initial_memory.current_stage = "intake" =>
    final_memory.current_stage = "triage" and
    final_memory.transition_count = initial_memory.transition_count + 1 and
    final_memory.progress_sent = 1

POSTCONDITION_TRIAGE_AUTO_PROGRESS_ADVANCES_TO_ACTIVE:
  message.action = "auto_progress" and initial_memory.current_stage = "triage" =>
    final_memory.current_stage = "active" and
    final_memory.transition_count = initial_memory.transition_count + 1 and
    final_memory.progress_sent = 1

POSTCONDITION_ACTIVE_AUTO_PROGRESS_ADVANCES_TO_REVIEW:
  message.action = "auto_progress" and initial_memory.current_stage = "active" =>
    final_memory.current_stage = "review" and
    final_memory.transition_count = initial_memory.transition_count + 1 and
    final_memory.progress_sent = 1

POSTCONDITION_REVIEW_REQUESTS_A_TRANSITION_DECISION:
  message.action = "auto_progress" and initial_memory.current_stage = "review" =>
    final_memory.evaluate_sent = 1

POSTCONDITION_ADVANCE_DECISION_EMITS_A_SUMMARY:
  message.action = "transition_decision" and message.outcome = "advance" =>
    final_memory.current_stage = message.next_stage and
    final_memory.current_status = message.status and
    final_memory.last_reason = message.reason and
    final_memory.summary_sent = 1

POSTCONDITION_REJECT_DECISION_EMITS_A_SUMMARY:
  message.action = "transition_decision" and message.outcome = "reject" =>
    final_memory.terminal_outcome = message.terminal_outcome and
    final_memory.last_reason = message.reason and
    final_memory.summary_sent = 1

POSTCONDITION_STAY_DECISION_EMITS_PROGRESS_INSTEAD_OF_SUMMARY:
  message.action = "transition_decision" and message.outcome = "stay" =>
    final_memory.current_stage = initial_memory.current_stage and
    final_memory.last_reason = message.reason and
    final_memory.progress_sent = 1
```

## Inputs

### `action=initialize`

Expected fields:
- `sender`
- `workflow_name`
- `item_id`
- `title`
- `priority`
- `owner`
- `review_status`
- `definition_agent_id`
- `reporter_agent_id`
- `initial_stage`

Behavior:
- initializes the item memory
- emits the first `progress` event with `reason=initialized`
- queues `auto_progress` to continue the demo flow

### `action=auto_progress`

Behavior:
- advances automatically through `intake -> triage -> active -> review`
- emits a `progress` message on each stage change
- when the item reaches `review`, sends `evaluate_transition` to `workflow-definition`

### `action=transition_decision`

Expected fields:
- `outcome`
- `next_stage`
- `status`
- `reason`
- `retryable`
- `terminal_outcome`

Behavior:
- `advance`: applies `next_stage`, updates status, increments transition count, and emits `summary`
- `reject`: keeps the current stage, sets rejected state, and emits `summary`
- `stay`: keeps the current stage and emits a visible `progress` update instead of a summary

## Lifecycle State

The method persists:
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

Default bundled progression:
1. initialize at `intake`
2. auto-progress to `triage`
3. auto-progress to `active`
4. auto-progress to `review`
5. ask `workflow-definition` for a review decision
6. emit summary on `advance`/`reject`, or progress on retryable `stay`

## Method Code

```agerun
memory.is_initialize := if(message.action = "initialize", 1, 0)
memory.is_auto_progress := if(message.action = "auto_progress", 1, 0)
memory.is_transition_decision := if(message.action = "transition_decision", 1, 0)
memory.is_current_intake := 0
memory.is_current_triage := 0
memory.is_current_active := 0
memory.is_current_review := 0
memory.advance_intake := 0
memory.advance_triage := 0
memory.advance_active := 0
memory.request_review := 0
memory.should_emit_progress := 0
memory.outcome_is_advance := 0
memory.outcome_is_reject := 0
memory.outcome_is_stay := 0
memory.apply_advance := 0
memory.apply_reject := 0
memory.apply_stay := 0
memory.self_agent_id := if(memory.is_initialize = 1, message.sender, memory.self_agent_id)
memory.workflow_name := if(memory.is_initialize = 1, message.workflow_name, memory.workflow_name)
memory.item_id := if(memory.is_initialize = 1, message.item_id, memory.item_id)
memory.title := if(memory.is_initialize = 1, message.title, memory.title)
memory.priority := if(memory.is_initialize = 1, message.priority, memory.priority)
memory.owner := if(memory.is_initialize = 1, message.owner, memory.owner)
memory.review_status := if(memory.is_initialize = 1, message.review_status, memory.review_status)
memory.definition_agent_id := if(memory.is_initialize = 1, message.definition_agent_id, memory.definition_agent_id)
memory.reporter_agent_id := if(memory.is_initialize = 1, message.reporter_agent_id, memory.reporter_agent_id)
memory.current_stage := if(memory.is_initialize = 1, message.initial_stage, memory.current_stage)
memory.current_status := if(memory.is_initialize = 1, "created", memory.current_status)
memory.transition_count := if(memory.is_initialize = 1, 0, memory.transition_count)
memory.terminal_outcome := if(memory.is_initialize = 1, "", memory.terminal_outcome)
memory.last_reason := if(memory.is_initialize = 1, "initialized", memory.last_reason)
memory.progress_text := build("workflow={workflow_name} item={item_id} stage={current_stage} status={current_status} reason={last_reason}", memory)
memory.progress_input := build("action=progress workflow_name={workflow_name} item_id={item_id} stage={current_stage} status={current_status} owner={owner} transition_count={transition_count} terminal_outcome={terminal_outcome} reason={last_reason} text={progress_text}", memory)
memory.progress_payload := parse("action={action} workflow_name={workflow_name} item_id={item_id} stage={stage} status={status} owner={owner} transition_count={transition_count} terminal_outcome={terminal_outcome} reason={reason} text={text}", memory.progress_input)
memory.progress_sent := send(memory.reporter_agent_id * memory.is_initialize, memory.progress_payload)
memory.auto_input := build("action=auto_progress", memory)
memory.auto_payload := parse("action={action}", memory.auto_input)
memory.auto_sent := send(memory.self_agent_id * memory.is_initialize, memory.auto_payload)
memory.is_current_intake := if(memory.current_stage = "intake", 1, 0)
memory.is_current_triage := if(memory.current_stage = "triage", 1, 0)
memory.is_current_active := if(memory.current_stage = "active", 1, 0)
memory.is_current_review := if(memory.current_stage = "review", 1, 0)
memory.advance_intake := memory.is_auto_progress * memory.is_current_intake
memory.advance_triage := memory.is_auto_progress * memory.is_current_triage
memory.advance_active := memory.is_auto_progress * memory.is_current_active
memory.request_review := memory.is_auto_progress * memory.is_current_review
memory.current_stage := if(memory.advance_intake = 1, "triage", memory.current_stage)
memory.current_stage := if(memory.advance_triage = 1, "active", memory.current_stage)
memory.current_stage := if(memory.advance_active = 1, "review", memory.current_stage)
memory.current_status := if(memory.advance_intake = 1, "triage", memory.current_status)
memory.current_status := if(memory.advance_triage = 1, "active", memory.current_status)
memory.current_status := if(memory.advance_active = 1, "review", memory.current_status)
memory.last_reason := if(memory.advance_intake = 1, "entered_triage", memory.last_reason)
memory.last_reason := if(memory.advance_triage = 1, "entered_active", memory.last_reason)
memory.last_reason := if(memory.advance_active = 1, "entered_review", memory.last_reason)
memory.transition_count := if(memory.advance_intake = 1, memory.transition_count + 1, memory.transition_count)
memory.transition_count := if(memory.advance_triage = 1, memory.transition_count + 1, memory.transition_count)
memory.transition_count := if(memory.advance_active = 1, memory.transition_count + 1, memory.transition_count)
memory.should_emit_progress := memory.advance_intake + memory.advance_triage
memory.should_emit_progress := memory.should_emit_progress + memory.advance_active
memory.progress_text := build("workflow={workflow_name} item={item_id} stage={current_stage} status={current_status} reason={last_reason}", memory)
memory.progress_input := build("action=progress workflow_name={workflow_name} item_id={item_id} stage={current_stage} status={current_status} owner={owner} transition_count={transition_count} terminal_outcome={terminal_outcome} reason={last_reason} text={progress_text}", memory)
memory.progress_payload := parse("action={action} workflow_name={workflow_name} item_id={item_id} stage={stage} status={status} owner={owner} transition_count={transition_count} terminal_outcome={terminal_outcome} reason={reason} text={text}", memory.progress_input)
memory.progress_sent := send(memory.reporter_agent_id * memory.should_emit_progress, memory.progress_payload)
memory.auto_sent := send(memory.self_agent_id * memory.should_emit_progress, memory.auto_payload)
memory.evaluate_input := build("action=evaluate_transition sender={self_agent_id} workflow_name={workflow_name} stage={current_stage} item_id={item_id} title={title} priority={priority} owner={owner} review_status={review_status} transition_count={transition_count}", memory)
memory.evaluate_payload := parse("action={action} sender={sender} workflow_name={workflow_name} stage={stage} item_id={item_id} title={title} priority={priority} owner={owner} review_status={review_status} transition_count={transition_count}", memory.evaluate_input)
memory.evaluate_sent := send(memory.definition_agent_id * memory.request_review, memory.evaluate_payload)
memory.outcome_is_advance := if(message.outcome = "advance", 1, 0)
memory.outcome_is_reject := if(message.outcome = "reject", 1, 0)
memory.outcome_is_stay := if(message.outcome = "stay", 1, 0)
memory.apply_advance := memory.is_transition_decision * memory.outcome_is_advance
memory.apply_reject := memory.is_transition_decision * memory.outcome_is_reject
memory.apply_stay := memory.is_transition_decision * memory.outcome_is_stay
memory.current_stage := if(memory.apply_advance = 1, message.next_stage, memory.current_stage)
memory.current_status := if(memory.is_transition_decision = 1, message.status, memory.current_status)
memory.last_reason := if(memory.is_transition_decision = 1, message.reason, memory.last_reason)
memory.terminal_outcome := if(memory.apply_advance = 1, message.terminal_outcome, memory.terminal_outcome)
memory.terminal_outcome := if(memory.apply_reject = 1, message.terminal_outcome, memory.terminal_outcome)
memory.transition_count := if(memory.apply_advance = 1, memory.transition_count + 1, memory.transition_count)
memory.summary := build("workflow={workflow_name} item={item_id} stage={current_stage} terminal={terminal_outcome} reason={last_reason}", memory)
memory.summary_input := build("action=summary workflow_name={workflow_name} item_id={item_id} stage={current_stage} status={current_status} owner={owner} transition_count={transition_count} terminal_outcome={terminal_outcome} reason={last_reason} text={summary}", memory)
memory.summary_payload := parse("action={action} workflow_name={workflow_name} item_id={item_id} stage={stage} status={status} owner={owner} transition_count={transition_count} terminal_outcome={terminal_outcome} reason={reason} text={text}", memory.summary_input)
memory.summary_sent := send(memory.reporter_agent_id * memory.apply_advance, memory.summary_payload)
memory.summary_sent := send(memory.reporter_agent_id * memory.apply_reject, memory.summary_payload)
memory.progress_text := build("workflow={workflow_name} item={item_id} stage={current_stage} status={current_status} reason={last_reason}", memory)
memory.progress_input := build("action=progress workflow_name={workflow_name} item_id={item_id} stage={current_stage} status={current_status} owner={owner} transition_count={transition_count} terminal_outcome={terminal_outcome} reason={last_reason} text={progress_text}", memory)
memory.progress_payload := parse("action={action} workflow_name={workflow_name} item_id={item_id} stage={stage} status={status} owner={owner} transition_count={transition_count} terminal_outcome={terminal_outcome} reason={reason} text={text}", memory.progress_input)
memory.progress_sent := send(memory.reporter_agent_id * memory.apply_stay, memory.progress_payload)
```

## Testing

Validated by `methods/workflow_item_tests.c`.

The tests verify:
- initialization stores stable item metadata
- early lifecycle progress is emitted in order
- review-stage decisions from `workflow-definition` are applied correctly
- `advance`, `reject`, and retryable `stay` preserve consistent item state
- final summaries and reason-bearing progress messages are emitted to the reporter
