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
| [`supervision`](supervision-1.0.0.md) | [`supervision-1.0.0.method`](supervision-1.0.0.method) | [`supervision_tests.c`](supervision_tests.c) | Creates, tracks, stops, and event-restarts a child agent. | Keeps coordination agents available. |
| [`distribution`](distribution-1.0.0.md) | [`distribution-1.0.0.method`](distribution-1.0.0.method) | [`distribution_tests.c`](distribution_tests.c) | Assigns caller-provided portions of work to workers. | Builds on routing for fan-out. |
| [`aggregation`](aggregation-1.0.0.md) | [`aggregation-1.0.0.method`](aggregation-1.0.0.method) | [`aggregation_tests.c`](aggregation_tests.c) | Collects fixed-slot results and emits a combined result. | Completes fan-in. |
| [`scheduling`](scheduling-1.0.0.md) | [`scheduling-1.0.0.method`](scheduling-1.0.0.method) | [`scheduling_tests.c`](scheduling_tests.c) | Stores pending work and triggers it on explicit tick messages. | Delayed execution primitive. |
| [`synchronization`](synchronization-1.0.0.md) | [`synchronization-1.0.0.method`](synchronization-1.0.0.method) | [`synchronization_tests.c`](synchronization_tests.c) | Waits for fixed dependencies before sending a continuation. | Dependency gate. |
| [`workflow`](workflow-1.0.0.md) | [`workflow-1.0.0.method`](workflow-1.0.0.method) | [`workflow_tests.c`](workflow_tests.c) | Maintains a small step graph, routes steps, branches, and completes. | Higher-level sequence and branch coordinator. |
| [`conversation`](conversation-1.0.0.md) | [`conversation-1.0.0.method`](conversation-1.0.0.method) | [`conversation_tests.c`](conversation_tests.c) | Tracks conversation state across related messages. | Context and correlation memory. |
| [`retry`](retry-1.0.0.md) | [`retry-1.0.0.method`](retry-1.0.0.method) | [`retry_tests.c`](retry_tests.c) | Re-executes failed operations within a retry policy. | Uses direct send or scheduled retry. |

## Message Contracts

The contracts below are map-shaped messages. Method implementations that create new messages
initialize an empty map and then populate its keys with ordinary nested assignments such as
`memory.output.action := "..."`.

### Routing

Request:

```text
action=route mode=one target=<agent> payload_action=<action> payload_text=<text> correlation_id=<id> reply_to=<agent>
action=route mode=many target_a=<agent> target_b=<agent> target_c=<agent> payload_action=<action> payload_text=<text> correlation_id=<id> reply_to=<agent>
```

Optional keyed selection for `mode=one`:

```text
route_key=<key> route_a_key=<key> target_a=<agent> route_b_key=<key> target_b=<agent> route_c_key=<key> target_c=<agent>
```

Forwarded message:

```text
action=<payload_action> correlation_id=<correlation_id> text=<payload_text> source=<routing-agent>
```

Reply:

```text
action=route_result status=<routed|ignored> routed_count=<count> sent_one=<0|1> sent_a=<0|1> sent_b=<0|1> sent_c=<0|1>
```

### Supervision

Requests:

```text
action=start child_method_name=<method> child_method_version=<version> policy=restart reply_to=<agent>
action=child_failed
action=child_exited
action=stop
```

Reply:

```text
action=supervision_status status=<running|restarted|stopped> child_agent_id=<agent> policy=<policy>
```

### Distribution

Request:

```text
action=distribute work_id=<id> routing_agent=<agent> reply_to=<agent> worker_a=<agent> portion_a=<text> worker_b=<agent> portion_b=<text> worker_c=<agent> portion_c=<text>
```

Forwarded through `routing`:

```text
action=route mode=one target=<worker> payload_action=work payload_text=<portion> correlation_id=<work_id> reply_to=0
```

Reply:

```text
action=distribution_result status=distributed work_id=<id> assignment_count=<count> sent_a=<0|1> sent_b=<0|1> sent_c=<0|1>
```

### Aggregation

Requests:

```text
action=start aggregate_id=<id> required_count=<1-3> reply_to=<agent>
action=result slot=<a|b|c> value=<text>
```

Completion:

```text
action=aggregate_complete aggregate_id=<id> status=complete result_a=<text> result_b=<text> result_c=<text> received_count=<count>
```

### Scheduling

Requests:

```text
action=schedule schedule_id=<id> due_tick=<number> target=<agent> payload_action=<action> payload_text=<text> correlation_id=<id> reply_to=<agent>
action=tick tick=<number>
action=cancel schedule_id=<id>
```

Triggered message:

```text
action=<payload_action> correlation_id=<correlation_id> text=<payload_text> schedule_id=<schedule_id>
```

Status:

```text
action=schedule_status schedule_id=<id> status=<scheduled|cancelled|triggered> pending=<0|1> current_tick=<number>
```

### Synchronization

Requests:

```text
action=wait sync_id=<id> required_count=<1-3> required_a=<name> required_b=<name> required_c=<name> continuation_target=<agent> continuation_action=<action> continuation_text=<text> reply_to=<agent>
action=dependency dependency=<name>
```

Continuation:

```text
action=<continuation_action> sync_id=<id> text=<continuation_text> done_count=<count>
```

Status:

```text
action=synchronization_status sync_id=<id> status=complete done_count=<count>
```

### Workflow

Start:

```text
action=start workflow_id=<id> routing_agent=<agent> reply_to=<agent> step1_target=<agent> step1_action=<action> step1_text=<text> step2_target=<agent> step2_action=<action> step2_text=<text> step3_target=<agent> step3_action=<action> step3_text=<text> branch_value=<outcome>
```

Step completion:

```text
action=step_done step=<1|2|3> outcome=<value>
```

Completion:

```text
action=workflow_complete workflow_id=<id> status=complete current_step=3
```

### Conversation

Requests:

```text
action=start conversation_id=<id> user_id=<id> reply_to=<agent>
action=message text=<text> intent=<intent>
action=summary
action=close
```

Responses:

```text
action=conversation_status conversation_id=<id> state=active text=<state-or-summary> turn_count=<count>
action=conversation_summary conversation_id=<id> state=<state> text=conversation=<id>|user=<id>|state=<state>|turns=<count>|last=<text>|previous=<text> turn_count=<count>
action=conversation_closed conversation_id=<id> state=closed text=closed turn_count=<count>
```

### Retry

Requests:

```text
action=start operation_id=<id> operation_target=<agent> operation_action=<action> operation_text=<text> max_attempts=<number> strategy=<immediate|scheduled> scheduler_agent=<agent> delay_ticks=<tick> reply_to=<agent>
action=failure
action=success
```

Attempt:

```text
action=<operation_action> correlation_id=<operation_id> text=<operation_text> attempt=<number>
```

Scheduled retry request:

```text
action=schedule schedule_id=<operation_id> due_tick=<delay_ticks> target=<operation_target> payload_action=<operation_action> payload_text=<operation_text> correlation_id=<operation_id> reply_to=0
```

Result:

```text
action=retry_result operation_id=<id> status=<succeeded|failed> attempts=<count>
```

## Composition Examples

Fan-out and fan-in:

```text
1. Send action=distribute to a distribution agent with worker_a, worker_b, worker_c, and routing_agent.
2. Workers send action=result slot=a|b|c value=<text> to an aggregation agent.
3. Aggregation emits action=aggregate_complete when required_count results have arrived.
```

Delayed retry:

```text
1. Send action=start strategy=scheduled scheduler_agent=<scheduling-agent> to a retry agent.
2. On action=failure, retry sends action=schedule to the scheduling agent.
3. An external tick source sends action=tick tick=<number> to scheduling.
4. Scheduling re-emits the operation attempt at the requested tick.
```

Branching workflow:

```text
1. Send action=start to workflow with routing_agent and three step targets.
2. Workflow routes step 1 through routing.
3. Send action=step_done step=1 outcome=<branch_value> to route directly to step 3.
4. Send action=step_done step=3 to emit action=workflow_complete.
```

Conversation-scoped workflow:

```text
1. Conversation receives action=start for a user conversation.
2. Related action=message requests update last and previous text.
3. A workflow or routing agent can use conversation_id as correlation_id.
4. Conversation can emit action=conversation_summary when another agent needs context.
```

## Gap Analysis

| Method | Status | Gap |
| --- | --- | --- |
| Routing | Fully implementable for the bounded contract. | Arbitrary recipient lists and richer message inspection require collection iteration or a richer data query layer. |
| Supervision | Partially implementable. | Methods cannot autonomously observe child crashes or exits; callers must send `child_failed` or `child_exited` events. |
| Distribution | Partially implementable. | The method assigns caller-provided portions; dynamic decomposition and arbitrary worker lists require collection iteration. |
| Aggregation | Fully implementable for the bounded contract. | Arbitrary result sets require dynamic collections or iteration. |
| Scheduling | Partially implementable. | There is no runtime clock or timer callback; scheduling requires explicit `tick` messages from another agent or host process. |
| Synchronization | Fully implementable for the bounded contract. | Arbitrary dependency sets require collection iteration. |
| Workflow | Partially implementable. | General workflow graphs require dynamic graph storage and iteration; this method supports a three-step graph with one branch condition. |
| Conversation | Fully implementable for bounded memory. | Unbounded history, search, or summarization require additional memory conventions or completion-backed methods. |
| Retry | Fully implementable for immediate retry and scheduled retry by composition. | Backoff policies need an external tick convention and richer arithmetic/time policy support. |

No method in this methodology is blocked entirely. The missing runtime capabilities are real-time
timers, autonomous lifecycle event observation, and dynamic collection iteration. Those gaps are
documented here so the reusable behaviors remain ordinary methods rather than hidden runtime
features.
