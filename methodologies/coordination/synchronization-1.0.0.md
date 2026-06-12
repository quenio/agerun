# Synchronization Method v1.0.0

## Overview

Synchronization waits for an unbounded stream of dependency messages before sending a continuation
message. It is a reusable dependency gate for workflows and distributed work.

## Behavior

Only messages with a recognized `request` value are handled as coordination requests.

On `request: "synchronization_wait"`, the method stores the sync id, required count, continuation
target agent, continuation request, continuation text, `trace_id`, and `source`. On matching
`synchronization_dependency`, it appends the dependency value until the required count is reached.

Completion is recorded only after the continuation and response are delivered. Failed delivery
keeps the gate open for retry.

## Message Format

Requests:

```text
{ source: <agent>, request: "synchronization_wait", trace_id: <trace_id>, sync_id: <id>, required_count: <count>, continuation_target: <agent>, continuation_request: <request>, continuation_text: <text> }
{ source: <agent>, request: "synchronization_dependency", trace_id: <trace_id>, sync_id: <id>, dependency: <name> }
```

Continuation:

```text
{
  source: <synchronization-agent>,
  request: <continuation_request>,
  trace_id: <trace_id>,
  sync_id: <id>,
  text: <continuation_text>,
  done_count: <count>,
  dependencies: [<dependency>, <dependency>, ...]
}
```

Response:

```text
{
  source: <synchronization-agent>,
  response: "synchronization_result",
  trace_id: <trace_id>,
  status: "success",
  state: "complete",
  sync_id: <id>,
  success_count: <count>,
  failure_count: 0,
  done_count: <count>,
  dependencies: [<dependency>, <dependency>, ...]
}
```

## Implementation and Tests

Implementation: [`synchronization-1.0.0.method`](synchronization-1.0.0.method)

Test: [`synchronization_tests.c`](synchronization_tests.c)
