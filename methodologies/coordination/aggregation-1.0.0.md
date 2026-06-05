# Aggregation Method v1.0.0

## Overview

The aggregation method collects multiple result messages and emits one aggregate completion message
when the configured completion condition is met. It provides a bounded fan-in counterpart to
distribution.

## Behavior

On a map whose `action` field is `"start"`, the method stores the aggregate id, required result
count, and reply target. It also clears the three bounded result slots and their received markers.

On a map whose `action` field is `"result"`, the method stores the incoming value in slot `a`, `b`,
or `c`. When the number of received slots is greater than or equal to `required_count`, it sends a
map whose `action` field is `"aggregate_complete"` to the stored reply target.

## Message Format

Start request:

```text
{
  action: "start",
  aggregate_id: <id>,
  required_count: <1-3>,
  reply_to: <agent>
}
```

Result request:

```text
{
  action: "result",
  slot: <a|b|c>,
  value: <text>
}
```

Current bounded completion response:

```text
{
  action: "aggregate_complete",
  aggregate_id: <id>,
  status: "complete",
  result_a: <text>,
  result_b: <text>,
  result_c: <text>,
  received_count: <count>
}
```

Intended general completion response:

```text
{
  action: "aggregate_complete",
  aggregate_id: <id>,
  status: "complete",
  result: [<input-1>, <input-2>, ...],
  received_count: <count>
}
```

The general list-valued `result` form is not fully implementable as an ordinary method today because
the method language can carry lists but cannot create an empty list, append to a list, or assign by
list index.

## Action Field

The input `action` field is a command discriminator in the request map. The aggregation agent runs
this method for every message it receives, so the field separates setup messages from result messages
and prevents unrelated maps from changing collection state.

## Composition Notes

Use aggregation after distribution to combine worker outputs when the bounded three-slot contract is
sufficient. Synchronization or workflow can wait for the aggregate completion before advancing a
larger process.

## Limitations

The method supports three fixed slots and emits `result_a`, `result_b`, and `result_c`. The intended
general contract should emit `result` as a list, but ordinary methods currently lack list
construction, append, and indexed assignment. Dynamic result sets, duplicate handling policies, and
custom merge functions require richer collection operations or specialized aggregate methods.

## Implementation and Tests

Implementation: [`aggregation-1.0.0.method`](aggregation-1.0.0.method)

Test: [`aggregation_tests.c`](aggregation_tests.c)
