# Workflow Method v1.0.0

## Overview

Workflow sends an unbounded sequence of activity steps to configured target agents. It demonstrates
a higher-level coordination behavior built on the `head(...)` and `tail(...)` list traversal
pattern.

## Behavior

Only messages with a recognized `request` value are handled as coordination requests.

On `request: "workflow_start"`, the method stores workflow metadata, `trace_id`, `source_agent`,
branch value, and aligned `step_target_agents` and `step_payloads` lists. Each step payload is sent
directly and as-is to the corresponding positive step target agent. Integer `0` step target agents
can be skipped when followed by a later positive target.

On `request: "workflow_step_done"`, the method advances only when the workflow id and step number
match the currently active sent step. Duplicate, stale, premature, or out-of-order completions are
ignored.

## Message Format

Requests:

```text
{ request: "workflow_start", workflow_id: <id>, trace_id: <trace_id>, source_agent: <agent>, step_target_agents: [<agent>, ...], step_payloads: [<message>, ...], branch_value: <outcome> }
{ request: "workflow_step_done", workflow_id: <id>, step: <current-step-number>, outcome: <value> }
```

Completion response:

```text
{
  response: "workflow_result",
  workflow_id: <id>,
  trace_id: <trace_id>,
  status: <success|failure>,
  state: <complete|handoff_failed>,
  success_count: <count>,
  failure_count: <count>,
  current_step: <last-step-number>,
  completed_step_count: <executed-step-count>
}
```

Terminal status is recorded only after the response is delivered; failed completion delivery leaves
completion pending and retries do not increment `completed_step_count`.

## Implementation and Tests

Implementation: [`workflow-1.0.0.method`](workflow-1.0.0.method)

Test: [`workflow_tests.c`](workflow_tests.c)
