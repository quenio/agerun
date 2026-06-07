# Workflow Method v1.0.0

## Overview

The workflow method maintains workflow state and sends an unbounded sequence of activity steps to
their configured target agents. It demonstrates a higher-level coordination behavior built on the
`head(...)`/`tail(...)` list traversal pattern.

## Behavior

On a map whose `action` field is `"start"`, the method stores the workflow id, reply target, branch
value, and active status. It expects three aligned step lists: `step_targets`, `step_actions`, and
`step_texts`. The first list contains target agent ids where positive integers are deliverable
targets and integer `0` can be used as a placeholder; the second and third lists contain the action
and text to send to the target at the same position.

The method sends itself an `execute_step` message. Each `execute_step` message reads the head item
from each step list and sends the step payload directly to the target agent. Only a successful step
send marks the workflow as waiting for `step_done` and stores the remaining tails as pending
workflow state. If the current step send fails, the method emits `workflow_complete` with
`status: "handoff_failed"`. If the current target is `0`
and the next target is positive, the method sends itself another `execute_step` message with the tail
lists and the same step number, skipping the placeholder without counting it as an executed step.
Because each continuation carries the tail lists, the method can process any number of steps
supported by ordinary AgeRun messages and memory.
If the initial internal continuation or a later step continuation cannot be queued, the method emits
`workflow_complete` with `status: "handoff_failed"` instead of leaving callers waiting indefinitely.

On a `step_done` map whose `workflow_id` matches the active workflow and whose `step` matches the
current active step, the method advances to the next pending step only if it is waiting for a
completion from a sent step. Stale, duplicate, premature, or out-of-order completion messages are
ignored. The waiting flag is cleared as soon as a completion is accepted, before the next internal
`execute_step` handoff runs, so duplicate completions cannot count the same step twice. When
`outcome` equals `branch_value`, it skips one pending step before advancing. If the next pending
target is `0` and the following target is positive, the method still queues the next `execute_step`
so the same placeholder-skip path can send the later positive step. When no next step remains, it
marks the workflow complete and sends a map whose `action` field is `"workflow_complete"` to the
stored reply target.
Terminal status is recorded only after the completion message is delivered. If completion delivery
fails, the workflow stays active with completion pending; a repeated matching `step_done` retries the
completion message without counting the final step again.

## Message Format

Start request:

```text
{
  action: "start",
  workflow_id: <id>,
  reply_to: <agent>,
  step_targets: [<agent>, <agent>, ...],
  step_actions: [<action>, <action>, ...],
  step_texts: [<text>, <text>, ...],
  branch_value: <outcome>
}
```

Step completion request:

```text
{
  action: "step_done",
  workflow_id: <id>,
  step: <current-step-number>,
  outcome: <value>
}
```

Step message sent to the current step target:

```text
{
  action: <step-action>,
  correlation_id: <workflow_id>,
  text: <step-text>,
  source: <workflow-agent>
}
```

Completion response:

```text
{
  action: "workflow_complete",
  workflow_id: <id>,
  status: <complete|handoff_failed>,
  current_step: <last-step-number>,
  completed_step_count: <executed-step-count>
}
```

## Action Field

The input `action` field is a command discriminator in the request map. The workflow agent runs this
method for every message it receives, so the field separates workflow start messages, internal step
execution messages, and step completion messages. That prevents unrelated maps from starting,
advancing, or completing the workflow.

## Composition Notes

Workflow sends directly to each step target. It can coordinate distribution, aggregation,
synchronization, conversation, retry, routing, and broadcasting agents by configuring those agents as
step targets.

The method uses aligned primitive lists instead of a list of step maps because the current method
evaluator cannot safely access fields from a headed map value in this workflow path. The lists are
still structured data rather than packed strings: each step position is represented by one entry in
each of `step_targets`, `step_actions`, and `step_texts`.

## Limitations

This method supports an unbounded linear workflow with a one-step branch skip when a step outcome
matches `branch_value`. Arbitrary workflow graphs, branch destinations by id, validation that the
three step lists have identical lengths, and dynamic step descriptor maps require richer collection
querying or a specialized validation/transition method. A single zero placeholder before a positive
step is skipped at workflow start and after completed steps, but consecutive zero placeholders can
still terminate scanning early because ordinary methods do not have a list length or type predicate
that distinguishes an empty list from a list whose next item is integer `0`.

## Implementation and Tests

Implementation: [`workflow-1.0.0.method`](workflow-1.0.0.method)

Test: [`workflow_tests.c`](workflow_tests.c)
