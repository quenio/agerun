# Routing Method v1.0.0

## Overview

Routing selects exactly one recipient by matching a request key against an unbounded keyed route
table. It is a keyed-selection primitive, not a direct recipient delivery or fan-out primitive.
Because routing is a stateless single-call method, its request and response use `trace_id` but do
not require `session_id`; an omitted `trace_id` is generated for the result envelope and internal
scan messages.

## Behavior

Only messages with a recognized `request` value are handled as coordination requests.

When the agent receives `request: "routing_start"`, the method scans `routes.keys` and
`routes.recipients` as paired unbounded lists. It sends the sender-provided `payload` as-is to
the first positive recipient agent whose paired key matches `route_key`.

If no keyed candidate selects a positive recipient agent, or if the matching recipient cannot receive the
payload, routing responds with standard `status: "failure"`.

## Message Format

Request:

```text
{
  sender: <sender-agent>,
  request: "routing_start",
  trace_id: <trace_id>,
  payload: <message>,
  route_key: <key>,
  routes: {
    keys: [<key>, <key>, ...],
    recipients: [<recipient-agent-1>, <recipient-agent-2>, ...]
  }
}
```

Response:

```text
{
  sender: <routing-agent>,
  response: "routing_result",
  trace_id: <trace_id>,
  status: <success|failure>,
  routed_count: <0|1>,
  success_count: <0|1>,
  failure_count: <0|1>,
  sent_count: <0|1>,
  failed_count: <0|1>
}
```

Count semantics: `success_count` increments to `1` only when the matched positive recipient receives
the sender-provided `payload`. `failure_count` increments to `1` only when that matched recipient
send is attempted and fails. A route miss, a non-positive recipient, or an internal scan handoff
failure leaves both counts at `0` even though the response status is `failure`.

Status semantics: the response status is `success` only when routing delivers the payload to the
matched positive recipient. It is `failure` when no route selects a positive recipient, when the
matched recipient send fails, or when an internal scan handoff fails.

A direct `recipient` field is ignored; callers that already know the recipient should use direct
`send(...)`. The `head(...)` empty sentinel is integer `0`, so `0` cannot be used as a valid route
key.

## Implementation and Tests

Implementation: [`routing-1.0.0.method`](routing-1.0.0.method)

Test: [`routing_tests.c`](routing_tests.c)
