# Agent FIFO Queue and Continuation Interleaving

## Learning

AgeRun agents are not re-entrant in the current runtime. Each agent owns a FIFO message queue, and
the system processes one dequeued message by executing one method invocation to completion before
processing another message.

Self-sent continuation messages are still ordinary queued messages. If other messages are already in
the same agent's queue, a continuation sent during the current invocation is appended after those
older messages. This can create split-step interleaving, but not simultaneous execution.

## Importance

Review findings and bug reports must distinguish true concurrency from FIFO continuation ordering.
Treating every split-step issue as a re-entrancy bug leads to unnecessary defensive state machines.
Ignoring FIFO interleaving is also unsafe when a method implements one logical operation across
multiple self-sent continuation messages.

## Example

The runtime path establishes the invariant:

1. `ar_agent_t` stores one `own_message_queue` per agent
   ([modules/ar_agent.c](../modules/ar_agent.c)).
2. `ar_agent__send()` appends messages with `ar_list__add_last()`.
3. `ar_agent__get_message()` dequeues with `ar_list__remove_first()`.
4. `ar_system__process_next_message()` removes one message and calls
   `ar_interpreter__execute_method()` once for that message
   ([modules/ar_system.c](../modules/ar_system.c)).
5. `send(...)` queues the message through `ar_agency__send_to_agent()`; it does not execute the
   target agent inline ([modules/ar_send_instruction_evaluator.zig](../modules/ar_send_instruction_evaluator.zig)).

This means a method body cannot be interrupted by another invocation of the same agent.

However, FIFO ordering still matters:

```text
Initial queue for agent C:
[conversation_message A, conversation_message B]

Processing A sends conversation_select_recipients to C.

Queue after A completes:
[conversation_message B, conversation_select_recipients for A]
```

`conversation_message B` can run before the continuation for A. That is not re-entrancy; it is
ordinary FIFO ordering across multiple method invocations.

## Generalization

Use this triage rule for coordination-method review comments:

- Invalid if the finding requires simultaneous execution or re-entrant mutation of one agent's
  memory.
- Potentially valid if the finding depends on already queued messages running before a self-sent
  continuation.
- Hardening-only if the finding depends on a self-send to `memory.self` failing while the current
  agent is alive and `memory.self` remains agency-managed.

## Implementation

When a coordination method uses self-sent continuations:

1. Record enough session/trace state before sending the continuation to recognize stale or
   out-of-order continuation messages.
2. Decide whether public requests arriving while a continuation is pending should be ignored,
   deferred, or failed.
3. Avoid assuming a self-continuation is the next message the agent will process.
4. Avoid designing for same-agent re-entrancy unless the runtime architecture changes.

When reviewing a reported bug:

```text
Question: Does this require two method invocations to run at the same time?
Answer: If yes, it is invalid for the current runtime.

Question: Does this require message B to be queued before A's continuation is sent?
Answer: If yes, it may be valid FIFO interleaving.
```

## Related Patterns

- [Message Processing Loop Pattern](message-processing-loop-pattern.md)
- [Message Ownership Flow](message-ownership-flow.md)
- [System Message Flow Architecture](system-message-flow-architecture.md)
