# Broadcasting Method v1.0.0

## Overview

Broadcasting sends the same caller-provided payload to every positive recipient in an unbounded
`target_agents` list. It is the coordination methodology's opaque fan-out delivery primitive.

## Behavior

Only messages with `type: "request"` are handled as coordination requests.

When the agent receives `action: "broadcast"`, the method sends `payload` as-is to each positive
target agent. It processes the list with `head(...)` and `tail(...)`, sending continuation messages
to itself until the list is exhausted.

Positive target agents that cannot receive messages are counted as failures, but valid later target
agents are still processed. Integer `0` entries are placeholders and are skipped.

## Message Format

Request:

```text
{
  action: "broadcast",
  type: "request",
  target_agents: [<agent>, <agent>, ...],
  payload: <message>,
  trace_id: <trace_id>,
  source_agent: <agent>
}
```

Delivered message:

```text
<message>
```

Response:

```text
{
  action: "broadcast",
  type: "response",
  status: <success|failure>,
  state: <broadcasted|broadcast_failed>,
  trace_id: <trace_id>,
  success_count: <count>,
  failure_count: <count>,
  recipient_count: <count>,
  sent_count: <count>,
  failed_count: <count>
}
```

Broadcasting sends the caller-provided `payload` as-is.

## Implementation and Tests

Implementation: [`broadcasting-1.0.0.method`](broadcasting-1.0.0.method)

Test: [`broadcasting_tests.c`](broadcasting_tests.c)
