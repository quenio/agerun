# Routing Method v1.0.0

## Overview

The routing method selects recipients for a message and forwards a normalized payload to those
recipients. It is the lowest-level delivery primitive in the coordination methodology and is used
by higher-level methods such as distribution and workflow.

## Behavior

When the agent receives a map whose `action` field is `"route"`, it builds a forwarded message from
the payload fields and sends it to either one selected target or up to three explicit targets.

For `mode=one`, the method sends to `target` when it is greater than zero. If `target` is not set,
it can select `target_a`, `target_b`, or `target_c` by matching `route_key` against the corresponding
`route_*_key` field.

For `mode=many`, the method sends the same forwarded message to `target_a`, `target_b`, and
`target_c`. A target value of zero relies on the ordinary `send(0, message)` no-op behavior.

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
  target_a: <agent>,
  target_b: <agent>,
  target_c: <agent>,
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
  status: <routed|ignored>,
  routed_count: <count>,
  sent_one: <0|1>,
  sent_a: <0|1>,
  sent_b: <0|1>,
  sent_c: <0|1>
}
```

## Composition Notes

Distribution uses routing to assign work portions to workers. Workflow uses routing to dispatch
activity steps without embedding worker-specific delivery logic.

## Limitations

The method supports a bounded contract of one selected target or three fan-out targets. Arbitrary
recipient lists require collection iteration or a richer data query convention outside the current
ordinary method language.

## Implementation and Tests

Implementation: [`routing-1.0.0.method`](routing-1.0.0.method)

Test: [`routing_tests.c`](routing_tests.c)
