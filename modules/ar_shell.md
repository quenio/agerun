# ar_shell

## Purpose

`ar_shell` provides the dedicated shell module behind the `arsh` executable scaffold.
It wraps an `ar_system_t`, manages shell sessions, and starts a receiving agent that runs the
built-in `shell` method.

## Public API

- `ar_shell__create()` creates an instantiable shell wrapper.
- `ar_shell__destroy()` destroys the shell, its sessions, and the wrapped `ar_system_t`.
- `ar_shell__get_default_mode()` reports the shell default acknowledgement mode.
- `ar_shell__get_system()` exposes the wrapped `ar_system_t` as a borrowed reference.
- `ar_shell__get_session_count()` reports how many sessions are currently managed.
- `ar_shell__start_session()` creates one `ar_shell_session_t`, registers the built-in shell
  method through `ar_methodology__register_shell_method()`, and creates the receiving agent.
- `ar_shell__main()` provides the scaffold executable entry point used by the Makefile `run-shell`
  target.

## Ownership Notes

- `ar_shell__create()` returns an owned `ar_shell_t*`.
- `ar_shell__get_system()` returns a borrowed `ar_system_t*`.
- `ar_shell__start_session()` returns a borrowed `ar_shell_session_t*` that remains owned by the
  shell instance.

## Current Scope

The current implementation covers shell creation, session tracking, built-in shell-method
registration, receiving-agent startup, startup-mode propagation, and repeated stdin handoff through
`ar_shell_delegate__process_input_stream()`. Asynchronous reply handling and shutdown-time reply
discard remain future work.
