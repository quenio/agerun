# Broadcasting Method v1.0.0

## Overview

Broadcasting sends the same sender-provided payload to every positive recipient in an unbounded
`recipients` list. It is the coordination methodology's opaque fan-out delivery primitive.
Because broadcasting is a stateless single-call method, its request and response use `trace_id` but
do not require `session_id`; an omitted `trace_id` is generated for the result envelope and
internal continuation messages.

## Behavior

Only messages with a recognized `request` value are handled as coordination requests. Public
callers use `broadcasting_start`; self-sent `broadcasting_continue` messages carry internal
continuation counters. External `broadcasting_continue` requests are ignored.

When the agent receives `request: "broadcasting_start"`, the method sends `payload` as-is to
each positive recipient agent. It processes the list with `head(...)` and `tail(...)`, sending
continuation messages to itself until the list is exhausted. Missing `recipients` are normalized to
an empty recipient list before traversal.

Positive recipient agents that cannot receive messages are counted as failures, but valid later recipient
agents are still processed. Integer `0` entries are placeholders and are skipped.

## Message Format

Request:

```text
{
  sender: <sender-agent>,
  request: "broadcasting_start",
  trace_id: <trace_id>,
  payload: <message>,
  recipients: [<recipient-agent-1>, <recipient-agent-2>, ...]
}
```

Delivered message:

```text
<message>
```

Response:

```text
{
  sender: <broadcasting-agent>,
  response: "broadcasting_result",
  trace_id: <trace_id>,
  status: <success|failure>,
  success_count: <count>,
  failure_count: <count>
}
```

Count semantics: `success_count` increments once for each positive recipient that accepts the
sender-provided `payload`. `failure_count` increments once for each positive recipient send that
fails. Integer `0` placeholders are skipped without affecting either count.

Status semantics: the response status is `success` when the broadcast completes with at least one
successful positive-recipient delivery and no recipient or continuation send failures. It is
`failure` when no positive recipient receives the payload, when any positive-recipient send fails,
or when an internal continuation handoff fails.

Broadcasting sends the sender-provided `payload` as-is.

## Implementation and Tests

Implementation: [`broadcasting-1.0.0.method`](broadcasting-1.0.0.method)

Test: [`broadcasting_tests.c`](broadcasting_tests.c)
