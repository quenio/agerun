# Supervision Method v1.0.0

## Overview

The supervision method creates and tracks an unbounded list of child agents and applies a simple
restart policy when the supervisor receives explicit lifecycle event messages. It demonstrates
supervision as methodology logic rather than a runtime capability.

## Behavior

On a map whose `action` field is `"start"`, the method stores the policy, reply target, and shared
child method version, clears its tracked child lists, and sends itself a `spawn_child` continuation
for the request's `child_method_names` list. The method consumes that list with `head(...)` and
`tail(...)`, spawning one child per continuation and appending the agent id and child record into
memory. When the list is exhausted, it reports `status=running` with the tracked child lists and
count. If the initial spawn continuation or a later continuation cannot be queued, it reports
`status=handoff_failed` instead of remaining in `starting`.
If a requested child cannot be spawned, the failed `spawn(...)` instruction aborts the remaining
ordinary method evaluation. The supervisor does not report `running` for the incomplete child set,
but it also cannot emit a catchable spawn-failure status without a runtime-level non-aborting spawn
result or a separate method-existence check.

On a map whose `action` field is `"child_failed"` or `"child_exited"`, the method scans the tracked
`child_agent_ids` list before applying the lifecycle event. If the child is tracked and the stored
policy is `restart`, it spawns a replacement child using the event's `child_method_name` and
`child_method_version`, appends the replacement to the tracked lists, and reports
`status=restarted`. If the child is tracked and the policy is not `restart`, it reports
`status=stopped`. If the child is not tracked, it leaves supervisor state unchanged and reports
`status=ignored`. If the internal validation message cannot be queued, it reports
`status=handoff_failed` and leaves lifecycle handling unapplied.

On a map whose `action` field is `"stop"`, the method scans the tracked `child_agent_ids` list with
`head(...)` and `tail(...)`. It exits and reports `status=stopped` only when the supplied
`child_agent_id` belongs to that tracked list. If the id is not tracked, it leaves the agent alive,
keeps the stored supervisor status unchanged, and reports `status=ignored`. If the internal stop
validation message cannot be queued, it reports `status=handoff_failed` and does not exit the child.

## Message Format

Start request:

```text
{
  action: "start",
  child_method_names: [<method>, <method>, ...],
  child_method_version: <version>,
  policy: "restart",
  reply_to: <agent>
}
```

Lifecycle event requests:

```text
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

Status response:

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

## Action Field

The input `action` field is a command discriminator in the request map. The supervision agent runs
this method for every message it receives, so the field distinguishes startup, failure, exit, and
stop commands from unrelated messages.

## Composition Notes

Use supervision around long-lived routing, scheduling, workflow, or worker agents. A supervision
agent can start many children from one `child_method_names` list. Other methods can report lifecycle
events to the supervisor when they observe a child failure through application-level messages.
Composed callers can safely send stop requests through supervisors because untracked child ids are
reported as ignored instead of being exited. Lifecycle events are guarded the same way, so a stale
or misaddressed failure cannot add bogus replacement children.

## Limitations

The method cannot autonomously detect crashes or exits. AgeRun methods do not receive implicit child
lifecycle events, so callers must send `child_failed` or `child_exited` messages. The start contract
uses one shared child method version for the unbounded method-name list; heterogeneous versions can
be modeled with separate supervisors or by sending restart events with explicit method versions. The
method appends replacement child ids to its tracked lists; it does not remove arbitrary failed ids
from the middle of the list because ordinary methods do not currently have an atomic list-filter
operation. A failed `spawn(...)` instruction aborts method evaluation, so ordinary supervision
methods cannot catch that failure and send a terminal failure report after the failed instruction.

## Implementation and Tests

Implementation: [`supervision-1.0.0.method`](supervision-1.0.0.method)

Test: [`supervision_tests.c`](supervision_tests.c)
