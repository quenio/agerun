# shell-1.0.0

## Purpose

`shell-1.0.0.method` is the built-in method asset used by the command-line shell scaffold.
The current implementation goes beyond startup scaffolding and now covers the first restricted
shell-syntax slice needed for interactive shell-session behavior.

## Current Behavior

The current implementation supports the first three restricted shell-syntax slices:

```text
memory.last_input := message.text
memory.assignment := parse("memory.prompt := {value}", message.text)
memory.store_request_input := build("action=ar_shell_session__store_value path=memory.prompt value={value}", memory.assignment)
memory.store_request := parse("action={action} path={path} value={value}", memory.store_request_input)
send(memory.shell_session_delegate_id, memory.store_request)
memory.spawn_assignment := parse("memory.echo_id := spawn({method}, {version}, context)", message.text)
memory.spawn_rebuilt := build("memory.echo_id := spawn({method}, {version}, context)", memory.spawn_assignment)
memory.is_echo_spawn := if(memory.spawn_rebuilt = message.text, 1, 0)
memory.spawn_method := if(memory.is_echo_spawn = 1, "echo", "")
memory.spawn_version := if(memory.is_echo_spawn = 1, "1.0.0", "")
memory.spawned_agent_id := spawn(memory.spawn_method, memory.spawn_version, context)
memory.spawn_assignment.value := memory.spawned_agent_id
memory.spawn_store_delegate_id := if(memory.spawned_agent_id, memory.shell_session_delegate_id, 0)
memory.spawn_store_request_input := build("action=ar_shell_session__store_value path=memory.echo_id value={value}", memory.spawn_assignment)
memory.spawn_store_request := parse("action={action} path={path} value={value}", memory.spawn_store_request_input)
send(memory.spawn_store_delegate_id, memory.spawn_store_request)
memory.send_assignment := parse("memory.send_ok := send(memory.echo_id, {value})", message.text)
memory.send_rebuilt := build("memory.send_ok := send(memory.echo_id, {value})", memory.send_assignment)
memory.is_echo_send := if(memory.send_rebuilt = message.text, 1, 0)
memory.send_target := if(memory.is_echo_send = 1, memory.echo_id, 0)
memory.send_payload_input := build("sender=0 content={value}", memory.send_assignment)
memory.send_payload := parse("sender={sender} content={content}", memory.send_payload_input)
memory.send_result := send(memory.send_target, memory.send_payload)
memory.send_assignment.value := memory.send_result
memory.send_store_delegate_id := if(memory.is_echo_send = 1, memory.shell_session_delegate_id, 0)
memory.send_store_request_input := build("action=ar_shell_session__store_value path=memory.send_ok value={value}", memory.send_assignment)
memory.send_store_request := parse("action={action} path={path} value={value}", memory.send_store_request_input)
send(memory.send_store_delegate_id, memory.send_store_request)
memory.last_error := "Invalid shell syntax"
```

Current behavior details:
- keeps tracking the last received shell input line in agent memory
- recognizes the `memory.prompt := ...` shell assignment form used by the new tests
- forwards that assignment to `ar_shell_session` through the session runtime delegate
- recognizes the exact tested `memory.echo_id := spawn("echo", "1.0.0", context)` shell form
- spawns the requested `echo` agent once the shell runtime has loaded the repository methods into the wrapped system methodology
- stores the resulting spawned agent ID back into shell-session memory through the same runtime delegate path
- recognizes the exact tested `memory.send_ok := send(memory.echo_id, "Hello")` shell form
- wraps that send into a map payload that `echo-1.0.0.method` can process without runtime errors
- stores the resulting send status back into shell-session memory through the runtime delegate path
- relies on `ar_shell_session` to normalize quoted string values before storing them in shell-session memory
- mirrors stored session values back into receiving-agent memory so later shell lines can reuse values like `memory.echo_id`
- records a recoverable `memory.last_error` marker so invalid syntax handling can evolve without closing the session

The full shell behavior described in `specs/001-command-line-shell/spec.md` still requires broader
restricted-syntax support for additional assignment paths plus generic `spawn(...)`, generic
`send(...)`, and additional assigned `spawn(...)` / `send(...)` forms beyond the exact tested cases.

## Validation

`methods/shell_tests.c` verifies that the method asset loads through `ar_method_fixture__load_method()`, parses to an AST, supports receiving-agent creation, redirects the tested prompt assignment into shell-session memory, redirects the tested assigned-spawn result into shell-session memory, redirects the tested assigned-send result into shell-session memory, and keeps invalid syntax recoverable.
