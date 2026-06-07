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
  distribution
  workflow

scheduling
  retry

aggregation
synchronization
supervision
conversation

workflow can coordinate distribution, aggregation, synchronization, retry, and conversation agents
by routing work to them, but its direct implementation dependency is routing.
```

Composition opportunities:

- Use `routing` as the lowest-level delivery primitive for higher-level coordination.
- Build fan-out with `distribution` and fan-in with `aggregation`.
- Combine `synchronization` with `workflow` to gate step advancement on required dependency
  messages.
- Use `scheduling` as the delayed execution primitive for `retry`.
- Use `conversation` to preserve correlation and context around workflow or routing requests.
- Use `supervision` to keep long-lived routing, scheduling, workflow, or worker agents present.

## Methods

| Method | Implementation | Test | Purpose | Composition Role |
| --- | --- | --- | --- | --- |
| [`routing`](routing-1.0.0.md) | [`routing-1.0.0.method`](routing-1.0.0.method) | [`routing_tests.c`](routing_tests.c) | Selects one or more recipients and forwards a message. | Foundation for delivery. |
| [`supervision`](supervision-1.0.0.md) | [`supervision-1.0.0.method`](supervision-1.0.0.method) | [`supervision_tests.c`](supervision_tests.c) | Creates, tracks, stops, and event-restarts unbounded child lists. | Keeps coordination agents available. |
| [`distribution`](distribution-1.0.0.md) | [`distribution-1.0.0.method`](distribution-1.0.0.method) | [`distribution_tests.c`](distribution_tests.c) | Assigns a work payload to an unbounded worker list. | Builds on routing for fan-out. |
| [`aggregation`](aggregation-1.0.0.md) | [`aggregation-1.0.0.method`](aggregation-1.0.0.method) | [`aggregation_tests.c`](aggregation_tests.c) | Appends result values and emits a result list. | Completes fan-in with append-backed state. |
| [`scheduling`](scheduling-1.0.0.md) | [`scheduling-1.0.0.method`](scheduling-1.0.0.method) | [`scheduling_tests.c`](scheduling_tests.c) | Stores pending work and triggers it on explicit tick messages. | Delayed execution primitive. |
| [`synchronization`](synchronization-1.0.0.md) | [`synchronization-1.0.0.method`](synchronization-1.0.0.method) | [`synchronization_tests.c`](synchronization_tests.c) | Waits for an unbounded count of dependency messages before sending a continuation. | Dependency gate. |
| [`workflow`](workflow-1.0.0.md) | [`workflow-1.0.0.method`](workflow-1.0.0.method) | [`workflow_tests.c`](workflow_tests.c) | Routes an unbounded step sequence, supports a branch skip, and completes. | Higher-level sequence and branch coordinator. |
| [`conversation`](conversation-1.0.0.md) | [`conversation-1.0.0.method`](conversation-1.0.0.method) | [`conversation_tests.c`](conversation_tests.c) | Coordinates a bounded conversation between two participant agents. | Mediated two-agent exchange. |
| [`retry`](retry-1.0.0.md) | [`retry-1.0.0.method`](retry-1.0.0.method) | [`retry_tests.c`](retry_tests.c) | Re-executes failed operations within a retry policy. | Uses direct send or scheduled retry. |

## Message Contracts

The contracts below are map-shaped messages. Method implementations that create new messages assign
a multi-line map literal to a named memory value, then pass that value to `send(...)`. This works for
flat and nested messages because `send(...)`, `append(...)`, `head(...)`, and `tail(...)` deep-copy
borrowed maps and lists.

The `action` field is a command discriminator inside those maps. AgeRun sends every incoming message
to the agent's single method, so reusable coordination methods need a conventional field that says
which operation the message is requesting. The field is not a runtime requirement and not a string
protocol; it is ordinary map data that lets a method guard behavior such as routing, scheduling, or
retrying and ignore unrelated messages safely.

### Routing

Request:

```text
{
  action: "route",
  mode: "one",
  target: <agent>,
  payload_action: <action>,
  payload_text: <text>,
  correlation_id: <id>,
  reply_to: <agent>
}

{
  action: "route",
  mode: "many",
  targets: [<agent>, <agent>, ...],
  payload_action: <action>,
  payload_text: <text>,
  correlation_id: <id>,
  reply_to: <agent>
}
```

Optional keyed selection for `mode=one`:

```text
{
  route_key: <key>,
  routes: {
    keys: [<key>, <key>, ...],
    targets: [<agent>, <agent>, ...]
  }
}
```

The `routes.keys` and `routes.targets` lists are paired by position and scanned with `head(...)` and
`tail(...)`, so keyed one-to-one routing is unbounded. Both the request `route_key` and the candidate
key must be nonzero/present before a candidate can match.

If a one-to-one request has no positive direct or keyed target, routing emits `route_failed` with zero
delivery counts instead of reporting a successful zero-send route.

Forwarded message:

```text
{
  action: <payload_action>,
  correlation_id: <correlation_id>,
  text: <payload_text>,
  source: <routing-agent>
}
```

Reply:

```text
{
  action: "route_result",
  status: <routed|ignored|route_failed>,
  correlation_id: <correlation_id>,
  routed_count: <count>,
  sent_count: <count>,
  failed_count: <count>,
  sent_one: <0|1>,
  sent_many: <0|1>,
  continuation_sent: <0|1>
}
```

For `mode=many`, `routed_count` and `sent_count` count successful target sends, while
`failed_count` counts positive target IDs that could not be sent to. Integer `0` entries are skipped
placeholders, not failed sends, and a single interior zero does not stop fan-out to a later positive
target. A target send failure keeps processing the remaining targets and makes the terminal status
`route_failed`. A failed self-continuation emits `route_failed` immediately with the partial
routed/sent counts instead of leaving callers without a terminal result.
If a many-route target list is empty or contains no positive targets, routing emits `route_failed`
with zero delivery counts instead of reporting a successful zero-send route.

### Supervision

Requests:

```text
{
  action: "start",
  child_method_names: [<method>, <method>, ...],
  child_method_version: <version>,
  policy: "restart",
  reply_to: <agent>
}

{
  action: "child_failed",
  child_agent_id: <agent>,
  child_method_name: <method>,
  child_method_version: <version>
}

{
  action: "child_exited",
  child_agent_id: <agent>,
  child_method_name: <method>,
  child_method_version: <version>
}

{ action: "stop", child_agent_id: <agent> }
```

Reply:

```text
{
  action: "supervision_status",
  status: <running|restarted|stopped|ignored|stop_failed|handoff_failed>,
  child_agent_id: <agent>,
  child_agent_ids: [<agent>, <agent>, ...],
  child_records: [<child-record>, <child-record>, ...],
  child_count: <count>,
  restart_count: <count>,
  policy: <policy>
}
```

Stop requests are validated against the tracked `child_agent_ids` list before any `exit(...)` call.
An untracked `child_agent_id` is reported as `ignored` and does not alter the stored supervisor
status or exit the named agent. A successfully stopped tracked child is recorded as handled so a
later `child_exited` or `child_failed` message for that child is ignored instead of restarted.
An empty `child_method_names` list is a successful zero-child start and reports `running`.
Lifecycle `child_failed` and `child_exited` events are validated against the same tracked list before
restart or stop handling. Untracked lifecycle events report `ignored` and do not append replacement
children or increment `restart_count`.
Duplicate lifecycle events for any previously handled child are also reported as `ignored` instead
of starting another replacement.
If an internal spawn continuation cannot be queued, supervision reports `handoff_failed` instead of
remaining in `starting`.
If an internal lifecycle or stop validation message cannot be queued, supervision also reports
`handoff_failed` and leaves the requested lifecycle or stop action unapplied.
If a requested child cannot be spawned, the failed `spawn(...)` instruction aborts the remaining
ordinary method evaluation. The supervisor therefore does not report `running` for an incomplete
child set, but it also cannot emit a catchable `spawn_failed` status without a runtime-level
non-aborting spawn result or a separate method-existence check.

### Distribution

Request:

```text
{
  action: "distribute",
  work_id: <id>,
  routing_agent: <agent>,
  reply_to: <agent>,
  workers: [<agent>, <agent>, ...],
  work_text: <text>
}

{
  action: "retry_report",
  work_id: <id>,
  reply_to: <agent>
}
```

Forwarded through `routing`:

```text
{
  action: "route",
  mode: "many",
  targets: [<agent>, <agent>, ...],
  payload_action: "work",
  payload_text: <work_text>,
  correlation_id: <work_id>,
  reply_to: <distribution-agent>,
  routed_count: 0,
  sent_count: 0,
  failed_count: 0
}
```

Reply:

```text
{
  action: "distribution_result",
  status: <distributed|route_failed>,
  work_id: <id>,
  assignment_count: <count>,
  sent_count: <count>,
  route_status: <status>,
  route_sent: <0|1>
}
```

If the route request cannot be handed to the routing agent, distribution emits this reply immediately
with `status: "route_failed"`, zero counts, and `route_sent: 0`.
If routing returns a matching terminal `route_result` whose `status` is `"route_failed"`,
distribution also reports `status: "route_failed"` and preserves the partial routed and sent counts.
This includes empty or no-positive worker lists, because routing reports those many-route requests as
zero-delivery failures.
Distribution accepts a `route_result` only when its `correlation_id` matches the active `work_id`.
After a terminal distribution reply is sent successfully, duplicate matching route results are
ignored.
Distribution marks itself completed only after the terminal reply is delivered.
If terminal reply delivery fails, distribution stores it as pending. A later `retry_report` request
for the same `work_id` retries the stored terminal `distribution_result` with the supplied `reply_to`
without sending another route request. The retry must arrive before the next `distribute` request,
because distribution keeps one active work slot. Retaining multiple keyed pending reports across
later `distribute` requests would require an exists/default operation or richer collection filtering.

### Aggregation

Requests:

```text
{
  action: "start",
  aggregate_id: <id>,
  required_count: <count>,
  reply_to: <agent>
}

{
  action: "result",
  aggregate_id: <id>,
  value: <text>
}
```

Completion:

```text
{
  action: "aggregate_complete",
  aggregate_id: <id>,
  status: "complete",
  result: [<input-1>, <input-2>, ...],
  received_count: <count>
}
```

The list contains each appended result value in arrival order. The method uses `append(...)` to
mutate an internal result list, so the number of collected values is not bounded by named slots.
Required counts below one behave as one required result, so aggregation never completes on `start`
alone.
Aggregation marks completion only after the `aggregate_complete` reply is sent successfully; failed
completion delivery leaves the aggregate open.
Result messages must carry the active `aggregate_id`; late results for a previous aggregate are
ignored.

### Scheduling

Requests:

```text
{
  action: "schedule",
  schedule_id: <id>,
  due_tick: <number>,
  target: <agent>,
  payload_action: <action>,
  payload_text: <text>,
  payload_attempt: <attempt>,
  correlation_id: <id>,
  reply_to: <agent>
}

{
  action: "tick",
  tick: <number>
}

{
  action: "cancel",
  schedule_id: <id>
}
```

Triggered message:

```text
{
  action: <payload_action>,
  correlation_id: <correlation_id>,
  text: <payload_text>,
  attempt: <payload_attempt>,
  schedule_id: <schedule_id>
}
```

Status:

```text
{
  action: "schedule_status",
  schedule_id: <id>,
  status: <scheduled|cancelled|triggered|trigger_failed>,
  pending: <0|1>,
  current_tick: <number>
}
```

A due tick clears `pending` only when the stored payload is sent successfully. If delivery fails, the
status is `trigger_failed` and the schedule remains pending for a later tick.
Cancellation only applies while the matching schedule is still pending; late cancels after a
successful trigger are ignored.

### Synchronization

Requests:

```text
{
  action: "wait",
  sync_id: <id>,
  required_count: <count>,
  continuation_target: <agent>,
  continuation_action: <action>,
  continuation_text: <text>,
  reply_to: <agent>
}

{
  action: "dependency",
  sync_id: <id>,
  dependency: <name>
}
```

Continuation:

```text
{
  action: <continuation_action>,
  sync_id: <id>,
  text: <continuation_text>,
  done_count: <count>,
  dependencies: [<dependency>, <dependency>, ...]
}
```

Status:

```text
{
  action: "synchronization_status",
  sync_id: <id>,
  status: "complete",
  done_count: <count>,
  dependencies: [<dependency>, <dependency>, ...]
}
```

Required counts below one behave as one required dependency, so synchronization never completes on
`wait` alone.
Synchronization marks completion only after the continuation is delivered and, when `reply_to` is a
positive agent id, the status reply is delivered; failed delivery keeps the gate open.
Once the required count is reached, the dependency list is frozen even if continuation delivery fails;
later matching dependency messages retry delivery with the same `done_count` and dependencies.
After continuation delivery, failed status replies are retried without re-emitting the continuation or
increasing `done_count`. Unrelated messages, including maps with other actions, do not trigger
pending continuation or status retries.

### Workflow

Start:

```text
{
  action: "start",
  workflow_id: <id>,
  routing_agent: <agent>,
  reply_to: <agent>,
  step_targets: [<agent>, <agent>, ...],
  step_actions: [<action>, <action>, ...],
  step_texts: [<text>, <text>, ...],
  branch_value: <outcome>
}
```

Step completion:

```text
{
  action: "step_done",
  workflow_id: <id>,
  step: <current-step-number>,
  outcome: <value>
}
```

The `step` value must match the workflow agent's active step; stale, duplicate, or out-of-order
completion maps are ignored. Workflow accepts a `step_done` only while it is waiting for the
currently routed step to report completion, then clears that waiting state before the next internal
`execute_step` handoff runs so duplicate completions cannot advance the workflow twice.
Workflow advances `current_step` and consumes a pending step only after the route handoff for that
step succeeds; failed route handoffs leave the step at the head of the pending queue.
Workflow asks routing to send `route_result` replies back to the workflow agent; a matching
`route_failed` result for the active step completes the workflow with `status: "handoff_failed"`.
If the initial internal step handoff or a later step continuation cannot be queued, workflow emits
`workflow_complete` with `status: "handoff_failed"`.
Workflow records terminal status only after `workflow_complete` is delivered; failed completion
delivery leaves completion pending and retries do not increment `completed_step_count`.

Completion:

```text
{
  action: "workflow_complete",
  workflow_id: <id>,
  status: <complete|handoff_failed>,
  current_step: <last-step-number>,
  completed_step_count: <executed-step-count>
}
```

### Conversation

Requests:

```text
{
  action: "start",
  conversation_id: <id>,
  participant_a: <agent>,
  participant_b: <agent>,
  reply_to: <agent>
}

{
  action: "message",
  conversation_id: <id>,
  sender: <agent>,
  text: <text>,
  intent: <intent>
}

{ action: "summary", conversation_id: <id> }
{ action: "close", conversation_id: <id> }
```

Responses:

```text
{
  action: "conversation_turn",
  conversation_id: <id>,
  from: <agent>,
  to: <agent>,
  text: <text>,
  intent: <intent>,
  turn_count: <count>
}

{
  action: <conversation_started|conversation_relayed|conversation_summary|conversation_closed>,
  conversation_id: <id>,
  state: <state>,
  status: <active|relayed|relay_failed|ignored|closed>,
  participant_a: <agent>,
  participant_b: <agent>,
  last_sender: <agent>,
  last_recipient: <agent>,
  last_text: <text>,
  turn_count: <count>,
  history: [<conversation_turn>, ...]
}
```

The `relayed` status is used only when a participant turn is delivered and recorded. If the
participant turn is valid for the conversation but the delivery send fails, the coordinator reports
`relay_failed` and leaves the history and turn count unchanged.

### Retry

Requests:

```text
{
  action: "start",
  operation_id: <id>,
  operation_target: <agent>,
  operation_action: <action>,
  operation_text: <text>,
  max_attempts: <number>,
  strategy: <immediate|scheduled>,
  scheduler_agent: <agent>,
  delay_ticks: <tick>,
  reply_to: <agent>
}

{ action: "failure", correlation_id: <operation_id>, attempt: <attempt>, current_tick: <tick> }
{ action: "success", correlation_id: <operation_id>, attempt: <attempt> }
```

Attempt:

```text
{
  action: <operation_action>,
  correlation_id: <operation_id>,
  text: <operation_text>,
  attempt: <number>
}
```

Scheduled retry request:

```text
{
  action: "schedule",
  schedule_id: <operation_id>,
  due_tick: <current_tick + delay_ticks>,
  target: <operation_target>,
  payload_action: <operation_action>,
  payload_text: <operation_text>,
  payload_attempt: <attempt>,
  correlation_id: <operation_id>,
  reply_to: 0
}
```

Result:

```text
{
  action: "retry_result",
  operation_id: <id>,
  status: <succeeded|failed|dispatch_failed>,
  attempts: <count>
}
```

Retry records the initial attempt only after the first operation send succeeds. If that initial send
fails, it reports `dispatch_failed` with zero attempts.
After retry reports terminal `succeeded` or `failed` status, later stale outcome messages are ignored.
A new `start` request opens a fresh active retry state, and late outcomes from previous operations are
ignored because their `correlation_id` does not match the active operation id.
Active outcomes must also match the current in-flight `attempt`; duplicate failure or success maps
from an earlier attempt are ignored instead of consuming another retry slot.
Retry attempts advance only after an immediate retry or scheduled retry handoff is sent
successfully; failed dispatch leaves the retry active at the previous attempt count.
Retry records terminal `succeeded` or `failed` status only after the `retry_result` report is
delivered; failed report delivery stores the pending terminal result so a matching outcome retries
that report without replacing it or changing the attempt count.

## Composition Examples

Fan-out and fan-in:

```text
1. Send a map with action: "distribute" to a distribution agent.
2. Workers send maps with action: "result", aggregate_id, and value: <text> to an aggregation agent.
3. Aggregation emits a map with action: "aggregate_complete" and a result list when required_count
   results arrive.
```

Delayed retry:

```text
1. Send a map with action: "start", strategy: "scheduled", and scheduler_agent to a retry agent.
2. On a map with action: "failure", correlation_id, and current_tick, retry sends a schedule map due
   at current_tick + delay_ticks.
3. An external tick source sends maps with action: "tick" to scheduling.
4. Scheduling re-emits the operation attempt at the requested tick.
```

Branching workflow:

```text
1. Send a map with action: "start" to workflow with routing_agent and aligned step lists.
2. Workflow routes step 1 through routing.
3. Send a step_done map for step 1 with the branch outcome to skip one pending step.
4. Continue sending step_done maps until workflow emits a workflow_complete map.
```

Conversation-scoped workflow:

```text
1. Conversation receives a map with action: "start" for two participant agents.
2. Participant A sends a map with action: "message"; conversation relays a `conversation_turn` to
   participant B.
3. Participant B replies through the same coordinator; conversation relays the turn back to
   participant A.
4. A workflow or aggregation agent can request `conversation_summary` and consume the structured
   turn history.
```

## Gap Analysis

| Method | Status | Gap |
| --- | --- | --- |
| Routing | Fully implementable for direct one-to-one, keyed unbounded one-to-one, and primitive unbounded fan-out. | Keyed routes use a map containing parallel `keys` and `targets` lists because ordinary methods do not have a safe type predicate for scanning a list of route-entry maps. Richer message inspection and nested recipient descriptors require a richer data query layer. |
| Supervision | Partially implementable. | The method can spawn and track unbounded child method-name lists with one shared start version, but methods cannot autonomously observe child crashes or exits; callers must send `child_failed` or `child_exited` events. A failed `spawn(...)` aborts the remaining ordinary method evaluation, so supervision can avoid reporting `running` for an incomplete child set but cannot emit a catchable spawn-failure status without a non-aborting spawn result or method-existence check. Removing arbitrary failed ids from the tracked list or starting one mixed-version list requires a list-filter operation, separate supervisors, or a specialized replacement method. |
| Distribution | Partially implementable. | The method assigns one work payload to an unbounded worker list by composing with routing. Dynamic decomposition into distinct per-worker portions and load-aware placement require additional decomposition methods or richer collection-processing conventions. |
| Aggregation | Fully implementable for list-valued fan-in. | Duplicate handling, custom merge functions, and richer aggregate policies require deeper collection operations or specialized aggregate methods. |
| Scheduling | Partially implementable. | There is no runtime clock or timer callback; scheduling requires explicit `tick` messages from another agent or host process. |
| Synchronization | Fully implementable for unbounded count-based gates. | Membership validation against a declared dependency set and duplicate suppression require richer collection querying/filtering or a specialized validation method. |
| Workflow | Partially implementable. | The method processes an unbounded linear step sequence and can skip one pending step on a branch outcome. Arbitrary workflow graphs, branch destinations by id, list length validation, and map-shaped step descriptors require richer collection querying or a specialized transition method. |
| Conversation | Fully implementable for bounded two-agent exchange. | Alternation rules, participant timeouts, semantic summaries, and searchable long-term history require additional methods or host-driven scheduling. |
| Retry | Fully implementable for immediate retry and scheduled retry by composition. | Backoff policies need an external tick convention and richer arithmetic/time policy support. |

No method in this methodology is blocked entirely. The missing capabilities are real-time timers,
autonomous lifecycle event observation, non-aborting spawn failure results, dynamic collection
iteration for non-routing cases, and ordinary-method indexed assignment. Those gaps are documented
here so the reusable behaviors remain ordinary methods rather than hidden runtime features.
