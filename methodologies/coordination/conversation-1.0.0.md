# Conversation Method v1.0.0

## Overview

Conversation coordinates a bounded exchange between two participant agents. It relays each turn,
tracks turn history, and exposes structured responses while remaining an ordinary AgeRun method.

## Behavior

Only messages with `type: "request"` are handled as coordination requests.

On `action: "start"`, the method stores the conversation id, `trace_id`, participant agent ids,
and `source_agent`. On `action: "message"`, it accepts messages from either participant while the
conversation is active, relays a `conversation_turn` request to the other participant, and records
the turn only after delivery succeeds. On `summary`, it responds with history. On `close`, it marks
the conversation closed and notifies the participants.

## Message Format

Requests:

```text
{ action: "start", type: "request", conversation_id: <id>, trace_id: <id>, participant_a: <agent>, participant_b: <agent>, source_agent: <agent> }
{ action: "message", type: "request", conversation_id: <id>, sender: <agent>, text: <text>, intent: <intent> }
{ action: "summary", type: "request", conversation_id: <id> }
{ action: "close", type: "request", conversation_id: <id> }
```

Relayed turn:

```text
{
  action: "conversation_turn",
  type: "request",
  conversation_id: <id>,
  trace_id: <trace_id>,
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
  action: <start|message|summary|close>,
  type: "response",
  conversation_id: <id>,
  trace_id: <trace_id>,
  state: <active|closed>,
  status: <success|failure>,
  result: <active|relayed|relay_failed|ignored|closed>,
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

## Implementation and Tests

Implementation: [`conversation-1.0.0.method`](conversation-1.0.0.method)

Test: [`conversation_tests.c`](conversation_tests.c)
