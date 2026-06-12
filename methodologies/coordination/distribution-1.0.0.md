# Distribution Method v1.0.0

## Overview

Distribution assigns a list of payload values across a list of worker agents. It sends each payload
item to the next positive worker and rotates back to the beginning of the worker list when needed.

## Behavior

Only messages with a recognized `request` value are handled as coordination requests.

When the agent receives `request: "distribution_distribute"`, it starts an assignment pass for
`payloads` and `workers`. Internal `distribution_assign_payload` requests carry the remaining
payloads, current worker list, original worker list, counters, `trace_id`, and `source`.
Because distribution is a one-shot caller-facing method, its request and response use `trace_id`
but do not require `session_id`; an omitted `trace_id` is generated for the result envelope and
internal assignment messages.

Each assigned payload is sent as-is. Integer `0` worker placeholders are skipped without consuming
the current payload when later workers remain.

## Message Format

Request:

```text
{
  source: <agent>,
  request: "distribution_distribute",
  trace_id: <trace_id>,
  payloads: [<payload>, <payload>, ...],
  work_id: <id>,
  workers: [<agent>, <agent>, ...]
}
```

Response:

```text
{
  source: <distribution-agent>,
  response: "distribution_result",
  trace_id: <trace_id>,
  status: <success|failure>,
  state: <distributed|distribution_failed>,
  work_id: <id>,
  success_count: <count>,
  failure_count: <count>,
  assignment_count: <count>,
  sent_count: <count>,
  failed_count: <count>
}
```

`assignment_count` counts payload items assigned to positive worker IDs, including assignments whose
send failed. Empty payload or worker lists produce `state: "distribution_failed"`.

## Implementation and Tests

Implementation: [`distribution-1.0.0.method`](distribution-1.0.0.method)

Test: [`distribution_tests.c`](distribution_tests.c)
