# Workflow Method v1.0.0

## Overview

The workflow method maintains a small workflow state machine and routes each activity step through a
routing agent. It demonstrates a higher-level coordination behavior built on the lower-level routing
primitive.

## Behavior

On a map whose `action` field is `"start"`, the method stores the workflow id, routing agent, reply
target, three step targets, step actions, step text values, and branch value. It marks the workflow
active, sets the current step to 1, and routes step 1.

On a step completion map for step 1, the method advances to step 3 when `outcome` equals
`branch_value`; otherwise it advances to step 2. On a step completion map for step 2, it advances to
step 3. On a step completion map for step 3, it marks the workflow complete and sends a map whose
`action` field is `"workflow_complete"` to the stored reply target.

## Message Format

Start request:

```text
{
  action: "start",
  workflow_id: <id>,
  routing_agent: <agent>,
  reply_to: <agent>,
  step1_target: <agent>,
  step1_action: <action>,
  step1_text: <text>,
  step2_target: <agent>,
  step2_action: <action>,
  step2_text: <text>,
  step3_target: <agent>,
  step3_action: <action>,
  step3_text: <text>,
  branch_value: <outcome>
}
```

Step completion request:

```text
{
  action: "step_done",
  step: <1|2|3>,
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
  current_step: 3
}
```

## Action Field

The input `action` field is a command discriminator in the request map. The workflow agent runs this
method for every message it receives, so the field separates workflow start messages from step
completion messages and avoids advancing the workflow for unrelated maps.

## Composition Notes

Workflow uses routing directly. It can coordinate distribution, aggregation, synchronization,
conversation, and retry agents by configuring those agents as step targets.

## Limitations

This method supports a bounded three-step workflow with one branch from step 1. General workflow
graphs require dynamic graph storage, iteration, and richer state transition conventions.

## Implementation and Tests

Implementation: [`workflow-1.0.0.method`](workflow-1.0.0.method)

Test: [`workflow_tests.c`](workflow_tests.c)
