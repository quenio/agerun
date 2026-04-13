# ar_shell_session

## Purpose

`ar_shell_session` owns the state for one shell interaction. It stores the session identifier,
acknowledgement mode, receiving-agent ID, and the owned shell-session memory map.

## Public API

- `ar_shell_session__create()` creates one shell session with an owned memory map.
- `ar_shell_session__destroy()` destroys the session and its memory map.
- `ar_shell_session__get_session_id()` returns the numeric session identifier.
- `ar_shell_session__get_mode()` returns the session acknowledgement mode.
- `ar_shell_session__get_agent_id()` returns the receiving-agent ID.
- `ar_shell_session__activate()` marks the session active and records the receiving-agent ID.
- `ar_shell_session__is_active()` reports whether the session is active.
- `ar_shell_session__get_memory()` returns the owned session memory map as a borrowed reference.

## Ownership Notes

- `ar_shell_session__create()` returns an owned `ar_shell_session_t*`.
- `ar_shell_session__get_memory()` returns a borrowed `ar_data_t*` reference owned by the session.

## Current Scope

The current implementation provides startup state ownership only. Message-based session mediation,
acknowledgement reporting, output rendering, and shutdown-state transitions remain future work.
