# Synchronization Method v1.0.0

## Overview

The synchronization method waits for a bounded set of dependency messages before sending a
continuation message. It is a reusable dependency gate for workflows and distributed work.

## Behavior

On `action=wait`, the method stores the sync id, required count, dependency names, continuation
target, continuation action, continuation text, and reply target. It clears all dependency markers.

On `action=dependency`, the method compares `dependency` with `required_a`, `required_b`, and
`required_c`. When the number of satisfied dependencies reaches `required_count`, it sends the
continuation and reports completion.

## Message Format

Wait request:

```text
action=wait sync_id=<id> required_count=<1-3> required_a=<name> required_b=<name> required_c=<name> continuation_target=<agent> continuation_action=<action> continuation_text=<text> reply_to=<agent>
```

Dependency request:

```text
action=dependency dependency=<name>
```

Continuation message:

```text
action=<continuation_action> sync_id=<id> text=<continuation_text> done_count=<count>
```

Status response:

```text
action=synchronization_status sync_id=<id> status=complete done_count=<count>
```

## Composition Notes

Use synchronization when several workers, approvals, or prerequisite steps must complete before a
workflow advances. It can also gate an aggregation completion before sending a follow-up message.

## Limitations

The method supports three named dependencies. Arbitrary dependency sets require collection iteration
or a convention that maps dynamic inputs into fixed slots.

## Implementation and Tests

Implementation: [`synchronization-1.0.0.method`](synchronization-1.0.0.method)

Test: [`synchronization_tests.c`](synchronization_tests.c)

