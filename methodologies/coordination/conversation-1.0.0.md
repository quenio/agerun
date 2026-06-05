# Conversation Method v1.0.0

## Overview

The conversation method keeps lightweight conversational state across related messages. It provides
context and correlation memory that other coordination methods can use while still remaining an
ordinary AgeRun method.

## Behavior

On `action=start`, the method stores the conversation id, user id, and reply target. It marks the
conversation active and clears turn state.

On `action=message`, it moves the previous last text into `previous_text`, stores the new text and
intent, increments the turn count, and replies with a compact summary string.

On `action=summary`, it sends the current summary. On `action=close`, it marks the conversation
closed and sends a closed response.

## Message Format

Requests:

```text
action=start conversation_id=<id> user_id=<id> reply_to=<agent>
action=message text=<text> intent=<intent>
action=summary
action=close
```

Status response:

```text
action=conversation_status conversation_id=<id> state=active text=<state-or-summary> turn_count=<count>
```

Summary response:

```text
action=conversation_summary conversation_id=<id> state=<state> text=conversation=<id>|user=<id>|state=<state>|turns=<count>|last=<text>|previous=<text> turn_count=<count>
```

Closed response:

```text
action=conversation_closed conversation_id=<id> state=closed text=closed turn_count=<count>
```

## Composition Notes

Use conversation to carry context around workflow, routing, or retry messages. Other methods can use
`conversation_id` as a correlation id and request `action=summary` when they need a compact context
snapshot.

## Limitations

The method stores bounded memory only: current text, previous text, last intent, state, and turn
count. Unbounded history, search, and summarization need additional memory conventions or
completion-backed methods.

## Implementation and Tests

Implementation: [`conversation-1.0.0.method`](conversation-1.0.0.method)

Test: [`conversation_tests.c`](conversation_tests.c)

