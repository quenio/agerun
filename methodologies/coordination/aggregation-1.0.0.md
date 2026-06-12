# Aggregation Method v1.0.0

## Overview

Aggregation collects result messages and emits one aggregate completion response when the configured
result count is met. It provides an unbounded fan-in counterpart to distribution.

## Behavior

Only messages with `type: "request"` are handled as coordination requests.

On `action: "start"`, the method stores the aggregate id, required result count, `trace_id`, and
`source_agent`, then clears the append-backed result list. On `action: "result"`, it appends the
incoming `value` only when `aggregate_id` matches the active aggregate and completion has not been
delivered.

Required counts below one behave as one required result. Completion is recorded only after the
`start` response is sent successfully.

## Message Format

Requests:

```text
{ action: "start", type: "request", aggregate_id: <id>, required_count: <count>, trace_id: <id>, source_agent: <agent> }
{ action: "result", type: "request", aggregate_id: <id>, value: <text> }
```

Completion response:

```text
{
  action: "start",
  type: "response",
  aggregate_id: <id>,
  trace_id: <trace_id>,
  status: "success",
  state: "complete",
  success_count: <count>,
  failure_count: 0,
  result: [<input-1>, <input-2>, ...],
  received_count: <count>
}
```

## Implementation and Tests

Implementation: [`aggregation-1.0.0.method`](aggregation-1.0.0.method)

Test: [`aggregation_tests.c`](aggregation_tests.c)
