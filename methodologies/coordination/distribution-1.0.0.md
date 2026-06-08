# Distribution Method v1.0.0

## Overview

The distribution method assigns a list of payload values across a list of worker agents. It performs
round-robin work assignment: each payload item is sent to the next worker, and the worker list rotates
back to the beginning when the end is reached.

## Behavior

On a map whose `action` field is `"distribute"`, the method treats the request as the first
assignment pass and initializes the work id, reply target, and assignment counters. Later
`assign_payload` continuation messages carry the remaining payload list, current worker list,
original worker list, work id, reply target, and counters.

Each assignment pass reads the head payload and head worker. If both are present and the worker is
positive, the method sends the payload item as-is to that worker:

```text
<payload>
```

The continuation then advances to the next payload and rotates the worker list. If the remaining
worker list has a positive head, that tail becomes the next worker list. Otherwise, the next
continuation resets to the original worker list, implementing round-robin assignment. If the current
worker is integer `0` and later workers remain, the method skips that worker placeholder while
keeping the same payload for the next positive worker.

When all payload items have been attempted, the method emits one `distribution_result`. The result is
`distributed` when at least one assignment was attempted and no assignment send failed. It is
`distribution_failed` when the payload list is empty, the worker list is empty, a worker send fails,
or a required self-continuation cannot be queued.

## Message Format

Distribution request:

```text
{
  action: "distribute",
  work_id: <id>,
  payloads: [<payload>, <payload>, ...],
  workers: [<agent>, <agent>, ...],
  reply_to: <agent>
}
```

Result response:

```text
{
  action: "distribution_result",
  status: <distributed|distribution_failed>,
  work_id: <id>,
  assignment_count: <count>,
  sent_count: <count>,
  failed_count: <count>
}
```

`assignment_count` counts payload items assigned to positive worker IDs, including assignments whose
send failed. `sent_count` counts successful assignment sends. `failed_count` counts positive worker
IDs that could not receive an assignment.

## Action Field

The input `action` field is a command discriminator in the request map. The distribution agent runs
this method for every message it receives, so `action: "distribute"` marks the message as a request
to assign payload items. `action: "assign_payload"` is the method's internal continuation command.

## Composition Notes

Pair distribution with aggregation for fan-out and fan-in when each worker should receive a distinct
payload item. Use broadcasting instead when every recipient should receive the same payload. A
workflow step can send a distribution request directly to a distribution agent.

## Limitations

The method treats payload items as opaque values and only checks whether the payload list itself is
empty by comparing it with `[]`. The method uses primitive worker IDs and does not perform load-aware
placement, weighted assignment, or worker health checks.

## Implementation and Tests

Implementation: [`distribution-1.0.0.method`](distribution-1.0.0.method)

Test: [`distribution_tests.c`](distribution_tests.c)
