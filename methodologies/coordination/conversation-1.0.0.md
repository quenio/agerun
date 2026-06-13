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
`payload` only when `sender` is in the participant list and no turn relay is already pending. For
participant senders, it builds a `conversation_turn` request and sends that same turn message
through broadcasting to all participants except the sender. The turn is recorded after broadcasting
reports success, or immediately when the sender is the only participant. On `conversation_history`,
it responds with history. On `conversation_close`, it marks the conversation closed and clears
pending relay work.

## Message Format

Requests:

```text
{ sender: <sender-agent>, request: "conversation_start", trace_id: <trace_id>, session_id: <session_id>, participants: [<recipient-agent-1>, <recipient-agent-2>, ...] }
{ sender: <sender-agent>, request: "conversation_message", trace_id: <trace_id>, session_id: <session_id>, payload: <payload> }
{ sender: <sender-agent>, request: "conversation_history", trace_id: <trace_id>, session_id: <session_id> }
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
  result: <active|relayed|relay_failed|ignored|closed>,
  success_count: <count>,
  failure_count: <count>,
  participants: [<recipient-agent-1>, <recipient-agent-2>, ...],
  last_sender: <agent>,
  last_payload: <payload>,
  turn_count: <count>,
  history: [<conversation_turn>, ...]
}
```

Count semantics: `success_count` reports the current successful turn count. A
`conversation_message` increments that count when the participant turn is broadcast successfully, or
when no other participant needs to receive it and the turn is recorded locally. History and close
responses report the current successful turn count, and start responses report `0`. `failure_count`
increments to `1` when the broadcasting helper cannot be spawned or when a turn relay fails before
or during broadcasting; history and close responses report `0`. Non-participant
`conversation_message` requests and turns received while another relay is pending are ignored and do
not change status or count attributes.

Status semantics: the response status is `success` for a successful start, history, close, or
participant turn relay. It is `failure` when the broadcasting helper cannot be spawned or when a
turn relay fails.

If broadcast delivery fails for any recipient, the coordinator reports `result: "relay_failed"` and
leaves the history and turn count unchanged.

If the `sender` of a `conversation_message` is not in the participant list, the coordinator ignores
the request. It does not broadcast, record, respond, or update conversation state.

## Implementation and Tests

Implementation: [`conversation-1.0.0.method`](conversation-1.0.0.method)

Test: [`conversation_tests.c`](conversation_tests.c)
