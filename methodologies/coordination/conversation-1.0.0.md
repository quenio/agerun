# Conversation Method v1.0.0

## Overview

Conversation coordinates an exchange among participant agents. It relays each turn to every
participant except the sender, tracks turn history, and exposes structured responses while
remaining an ordinary AgeRun method.

## Behavior

Only messages with a recognized `request` value are handled as coordination requests.

On `request: "conversation_start"`, the method stores the effective `trace_id`, `session_id`,
participant list, and `sender`, then spawns one `broadcasting` method agent for the conversation
session. Later turns reuse that broadcasting agent.

On `request: "conversation_message"` with the same `session_id`, it accepts the sender-provided
`payload` only when `sender` is in the participant list. For participant senders, it builds a
`conversation_turn` request and sends that same turn message through broadcasting to all
participants except the sender. The turn is recorded only after broadcasting reports success. On
`conversation_summary`, it responds with history. On `conversation_close`, it marks the conversation
closed.

## Message Format

Requests:

```text
{ sender: <sender-agent>, request: "conversation_start", trace_id: <trace_id>, session_id: <session_id>, participants: [<recipient-agent-1>, <recipient-agent-2>, ...] }
{ sender: <sender-agent>, request: "conversation_message", trace_id: <trace_id>, session_id: <session_id>, payload: <payload> }
{ sender: <sender-agent>, request: "conversation_summary", trace_id: <trace_id>, session_id: <session_id> }
{ sender: <sender-agent>, request: "conversation_close", trace_id: <trace_id>, session_id: <session_id> }
```

Relayed turn:

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

Coordinator response:

```text
{
  sender: <conversation-agent>,
  response: "conversation_result",
  trace_id: <trace_id>,
  session_id: <session_id>,
  status: <success|failure>,
  state: <active|closed>,
  result: <active|relayed|not_participant|relay_failed|ignored|closed>,
  success_count: <count>,
  failure_count: <count>,
  participants: [<recipient-agent-1>, <recipient-agent-2>, ...],
  last_sender: <agent>,
  last_payload: <payload>,
  turn_count: <count>,
  history: [<conversation_turn>, ...]
}
```

Count semantics: `success_count` increments for a `conversation_message` only when the participant
turn is broadcast successfully and appended to history; summary and close responses report the
current successful turn count, and start responses report `0`. `failure_count` increments to `1`
when the broadcasting helper cannot be spawned, when a turn relay fails before or during
broadcasting or history append, or when a non-participant sends `conversation_message`; summary and
close responses report `0`.

Status semantics: the response status is `success` for a successful start, summary, close, or
participant turn relay. It is `failure` when the broadcasting helper cannot be spawned, when a turn
relay fails, or when a non-participant sends `conversation_message`.

If broadcast delivery fails for any recipient, the coordinator reports `result: "relay_failed"` and
leaves the history and turn count unchanged.

If the `sender` of a `conversation_message` is not in the participant list, the coordinator reports
`result: "not_participant"` and does not broadcast, record, or retain the sender-provided payload.

## Implementation and Tests

Implementation: [`conversation-1.0.0.method`](conversation-1.0.0.method)

Test: [`conversation_tests.c`](conversation_tests.c)
