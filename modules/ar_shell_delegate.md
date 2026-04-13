# ar_shell_delegate

## Purpose

`ar_shell_delegate` is the session-specific transport helper for the shell feature. It wraps one
terminal input line into the required `{text = ...}` envelope and forwards that envelope to the
receiving agent for the active shell session.

## Public API

- `ar_shell_delegate__create()` creates a delegate bound to one `ar_shell_session_t` and one
  receiving-agent ID.
- `ar_shell_delegate__destroy()` destroys the delegate.
- `ar_shell_delegate__get_agent_id()` returns the configured receiving-agent ID.
- `ar_shell_delegate__create_input_envelope()` builds the input map expected by the shell method.
- `ar_shell_delegate__forward_input()` sends the wrapped input envelope into the wrapped
  `ar_system_t` agency.

## Ownership Notes

- `ar_shell_delegate__create()` returns an owned `ar_shell_delegate_t*`.
- `ar_shell_delegate__create_input_envelope()` returns an owned `ar_data_t*` map.
- `ar_shell_delegate__forward_input()` transfers envelope ownership into `ar_agency__send_to_agent()` on success.

## Current Scope

The current implementation covers envelope creation and forwarding only. Session callback routing
for returned messages and output rendering remain future work.
