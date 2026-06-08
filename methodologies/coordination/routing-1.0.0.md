# Routing Method v1.0.0

## Overview

The routing method selects exactly one recipient by matching a request key against an unbounded route
table. It is a keyed-selection primitive, not a direct target delivery or fan-out primitive.

## Behavior

When the agent receives a map whose `action` field is `"route"`, the method scans `routes.keys` and
`routes.targets` as paired unbounded lists. It compares `route_key` with each candidate key and sends
the caller-provided `payload` as-is to the paired positive target for the first matching candidate.

If no keyed candidate selects a positive target, the method emits `route_result` with
`status: "route_failed"` and zero delivery counts. A direct `target` field is ignored; callers that
already know the recipient should send directly rather than using routing. Broadcasting to multiple
recipients is handled by the broadcasting method.

## Message Format

Keyed route request:

```text
{
  action: "route",
  route_key: <key>,
  routes: {
    keys: [<key>, <key>, ...],
    targets: [<agent>, <agent>, ...]
  },
  payload: <message>,
  correlation_id: <id>,
  reply_to: <agent>
}
```

Delivered message is exactly the caller-provided `payload`:

```text
<message>
```

Reply:

```text
{
  action: "route_result",
  status: <routed|route_failed>,
  correlation_id: <correlation_id>,
  routed_count: <0|1>,
  success_count: <0|1>,
  failure_count: <0|1>,
  sent_count: <0|1>,
  failed_count: <0|1>
}
```

The `routes.keys` and `routes.targets` lists are paired by position. The method scans them with
`head(...)` and `tail(...)`, so keyed routing is not limited to three candidates. Both the request
`route_key` and the candidate key must be nonzero/present before the candidate can match. If a
matching positive target cannot receive the delivery payload, the terminal result is
`route_failed`.

## Action Field

The input `action` field is a command discriminator in the request map. The routing agent runs this
method for every message it receives, so `action: "route"` marks the message as a keyed route request
and lets the method ignore unrelated maps that happen to contain fields such as `route_key`,
`routes`, or `payload`.

## Composition Notes

Use routing when an agency has a route table and needs to choose one recipient by key. Use direct
`send(...)` when the recipient is already known. Use broadcasting when the same payload should go to
all recipients in a list.

## Limitations

The method supports unbounded keyed one-to-one selection through parallel `routes.keys` and
`routes.targets` lists. The `head(...)` empty sentinel is integer `0`, so `0` cannot be used as a
valid route key. A list of route-entry maps would be a more natural external shape, but this v1 keeps
the route key and target lists separate so the method can scan them with ordinary `head(...)` and
`tail(...)` operations.

## Implementation and Tests

Implementation: [`routing-1.0.0.method`](routing-1.0.0.method)

Test: [`routing_tests.c`](routing_tests.c)
