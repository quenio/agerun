# Routing Method v1.0.0

## Overview

The routing method selects recipients for a message and forwards a normalized payload to those
recipients. It is the lowest-level delivery primitive in the coordination methodology and is used
by higher-level methods such as distribution and workflow.

## Behavior

When the agent receives a map whose `action` field is `"route"`, it builds a forwarded message from
the payload fields and sends it to either one selected target or an unbounded list of targets.

For `mode=one`, the method sends to `target` when it is greater than zero. If `target` is not set,
it can select `target_a`, `target_b`, or `target_c` by matching `route_key` against the corresponding
`route_*_key` field.

For `mode=many`, the method reads `targets` as a list of nonzero agent IDs. It uses `head(...)` to
send to the next target and `tail(...)` to send a continuation message to itself with the remaining
targets. This keeps fan-out in ordinary method code instead of adding a runtime routing capability.
If a required self-continuation cannot be queued, the method emits a terminal `route_result` with
`status` set to `"route_failed"` and the routed/sent counts accumulated so far.

## Message Format

One-to-one route request:

```text
{
  action: "route",
  mode: "one",
  target: <agent>,
  payload_action: <action>,
  payload_text: <text>,
  correlation_id: <id>,
  reply_to: <agent>
}
```

One-to-many route request:

```text
{
  action: "route",
  mode: "many",
  targets: [<agent>, <agent>, ...],
  payload_action: <action>,
  payload_text: <text>,
  correlation_id: <id>,
  reply_to: <agent>
}
```

Optional keyed fields for one-to-one selection:

```text
{
  route_key: <key>,
  route_a_key: <key>,
  route_b_key: <key>,
  route_c_key: <key>
}
```

Forwarded message:

```text
{
  action: <payload_action>,
  correlation_id: <correlation_id>,
  text: <payload_text>,
  source: <routing-agent>
}
```

Reply:

```text
{
  action: "route_result",
  status: <routed|ignored|route_failed>,
  correlation_id: <correlation_id>,
  routed_count: <count>,
  sent_count: <count>,
  sent_one: <0|1>,
  sent_many: <0|1>,
  continuation_sent: <0|1>
}
```

For `mode=many`, `routed_count` and `sent_count` accumulate across the self-message chain. The final
reply is emitted after the target list is exhausted. The reply preserves the original
`correlation_id` so downstream coordination methods can match route results to their active work.
If a self-continuation send fails before the list is exhausted, the reply is emitted immediately with
partial counts and `continuation_sent` set to `0`.

## Action Field

The input `action` field is a command discriminator in the request map. The routing agent runs this
method for every message it receives, so `action: "route"` marks the message as a routing request and
lets the method avoid forwarding unrelated maps that happen to contain fields such as `target`,
`mode`, or `payload_action`.

## Composition Notes

Distribution uses routing to assign work portions to workers. Workflow uses routing to dispatch
activity steps without embedding worker-specific delivery logic. Larger agencies can pass a
primitive target list to `mode=many` when the fan-out size is not known ahead of time.

## Limitations

The method supports unbounded fan-out for primitive nonzero agent IDs. The `head(...)` empty
sentinel is integer `0`, so `0` cannot be used as a valid fan-out target. Continuation messages keep
the remaining target list in `memory.continuation_message`; `send(...)` deep-copies that nested list
when routing back to the same agent.

## Implementation and Tests

Implementation: [`routing-1.0.0.method`](routing-1.0.0.method)

Test: [`routing_tests.c`](routing_tests.c)
