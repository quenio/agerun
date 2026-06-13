# Conversation Method v1.0.0

## Overview

Conversation coordinates a bounded exchange between two participant agents. It relays each turn,
tracks turn history, and exposes structured responses while remaining an ordinary AgeRun method.

## Behavior

Only messages with a recognized `request` value are handled as coordination requests.

On `request: "conversation_start"`, the method stores the effective `trace_id`, `session_id`,
participant agent ids, and `source`. On `request: "conversation_message"` with the same
`session_id`, it accepts messages from either participant while the conversation is active, relays
a `conversation_turn` request to the other participant, and records the turn only after delivery
succeeds. On `conversation_summary`, it responds with history. On `conversation_close`, it marks
the conversation closed and notifies the participants.

## Message Format

Requests:

```text
{ source: <sender-agent>, request: "conversation_start", trace_id: <trace_id>, session_id: <session_id>, participant_a: <agent>, participant_b: <agent> }
{ source: <sender-agent>, request: "conversation_message", trace_id: <trace_id>, session_id: <session_id>, sender: <agent>, text: <text>, intent: <intent> }
{ source: <sender-agent>, request: "conversation_summary", trace_id: <trace_id>, session_id: <session_id> }
{ source: <sender-agent>, request: "conversation_close", trace_id: <trace_id>, session_id: <session_id> }
```

Relayed turn:

```text
{
  source: <sender-agent>,
  request: "conversation_turn",
  trace_id: <trace_id>,
  session_id: <session_id>,
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
  source: <conversation-agent>,
  response: "conversation_result",
  trace_id: <trace_id>,
  session_id: <session_id>,
  status: <success|failure>,
  state: <active|closed>,
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
