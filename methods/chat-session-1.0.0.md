# Chat Session Method

Version: 1.0.0

## Overview

The `chat-session` method is a concrete session-backend example for AgeRun. It models one
agent per customer conversation and keeps durable per-session state in agent memory.

This is useful for support chat, onboarding wizards, and other conversational workflows where a
session must remember identity, channel, recent messages, escalation state, and a compact summary.

## Message Format

The method expects MAP messages with these common fields:

- `action`: One of `"start"`, `"message"`, `"summary"`, or `"close"`
- `session_id`: External conversation/session identifier
- `user_id`: User identifier
- `channel`: Channel name such as `"web"` or `"sms"`
- `content`: User message content for `"message"` actions
- `intent`: High-level intent for `"message"` actions
- `sender`: Agent ID that should receive the response

## Behavior

### `action = "start"`
Initializes or refreshes the core session record:
- Sets `memory.session_id`, `memory.user_id`, and `memory.channel`
- Sets `memory.state` to `"active"`
- Resets `memory.turn_count` to `0`
- Clears escalation state
- Stores `memory.last_reply = "session_started"`

### `action = "message"`
Processes a new inbound user turn:
- Increments `memory.turn_count`
- Stores `memory.last_user_message`
- Stores `memory.last_intent`
- Keeps `memory.state = "active"` for normal traffic
- Switches to `memory.state = "waiting_for_human"` when `intent = "human"`
- Switches to `memory.state = "closed"` when `intent = "close"`
- Stores `memory.last_reply` as `"message_received"`, `"handoff_requested"`, or
  `"session_closed"`

### `action = "summary"`
Builds a compact text snapshot in `memory.summary` using the current memory map:

```text
session={session_id} user={user_id} channel={channel} state={state} turns={turn_count} escalation={escalation_requested}
```

### `action = "close"`
Closes the session explicitly:
- Sets `memory.state = "closed"`
- Stores `memory.last_reply = "session_closed"`

## Example Usage

### Start a session

```json
{
  "action": "start",
  "session_id": "sess-1001",
  "user_id": "user-42",
  "channel": "web",
  "content": "",
  "intent": "",
  "sender": 0
}
```

Resulting memory highlights:
- `state = "active"`
- `turn_count = 0`
- `last_reply = "session_started"`

### Process a normal user message

```json
{
  "action": "message",
  "session_id": "sess-1001",
  "user_id": "user-42",
  "channel": "web",
  "content": "I need help with my order",
  "intent": "general",
  "sender": 0
}
```

Resulting memory highlights:
- `state = "active"`
- `turn_count = 1`
- `last_user_message = "I need help with my order"`
- `last_reply = "message_received"`

### Request human handoff

```json
{
  "action": "message",
  "session_id": "sess-1001",
  "user_id": "user-42",
  "channel": "web",
  "content": "I want a human",
  "intent": "human",
  "sender": 0
}
```

Resulting memory highlights:
- `state = "waiting_for_human"`
- `escalation_requested = 1`
- `last_reply = "handoff_requested"`

## Implementation

```text
memory.is_start := if(message.action = "start", 1, 0)
memory.is_message := if(message.action = "message", 1, 0)
memory.is_summary := if(message.action = "summary", 1, 0)
memory.is_close := if(message.action = "close", 1, 0)

memory.session_id := if(memory.is_start = 1, message.session_id, memory.session_id)
memory.user_id := if(memory.is_start = 1, message.user_id, memory.user_id)
memory.channel := if(memory.is_start = 1, message.channel, memory.channel)
memory.state := if(memory.is_start = 1, "active", memory.state)
memory.turn_count := if(memory.is_start = 1, 0, memory.turn_count)
memory.escalation_requested := if(memory.is_start = 1, 0, memory.escalation_requested)
memory.last_user_message := if(memory.is_start = 1, "", memory.last_user_message)
memory.last_intent := if(memory.is_start = 1, "", memory.last_intent)
memory.last_reply := if(memory.is_start = 1, "session_started", memory.last_reply)

memory.last_user_message := if(memory.is_message = 1, message.content, memory.last_user_message)
memory.last_intent := if(memory.is_message = 1, message.intent, memory.last_intent)
memory.turn_count := if(memory.is_message = 1, memory.turn_count + 1, memory.turn_count)
memory.needs_human := if(message.intent = "human", 1, 0)
memory.close_requested := if(message.intent = "close", 1, 0)
memory.escalation_requested := if(memory.needs_human = 1, 1, memory.escalation_requested)
memory.state := if(memory.needs_human = 1, "waiting_for_human", memory.state)
memory.state := if(memory.close_requested = 1, "closed", memory.state)
memory.last_reply := if(memory.is_message = 1, "message_received", memory.last_reply)
memory.last_reply := if(memory.needs_human = 1, "handoff_requested", memory.last_reply)
memory.last_reply := if(memory.close_requested = 1, "session_closed", memory.last_reply)

memory.state := if(memory.is_close = 1, "closed", memory.state)
memory.last_reply := if(memory.is_close = 1, "session_closed", memory.last_reply)

memory.summary := build("session={session_id} user={user_id} channel={channel} state={state} turns={turn_count} escalation={escalation_requested}", memory)
memory.response := if(memory.is_summary = 1, memory.summary, memory.last_reply)
send(message.sender, memory.response)
```

## Notes

This method demonstrates a practical AgeRun pattern:
- one agent per conversation
- persistent session memory
- asynchronous message-driven updates
- explicit escalation and closure states
- generated summaries for polling clients, dashboards, or audit trails

## Walkthrough Reference

The `chat-session` method is a **single-agent conversation state machine**.

Think of it as:

- **one agent = one chat session**
- each incoming message is an event
- the agent stores session state in `memory.*`

## What it tracks

The method keeps these main fields in agent memory:

- `session_id`
- `user_id`
- `channel`
- `state`
- `turn_count`
- `last_user_message`
- `last_intent`
- `last_reply`
- `escalation_requested`
- `summary`

## What messages it understands

It expects MAP messages with fields like:

- `action`
- `session_id`
- `user_id`
- `channel`
- `content`
- `intent`
- `sender`

The important `action` values are:

- `"start"`
- `"message"`
- `"summary"`
- `"close"`

---

# Behavior by action

## 1. `action = "start"`

This initializes the session.

It does things like:

- store the session identity:
  - `memory.session_id := message.session_id`
  - `memory.user_id := message.user_id`
  - `memory.channel := message.channel`
- mark the session active:
  - `memory.state := "active"`
- reset counters and flags:
  - `memory.turn_count := 0`
  - `memory.escalation_requested := 0`
- clear prior conversational fields:
  - `memory.last_user_message := ""`
  - `memory.last_intent := ""`
- set:
  - `memory.last_reply := "session_started"`

### Result
After `start`, the agent has a clean, initialized session record.

---

## 2. `action = "message"`

This processes a normal inbound user turn.

It:

- stores the latest user text:
  - `memory.last_user_message := message.content`
- stores the interpreted intent:
  - `memory.last_intent := message.intent`
- increments the turn counter:
  - `memory.turn_count := memory.turn_count + 1`

Then it checks for special intents.

### Normal message
If `intent` is something ordinary like `"general"`:

- `state` stays `"active"`
- `last_reply` becomes `"message_received"`

### Human handoff
If `intent = "human"`:

- `memory.escalation_requested := 1`
- `memory.state := "waiting_for_human"`
- `memory.last_reply := "handoff_requested"`

### Close request
If `intent = "close"`:

- `memory.state := "closed"`
- `memory.last_reply := "session_closed"`

So a single `"message"` event can either:
- just update the session normally,
- escalate it,
- or close it.

---

## 3. `action = "summary"`

This doesn’t change the conversation much; it builds a compact report string.

It creates:

```text
session={session_id} user={user_id} channel={channel} state={state} turns={turn_count} escalation={escalation_requested}
```

using `build(...)`, and stores it in:

- `memory.summary`

Then for summary requests, the response sent back is the summary text instead of the usual status token.

---

## 4. `action = "close"`

This explicitly closes the session.

It sets:

- `memory.state := "closed"`
- `memory.last_reply := "session_closed"`

This is the direct administrative close path, separate from `"message"` with `intent = "close"`.

---

# How responses work

At the end, the method decides what to send back:

- for most actions, it sends `memory.last_reply`
- for `"summary"`, it sends `memory.summary`

Then it does:

- `send(message.sender, memory.response)`

In the tests, `sender` is usually `0`, so the response is effectively consumed by the system/test harness.

---

# The logic shape

The method is written in a very AgeRun way:

- compute booleans as integers:
  - `memory.is_start := if(message.action = "start", 1, 0)`
- then use those flags to conditionally update memory
- then build a summary
- then send a response

Because AgeRun methods are simple and message-driven, it uses **many sequential conditional assignments** instead of deeply nested logic.

---

# Example flow

## Start
Input:
```json
{
  "action": "start",
  "session_id": "sess-1001",
  "user_id": "user-42",
  "channel": "web",
  "content": "",
  "intent": "",
  "sender": 0
}
```

Memory after:
- `state = "active"`
- `turn_count = 0`
- `last_reply = "session_started"`

## User message
Input:
```json
{
  "action": "message",
  "session_id": "sess-1001",
  "user_id": "user-42",
  "channel": "web",
  "content": "I need help with my order",
  "intent": "general",
  "sender": 0
}
```

Memory after:
- `last_user_message = "I need help with my order"`
- `last_intent = "general"`
- `turn_count = 1`
- `state = "active"`
- `last_reply = "message_received"`

## Human escalation
Input:
```json
{
  "action": "message",
  "session_id": "sess-1001",
  "user_id": "user-42",
  "channel": "web",
  "content": "I want a human",
  "intent": "human",
  "sender": 0
}
```

Memory after:
- `state = "waiting_for_human"`
- `escalation_requested = 1`
- `last_reply = "handoff_requested"`

## Summary
Input:
```json
{
  "action": "summary",
  "session_id": "sess-1001",
  "user_id": "user-42",
  "channel": "web",
  "content": "",
  "intent": "",
  "sender": 0
}
```

Memory gets:
```text
session=sess-1001 user=user-42 channel=web state=waiting_for_human turns=1 escalation=1
```

---

# Why this is a good AgeRun example

It shows the core AgeRun pattern well:

- **long-lived stateful entity**
- **event-driven updates**
- **memory as durable session state**
- **simple workflow transitions**
- **message-based interface**

That maps well to:
- support chat
- onboarding flows
- ticket conversations
- conversational assistants
- human handoff pipelines

---

# What it does not do yet

The current method is intentionally small. It does **not** yet:

- keep full transcript history
- classify intents itself
- send to a real human queue agent
- manage timeouts/SLA timers
- persist only selected fields separately
- support multiple channels with different policies

It’s a compact MVP example.

---

# How bootstrap uses it

The updated `bootstrap` method demonstrates it automatically by:

1. spawning a `chat-session` agent
2. sending `start`
3. sending one `message`
4. sending `summary`

So the repo now includes both:
- the reusable session method
- and a runnable startup demo that exercises it

If you want, I can next walk through the **actual `.method` line by line**.
