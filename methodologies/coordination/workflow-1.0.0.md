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
`0` step recipient agents are placeholders and are skipped until the next positive recipient or
workflow completion. When a branch outcome matches, workflow skips the next positive recipient step,
ignoring any intervening `0` placeholders.
If a positive step recipient has no corresponding payload item, workflow completes with failure
instead of inventing a payload.

Internal `workflow_execute_step` continuations are accepted only when self-sent by the workflow
agent for the active `session_id`.

On `request: "workflow_step_done"`, the method advances only when the step number matches the
currently active sent step and the `session_id` matches the active workflow session. Duplicate,
stale, premature, or out-of-order completions are ignored.

## Message Format

Requests:

```text
{ sender: <sender-agent>, request: "workflow_start", trace_id: <trace_id>, session_id: <session_id>, recipients: [<recipient-agent-1>, <recipient-agent-2>, ...], payloads: [<payload>, <payload>, ...], branch_value: <outcome> }
{ sender: <sender-agent>, request: "workflow_step_done", trace_id: <trace_id>, session_id: <session_id>, step: <current-step-number>, outcome: <value> }
```

Completion response:

```text
{
  sender: <workflow-agent>,
  response: "workflow_result",
  trace_id: <trace_id>,
  session_id: <session_id>,
  status: <success|failure>,
  success_count: <count>,
  failure_count: <count>
}
```

Count semantics: `success_count` increments when a matching `workflow_step_done` completes the
currently awaited sent step. Skipped zero-recipient placeholders, stale completions, duplicate
completions, out-of-order completions, and pending completion retries do not increment it.
`failure_count` becomes `1` when any workflow handoff fails, including start or continuation
self-sends, skipped-step self-sends, missing step payloads, or direct step payload sends; otherwise
it is `0`.

Status semantics: the completion response status is `success` when workflow completion is reached
without a handoff failure, including empty completion after skipped placeholders or completion after
the final step. It is `failure` when any workflow handoff fails or a positive step recipient has no
corresponding payload item.

Terminal status is recorded only after the response is delivered; failed completion delivery leaves
completion pending and retries do not increment the completed-step counter. Completion responses use the
triggering workflow control request's effective `trace_id` and the active workflow `session_id`.

## Implementation and Tests

Implementation: [`workflow-1.0.0.method`](workflow-1.0.0.method)

Test: [`workflow_tests.c`](workflow_tests.c)
