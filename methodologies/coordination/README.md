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

workflow can coordinate other method agents by sending sender-provided step payloads directly to
those agents.
```

Composition opportunities:

- Use `routing` when one recipient must be selected from a key-based route table.
- Use `broadcasting` when one payload must be sent to every recipient in a recipient list.
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
| [`broadcasting`](broadcasting-1.0.0.md) | [`broadcasting-1.0.0.method`](broadcasting-1.0.0.method) | [`broadcasting_tests.c`](broadcasting_tests.c) | Sends one payload to every recipient in an unbounded recipient list. | Same-payload fan-out primitive. |
| [`supervision`](supervision-1.0.0.md) | [`supervision-1.0.0.method`](supervision-1.0.0.method) | [`supervision_tests.c`](supervision_tests.c) | Creates, tracks, stops, and event-restarts unbounded child lists. | Keeps coordination agents available. |
| [`distribution`](distribution-1.0.0.md) | [`distribution-1.0.0.method`](distribution-1.0.0.method) | [`distribution_tests.c`](distribution_tests.c) | Round-robins payload items across an unbounded recipient list. | Distinct-payload assignment. |
| [`aggregation`](aggregation-1.0.0.md) | [`aggregation-1.0.0.method`](aggregation-1.0.0.method) | [`aggregation_tests.c`](aggregation_tests.c) | Appends opaque payloads and emits a payload list. | Completes fan-in with append-backed state. |
| [`scheduling`](scheduling-1.0.0.md) | [`scheduling-1.0.0.method`](scheduling-1.0.0.method) | [`scheduling_tests.c`](scheduling_tests.c) | Stores pending work and triggers it on explicit tick messages. | Delayed execution primitive. |
| [`synchronization`](synchronization-1.0.0.md) | [`synchronization-1.0.0.method`](synchronization-1.0.0.method) | [`synchronization_tests.c`](synchronization_tests.c) | Waits for an unbounded count of dependency messages before sending a continuation. | Dependency gate. |
| [`workflow`](workflow-1.0.0.md) | [`workflow-1.0.0.method`](workflow-1.0.0.method) | [`workflow_tests.c`](workflow_tests.c) | Sends an unbounded step sequence, supports a branch skip, and completes. | Higher-level sequence and branch coordinator. |
| [`conversation`](conversation-1.0.0.md) | [`conversation-1.0.0.method`](conversation-1.0.0.method) | [`conversation_tests.c`](conversation_tests.c) | Coordinates a participant-list conversation by broadcasting turns to all other participants. | Mediated multi-agent exchange. |
| [`retry`](retry-1.0.0.md) | [`retry-1.0.0.method`](retry-1.0.0.method) | [`retry_tests.c`](retry_tests.c) | Re-executes failed operations within a retry policy. | Uses direct send or scheduled retry. |

## Message Contracts

The contracts below are map-shaped messages. Method implementations that create new messages assign
a multi-line map literal to a named memory value, then pass that value to `send(...)`. This works for
flat and nested messages because `send(...)`, `append(...)`, `head(...)`, and `tail(...)` deep-copy
borrowed maps and lists.

By AgeRun method convention, every external request includes a `sender` value identifying the
originating agent and a `request` value that identifies the requested command with the
`<method>_<action>` naming convention. Every external response includes a `sender` value identifying
the method agent that produced the response and a `response` value that identifies the result
envelope with the `<method>_result` naming convention, preserves the triggering request's
effective `trace_id`, and reports standard `status: "success"` or `status: "failure"` with
`success_count` and `failure_count`. The `trace_id` is a call-chain correlation: callers pass the
received trace to continue a chain, pass a fresh trace to reset at a top-level boundary, or omit it
to let the receiving method generate one for that request edge. Coordination methods use the
effective trace for responses and method-owned control messages they create; opaque sender payloads
are sent as-is. The `session_id` is a per-session correlation for methods that receive different
request kinds in one logical session. Those sessionful requests, generated control requests, and
responses include `session_id`; stateless single-request methods that are expected to be called
once use only `trace_id`. When a method has only one request kind, that request is named
`<method>_start`.
Method-specific contracts define any additional response fields. When a method eventually returns
multiple distinct result envelopes, the `response` value should use `<method>_<result_kind>`.
Method-specific outcome detail is carried by method-specific fields only when the response needs
that detail. Coordination methods handle only recognized `request` values as coordination commands;
messages with `response` values are not treated as new coordination requests.

### Routing

Request:

```text
{
  sender: <sender-agent>,
  request: "routing_start",
  trace_id: <trace_id>,
  payload: <message>,
  route_key: <key>,
  routes: {
    keys: [<key>, <key>, ...],
    recipients: [<recipient-agent-1>, <recipient-agent-2>, ...]
  }
}
```

Reply:

```text
{
  sender: <routing-agent>,
  response: "routing_result",
  trace_id: <trace_id>,
  status: <success|failure>,
  routed_count: <0|1>,
  success_count: <0|1>,
  failure_count: <0|1>,
  sent_count: <0|1>,
  failed_count: <0|1>
}
```

Routing delivers exactly the sender-provided `payload` to the first positive recipient agent whose
paired route key matches `route_key`. A direct `recipient` field is not a supported routing
mechanism; callers that already know the recipient should use direct `send(...)`.

Count semantics: `success_count` increments to `1` only when the matched positive recipient receives
the sender-provided `payload`. `failure_count` increments to `1` only when that matched recipient
send is attempted and fails. Route misses, non-positive recipients, and internal scan handoff
failures leave both counts at `0` even when the response status is `failure`.

Status semantics: the response status is `success` only when routing delivers the payload to the
matched positive recipient. It is `failure` when no route selects a positive recipient, when the
matched recipient send fails, or when an internal scan handoff fails.

### Broadcasting

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

Reply:

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

Broadcasting sends the sender-provided `payload` as-is to every positive `recipients` entry.
Integer `0` entries are skipped placeholders, not failed sends.

Count semantics: `success_count` increments once for each positive recipient that accepts the
sender-provided `payload`. `failure_count` increments once for each positive recipient send that
fails.

Status semantics: the response status is `success` when the broadcast completes with at least one
successful positive-recipient delivery and no recipient or continuation send failures. It is
`failure` when no positive recipient receives the payload, when any positive-recipient send fails,
or when an internal continuation handoff fails.

### Supervision

Requests:

```text
{ sender: <sender-agent>, request: "supervision_start", trace_id: <trace_id>, session_id: <session_id>, child_method_names: [<method>, ...], child_method_version: <version>, policy: "restart" }
{ sender: <sender-agent>, request: "supervision_child_failed", trace_id: <trace_id>, session_id: <session_id>, child_agent_id: <agent>, child_method_name: <method>, child_method_version: <version> }
{ sender: <sender-agent>, request: "supervision_child_exited", trace_id: <trace_id>, session_id: <session_id>, child_agent_id: <agent>, child_method_name: <method>, child_method_version: <version> }
{ sender: <sender-agent>, request: "supervision_stop", trace_id: <trace_id>, session_id: <session_id>, child_agent_id: <agent> }
```

Reply:

```text
{
  sender: <supervision-agent>,
  response: "supervision_result",
  trace_id: <trace_id>,
  session_id: <session_id>,
  status: <success|failure>,
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

Count semantics: `success_count` increases by the number of children tracked when start completes,
by `1` for a successful restart, and by `1` for a successful tracked stop. `failure_count` increases
for spawn or validation handoff failures and for failed tracked stop exits. Untracked or duplicate
lifecycle events do not increment either count.

Status semantics: the response status is `success` for running, restarted, stopped, and ignored
results. It is `failure` when a spawn or validation handoff fails, or when a tracked stop cannot
exit the child.

### Distribution

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

Reply:

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

Distribution sends each payload item as-is, round-robin, to positive recipient IDs. Integer `0`
recipient placeholders are skipped without consuming the current payload when later recipients
remain.
The method queues private `distribution_continue` requests to itself to carry the remaining
payloads, current recipients, original recipients, counters, and `result_recipient` between
recursive assignment steps. Continuations are processed only when `sender` is the distribution
agent.
Because distribution is a one-shot caller-facing method, its request and response use `trace_id`
but do not require `session_id`; an omitted `trace_id` is generated for the result envelope.

Count semantics: `success_count` increments once for each assignment send of one payload item to a
positive recipient that succeeds. `failure_count` increments once for each attempted assignment send
that fails. Integer `0` recipient placeholders are skipped without consuming the payload or
affecting either count. Empty payload or recipient lists produce `status: "failure"` with no
assignment count increments.

Status semantics: the response status is `success` when at least one assignment is attempted and all
attempted assignment sends succeed. It is `failure` when no assignment is attempted, when any
assignment send fails, or when an internal continuation handoff fails.

### Aggregation

Requests:

```text
{ sender: <sender-agent>, request: "aggregation_start", trace_id: <trace_id>, session_id: <session_id>, expected_count: <count> }
{ sender: <sender-agent>, request: "aggregation_collect", trace_id: <trace_id>, session_id: <session_id>, payload: <payload> }
```

Completion response:

```text
{
  sender: <aggregation-agent>,
  response: "aggregation_result",
  trace_id: <trace_id>,
  session_id: <session_id>,
  payloads: [<payload-1>, <payload-2>, ...],
  status: <success|failure>,
  success_count: <count>,
  failure_count: <count>
}
```

Aggregation marks completion only after the aggregate response is sent successfully; failed
completion delivery leaves the aggregate open. An `aggregation_start` request resets aggregation
and starts a fresh payload list with the configured `expected_count`, normalizing non-positive counts
to one. Payload collection requests use `request: "aggregation_collect"` and must carry the same
`session_id` as the active start
request. Collection requests that omit `trace_id` use a generated trace and still append their
payload.

Count semantics: `success_count` increments when a matching active `aggregation_collect` appends its
payload. `failure_count` increments when such a collection attempt is accepted for the active
session but append fails. Wrong-session, inactive, or post-completion collect requests are ignored
and do not affect either count. The response is sent when `success_count + failure_count` equals
the configured `expected_count`;
the response status is `success` only when `success_count` equals that `expected_count`, and
`failure` otherwise.

Status semantics: the response status is `success` only when collected payload count reaches the
configured `expected_count`. It is `failure` when the response threshold is reached only because one
or more accepted collection attempts failed to append.

### Scheduling

Requests:

```text
{ sender: <sender-agent>, request: "scheduling_schedule", trace_id: <trace_id>, session_id: <session_id>, due_tick: <number>, recipient: <recipient-agent>, payload_request: <request>, payload_text: <text>, payload_attempt: <attempt> }
{ sender: <sender-agent>, request: "scheduling_tick", trace_id: <trace_id>, session_id: <session_id>, tick: <number> }
{ sender: <sender-agent>, request: "scheduling_cancel", trace_id: <trace_id>, session_id: <session_id> }
```

Triggered message:

```text
{
  sender: <sender-agent>,
  request: <payload_request>,
  trace_id: <trace_id>,
  session_id: <session_id>,
  text: <payload_text>,
  attempt: <payload_attempt>
}
```

Response:

```text
{
  sender: <scheduling-agent>,
  response: "scheduling_result",
  trace_id: <trace_id>,
  session_id: <session_id>,
  status: <success|failure>,
  success_count: <count>,
  failure_count: <count>,
  pending: <0|1>,
  current_tick: <number>
}
```

A due tick clears `pending` only when the stored payload is sent successfully. If delivery fails,
the response status is `failure` and the schedule remains pending for a later tick. Trigger responses
and triggered payload requests use the tick request's `trace_id`; triggered payload requests
preserve the original schedule request's `sender`. Cancel responses use the cancel request's
`trace_id`; all scheduling requests and responses for one schedule use the same `session_id`.

Count semantics: `success_count` increments when a due tick successfully sends the stored payload,
and when a matching cancel clears a pending schedule. The schedule creation response does not
increment it. `failure_count` increments when a due tick should trigger but the stored payload send
fails.

Status semantics: the response status is `success` for schedule creation, for a matching cancel
that clears a pending schedule, and for a due tick that successfully sends the stored payload. It is
`failure` only when a due tick should trigger but the stored payload send fails.

### Synchronization

Requests:

```text
{ sender: <sender-agent>, request: "synchronization_wait", trace_id: <trace_id>, session_id: <session_id>, required_count: <count>, continuation_recipient: <agent>, continuation_request: <request>, continuation_text: <text> }
{ sender: <sender-agent>, request: "synchronization_dependency", trace_id: <trace_id>, session_id: <session_id>, dependency: <name> }
```

Continuation:

```text
{
  sender: <sender-agent>,
  request: <continuation_request>,
  trace_id: <trace_id>,
  session_id: <session_id>,
  text: <continuation_text>,
  done_count: <count>,
  dependencies: [<dependency>, <dependency>, ...]
}
```

Response:

```text
{
  sender: <synchronization-agent>,
  response: "synchronization_result",
  trace_id: <trace_id>,
  session_id: <session_id>,
  status: "success",
  success_count: <count>,
  failure_count: 0,
  done_count: <count>,
  dependencies: [<dependency>, <dependency>, ...]
}
```

Synchronization marks completion only after the continuation and response are delivered. Failed
delivery keeps the gate open for retry.

Count semantics: `success_count` increments by one for each matching dependency whose value is
appended before completion. No current synchronization event increments `failure_count`; failed
continuation or result delivery keeps the gate open instead of producing a failure result, so result
`failure_count` is always `0`.

Status semantics: the only emitted synchronization result has status `success`, and it is emitted
after the required dependencies have been collected and the continuation has been delivered. Failed
continuation or result delivery emits no failure result; it keeps the synchronization open for
retry.

### Workflow

Requests:

```text
{ sender: <sender-agent>, request: "workflow_start", trace_id: <trace_id>, session_id: <session_id>, recipients: [<recipient-agent-1>, <recipient-agent-2>, ...], payloads: [<payload>, <payload>, ...], branch_value: <outcome> }
{ sender: <sender-agent>, request: "workflow_step_done", trace_id: <trace_id>, session_id: <session_id>, step: <current-step-number>, outcome: <value> }
```

Step messages sent to step agents are exactly the sender-provided payloads.

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

Workflow records terminal status only after the `start` response is delivered; failed completion
delivery leaves completion pending and retries do not increment the completed-step counter.

Count semantics: `success_count` increments when a matching `workflow_step_done` completes the
currently awaited sent step. Skipped zero-recipient placeholders, stale completions, duplicate
completions, out-of-order completions, and pending completion retries do not increment it.
`failure_count` becomes `1` when any workflow handoff fails, including start or continuation
self-sends, skipped-step self-sends, or direct step payload sends; otherwise it is `0`.

Status semantics: the completion response status is `success` when workflow completion is reached
without a handoff failure, including empty completion after skipped placeholders or completion after
the final step. It is `failure` when any workflow handoff fails.

### Conversation

Requests:

```text
{ sender: <sender-agent>, request: "conversation_start", trace_id: <trace_id>, session_id: <session_id>, participants: [<recipient-agent-1>, <recipient-agent-2>, ...] }
{ sender: <sender-agent>, request: "conversation_message", trace_id: <trace_id>, session_id: <session_id>, payload: <payload> }
{ sender: <sender-agent>, request: "conversation_history", trace_id: <trace_id>, session_id: <session_id> }
{ sender: <sender-agent>, request: "conversation_close", trace_id: <trace_id>, session_id: <session_id> }
```

Participant turn:

```text
{
  sender: <conversation-agent>,
  request: "conversation_turn",
  trace_id: <trace_id>,
  session_id: <session_id>,
  payload: <payload>,
  participant: <sender-agent>,
  turn_count: <count>
}
```

Response:

```text
{
  sender: <conversation-agent>,
  response: "conversation_result",
  trace_id: <trace_id>,
  session_id: <session_id>,
  status: <success|failure>,
  result: <active|relayed|relay_failed|ignored|closed>,
  success_count: <count>,
  failure_count: <count>,
  participants: [<recipient-agent-1>, <recipient-agent-2>, ...],
  last_sender: <agent>,
  last_payload: <payload>,
  turn_count: <count>,
  history: [<conversation_turn>, ...]
}
```

Conversation spawns one broadcasting agent when the conversation starts. It reuses that agent for
every turn and excludes the sender from the broadcast recipients. Turns received while another turn
relay is pending are ignored. If broadcast delivery fails for any recipient, the coordinator reports
`result: "relay_failed"` and leaves the history and turn count unchanged. If no other participant
needs to receive the turn, the coordinator records it without broadcasting.

If the `sender` of a `conversation_message` is not in the participant list, the coordinator ignores
the request. It does not broadcast, record, respond, or update conversation state.

Count semantics: `success_count` reports the current successful turn count. A
`conversation_message` increments that count when the participant turn is broadcast successfully, or
when no other participant needs to receive it and the turn is recorded locally. History and close
responses report the current successful turn count, and start responses report `0`. `failure_count`
increments to `1` when the broadcasting helper cannot be spawned or when a turn relay fails before
or during broadcasting; history and close responses report `0`. Non-participant
`conversation_message` requests and turns received while another relay is pending are ignored and do
not change status or count attributes.

Status semantics: the response status is `success` for a successful start, history, close, or
participant turn relay. It is `failure` when the broadcasting helper cannot be spawned, when a turn
relay fails.

### Retry

Requests:

```text
{ sender: <sender-agent>, request: "retry_start", trace_id: <trace_id>, session_id: <session_id>, operation_recipient: <agent>, operation_request: <request>, operation_text: <text>, max_attempts: <number>, strategy: <immediate|scheduled>, scheduler_agent: <agent>, delay_ticks: <tick> }
{ sender: <sender-agent>, request: "retry_failure", trace_id: <trace_id>, session_id: <session_id>, attempt: <attempt>, current_tick: <tick> }
{ sender: <sender-agent>, request: "retry_success", trace_id: <trace_id>, session_id: <session_id>, attempt: <attempt> }
```

Operation attempt:

```text
{ sender: <sender-agent>, request: <operation_request>, trace_id: <trace_id>, session_id: <session_id>, text: <operation_text>, attempt: <number> }
```

Scheduled retry request:

```text
{
  sender: <sender-agent>,
  request: "scheduling_schedule",
  trace_id: <trace_id>,
  session_id: <session_id>,
  due_tick: <current_tick + delay_ticks>,
  recipient: <recipient-agent>,
  payload_request: <operation_request>,
  payload_text: <operation_text>,
  payload_attempt: <attempt>
}
```

Terminal response:

```text
{
  sender: <retry-agent>,
  response: "retry_result",
  trace_id: <trace_id>,
  session_id: <session_id>,
  status: <success|failure>,
  success_count: <0|1>,
  failure_count: <0|1>,
  attempts: <count>
}
```

Retry records a terminal outcome only after the `start` response is delivered; failed report delivery
stores the pending terminal result so a matching outcome retries that report without replacing it or
changing the attempt count. Failure and success requests match the active retry by `session_id`;
terminal responses use the triggering outcome request's effective `trace_id`.

Count semantics: `success_count` is `1` only for the terminal response produced by a matching
`retry_success` outcome. `failure_count` is `1` when initial operation dispatch fails or when a
matching `retry_failure` reaches the final allowed attempt. Non-terminal failures that schedule or
dispatch another attempt do not increment either terminal count.

Status semantics: the terminal response status is `success` only for a matching `retry_success`
outcome. It is `failure` when the initial operation dispatch fails or when a matching
`retry_failure` reaches the final allowed attempt.

## Composition Examples

Fan-out and fan-in:

```text
1. Send a request with `request: "aggregation_start"` and expected_count to an aggregation agent.
2. Send a request with `request: "distribution_start"` to a distribution agent.
3. Recipients send requests with `request: "aggregation_collect"`, the same session_id, and payload to an aggregation agent.
4. Aggregation emits a response when `expected_count` collection outcomes are accounted for.
```

Delayed retry:

```text
1. Send a request with `request: "retry_start"`, strategy: "scheduled", and scheduler_agent to a retry agent.
2. On a request with `request: "retry_failure"`, trace_id, session_id, and current_tick, retry sends a schedule request
   due at current_tick + delay_ticks.
3. An external tick sender sends requests with `request: "scheduling_tick"` and the schedule session_id to scheduling.
4. Scheduling re-emits the operation attempt at the requested tick.
```

Branching workflow:

```text
1. Send a request with `request: "workflow_start"` to workflow with aligned recipients and payloads.
2. Workflow sends step 1 directly to the first configured step recipient.
3. Send a `workflow_step_done` request for step 1 with the workflow session_id and branch outcome to skip one pending step.
4. Continue sending `workflow_step_done` requests with the same session_id until workflow emits `workflow_result`.
```

Conversation-scoped workflow:

```text
1. Conversation receives a request with `request: "conversation_start"` and a participant list.
2. Participant A sends a request with `request: "conversation_message"` and the conversation session_id; conversation sends one conversation_turn
   request through broadcasting to every other participant.
3. Participant B replies through the same coordinator; conversation reuses the same broadcasting
   agent and excludes participant B from that turn.
4. A workflow or aggregation agent can request history and consume the structured turn history.
```

## Gap Analysis

| Method | Status | Gap |
| --- | --- | --- |
| Routing | Fully implementable for keyed unbounded one-to-one selection. | Keyed routes use a map containing parallel `keys` and `recipients` lists because ordinary methods do not have a safe type predicate for scanning a list of route-entry maps. Direct recipient delivery belongs to direct `send(...)`; same-payload fan-out belongs to broadcasting. |
| Broadcasting | Fully implementable for unbounded same-payload fan-out to primitive recipient IDs. | Recipient lists should contain positive IDs for all intended recipients; integer `0` is treated as a placeholder rather than a recipient. |
| Supervision | Partially implementable. | The method can spawn and track unbounded child method-name lists with one shared start version, but methods cannot autonomously observe child crashes or exits; callers must send `child_failed` or `child_exited` events. A failed `spawn(...)` aborts the remaining ordinary method evaluation, so supervision can avoid reporting `running` for an incomplete child set but cannot emit a catchable `spawn_failed` outcome without a non-aborting spawn result or method-existence check. Removing arbitrary failed ids from the tracked list or starting one mixed-version list requires a list-filter operation, separate supervisors, or a specialized replacement method. |
| Distribution | Fully implementable for round-robin assignment of opaque payload lists to primitive recipient IDs. | Load-aware placement, weighted assignment, and recipient health checks require additional methods or richer collection-processing conventions. |
| Aggregation | Fully implementable for list-valued fan-in. | Duplicate handling, custom merge functions, and richer aggregate policies require deeper collection operations or specialized aggregate methods. |
| Scheduling | Partially implementable. | There is no runtime clock or timer callback; scheduling requires explicit `tick` messages from another agent or host process. |
| Synchronization | Fully implementable for unbounded count-based gates. | Membership validation against a declared dependency set and duplicate suppression require richer collection querying/filtering or a specialized validation method. |
| Workflow | Partially implementable. | The method processes an unbounded linear step sequence and can skip one pending step on a branch outcome. Arbitrary workflow graphs, branch destinations by id, list length validation, and map-shaped step descriptors require richer collection querying or a specialized transition method. |
| Conversation | Fully implementable for participant-list exchange by composition with broadcasting. | Alternation rules, participant timeouts, semantic summaries, and searchable long-term history require additional methods or host-driven scheduling. |
| Retry | Fully implementable for immediate retry and scheduled retry by composition. | Backoff policies need an external tick convention and richer arithmetic/time policy support. |

No method in this methodology is blocked entirely. The missing capabilities are real-time timers,
autonomous lifecycle event observation, non-aborting spawn failure results, dynamic collection
filtering, safe type predicates for map-valued list items, and ordinary-method indexed assignment.
Those gaps are documented here so the reusable behaviors remain ordinary methods rather than hidden
runtime features.
