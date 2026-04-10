# Bootstrap Method

## Overview

The `bootstrap` method now demonstrates a concrete startup flow for AgeRun by spawning a
`chat-session` agent and driving a short sample conversation automatically.

This turns bootstrap from a placeholder into a runnable demo of a real-world use case:
a stateful chat/session backend where one agent owns one conversation.

## Current Implementation

On `"__boot__"`, the bootstrap agent:
- spawns a `chat-session` agent
- sends a `start` message for a demo session
- sends a `message` event representing one user turn
- sends a `summary` request so the chat agent generates a session summary
- stores demo metadata in bootstrap memory
- emits a status message to agent `0`

## Demo Conversation

The bootstrap flow drives this sample conversation state:
- `session_id = "demo-session"`
- `user_id = "demo-user"`
- `channel = "web"`
- one user turn with content `"Need_help"`
- resulting summary:
  `session=demo-session user=demo-user channel=web state=active turns=1 escalation=0`

## Method Code

```agerun
memory.is_boot := if(message = "__boot__", 1, 0)
memory.method_name := if(memory.is_boot = 1, "chat-session", 0)
memory.method_version := "1.0.0"
memory.chat_session_id := spawn(memory.method_name, memory.method_version, context)
memory.start_message := parse("sender={sender} action={action} session_id={session_id} user_id={user_id} channel={channel} content={content} intent={intent}", "sender=0 action=start session_id=demo-session user_id=demo-user channel=web content=none intent=none")
send(memory.chat_session_id, memory.start_message)
memory.user_message := parse("sender={sender} action={action} session_id={session_id} user_id={user_id} channel={channel} content={content} intent={intent}", "sender=0 action=message session_id=demo-session user_id=demo-user channel=web content=Need_help intent=general")
send(memory.chat_session_id, memory.user_message)
memory.summary_message := parse("sender={sender} action={action} session_id={session_id} user_id={user_id} channel={channel} content={content} intent={intent}", "sender=0 action=summary session_id=demo-session user_id=demo-user channel=web content=none intent=none")
send(memory.chat_session_id, memory.summary_message)
memory.demo_status := if(memory.chat_session_id > 0, "Chat session demo queued", "Bootstrap failed")
send(0, memory.demo_status)
```

## Testing

The method is tested in `methods/bootstrap_tests.c`.

The test verifies that bootstrap:
- spawns a `chat-session` agent on boot
- queues `start`, `message`, and `summary` messages
- drives the chat agent to a valid final state
- produces the expected session summary
- leaves no extra messages pending

## Usage

Load `bootstrap` and `chat-session`, create the bootstrap agent, then send `"__boot__"`.
Processing the next four messages runs the complete demo:
1. bootstrap handles `"__boot__"`
2. chat-session handles `start`
3. chat-session handles `message`
4. chat-session handles `summary`
