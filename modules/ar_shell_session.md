# ar_shell_session

## Purpose

`ar_shell_session` owns the state for one shell interaction. It stores the session identifier,
acknowledgement mode, receiving-agent ID, the owned shell-session memory map, and the owned
per-session runtime context used when the receiving agent executes.

## Public API

- `ar_shell_session__create()` creates one shell session with an owned memory map.
- `ar_shell_session__destroy()` destroys the session and its memory map.
- `ar_shell_session__get_session_id()` returns the numeric session identifier.
- `ar_shell_session__get_mode()` returns the session acknowledgement mode.
- `ar_shell_session__get_agent_id()` returns the receiving-agent ID.
- `ar_shell_session__activate()` marks the session active and records the receiving-agent ID.
- `ar_shell_session__get_runtime_delegate_id()` derives the negative runtime delegate ID used for
  shell-session mediation.
- `ar_shell_session__create_runtime_delegate()` builds the delegate handler that routes protocol
  messages into the shell session.
- `ar_shell_session__store_value()` stores one value addressed through the `memory...` root.
  The current runtime-delegate path also mirrors stored values back into the receiving-agent memory
  map so later shell lines can reuse session values like `memory.echo_id`.
- `ar_shell_session__load_value()` loads one stored value and returns a protocol-shaped reply map.
- `ar_shell_session__return_loaded_value()` builds the loaded-value reply payload.
- `ar_shell_session__report_operation_failure()` builds the failure reply payload.
- `ar_shell_session__is_active()` reports whether the session is active.
- `ar_shell_session__get_memory()` returns the owned session memory map as a borrowed reference.
- `ar_shell_session__get_context()` returns the owned per-session runtime context as a borrowed
  reference.

## Ownership Notes

- `ar_shell_session__create()` returns an owned `ar_shell_session_t*`.
- `ar_shell_session__store_value()` takes ownership of the provided value only on success.
- `ar_shell_session__load_value()`, `ar_shell_session__return_loaded_value()`, and
  `ar_shell_session__report_operation_failure()` each return owned reply maps.
- `ar_shell_session__get_memory()` returns a borrowed `ar_data_t*` reference owned by the session.
- `ar_shell_session__get_context()` returns a borrowed `ar_data_t*` reference owned by the session.

## Current Scope

The current implementation provides startup state ownership plus message-shaped store/load/failure
mediation for the shell session memory map and a runtime delegate that processes store/load
protocol messages on behalf of the shell session. Stored values are also mirrored into the
receiving-agent memory map to support the current shell-method reuse path for later lines such as
assigned send after assigned spawn. Acknowledgement reporting, output rendering, and shutdown-state
transitions remain future work.
