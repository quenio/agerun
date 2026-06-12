# Coordination Methodology

The coordination methodology is a reusable standard library of ordinary AgeRun methods for
building agencies out of common coordination behaviors. These methods are stored as loadable
`.method` files in this methodology directory and use only existing AgeRun instructions:
`send`, `parse`, `build`, `complete`, `compile`, `spawn`, `exit`, `deprecate`, and `if`.

No runtime capability is introduced by this methodology. Composition happens by sending messages
between agents that run these methods.

## Dependency Graph

```text
routing
broadcasting
distribution
workflow

scheduling
  retry

aggregation
synchronization
supervision
conversation

workflow can coordinate other method agents by sending caller-provided step payloads directly to
those agents.
```

Composition opportunities:

- Use `routing` when one recipient must be selected from a key-based route table.
- Use `broadcasting` when one payload must be sent to every recipient in a target list.
- Build distinct work assignment with `distribution` and fan-in with `aggregation`.
- Combine `synchronization` with `workflow` to gate step advancement on required dependency
  messages.
- Use `scheduling` as the delayed execution primitive for `retry`.
- Use `conversation` to preserve trace and context around workflow or routing requests.
- Use `supervision` to keep long-lived routing, broadcasting, scheduling, workflow, or worker
  agents present.

## Methods

| Method | Implementation | Test | Purpose | Composition Role |
| --- | --- | --- | --- | --- |
| [`routing`](routing-1.0.0.md) | [`routing-1.0.0.method`](routing-1.0.0.method) | [`routing_tests.c`](routing_tests.c) | Selects one recipient by route key and delivers a message. | Keyed selection primitive. |
| [`broadcasting`](broadcasting-1.0.0.md) | [`broadcasting-1.0.0.method`](broadcasting-1.0.0.method) | [`broadcasting_tests.c`](broadcasting_tests.c) | Sends one payload to every recipient in an unbounded target list. | Same-payload fan-out primitive. |
| [`supervision`](supervision-1.0.0.md) | [`supervision-1.0.0.method`](supervision-1.0.0.method) | [`supervision_tests.c`](supervision_tests.c) | Creates, tracks, stops, and event-restarts unbounded child lists. | Keeps coordination agents available. |
| [`distribution`](distribution-1.0.0.md) | [`distribution-1.0.0.method`](distribution-1.0.0.method) | [`distribution_tests.c`](distribution_tests.c) | Round-robins payload items across an unbounded worker list. | Distinct-payload assignment. |
| [`aggregation`](aggregation-1.0.0.md) | [`aggregation-1.0.0.method`](aggregation-1.0.0.method) | [`aggregation_tests.c`](aggregation_tests.c) | Appends opaque payloads and emits a payload list. | Completes fan-in with append-backed state. |
| [`scheduling`](scheduling-1.0.0.md) | [`scheduling-1.0.0.method`](scheduling-1.0.0.method) | [`scheduling_tests.c`](scheduling_tests.c) | Stores pending work and triggers it on explicit tick messages. | Delayed execution primitive. |
| [`synchronization`](synchronization-1.0.0.md) | [`synchronization-1.0.0.method`](synchronization-1.0.0.method) | [`synchronization_tests.c`](synchronization_tests.c) | Waits for an unbounded count of dependency messages before sending a continuation. | Dependency gate. |
| [`workflow`](workflow-1.0.0.md) | [`workflow-1.0.0.method`](workflow-1.0.0.method) | [`workflow_tests.c`](workflow_tests.c) | Sends an unbounded step sequence, supports a branch skip, and completes. | Higher-level sequence and branch coordinator. |
| [`conversation`](conversation-1.0.0.md) | [`conversation-1.0.0.method`](conversation-1.0.0.method) | [`conversation_tests.c`](conversation_tests.c) | Coordinates a bounded conversation between two participant agents. | Mediated two-agent exchange. |
| [`retry`](retry-1.0.0.md) | [`retry-1.0.0.method`](retry-1.0.0.method) | [`retry_tests.c`](retry_tests.c) | Re-executes failed operations within a retry policy. | Uses direct send or scheduled retry. |

## Message Contracts

The contracts below are map-shaped messages. Method implementations that create new messages assign
a multi-line map literal to a named memory value, then pass that value to `send(...)`. This works for
flat and nested messages because `send(...)`, `append(...)`, `head(...)`, and `tail(...)` deep-copy
borrowed maps and lists.

By AgeRun method convention, every external request includes a `source` value identifying the
originating agent and a `request` value that identifies the requested command with the
`<method>_<action>` naming convention. Every external response includes a `source` value identifying
the method agent that produced the response and a `response` value that identifies the result
envelope with the `<method>_result` naming convention, preserves the request's `trace_id`, and
reports standard `status: "success"` or `status: "failure"` with `success_count` and
`failure_count`.
Method-specific contracts define any additional response fields. When a method eventually returns
multiple distinct result envelopes, the `response` value should use `<method>_<result_kind>`.
Method-specific outcomes such as `routed`, `broadcast_failed`, or `handoff_failed` are carried in
`state` when the response needs that detail. Coordination methods handle only recognized `request`
values as coordination commands; messages with `response` values are not treated as new
coordination requests.

### Routing

Request:

```text
{
  source: <agent>,
  request: "routing_route",
  trace_id: <trace_id>,
  payload: <message>,
  route_key: <key>,
  routes: {
    keys: [<key>, <key>, ...],
    targets: [<agent>, <agent>, ...]
  }
}
```

Reply:

```text
{
  source: <routing-agent>,
  response: "routing_result",
  trace_id: <trace_id>,
  status: <success|failure>,
  state: <routed|route_failed>,
  routed_count: <0|1>,
  success_count: <0|1>,
  failure_count: <0|1>,
  sent_count: <0|1>,
  failed_count: <0|1>
}
```

Routing delivers exactly the caller-provided `payload` to the first positive target agent whose
paired route key matches `route_key`. A direct `target` field is not a supported routing
mechanism; callers that already know the recipient should use direct `send(...)`.

### Broadcasting

Request:

```text
{
  source: <agent>,
  request: "broadcasting_broadcast",
  trace_id: <trace_id>,
  payload: <message>,
  targets: [<agent>, <agent>, ...]
}
```

Reply:

```text
{
  source: <broadcasting-agent>,
  response: "broadcasting_result",
  trace_id: <trace_id>,
  status: <success|failure>,
  state: <broadcasted|broadcast_failed>,
  success_count: <count>,
  failure_count: <count>,
  recipient_count: <count>,
  sent_count: <count>,
  failed_count: <count>
}
```

Broadcasting sends the caller-provided `payload` as-is to every positive `targets` entry.
Integer `0` entries are skipped placeholders, not failed sends.

### Supervision

Requests:

```text
{ source: <agent>, request: "supervision_start", trace_id: <trace_id>, child_method_names: [<method>, ...], child_method_version: <version>, policy: "restart" }
{ source: <agent>, request: "supervision_child_failed", trace_id: <trace_id>, child_agent_id: <agent>, child_method_name: <method>, child_method_version: <version> }
{ source: <agent>, request: "supervision_child_exited", trace_id: <trace_id>, child_agent_id: <agent>, child_method_name: <method>, child_method_version: <version> }
{ source: <agent>, request: "supervision_stop", trace_id: <trace_id>, child_agent_id: <agent> }
```

Reply:

```text
{
  source: <supervision-agent>,
  response: "supervision_result",
  trace_id: <trace_id>,
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

Stop and lifecycle requests are validated against tracked `child_agent_ids`. Untracked or duplicate
lifecycle events report `ignored`; successful tracked stop requests report `stopped`; restart policy
can report `restarted`.

### Distribution

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

Reply:

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

Distribution sends each payload item as-is, round-robin, to positive worker IDs. Integer `0` worker
placeholders are skipped without consuming the current payload when later workers remain.

### Aggregation

Requests:

```text
{ source: <agent>, request: "aggregation_start", trace_id: <trace_id>, expected_count: <count> }
{ source: <agent>, request: "aggregation_collect", trace_id: <trace_id>, payload: <payload> }
```

Completion response:

```text
{
  source: <aggregation-agent>,
  response: "aggregation_result",
  trace_id: <trace_id>,
  payloads: [<payload-1>, <payload-2>, ...],
  status: <success|failure>,
  success_count: <count>,
  failure_count: <count>
}
```

Aggregation marks completion only after the aggregate response is sent successfully; failed
completion delivery leaves the aggregate open. An `aggregation_start` request resets aggregation
and starts a fresh payload list with the configured `expected_count`. Payload collection requests
use `request: "aggregation_collect"` and must carry the same `trace_id` as the active start
request; mismatched or missing collection traces fail without appending the payload.
Failed collection attempts are reported in the eventual aggregate response's `failure_count`. The
response is sent when `success_count + failure_count` equals the configured `expected_count`; the
response status is `success` only when `success_count` equals that `expected_count`, and `failure`
otherwise.

### Scheduling

Requests:

```text
{ source: <agent>, request: "scheduling_schedule", trace_id: <trace_id>, schedule_id: <id>, due_tick: <number>, target: <agent>, payload_request: <request>, payload_text: <text>, payload_attempt: <attempt> }
{ source: <agent>, request: "scheduling_tick", trace_id: <trace_id>, tick: <number> }
{ source: <agent>, request: "scheduling_cancel", trace_id: <trace_id>, schedule_id: <id> }
```

Triggered message:

```text
{
  source: <scheduling-agent>,
  request: <payload_request>,
  trace_id: <trace_id>,
  text: <payload_text>,
  attempt: <payload_attempt>,
  schedule_id: <schedule_id>
}
```

Response:

```text
{
  source: <scheduling-agent>,
  response: "scheduling_result",
  trace_id: <trace_id>,
  status: <success|failure>,
  state: <scheduled|cancelled|triggered|trigger_failed>,
  schedule_id: <id>,
  success_count: <count>,
  failure_count: <count>,
  pending: <0|1>,
  current_tick: <number>
}
```

A due tick clears `pending` only when the stored payload is sent successfully. If delivery fails,
the state is `trigger_failed` and the schedule remains pending for a later tick. Trigger responses
use the stored schedule trace because they report the stored schedule request; cancel responses use
the cancel request trace.

### Synchronization

Requests:

```text
{ source: <agent>, request: "synchronization_wait", trace_id: <trace_id>, sync_id: <id>, required_count: <count>, continuation_target: <agent>, continuation_request: <request>, continuation_text: <text> }
{ source: <agent>, request: "synchronization_dependency", trace_id: <trace_id>, sync_id: <id>, dependency: <name> }
```

Continuation:

```text
{
  source: <synchronization-agent>,
  request: <continuation_request>,
  trace_id: <trace_id>,
  sync_id: <id>,
  text: <continuation_text>,
  done_count: <count>,
  dependencies: [<dependency>, <dependency>, ...]
}
```

Response:

```text
{
  source: <synchronization-agent>,
  response: "synchronization_result",
  trace_id: <trace_id>,
  status: "success",
  state: "complete",
  sync_id: <id>,
  success_count: <count>,
  failure_count: 0,
  done_count: <count>,
  dependencies: [<dependency>, <dependency>, ...]
}
```

Synchronization marks completion only after the continuation and response are delivered. Failed
delivery keeps the gate open for retry.

### Workflow

Requests:

```text
{ source: <agent>, request: "workflow_start", trace_id: <trace_id>, workflow_id: <id>, step_targets: [<agent>, ...], step_payloads: [<message>, ...], branch_value: <outcome> }
{ source: <agent>, request: "workflow_step_done", trace_id: <trace_id>, workflow_id: <id>, step: <current-step-number>, outcome: <value> }
```

Step messages sent to step agents are exactly the caller-provided step payloads.

Completion response:

```text
{
  source: <workflow-agent>,
  response: "workflow_result",
  trace_id: <trace_id>,
  status: <success|failure>,
  state: <complete|handoff_failed>,
  workflow_id: <id>,
  success_count: <count>,
  failure_count: <count>,
  current_step: <last-step-number>,
  completed_step_count: <executed-step-count>
}
```

Workflow records terminal status only after the `start` response is delivered; failed completion
delivery leaves completion pending and retries do not increment `completed_step_count`.

### Conversation

Requests:

```text
{ source: <agent>, request: "conversation_start", trace_id: <trace_id>, conversation_id: <id>, participant_a: <agent>, participant_b: <agent> }
{ source: <agent>, request: "conversation_message", trace_id: <trace_id>, conversation_id: <id>, sender: <agent>, text: <text>, intent: <intent> }
{ source: <agent>, request: "conversation_summary", trace_id: <trace_id>, conversation_id: <id> }
{ source: <agent>, request: "conversation_close", trace_id: <trace_id>, conversation_id: <id> }
```

Participant turn:

```text
{
  source: <conversation-agent>,
  request: "conversation_turn",
  trace_id: <trace_id>,
  conversation_id: <id>,
  from: <agent>,
  to: <agent>,
  text: <text>,
  intent: <intent>,
  turn_count: <count>
}
```

Response:

```text
{
  source: <conversation-agent>,
  response: "conversation_result",
  trace_id: <trace_id>,
  status: <success|failure>,
  state: <active|closed>,
  conversation_id: <id>,
  result: <active|relayed|relay_failed|ignored|closed>,
  success_count: <count>,
  failure_count: <count>,
  participant_a: <agent>,
  participant_b: <agent>,
  last_sender: <agent>,
  last_recipient: <agent>,
  last_text: <text>,
  turn_count: <count>,
  history: [<conversation_turn>, ...]
}
```

If participant turn delivery fails, the coordinator reports `result: "relay_failed"` and leaves the
history and turn count unchanged.

### Retry

Requests:

```text
{ source: <agent>, request: "retry_start", trace_id: <trace_id>, operation_id: <id>, operation_target: <agent>, operation_request: <request>, operation_text: <text>, max_attempts: <number>, strategy: <immediate|scheduled>, scheduler_agent: <agent>, delay_ticks: <tick> }
{ source: <agent>, request: "retry_failure", trace_id: <trace_id>, attempt: <attempt>, current_tick: <tick> }
{ source: <agent>, request: "retry_success", trace_id: <trace_id>, attempt: <attempt> }
```

Operation attempt:

```text
{ source: <retry-agent>, request: <operation_request>, trace_id: <trace_id>, text: <operation_text>, attempt: <number> }
```

Scheduled retry request:

```text
{
  source: <retry-agent>,
  request: "scheduling_schedule",
  trace_id: <trace_id>,
  schedule_id: <operation_id>,
  due_tick: <current_tick + delay_ticks>,
  target: <operation_target>,
  payload_request: <operation_request>,
  payload_text: <operation_text>,
  payload_attempt: <attempt>
}
```

Terminal response:

```text
{
  source: <retry-agent>,
  response: "retry_result",
  trace_id: <trace_id>,
  status: <success|failure>,
  state: <succeeded|failed|dispatch_failed>,
  operation_id: <id>,
  success_count: <0|1>,
  failure_count: <0|1>,
  attempts: <count>
}
```

Retry records terminal state only after the `start` response is delivered; failed report delivery
stores the pending terminal result so a matching outcome retries that report without replacing it or
changing the attempt count.

## Composition Examples

Fan-out and fan-in:

```text
1. Send a request with `request: "aggregation_start"` and expected_count to an aggregation agent.
2. Send a request with `request: "distribution_distribute"` to a distribution agent.
3. Workers send requests with `request: "aggregation_collect"`, the same trace_id, and payload to an aggregation agent.
4. Aggregation emits a response when `expected_count` collection outcomes are accounted for.
```

Delayed retry:

```text
1. Send a request with `request: "retry_start"`, strategy: "scheduled", and scheduler_agent to a retry agent.
2. On a request with `request: "retry_failure"`, trace_id, and current_tick, retry sends a schedule request
   due at current_tick + delay_ticks.
3. An external tick source sends requests with `request: "scheduling_tick"` to scheduling.
4. Scheduling re-emits the operation attempt at the requested tick.
```

Branching workflow:

```text
1. Send a request with `request: "workflow_start"` to workflow with aligned step targets and payload lists.
2. Workflow sends step 1 directly to the first configured step target.
3. Send a `workflow_step_done` request for step 1 with the branch outcome to skip one pending step.
4. Continue sending `workflow_step_done` requests until workflow emits `workflow_result`.
```

Conversation-scoped workflow:

```text
1. Conversation receives a request with `request: "conversation_start"` for two participant agents.
2. Participant A sends a request with `request: "conversation_message"`; conversation relays a conversation_turn
   request to participant B.
3. Participant B replies through the same coordinator; conversation relays the turn back to
   participant A.
4. A workflow or aggregation agent can request summary and consume the structured turn history.
```

## Gap Analysis

| Method | Status | Gap |
| --- | --- | --- |
| Routing | Fully implementable for keyed unbounded one-to-one selection. | Keyed routes use a map containing parallel `keys` and `targets` lists because ordinary methods do not have a safe type predicate for scanning a list of route-entry maps. Direct target delivery belongs to direct `send(...)`; same-payload fan-out belongs to broadcasting. |
| Broadcasting | Fully implementable for unbounded same-payload fan-out to primitive target IDs. | Target lists should contain positive IDs for all intended recipients; integer `0` is treated as a placeholder rather than a recipient. |
| Supervision | Partially implementable. | The method can spawn and track unbounded child method-name lists with one shared start version, but methods cannot autonomously observe child crashes or exits; callers must send `child_failed` or `child_exited` events. A failed `spawn(...)` aborts the remaining ordinary method evaluation, so supervision can avoid reporting `running` for an incomplete child set but cannot emit a catchable `spawn_failed` state without a non-aborting spawn result or method-existence check. Removing arbitrary failed ids from the tracked list or starting one mixed-version list requires a list-filter operation, separate supervisors, or a specialized replacement method. |
| Distribution | Fully implementable for round-robin assignment of opaque payload lists to primitive worker IDs. | Load-aware placement, weighted assignment, and worker health checks require additional methods or richer collection-processing conventions. |
| Aggregation | Fully implementable for list-valued fan-in. | Duplicate handling, custom merge functions, and richer aggregate policies require deeper collection operations or specialized aggregate methods. |
| Scheduling | Partially implementable. | There is no runtime clock or timer callback; scheduling requires explicit `tick` messages from another agent or host process. |
| Synchronization | Fully implementable for unbounded count-based gates. | Membership validation against a declared dependency set and duplicate suppression require richer collection querying/filtering or a specialized validation method. |
| Workflow | Partially implementable. | The method processes an unbounded linear step sequence and can skip one pending step on a branch outcome. Arbitrary workflow graphs, branch destinations by id, list length validation, and map-shaped step descriptors require richer collection querying or a specialized transition method. |
| Conversation | Fully implementable for bounded two-agent exchange. | Alternation rules, participant timeouts, semantic summaries, and searchable long-term history require additional methods or host-driven scheduling. |
| Retry | Fully implementable for immediate retry and scheduled retry by composition. | Backoff policies need an external tick convention and richer arithmetic/time policy support. |

No method in this methodology is blocked entirely. The missing capabilities are real-time timers,
autonomous lifecycle event observation, non-aborting spawn failure results, dynamic collection
filtering, safe type predicates for map-valued list items, and ordinary-method indexed assignment.
Those gaps are documented here so the reusable behaviors remain ordinary methods rather than hidden
runtime features.
