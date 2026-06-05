# Aggregation Method v1.0.0

## Overview

The aggregation method collects multiple result messages and emits one aggregate completion message
when the configured completion condition is met. It provides the fan-in counterpart to distribution.

## Behavior

On a map whose `action` field is `"start"`, the method stores the aggregate id, required result
count, and reply target. It also clears the three result slots and their received markers.

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

Completion response:

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

## Action Field

The input `action` field is a command discriminator in the request map. The aggregation agent runs
this method for every message it receives, so the field separates setup messages from result messages
and prevents unrelated maps from changing collection state.

## Composition Notes

Use aggregation after distribution to combine worker outputs. Synchronization or workflow can wait
for the aggregate completion before advancing a larger process.

## Limitations

The method supports three fixed slots. Dynamic result sets, duplicate handling policies, and custom
merge functions require richer collection operations or specialized aggregate methods.

## Implementation and Tests

Implementation: [`aggregation-1.0.0.method`](aggregation-1.0.0.method)

Test: [`aggregation_tests.c`](aggregation_tests.c)
