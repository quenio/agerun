# Conversation Method v1.0.0

## Overview

Conversation coordinates an exchange among participant agents. It relays each turn to every
participant except the sender, tracks turn history, and exposes structured responses while
remaining an ordinary AgeRun method.

## Behavior

Only messages with a recognized `request` value are handled as coordination requests.

On `request: "conversation_start"`, the method stores the effective `trace_id`, `session_id`,
participant list, and `source`, then spawns one `broadcasting` method agent for the conversation
session. Later turns reuse that broadcasting agent.

On `request: "conversation_message"` with the same `session_id`, it accepts the sender-provided
`payload` while the conversation is active, builds a `conversation_turn` request, and sends that
same turn message through broadcasting to all participants except the sender. The turn is recorded
only after broadcasting reports success. On `conversation_summary`, it responds with history. On
`conversation_close`, it marks the conversation closed.

## Message Format

Requests:

```text
{ source: <sender-agent>, request: "conversation_start", trace_id: <trace_id>, session_id: <session_id>, participants: [<recipient-agent-1>, <recipient-agent-2>, ...] }
{ source: <sender-agent>, request: "conversation_message", trace_id: <trace_id>, session_id: <session_id>, payload: <payload>, sender: <agent> }
{ source: <sender-agent>, request: "conversation_summary", trace_id: <trace_id>, session_id: <session_id> }
{ source: <sender-agent>, request: "conversation_close", trace_id: <trace_id>, session_id: <session_id> }
```

Relayed turn:

```text
{
  source: <conversation-agent>,
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
  source: <conversation-agent>,
  response: "conversation_result",
  trace_id: <trace_id>,
  session_id: <session_id>,
  status: <success|failure>,
  state: <active|closed>,
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

If broadcast delivery fails for any recipient, the coordinator reports `result: "relay_failed"` and
leaves the history and turn count unchanged.

## Implementation and Tests

Implementation: [`conversation-1.0.0.method`](conversation-1.0.0.method)

Test: [`conversation_tests.c`](conversation_tests.c)
