# ar_shell

## Purpose

`ar_shell` provides the dedicated shell module behind the `arsh` executable scaffold.
It wraps an `ar_system_t`, manages shell sessions, and starts a receiving agent that runs the
built-in `shell` method.

## Public API

- `ar_shell__create()` creates an instantiable shell wrapper and eagerly loads repository method
  assets into the wrapped system methodology when available.
- `ar_shell__destroy()` destroys the shell, its sessions, and the wrapped `ar_system_t`.
- `ar_shell__get_default_mode()` reports the shell default acknowledgement mode.
- `ar_shell__get_system()` exposes the wrapped `ar_system_t` as a borrowed reference.
- `ar_shell__get_session_count()` reports how many sessions are currently managed.
- `ar_shell__start_session()` creates one `ar_shell_session_t`, registers the built-in shell
  method through `ar_methodology__register_shell_method()`, registers the shell-session runtime
  delegate, stores that delegate ID on the receiving agent, and creates the receiving agent.
- `ar_shell__main()` provides the scaffold executable entry point used by the Makefile `run-shell`
  target.

## Ownership Notes

- `ar_shell__create()` returns an owned `ar_shell_t*`.
- `ar_shell__get_system()` returns a borrowed `ar_system_t*`.
- `ar_shell__start_session()` returns a borrowed `ar_shell_session_t*` that remains owned by the
  shell instance.

## Current Scope

The current implementation covers shell creation, repository method loading into the wrapped
system, session tracking, built-in shell-method registration, receiving-agent startup with
per-session execution context, startup-mode propagation, runtime delegate registration for
shell-session mediation, and repeated stdin handoff through `ar_shell_delegate__process_input_stream()`.
For real shell sessions started by `ar_shell`, that loop now also auto-enables `IN:` / `OUT:`
transcript labels when both stdin and stdout are terminals, processes runtime work, renders
returned replies using only the runtime sender ID, supports the built-in `agents` / `list agents`
inspection command for enumerating active agents with method name/version, closes the session on
EOF / Ctrl-D, and destroys the dedicated receiving agent before shell teardown.
