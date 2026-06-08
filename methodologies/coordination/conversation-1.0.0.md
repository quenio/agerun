# Conversation Method v1.0.0

## Overview

The conversation method coordinates a bounded conversation between two participant agents. It relays
each turn from one participant to the other, tracks turn history, and exposes structured status and
summary messages while remaining an ordinary AgeRun method.

## Behavior

On a map whose `action` field is `"start"`, the method stores the conversation id, optional
correlation id, two participant agent ids, and reply target. It marks the conversation active and
clears turn state and history.

On a map whose `action` field is `"message"`, it accepts the message only when the
`conversation_id` matches the active conversation, the conversation is active, and `sender` is one
of the two participants. It sends a `conversation_turn` map to the other participant, appends the
turn to history, updates last-turn state, and reports relay status to the reply target. The
`relayed` status means the turn was delivered and recorded; if delivery fails, the method reports
`relay_failed` without appending the turn or incrementing `turn_count`.

On a map whose `action` field is `"summary"`, it sends a structured summary containing the turn
history. On a map whose `action` field is `"close"`, it marks the conversation closed and sends a
closed notice to both participants and the reply target.

## Message Format

Requests:

```text
{
  action: "start",
  conversation_id: <id>,
  correlation_id: <id>,
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

Relayed turn:

```text
{
  action: "conversation_turn",
  conversation_id: <id>,
  correlation_id: <correlation_id>,
  from: <agent>,
  to: <agent>,
  text: <text>,
  intent: <intent>,
  turn_count: <count>
}
```

Coordinator response:

```text
{
  action: <conversation_started|conversation_relayed|conversation_summary|conversation_closed>,
  conversation_id: <id>,
  correlation_id: <correlation_id>,
  state: <state>,
  status: <active|relayed|relay_failed|ignored|closed>,
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

## Action Field

The input `action` field is a command discriminator in the request map. The conversation agent runs
this method for every message it receives, so the field separates setup, participant turns,
summaries, and closure while preserving ordinary map-shaped conversational content.

## Composition Notes

Use conversation when two worker or assistant agents need a mediated exchange. Workflow can start a
conversation for a pair of agents, direct sends can deliver participant messages to the conversation
coordinator, and aggregation or workflow can consume the structured summary history when the
conversation closes.

## Limitations

The method stores an append-backed turn history but does not enforce alternation, speaker-specific
policies, participant timeouts, unbounded search, or semantic summarization. Those behaviors require
additional methods, completion-backed summaries, or host-driven scheduling.

## Implementation and Tests

Implementation: [`conversation-1.0.0.method`](conversation-1.0.0.method)

Test: [`conversation_tests.c`](conversation_tests.c)
