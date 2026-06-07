# Synchronization Method v1.0.0

## Overview

The synchronization method waits for an unbounded stream of dependency messages before sending a
continuation message. It is a reusable dependency gate for workflows and distributed work.

## Behavior

On a map whose `action` field is `"wait"`, the method stores the sync id, required count,
continuation target, continuation action, continuation text, and reply target. It clears the
append-backed `received` dependency list and resets the completion marker.
Required counts below one behave as one required dependency, so a wait message cannot complete the
gate before any dependency arrives.

On a map whose `action` field is `"dependency"`, the method only counts the message when its
`sync_id` matches the active wait request and the gate has not already completed. It appends the
dependency value to `memory.received`. When the number of received dependencies reaches
`required_count`, it sends the continuation and reports completion.

## Message Format

Wait request:

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
```

Dependency request:

```text
{
  action: "dependency",
  sync_id: <id>,
  dependency: <name>
}
```

Continuation message:

```text
{
  action: <continuation_action>,
  sync_id: <id>,
  text: <continuation_text>,
  done_count: <count>,
  dependencies: [<dependency>, <dependency>, ...]
}
```

Status response:

```text
{
  action: "synchronization_status",
  sync_id: <id>,
  status: "complete",
  done_count: <count>,
  dependencies: [<dependency>, <dependency>, ...]
}
```

## Action Field

The input `action` field is a command discriminator in the request map. The synchronization agent
runs this method for every message it receives, so the field separates wait setup from dependency
arrival and keeps unrelated messages from satisfying the gate.

## Composition Notes

Use synchronization when several workers, approvals, or prerequisite steps must complete before a
workflow advances. It can also gate an aggregation completion before sending a follow-up message.

## Limitations

The method supports an unbounded count of dependency messages by appending received dependencies to
a list. It does not validate membership against a declared dependency set or de-duplicate repeated
dependency names; callers that need those policies should put that validation in the producing
methods or compose a specialized filtering method before this gate.

## Implementation and Tests

Implementation: [`synchronization-1.0.0.method`](synchronization-1.0.0.method)

Test: [`synchronization_tests.c`](synchronization_tests.c)
