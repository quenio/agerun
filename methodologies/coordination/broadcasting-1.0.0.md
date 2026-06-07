# Broadcasting Method v1.0.0

## Overview

The broadcasting method sends the same caller-provided payload to every positive recipient in an
unbounded target list. It is the coordination methodology's opaque fan-out delivery primitive.

## Behavior

When the agent receives a map whose `action` field is `"broadcast"`, the method reads `targets` as a
list of agent IDs and sends `payload` as-is to each positive target. Broadcasting does not add,
remove, or normalize fields on that payload. If recipients should see a `reply_to`, `source`,
`correlation_id`, or any domain-specific field, the caller includes that field inside `payload`.

The method processes the list with `head(...)` and `tail(...)`, sending continuation messages to
itself until the target list is exhausted. Positive target IDs that cannot receive messages are
counted in the broadcasting agent's memory; valid later targets are still processed. If no positive
target is delivered, or if any positive target send fails, the terminal memory `status` is
`"broadcast_failed"`. Broadcasting does not emit a status reply; callers that need acknowledgements
or recipient replies should include those instructions in the payload or compose broadcasting with
another coordination method.

## Message Format

Broadcast request:

```text
{
  action: "broadcast",
  targets: [<agent>, <agent>, ...],
  payload: <message>
}
```

Delivered message is exactly the caller-provided `payload`:

```text
<message>
```

`recipient_count` and `sent_count` in the broadcasting agent's memory count successful recipient
sends. `failed_count` counts positive target IDs that could not receive the payload. Integer `0`
entries are skipped placeholders, not failed sends.

## Action Field

The input `action` field is a command discriminator in the request map. The broadcasting agent runs
this method for every message it receives, so `action: "broadcast"` marks the message as fan-out work
instead of arbitrary status or coordination data.

## Composition Notes

Use broadcasting when every recipient should receive the same message without changing its shape.
Use distribution when a list of distinct payloads should be assigned across a worker list. Use
routing when one recipient must be selected by key.

## Limitations

The method supports unbounded fan-out for primitive positive agent IDs. Integer `0` is treated as a
placeholder rather than a recipient; target lists should contain positive IDs for all intended
recipients.

## Implementation and Tests

Implementation: [`broadcasting-1.0.0.method`](broadcasting-1.0.0.method)

Test: [`broadcasting_tests.c`](broadcasting_tests.c)
