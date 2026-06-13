# Distribution Method v1.0.0

## Overview

Distribution assigns a list of payload values across a list of recipient agents. It sends each
payload item to the next positive recipient and rotates back to the beginning of the recipient list
when needed.

## Behavior

Only messages with a recognized `request` value are handled as coordination requests.

When the agent receives `request: "distribution_start"`, it starts an assignment pass for
`payloads` and `recipients`. Internal `distribution_continue` requests carry the remaining
payloads, current recipient list, original recipient list, counters, `trace_id`, and
`result_recipient`.
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

Internal continuation request:

```text
{
  sender: <distribution-agent>,
  request: "distribution_continue",
  trace_id: <trace_id>,
  payloads: [<payload>, <payload>, ...],
  recipients: [<recipient-agent-1>, <recipient-agent-2>, ...],
  all_recipients: [<recipient-agent-1>, <recipient-agent-2>, ...],
  result_recipient: <sender-agent>,
  assignment_count: <count>,
  sent_count: <count>,
  failed_count: <count>
}
```

`distribution_continue` is queued by the distribution agent to itself and is processed only when its
`sender` is the distribution agent. Callers should use `distribution_start`; the continuation format
is documented to make the recursive state explicit.

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

Count semantics: `success_count` increments once for each assignment send of one payload item to a
positive recipient that succeeds. `failure_count` increments once for each attempted assignment send
that fails. Integer `0` recipient placeholders are skipped without consuming the payload or
affecting either count. Empty payload or recipient lists produce `status: "failure"` with no
assignment count increments.

Status semantics: the response status is `success` when at least one assignment is attempted and all
attempted assignment sends succeed. It is `failure` when no assignment is attempted, when any
assignment send fails, or when an internal continuation handoff fails.

## Implementation and Tests

Implementation: [`distribution-1.0.0.method`](distribution-1.0.0.method)

Test: [`distribution_tests.c`](distribution_tests.c)
