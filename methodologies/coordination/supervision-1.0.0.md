# Supervision Method v1.0.0

## Overview

The supervision method keeps track of one child agent and applies a simple restart policy when the
supervisor receives explicit lifecycle event messages. It demonstrates supervision as methodology
logic rather than a runtime capability.

## Behavior

On `action=start`, the method stores the child method name, version, policy, and reply target, then
spawns the child method. It reports `status=running` with the child agent id.

On `action=child_failed` or `action=child_exited`, the method checks whether the stored policy is
`restart`. If so, it spawns a replacement child and reports `status=restarted`. Otherwise, it reports
`status=stopped`.

On `action=stop`, the method exits the tracked child agent and reports `status=stopped`.

## Message Format

Start request:

```text
action=start child_method_name=<method> child_method_version=<version> policy=restart reply_to=<agent>
```

Lifecycle event requests:

```text
action=child_failed
action=child_exited
action=stop
```

Status response:

```text
action=supervision_status status=<running|restarted|stopped> child_agent_id=<agent> policy=<policy>
```

## Composition Notes

Use supervision around long-lived routing, scheduling, workflow, or worker agents. Other methods can
report lifecycle events to the supervisor when they observe a child failure through application-level
messages.

## Limitations

The method cannot autonomously detect crashes or exits. AgeRun methods do not receive implicit child
lifecycle events, so callers must send `child_failed` or `child_exited` messages.

## Implementation and Tests

Implementation: [`supervision-1.0.0.method`](supervision-1.0.0.method)

Test: [`supervision_tests.c`](supervision_tests.c)

