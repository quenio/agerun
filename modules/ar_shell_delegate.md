# ar_shell_delegate

## Purpose

`ar_shell_delegate` is the session-specific transport helper for the shell feature. It wraps each
accepted terminal input line into the required `{text = ...}` envelope, forwards that envelope to
the receiving agent for the active shell session, and reports handoff acknowledgement to the shell
output stream.

## Public API

- `ar_shell_delegate__create()` creates a delegate bound to one `ar_shell_session_t` and one
  receiving-agent ID.
- `ar_shell_delegate__destroy()` destroys the delegate.
- `ar_shell_delegate__get_agent_id()` returns the configured receiving-agent ID.
- `ar_shell_delegate__create_input_envelope()` builds the input map expected by the shell method.
- `ar_shell_delegate__forward_input()` sends the wrapped input envelope into the wrapped
  `ar_system_t` agency.
- `ar_shell_delegate__process_input_stream()` reads accepted input lines until EOF, trims trailing
  line endings, forwards each wrapped envelope, and renders handoff acknowledgement in normal or
  verbose mode.

## Ownership Notes

- `ar_shell_delegate__create()` returns an owned `ar_shell_delegate_t*`.
- `ar_shell_delegate__create_input_envelope()` returns an owned `ar_data_t*` map.
- `ar_shell_delegate__forward_input()` transfers envelope ownership into `ar_agency__send_to_agent()` on success.
- `ar_shell_delegate__process_input_stream()` borrows the input/output streams and does not own
  them.

## Current Scope

The current implementation covers envelope creation, repeated stdin reading until EOF, and
mode-sensitive handoff acknowledgement rendering. Session callback routing for returned messages and
asynchronous reply output rendering remain future work.
