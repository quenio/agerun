# Supervision Method v1.0.0

## Overview

Supervision creates and tracks an unbounded list of child agents and applies a simple restart policy
when it receives explicit lifecycle event messages. It is methodology logic rather than a runtime
capability.

## Behavior

Only messages with a recognized `request` value are handled as coordination requests.

On `request: "supervision_start"`, the method stores policy, effective `trace_id`, `session_id`,
`source`, and child method version, then spawns one child per `child_method_names` entry.
Lifecycle and stop requests with the same `session_id` are validated against tracked child ids
before restart or exit behavior is applied.

Untracked lifecycle and stop requests report `state: "ignored"`. Handoff failures report
`state: "handoff_failed"` and standard `status: "failure"`.

## Message Format

Requests:

```text
{ source: <agent>, request: "supervision_start", trace_id: <trace_id>, session_id: <session_id>, child_method_names: [<method>, ...], child_method_version: <version>, policy: "restart" }
{ source: <agent>, request: "supervision_child_failed", trace_id: <trace_id>, session_id: <session_id>, child_agent_id: <agent>, child_method_name: <method>, child_method_version: <version> }
{ source: <agent>, request: "supervision_child_exited", trace_id: <trace_id>, session_id: <session_id>, child_agent_id: <agent>, child_method_name: <method>, child_method_version: <version> }
{ source: <agent>, request: "supervision_stop", trace_id: <trace_id>, session_id: <session_id>, child_agent_id: <agent> }
```

Response:

```text
{
  source: <supervision-agent>,
  response: "supervision_result",
  trace_id: <trace_id>,
  session_id: <session_id>,
  status: <success|failure>,
  state: <running|restarted|stopped|ignored|stop_failed|handoff_failed>,
  success_count: <count>,
  failure_count: <count>,
  child_agent_id: <agent>,
  child_agent_ids: [<agent>, <agent>, ...],
  child_records: [<child-record>, <child-record>, ...],
  child_count: <count>,
  restart_count: <count>,
  policy: <policy>
}
```

## Implementation and Tests

Implementation: [`supervision-1.0.0.method`](supervision-1.0.0.method)

Test: [`supervision_tests.c`](supervision_tests.c)
