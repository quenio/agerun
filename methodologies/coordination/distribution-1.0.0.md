# Distribution Method v1.0.0

## Overview

Distribution assigns a list of payload values across a list of worker agents. It sends each payload
item to the next positive worker and rotates back to the beginning of the worker list when needed.

## Behavior

Only messages with `type: "request"` are handled as coordination requests.

When the agent receives `action: "distribute"`, it starts an assignment pass for `payloads` and
`workers`. Internal `assign_payload` messages carry the remaining payloads, current worker list,
original worker list, counters, `trace_id`, and `source_agent`.

Each assigned payload is sent as-is. Integer `0` worker placeholders are skipped without consuming
the current payload when later workers remain.

## Message Format

Request:

```text
{
  action: "distribute",
  type: "request",
  work_id: <id>,
  payloads: [<payload>, <payload>, ...],
  workers: [<agent>, <agent>, ...],
  trace_id: <trace_id>,
  source_agent: <agent>
}
```

Response:

```text
{
  action: "distribute",
  type: "response",
  status: <success|failure>,
  state: <distributed|distribution_failed>,
  work_id: <id>,
  trace_id: <trace_id>,
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
