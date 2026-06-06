# Aggregation Method v1.0.0

## Overview

The aggregation method collects multiple result messages and emits one aggregate completion message
when the configured completion condition is met. It provides an unbounded fan-in counterpart to
distribution.

## Behavior

On a map whose `action` field is `"start"`, the method stores the aggregate id, required result
count, and reply target. It also clears the result list, received count, and completion marker.

On a map whose `action` field is `"result"`, the method appends the incoming value to the stored
result list. When the number of received values is greater than or equal to `required_count`, it
sends one map whose `action` field is `"aggregate_complete"` to the stored reply target. The
aggregate values are emitted as a list in the `result` field.

## Message Format

Start request:

```text
{
  action: "start",
  aggregate_id: <id>,
  required_count: <count>,
  reply_to: <agent>
}
```

Result request:

```text
{
  action: "result",
  value: <text>
}
```

Completion response:

```text
{
  action: "aggregate_complete",
  aggregate_id: <id>,
  status: "complete",
  result: [<input-1>, <input-2>, ...],
  received_count: <count>
}
```

The list contains each appended result value in arrival order. The method uses `append(...)` to
mutate an internal result list, so the number of collected values is not bounded by named slots.
Unlike the other coordination methods, aggregation sends the completion map as a fresh literal
instead of first storing it in `memory.output_message`. A stored output map would contain the nested
`result` list, and the current memory copy path cannot send memory-owned maps with nested
containers without deep-copy support.

## Action Field

The input `action` field is a command discriminator in the request map. The aggregation agent runs
this method for every message it receives, so the field separates setup messages from result messages
and prevents unrelated maps from changing collection state.

## Composition Notes

Use aggregation after distribution to combine worker outputs when fan-in can exceed a small fixed
set. Synchronization or workflow can wait for the aggregate completion before advancing a larger
process.

## Limitations

The method appends text result values and emits them as `result`. Duplicate handling policies,
custom merge functions, stored nested output maps, and aggregating borrowed nested containers
require richer collection operations, deep-copy support, or specialized aggregate methods.

## Implementation and Tests

Implementation: [`aggregation-1.0.0.method`](aggregation-1.0.0.method)

Test: [`aggregation_tests.c`](aggregation_tests.c)
