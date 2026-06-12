# Scheduling Method v1.0.0

## Overview

Scheduling stores one pending execution and triggers it when a later tick message reaches or passes
the stored due tick. It expresses delayed execution as ordinary message state.

## Behavior

Only messages with a recognized `request` value are handled as coordination requests.

On `request: "scheduling_schedule"`, the method stores schedule metadata, target agent, payload
fields, `trace_id`, and `source`. On a due `scheduling_tick`, it sends the stored payload to
the stored target agent. On `scheduling_cancel`, it clears pending state only when the matching
schedule is still pending.

## Message Format

Requests:

```text
{ source: <agent>, request: "scheduling_schedule", trace_id: <trace_id>, schedule_id: <id>, due_tick: <number>, target: <agent>, payload_request: <request>, payload_text: <text>, payload_attempt: <attempt> }
{ source: <agent>, request: "scheduling_tick", trace_id: <trace_id>, tick: <number> }
{ source: <agent>, request: "scheduling_cancel", trace_id: <trace_id>, schedule_id: <id> }
```

Triggered message:

```text
{
  source: <scheduling-agent>,
  request: <payload_request>,
  trace_id: <trace_id>,
  text: <payload_text>,
  attempt: <payload_attempt>,
  schedule_id: <schedule_id>
}
```

Response:

```text
{
  source: <scheduling-agent>,
  response: "scheduling_result",
  trace_id: <trace_id>,
  status: <success|failure>,
  state: <scheduled|cancelled|triggered|trigger_failed>,
  schedule_id: <id>,
  success_count: <count>,
  failure_count: <count>,
  pending: <0|1>,
  current_tick: <number>
}
```

Trigger responses use the stored schedule trace because they report the stored schedule request;
cancel responses use the cancel request trace.

## Implementation and Tests

Implementation: [`scheduling-1.0.0.method`](scheduling-1.0.0.method)

Test: [`scheduling_tests.c`](scheduling_tests.c)
