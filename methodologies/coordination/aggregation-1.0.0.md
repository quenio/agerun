# Aggregation Method v1.0.0

## Overview

Aggregation collects opaque payload messages and emits one aggregate completion response when the
configured `expected_count` from the start request is accounted for. It provides an unbounded
fan-in counterpart to distribution.

## Behavior

Only messages with `request: "aggregation_start"` or `request: "aggregation_collect"` are handled
as coordination requests.

A start request resets aggregation, stores the effective `trace_id`, `session_id`, and `sender`,
sets the recipient count from `expected_count`, and clears the append-backed payload list. A collect
request appends the incoming opaque `payload` only when its `session_id` matches the active
aggregate session while aggregation is active and completion has not been delivered.

Collection requests that omit `trace_id` use a generated trace and still append their payload.
Count semantics: `success_count` increments when a matching active `aggregation_collect` appends its
payload. `failure_count` increments when such a collection attempt is accepted for the active
session but append fails. Wrong-session, inactive, or post-completion collect requests are ignored
and do not affect either count. The response is sent when `success_count + failure_count` equals
the configured `expected_count` and uses the completing collection request's effective `trace_id`.
Completion is recorded only after the aggregate response is sent successfully. The `request` field
differentiates start requests from collect requests; `expected_count` only configures the start
request threshold.

## Message Format

Requests:

```text
{ sender: <sender-agent>, request: "aggregation_start", trace_id: <trace_id>, session_id: <session_id>, expected_count: <count> }
{ sender: <sender-agent>, request: "aggregation_collect", trace_id: <trace_id>, session_id: <session_id>, payload: <payload> }
```

Completion response:

```text
{
  sender: <aggregation-agent>,
  response: "aggregation_result",
  trace_id: <trace_id>,
  session_id: <session_id>,
  payloads: [<payload-1>, <payload-2>, ...],
  status: <success|failure>,
  success_count: <count>,
  failure_count: <count>
}
```

The response status is `success` only when `success_count` equals the configured `expected_count`;
otherwise it is `failure`.

## Implementation and Tests

Implementation: [`aggregation-1.0.0.method`](aggregation-1.0.0.method)

Test: [`aggregation_tests.c`](aggregation_tests.c)
