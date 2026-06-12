# Aggregation Method v1.0.0

## Overview

Aggregation collects opaque payload messages and emits one aggregate completion response when the
configured payload count is met. It provides an unbounded fan-in counterpart to distribution.

## Behavior

Only messages with `type: "request"` and `action: "aggregate"` are handled as coordination
requests.

An aggregate request with a positive `count` resets aggregation, stores `trace_id` and
`source_agent`, and clears the append-backed payload list. An aggregate request without a positive
`count` appends the incoming opaque `payload` while aggregation is active and completion has not
been delivered.

Completion is recorded only after the aggregate response is sent successfully.

## Message Format

Requests:

```text
{ action: "aggregate", type: "request", count: <count>, trace_id: <id>, source_agent: <agent> }
{ action: "aggregate", type: "request", payload: <payload> }
```

Completion response:

```text
{
  action: "aggregate",
  type: "response",
  trace_id: <trace_id>,
  status: "success",
  state: "complete",
  success_count: <count>,
  failure_count: 0,
  payloads: [<payload-1>, <payload-2>, ...]
}
```

## Implementation and Tests

Implementation: [`aggregation-1.0.0.method`](aggregation-1.0.0.method)

Test: [`aggregation_tests.c`](aggregation_tests.c)
