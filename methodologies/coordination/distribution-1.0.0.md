# Distribution Method v1.0.0

## Overview

The distribution method assigns a caller-provided work payload to an unbounded list of worker
agents. It composes with the routing method for delivery, making fan-out reusable without
duplicating route construction in every agency.

## Behavior

On a map whose `action` field is `"distribute"`, the method stores the work id, routing agent, reply
target, and work text. It sends one `mode: "many"` route request to the routing agent, using the
request's `workers` list as the route target list.

The route request has `payload_action` set to `"work"`, uses `work_text` as `payload_text`, and uses
the work id as the correlation id. The routing agent sends its final `route_result` back to the
distribution agent. Distribution translates that result into `distribution_result`, preserving the
router's routed and sent counts. Its `route_sent` field reports whether the original route request
was handed to the routing agent; later `route_result` processing does not overwrite that handoff
status. Route results are accepted only when their `correlation_id` matches the active work id, so
late results from other jobs cannot be misreported as the current distribution. After a terminal
`distribution_result` is sent successfully, duplicate `route_result` messages for the same work id
are ignored.
The distribution is marked completed only when a terminal result report is actually delivered.
If terminal report delivery fails, distribution leaves `pending_report` set and accepts a later
`retry_report` message for the same `work_id` with a replacement `reply_to` target. This retries the
stored `distribution_result` without rerouting the work. The retry must be sent before the next
`distribute` request, because the method maintains a single active work slot.

If the routing agent reports a matching terminal `route_result` with `status: "route_failed"`,
distribution propagates that failure as `distribution_result.status: "route_failed"` while preserving
the partial routed and sent counts.

If the initial route request cannot be sent to the routing agent, distribution immediately emits a
terminal `distribution_result` with `status: "route_failed"`, `assignment_count: 0`,
`sent_count: 0`, and `route_sent: 0`. Later stale `route_result` messages are ignored after that
failed handoff.

## Message Format

Distribution request:

```text
{
  action: "distribute",
  work_id: <id>,
  routing_agent: <agent>,
  reply_to: <agent>,
  workers: [<agent>, <agent>, ...],
  work_text: <text>
}
```

Report retry request:

```text
{
  action: "retry_report",
  work_id: <id>,
  reply_to: <agent>
}
```

Route message sent to routing:

```text
{
  action: "route",
  mode: "many",
  targets: [<agent>, <agent>, ...],
  payload_action: "work",
  payload_text: <work_text>,
  correlation_id: <work_id>,
  reply_to: <distribution-agent>,
  routed_count: 0,
  sent_count: 0
}
```

Result response:

```text
{
  action: "distribution_result",
  status: <distributed|route_failed>,
  work_id: <id>,
  assignment_count: <count>,
  sent_count: <count>,
  route_status: <status>,
  route_sent: <0|1>
}
```

## Action Field

The input `action` field is a command discriminator in the request map. The distribution agent runs
this method for every message it receives, so `action: "distribute"` marks the message as work to
fan out rather than an arbitrary status, worker result, or coordination message.
`action: "retry_report"` is a separate coordination command that retries a pending terminal report
for the matching `work_id` without sending a second route request.

## Composition Notes

Pair distribution with aggregation for fan-out and fan-in. A workflow step can send work to a
distribution agent, distribution can use routing for unbounded worker delivery, workers can return
maps whose `action` field is `"result"` to an aggregation agent, and aggregation can emit completion
when enough results arrive.

## Limitations

The method assigns one work payload to an unbounded worker list. Dynamic decomposition into distinct
per-worker portions, load-aware placement, and custom partitioning policies require another
decomposition method or richer collection-processing conventions. It keeps one pending terminal
report for the active work slot; keyed multi-report retention across later `distribute` requests
would require an exists/default operation or richer map/list filtering patterns.

## Implementation and Tests

Implementation: [`distribution-1.0.0.method`](distribution-1.0.0.method)

Test: [`distribution_tests.c`](distribution_tests.c)
