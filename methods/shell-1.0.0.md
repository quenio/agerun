# shell-1.0.0

## Purpose

`shell-1.0.0.method` is the built-in method asset used by the command-line shell scaffold.
The current implementation goes beyond startup scaffolding and now covers the first restricted
shell-syntax slice needed for interactive shell-session behavior.

## Current Behavior

The current implementation now supports the current User Story 2 restricted shell-syntax set used by the tests:

```text
memory.last_input := message.text
memory.assigned_spawn := parse("memory.{path} := spawn({method}, {version}, context)", message.text)
memory.assigned_spawn_rebuilt := build("memory.{path} := spawn({method}, {version}, context)", memory.assigned_spawn)
memory.is_assigned_spawn := if(memory.assigned_spawn_rebuilt = message.text, 1, 0)
memory.assigned_spawn_result := spawn(memory.assigned_spawn.method, memory.assigned_spawn.version, context)
...
memory.assigned_send := parse("memory.{path} := send(memory.echo_id, {value})", message.text)
memory.assigned_send_rebuilt := build("memory.{path} := send(memory.echo_id, {value})", memory.assigned_send)
memory.is_assigned_send := if(memory.assigned_send_rebuilt = message.text, 1, 0)
memory.assigned_send_result := send(memory.echo_id, <wrapped payload>)
...
memory.plain_spawn := parse("spawn({method}, {version}, context)", message.text)
memory.plain_spawn_rebuilt := build("spawn({method}, {version}, context)", memory.plain_spawn)
memory.is_plain_spawn := if(memory.plain_spawn_rebuilt = message.text, 1, 0)
memory.plain_spawn_result := spawn(memory.plain_spawn.method, memory.plain_spawn.version, context)
memory.plain_send_prompt := parse("send(memory.echo_id, memory.prompt)", message.text)
memory.plain_send_prompt_rebuilt := build("send(memory.echo_id, memory.prompt)", memory.plain_send_prompt)
memory.is_plain_send_prompt := if(memory.plain_send_prompt_rebuilt = message.text, 1, 0)
memory.plain_send_result := send(memory.echo_id, <wrapped payload built from memory.prompt>)
memory.assignment := parse("memory.{path} := {value}", message.text)
memory.assignment_rebuilt := build("memory.{path} := {value}", memory.assignment)
memory.is_assignment := memory.is_assignment_candidate - memory.is_assigned_spawn - memory.is_assigned_send
...
memory.last_error := if(memory.matched_forms, "", "Invalid shell syntax")
```

Current behavior details:
- keeps tracking the last received shell input line in agent memory
- supports top-level `memory.{path} := {value}` storage for the current shell-session memory map
- supports top-level `memory.{path} := spawn({method}, {version}, context)` and stores the spawned agent ID back into shell-session memory
- supports top-level `memory.{path} := send(memory.echo_id, {value})` and stores the send result back into shell-session memory
- supports plain `spawn({method}, {version}, context)` without storing the result
- supports the tested plain `send(memory.echo_id, memory.prompt)` form by resolving the stored session value before queuing the runtime payload
- wraps shell-driven sends into the map shape that `echo-1.0.0.method` expects
- relies on `ar_shell_session` to normalize quoted string values before storing them in shell-session memory
- mirrors stored session values back into receiving-agent memory so later shell lines can reuse values like `memory.echo_id` and `memory.prompt`
- clears `memory.last_error` on recognized shell syntax and preserves `"Invalid shell syntax"` only for unmatched lines

User Story 3 reply-routing behavior is still pending: the current send payloads are sufficient for
US2 delivery/assignment validation, but asynchronous reply rendering and EOF-discard behavior are
implemented in the next phase.

## Validation

`methods/shell_tests.c` verifies that the method asset loads through `ar_method_fixture__load_method()`, parses to an AST, supports receiving-agent creation, redirects tested session assignments into shell-session memory, supports plain spawn without assignment, supports plain send using a stored session value, redirects tested assigned-spawn and assigned-send results into shell-session memory, and keeps invalid syntax recoverable.
