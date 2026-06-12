# Routing Method v1.0.0

## Overview

Routing selects exactly one recipient by matching a request key against an unbounded keyed route
table. It is a keyed-selection primitive, not a direct target delivery or fan-out primitive.

## Behavior

Only messages with a recognized `request` value are handled as coordination requests.

When the agent receives `request: "routing_route"`, the method scans `routes.keys` and
`routes.targets` as paired unbounded lists. It sends the caller-provided `payload` as-is to
the first positive target agent whose paired key matches `route_key`.

If no keyed candidate selects a positive target agent, or if the matching target cannot receive the
payload, routing responds with `state: "route_failed"` and standard `status: "failure"`.

## Message Format

Request:

```text
{
  source: <agent>,
  request: "routing_route",
  trace_id: <trace_id>,
  payload: <message>,
  route_key: <key>,
  routes: {
    keys: [<key>, <key>, ...],
    targets: [<agent>, <agent>, ...]
  }
}
```

Response:

```text
{
  source: <routing-agent>,
  response: "routing_result",
  trace_id: <trace_id>,
  status: <success|failure>,
  state: <routed|route_failed>,
  routed_count: <0|1>,
  success_count: <0|1>,
  failure_count: <0|1>,
  sent_count: <0|1>,
  failed_count: <0|1>
}
```

A direct `target` field is ignored; callers that already know the recipient should use direct
`send(...)`. The `head(...)` empty sentinel is integer `0`, so `0` cannot be used as a valid route
key.

## Implementation and Tests

Implementation: [`routing-1.0.0.method`](routing-1.0.0.method)

Test: [`routing_tests.c`](routing_tests.c)
