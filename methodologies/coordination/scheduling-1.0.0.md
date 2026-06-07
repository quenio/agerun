# Scheduling Method v1.0.0

## Overview

The scheduling method stores one pending execution and triggers it when a later tick message reaches
or passes the stored due tick. It expresses delayed execution as ordinary message state.

## Behavior

On a map whose `action` field is `"schedule"`, the method stores the schedule id, due tick, target,
payload fields, correlation id, and reply target. It marks the work as pending and reports
`status=scheduled`.

On a map whose `action` field is `"tick"`, the method records the current tick. If work is pending
and `tick >= due_tick`, it sends the stored payload to the stored target. A successful send clears
pending state and reports `status=triggered`; a failed send reports `status=trigger_failed` and
keeps the work pending. A tick that does not trigger the pending work updates scheduler state without
sending a status response.

On a map whose `action` field is `"cancel"`, the method clears pending state when the requested
schedule id matches the stored schedule id, then reports `status=cancelled`.

## Message Format

Schedule request:

```text
{
  action: "schedule",
  schedule_id: <id>,
  due_tick: <number>,
  target: <agent>,
  payload_action: <action>,
  payload_text: <text>,
  correlation_id: <id>,
  reply_to: <agent>
}
```

Tick and cancel requests:

```text
{
  action: "tick",
  tick: <number>
}

{
  action: "cancel",
  schedule_id: <id>
}
```

Triggered message:

```text
{
  action: <payload_action>,
  correlation_id: <correlation_id>,
  text: <payload_text>,
  schedule_id: <schedule_id>
}
```

Status response:

```text
{
  action: "schedule_status",
  schedule_id: <id>,
  status: <scheduled|cancelled|triggered|trigger_failed>,
  pending: <0|1>,
  current_tick: <number>
}
```

## Action Field

The input `action` field is a command discriminator in the request map. The scheduling agent runs
this method for every message it receives, so the field separates schedule, tick, and cancel commands
from unrelated messages that should not trigger pending work.

## Composition Notes

Retry uses scheduling for delayed attempts. A host process or tick-source agent can send tick
messages to a scheduling agent, while retry or workflow agents submit pending work.

## Limitations

There is no runtime clock or timer callback in the ordinary method instruction set. Scheduling
therefore requires explicit `tick` messages from another agent or host process.

## Implementation and Tests

Implementation: [`scheduling-1.0.0.method`](scheduling-1.0.0.method)

Test: [`scheduling_tests.c`](scheduling_tests.c)
