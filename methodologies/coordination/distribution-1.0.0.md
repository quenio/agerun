# Distribution Method v1.0.0

## Overview

Distribution assigns a list of payload values across a list of recipient agents. It sends each
payload item to the next positive recipient and rotates back to the beginning of the recipient list
when needed.

## Behavior

Only messages with a recognized `request` value are handled as coordination requests.

When the agent receives `request: "distribution_start"`, it starts an assignment pass for
`payloads` and `recipients`. Internal `distribution_assign_payload` requests carry the remaining
payloads, current recipient list, original recipient list, counters, `trace_id`, and `sender`.
Because distribution is a one-shot caller-facing method, its request and response use `trace_id`
but do not require `session_id`; an omitted `trace_id` is generated for the result envelope and
internal assignment messages.

Each assigned payload is sent as-is. Integer `0` recipient placeholders are skipped without
consuming the current payload when later recipients remain.

## Message Format

Request:

```text
{
  sender: <sender-agent>,
  request: "distribution_start",
  trace_id: <trace_id>,
  payloads: [<payload>, <payload>, ...],
  recipients: [<recipient-agent-1>, <recipient-agent-2>, ...]
}
```

Response:

```text
{
  sender: <distribution-agent>,
  response: "distribution_result",
  trace_id: <trace_id>,
  status: <success|failure>,
  success_count: <count>,
  failure_count: <count>
}
```

`success_count` counts successful assignment sends, and `failure_count` counts failed assignment
sends. Empty payload or recipient lists produce `status: "failure"`.

## Implementation and Tests

Implementation: [`distribution-1.0.0.method`](distribution-1.0.0.method)

Test: [`distribution_tests.c`](distribution_tests.c)
