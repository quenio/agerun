# ar_shell_delegate

## Purpose

`ar_shell_delegate` is the session-specific transport helper for the shell feature. It wraps each
accepted terminal input line into the required `{text = ...}` envelope, forwards that envelope to
the receiving agent for the active shell session, binds the session output stream used for reply
rendering, and reports handoff acknowledgement to the shell output stream.

## Public API

- `ar_shell_delegate__create()` creates a delegate bound to one `ar_shell_session_t` and one
  receiving-agent ID.
- `ar_shell_delegate__destroy()` destroys the delegate.
- `ar_shell_delegate__get_agent_id()` returns the configured receiving-agent ID.
- `ar_shell_delegate__create_input_envelope()` builds the input map expected by the shell method.
- `ar_shell_delegate__forward_input()` sends the wrapped input envelope into the wrapped
  `ar_system_t` agency.
- `ar_shell_delegate__process_input_stream()` reads accepted input lines until EOF, trims trailing
  line endings, forwards each wrapped envelope, recognizes the built-in `agents` / `list agents`
  inspection command, renders handoff acknowledgement in normal or verbose mode, prefixes shell-
  emitted lines with `OUT:` when transcript labels are enabled, prints an `IN:` prompt before each
  read in labeled sessions, processes runtime work immediately for real shell sessions, and closes
  the active shell session on EOF / Ctrl-D.

## Ownership Notes

- `ar_shell_delegate__create()` returns an owned `ar_shell_delegate_t*`.
- `ar_shell_delegate__create_input_envelope()` returns an owned `ar_data_t*` map.
- `ar_shell_delegate__forward_input()` transfers envelope ownership into `ar_agency__send_to_agent()` on success.
- `ar_shell_delegate__process_input_stream()` borrows the input/output streams and does not own
  them.

## Current Scope

The current implementation covers envelope creation, repeated stdin reading until EOF,
mode-sensitive handoff acknowledgement rendering, `IN:` prompt rendering plus `OUT:` output
prefixes when transcript labels are enabled, output-stream binding for the owning shell session,
immediate runtime processing for active shell sessions created by `ar_shell`, direct rendering for
the built-in `agents` / `list agents` inspection command, and EOF / Ctrl-D shutdown that closes
the session and destroys the receiving agent. Returned runtime replies are rendered by
`ar_shell_session` through the bound output stream using only the runtime sender ID.
