# Aggregation Method v1.0.0

## Overview

Aggregation collects opaque payload messages and emits one aggregate completion response when the
configured `expected_count` from the start request is accounted for. It provides an unbounded
fan-in counterpart to distribution.

## Behavior

Only messages with `request: "aggregation_start"` or `request: "aggregation_collect"` are handled
as coordination requests.

A start request resets aggregation, stores `trace_id` and `source_agent`, sets the target count
from `expected_count`, and clears the append-backed payload list. A collect request appends the
incoming opaque `payload` only when its `trace_id` matches the active aggregate trace while
aggregation is active and completion has not been delivered.

Mismatched or missing collection traces fail the collection request and do not append its payload.
Those failed collection attempts are reported through `failure_count` when the aggregate response is
eventually returned. The response is sent when `success_count + failure_count` equals the configured
`expected_count`. Completion is recorded only after the aggregate response is sent successfully.
The `request` field differentiates start requests from collect requests; `expected_count`
only configures the start request threshold.

## Message Format

Requests:

```text
{ request: "aggregation_start", expected_count: <count>, trace_id: <trace_id>, source_agent: <agent> }
{ request: "aggregation_collect", trace_id: <trace_id>, payload: <payload> }
```

Completion response:

```text
{
  response: "aggregation_result",
  trace_id: <trace_id>,
  status: <success|failure>,
  success_count: <count>,
  failure_count: <count>,
  payloads: [<payload-1>, <payload-2>, ...]
}
```

The response status is `success` only when `success_count` equals the configured `expected_count`;
otherwise it is `failure`.

## Implementation and Tests

Implementation: [`aggregation-1.0.0.method`](aggregation-1.0.0.method)

Test: [`aggregation_tests.c`](aggregation_tests.c)
