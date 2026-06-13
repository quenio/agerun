# Scheduling Method v1.0.0

## Overview

Scheduling stores one pending execution and triggers it when a later tick message reaches or passes
the stored due tick. It expresses delayed execution as ordinary message state.

## Behavior

Only messages with a recognized `request` value are handled as coordination requests.

On `request: "scheduling_schedule"`, the method stores schedule metadata, recipient agent, payload
fields, effective `trace_id`, `session_id`, and `sender`. On a due `scheduling_tick` with the same
`session_id`, it sends the stored payload to the stored recipient agent. On `scheduling_cancel`, it
clears pending state only when the matching schedule and session are still pending.

## Message Format

Requests:

```text
{ sender: <sender-agent>, request: "scheduling_schedule", trace_id: <trace_id>, session_id: <session_id>, schedule_id: <id>, due_tick: <number>, recipient: <recipient-agent>, payload_request: <request>, payload_text: <text>, payload_attempt: <attempt> }
{ sender: <sender-agent>, request: "scheduling_tick", trace_id: <trace_id>, session_id: <session_id>, tick: <number> }
{ sender: <sender-agent>, request: "scheduling_cancel", trace_id: <trace_id>, session_id: <session_id>, schedule_id: <id> }
```

Triggered message:

```text
{
  sender: <sender-agent>,
  request: <payload_request>,
  trace_id: <trace_id>,
  session_id: <session_id>,
  text: <payload_text>,
  attempt: <payload_attempt>,
  schedule_id: <schedule_id>
}
```

Response:

```text
{
  sender: <scheduling-agent>,
  response: "scheduling_result",
  trace_id: <trace_id>,
  session_id: <session_id>,
  status: <success|failure>,
  state: <scheduled|cancelled|triggered|trigger_failed>,
  schedule_id: <id>,
  success_count: <count>,
  failure_count: <count>,
  pending: <0|1>,
  current_tick: <number>
}
```

Count semantics: `success_count` increments when a due tick successfully sends the stored payload,
and when a matching cancel clears a pending schedule. The schedule creation response does not
increment it. `failure_count` increments when a due tick should trigger but the stored payload send
fails; the schedule remains pending.

Status semantics: the response status is `success` for schedule creation, for a matching cancel
that clears a pending schedule, and for a due tick that successfully sends the stored payload. It is
`failure` only when a due tick should trigger but the stored payload send fails.

Trigger responses and triggered payload requests use the tick request's effective `trace_id`;
cancel responses use the cancel request's effective `trace_id`. All request kinds in one pending
schedule use the same `session_id`.

## Implementation and Tests

Implementation: [`scheduling-1.0.0.method`](scheduling-1.0.0.method)

Test: [`scheduling_tests.c`](scheduling_tests.c)
