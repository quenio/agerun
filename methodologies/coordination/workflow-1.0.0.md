# Workflow Method v1.0.0

## Overview

The workflow method maintains workflow state and routes an unbounded sequence of activity steps
through a routing agent. It demonstrates a higher-level coordination behavior built on the
lower-level routing primitive and the `head(...)`/`tail(...)` list traversal pattern.

## Behavior

On a map whose `action` field is `"start"`, the method stores the workflow id, routing agent, reply
target, branch value, and active status. It expects three aligned step lists: `step_targets`,
`step_actions`, and `step_texts`. The first list contains nonzero target agent ids; the second and
third lists contain the action and text to route to the target at the same position.

The method sends itself an `execute_step` message. Each `execute_step` message reads the head item
from each step list, stores the remaining tails as pending workflow state, and sends a one-to-one
route request to the routing agent. Because each continuation carries the tail lists, the method can
process any number of steps supported by ordinary AgeRun messages and memory.

On a `step_done` map whose `workflow_id` matches the active workflow, the method advances to the
next pending step. When `outcome` equals `branch_value`, it skips one pending step before advancing.
When no next step remains, it marks the workflow complete and sends a map whose `action` field is
`"workflow_complete"` to the stored reply target.

## Message Format

Start request:

```text
{
  action: "start",
  workflow_id: <id>,
  routing_agent: <agent>,
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

Route message sent to routing:

```text
{
  action: "route",
  mode: "one",
  target: <step-target>,
  payload_action: <step-action>,
  payload_text: <step-text>,
  correlation_id: <workflow_id>,
  reply_to: 0
}
```

Completion response:

```text
{
  action: "workflow_complete",
  workflow_id: <id>,
  status: "complete",
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

Workflow uses routing directly. It can coordinate distribution, aggregation, synchronization,
conversation, and retry agents by configuring those agents as step targets.

The method uses aligned primitive lists instead of a list of step maps because the current method
evaluator cannot safely access fields from a headed map value in this workflow path. The lists are
still structured data rather than packed strings: each step position is represented by one entry in
each of `step_targets`, `step_actions`, and `step_texts`.

## Limitations

This method supports an unbounded linear workflow with a one-step branch skip when a step outcome
matches `branch_value`. Arbitrary workflow graphs, branch destinations by id, validation that the
three step lists have identical lengths, and dynamic step descriptor maps require richer collection
querying or a specialized validation/transition method.

## Implementation and Tests

Implementation: [`workflow-1.0.0.method`](workflow-1.0.0.method)

Test: [`workflow_tests.c`](workflow_tests.c)
