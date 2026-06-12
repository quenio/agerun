# Scheduling Method v1.0.0

## Overview

Scheduling stores one pending execution and triggers it when a later tick message reaches or passes
the stored due tick. It expresses delayed execution as ordinary message state.

## Behavior

Only messages with `type: "request"` are handled as coordination requests.

On `action: "schedule"`, the method stores schedule metadata, target agent, payload fields,
`trace_id`, and `source_agent`. On a due `tick`, it sends the stored payload to the stored target
agent. On `cancel`, it clears pending state only when the matching schedule is still pending.

## Message Format

Requests:

```text
{ action: "schedule", type: "request", schedule_id: <id>, due_tick: <number>, target_agent: <agent>, payload_action: <action>, payload_text: <text>, payload_attempt: <attempt>, trace_id: <id>, source_agent: <agent> }
{ action: "tick", type: "request", tick: <number> }
{ action: "cancel", type: "request", schedule_id: <id> }
```

Triggered message:

```text
{
  action: <payload_action>,
  type: "request",
  trace_id: <trace_id>,
  text: <payload_text>,
  attempt: <payload_attempt>,
  schedule_id: <schedule_id>
}
```

Response:

```text
{
  action: <schedule|cancel>,
  type: "response",
  schedule_id: <id>,
  trace_id: <trace_id>,
  status: <success|failure>,
  state: <scheduled|cancelled|triggered|trigger_failed>,
  success_count: <count>,
  failure_count: <count>,
  pending: <0|1>,
  current_tick: <number>
}
```

Trigger responses use `action: "schedule"` because they report the stored schedule request; cancel
responses use `action: "cancel"`.

## Implementation and Tests

Implementation: [`scheduling-1.0.0.method`](scheduling-1.0.0.method)

Test: [`scheduling_tests.c`](scheduling_tests.c)
