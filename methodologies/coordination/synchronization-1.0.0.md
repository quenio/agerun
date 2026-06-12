# Synchronization Method v1.0.0

## Overview

Synchronization waits for an unbounded stream of dependency messages before sending a continuation
message. It is a reusable dependency gate for workflows and distributed work.

## Behavior

Only messages with a recognized `request` value are handled as coordination requests.

On `request: "synchronization_wait"`, the method stores the sync id, required count, continuation
target agent, continuation request, continuation text, `trace_id`, and `source_agent`. On matching
`synchronization_dependency`, it appends the dependency value until the required count is reached.

Completion is recorded only after the continuation is delivered and, when `source_agent` is
positive, the `wait` response is delivered. Failed delivery keeps the gate open for retry.

## Message Format

Requests:

```text
{ request: "synchronization_wait", sync_id: <id>, trace_id: <trace_id>, required_count: <count>, continuation_target_agent: <agent>, continuation_request: <request>, continuation_text: <text>, source_agent: <agent> }
{ request: "synchronization_dependency", sync_id: <id>, dependency: <name> }
```

Continuation:

```text
{
  request: <continuation_request>,
  sync_id: <id>,
  trace_id: <trace_id>,
  text: <continuation_text>,
  done_count: <count>,
  dependencies: [<dependency>, <dependency>, ...]
}
```

Response:

```text
{
  response: "synchronization_result",
  sync_id: <id>,
  trace_id: <trace_id>,
  status: "success",
  state: "complete",
  success_count: <count>,
  failure_count: 0,
  done_count: <count>,
  dependencies: [<dependency>, <dependency>, ...]
}
```

## Implementation and Tests

Implementation: [`synchronization-1.0.0.method`](synchronization-1.0.0.method)

Test: [`synchronization_tests.c`](synchronization_tests.c)
