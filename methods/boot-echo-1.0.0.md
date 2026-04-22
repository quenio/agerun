# Boot Echo Method v1.0.0

## Overview

The `boot-echo` method is a boot-safe wrapper around `echo-1.0.0`. It exists for fresh executable
startup, where the runtime always sends the selected boot agent the string `"__boot__"`.

Because `echo-1.0.0` expects a map with `sender` and `content` fields, it cannot be used directly as
an executable boot method. `boot-echo-1.0.0` handles the startup string, spawns an `echo` agent,
and queues a normal echo payload for it.

## Current Implementation

On `"__boot__"`, the `boot-echo` agent:
- selects `echo-1.0.0` as the child method
- spawns the child echo agent
- builds a standard echo payload with `sender=0` and `content="boot_echo_ready"`
- sends that payload to the spawned echo agent
- records whether the boot demo was queued or skipped

## Method Code

```agerun
memory.is_boot := if(message = "__boot__", 1, 0)
memory.method_name := if(memory.is_boot = 1, "echo", "")
memory.method_version := "1.0.0"
memory.sender := 0
memory.content := "boot_echo_ready"
memory.echo_spawn_result := spawn(memory.method_name, memory.method_version, context)
memory.echo_id := if(memory.is_boot = 1, memory.echo_spawn_result, memory.echo_id)
memory.echo_input := build("sender={sender} content={content}", memory)
memory.echo_payload := parse("sender={sender} content={content}", memory.echo_input)
memory.echo_sent := send(memory.echo_id * memory.is_boot, memory.echo_payload)
memory.demo_status := if(memory.is_boot = 1, "Boot echo queued", "Boot echo skipped")
```

## Testing

The method is tested in `methods/boot_echo_tests.c`.

The tests verify that `boot-echo`:
- loads and parses into an AST
- can create an agent instance
- handles the `"__boot__"` startup string safely
- spawns an `echo` child agent
- stores the queued echo payload in memory
- leaves exactly one child echo message to process after the boot step

## Usage

Use `boot-echo` when you want an alternate executable boot path that still demonstrates the `echo`
method without crashing on the raw `"__boot__"` startup message.

```bash
make run-exec BOOT_METHOD=boot-echo-1.0.0
```

This wrapper is intended for fresh startup only. If persisted agents are restored from
`agerun.agency`, the executable skips the override and continues with restored state.
