# Workflow Method v1.0.0

## Overview

Workflow sends an unbounded sequence of activity steps to configured recipient agents. It demonstrates
a higher-level coordination behavior built on the `head(...)` and `tail(...)` list traversal
pattern.

## Behavior

Only messages with a recognized `request` value are handled as coordination requests.

On `request: "workflow_start"`, the method stores workflow metadata, effective `trace_id`,
`session_id`, `sender`, branch value, and aligned `recipients` and `payloads` lists. Each
payload is sent directly and as-is to the corresponding positive step recipient agent. Integer
`0` step recipient agents can be skipped when followed by a later positive recipient.

On `request: "workflow_step_done"`, the method advances only when the workflow id and step number
match the currently active sent step and the `session_id` matches the active workflow session.
Duplicate, stale, premature, or out-of-order completions are ignored.

## Message Format

Requests:

```text
{ sender: <sender-agent>, request: "workflow_start", trace_id: <trace_id>, session_id: <session_id>, workflow_id: <id>, recipients: [<recipient-agent-1>, <recipient-agent-2>, ...], payloads: [<payload>, <payload>, ...], branch_value: <outcome> }
{ sender: <sender-agent>, request: "workflow_step_done", trace_id: <trace_id>, session_id: <session_id>, workflow_id: <id>, step: <current-step-number>, outcome: <value> }
```

Completion response:

```text
{
  sender: <workflow-agent>,
  response: "workflow_result",
  trace_id: <trace_id>,
  session_id: <session_id>,
  status: <success|failure>,
  state: <complete|handoff_failed>,
  workflow_id: <id>,
  success_count: <count>,
  failure_count: <count>
}
```

Count semantics: `success_count` increments when a matching `workflow_step_done` completes the
currently awaited sent step. Skipped zero-recipient placeholders, stale completions, duplicate
completions, out-of-order completions, and pending completion retries do not increment it.
`failure_count` becomes `1` when any workflow handoff fails, including start or continuation
self-sends, skipped-step self-sends, or direct step payload sends; otherwise it is `0`.

Terminal status is recorded only after the response is delivered; failed completion delivery leaves
completion pending and retries do not increment the completed-step counter. Completion responses use the
triggering workflow control request's effective `trace_id` and the active workflow `session_id`.

## Implementation and Tests

Implementation: [`workflow-1.0.0.method`](workflow-1.0.0.method)

Test: [`workflow_tests.c`](workflow_tests.c)
