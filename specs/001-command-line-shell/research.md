# Research: Command-Line Shell

## Decision 1: Use a session-specific shell delegate, not a generic stdio delegate

- **Decision**: Add a new `ar_shell_delegate` module that is created per shell session, owns that
  session's stdin/stdout transport, wraps each accepted input string into the required envelope map
  (`text = input string`), unwraps returned output envelopes back into terminal strings, and holds
  the receiving-agent target for the session.
- **Rationale**: This follows separation of concerns without introducing a reusable generic stdio
  abstraction the feature does not need. The design decision most likely to change here is the
  shell-session transport contract, not generic terminal I/O across the whole system.
- **Alternatives considered**:
  - Add a generic `ar_stdio_delegate` module: rejected because the feature needs shell-session-
    specific behavior, not a broad stdio abstraction.
  - Put stdin/stdout logic directly into `ar_executable.c`: rejected because it would mix transport,
    shell-session lifecycle, and startup orchestration.

## Decision 2: Keep shell behavior in a built-in `shell` method executed by a receiving agent

- **Decision**: Create a new built-in method file `methods/shell-1.0.0.method` and a dedicated
  receiving agent that starts from it automatically when `arsh` starts.
- **Rationale**: The specification attributes launch, send, and assignment capabilities to the
  built-in shell method. Keeping behavior in a real method preserves AgeRun's message-driven model,
  avoids delegate-side shell semantics, and makes the shell behavior testable and documentable like
  other methods.
- **Alternatives considered**:
  - Create a built-in `arsh` method: rejected because `arsh` is the executable command name, while
    the receiving agent should run the `shell` method.
  - Hard-code shell commands in C only: rejected because it would bypass the method-level behavior
    contract required by the spec.

## Decision 3: Add a non-instantiable `ar_shell` module that owns shell sessions without replacing `ar_shell_session`

- **Decision**: Add a dedicated `ar_shell` module as a non-instantiable C module that creates,
  holds, and destroys shell session instances plus their memory maps. The `arsh` entrypoint calls
  into this module so the entrypoint stays thin and the actual shell lifecycle/orchestration logic
  can be unit tested directly.
- **Rationale**: The feature needs an actual shell module in addition to the built-in `shell`
  method and the instantiable `ar_shell_session` runtime module. Putting shell-session ownership in
  `ar_shell` isolates shell startup/session orchestration from both the CLI entrypoint and the
  delegate transport layer.
- **Alternatives considered**:
  - Let `ar_shell_session` replace `ar_shell`: rejected because the feature still needs a thin
    entrypoint and a directly unit-testable shell orchestration module.
  - Keep shell state only in delegate-owned ad hoc structs: rejected because it blurs transport and
    shell-session lifecycle/state responsibilities.

## Decision 4: Retain `ar_shell_session` as an instantiable runtime module, but do not let it directly handle the session map

- **Decision**: Keep `ar_shell_session` as an instantiable runtime-facing module that mediates
  shell-session operations for the built-in `shell` method through messages, while the actual shell
  session memory map remains owned by the shell session held by `ar_shell`.
- **Rationale**: This preserves the original requirement that a shell session module exists within
  the runtime, while also honoring the clarified ownership boundary that the session map should not
  be directly handled by the session module itself.
- **Alternatives considered**:
  - Remove `ar_shell_session`: rejected because the architecture still requires an instantiable
    runtime shell-session boundary.
  - Move the session map directly into `ar_shell_session`: rejected because the user explicitly
    ruled out direct session-map handling in the session module.

## Decision 5: Keep input/output envelope handling in the session delegate

- **Decision**: The session-specific shell delegate wraps terminal input strings into input
  envelopes before forwarding them to the receiving agent and unwraps output envelopes received back
  from the runtime into displayed terminal strings with sender attribution.
- **Rationale**: This preserves the transport-only boundary: the delegate manages shell I/O
  packaging, while the receiving agent, shell method, and shell/session modules manage shell
  semantics and state routing.
- **Alternatives considered**:
  - Let the receiving agent print/read directly: rejected because it conflicts with delegate-
    mediated I/O and system-managed message flow.
  - Have `ar_shell` or `ar_shell_session` perform terminal I/O directly: rejected because it would
    mix shell-session lifecycle/state ownership with transport responsibilities.

## Decision 6: Restrict shell input to a canonical one-line AgeRun subset

- **Decision**: Support a bounded one-line syntax subset: `spawn(...)`, `send(...)`,
  `memory... := ...`, `memory... := spawn(...)`, and `memory... := send(...)`.
- **Rationale**: This satisfies the spec while staying implementable within AgeRun's existing
  method-language constraints. The subset is intentionally narrow and avoids inventing a broader
  shell language.
- **Alternatives considered**:
  - Invent a separate shell-only command language: rejected because the clarified spec prefers
    existing AgeRun instruction syntax.
  - Support the full AgeRun instruction grammar immediately: rejected as too broad for a first
    shell release and harder to validate.

## Decision 7: Redirect shell-mode `memory... := ...` to shell session state owned by `ar_shell`, mediated by `ar_shell_session`

- **Decision**: In shell mode, `memory... := ...` writes to the shell session memory map owned by
  `ar_shell` rather than the receiving agent's memory map, while `ar_shell_session` mediates the
  runtime-facing access path.
- **Rationale**: This preserves syntax consistency with method definitions while honoring both the
  encapsulation requirement and the clarified boundary that the session module does not directly
  handle the map itself.
- **Alternatives considered**:
  - Add a new root like `session... := ...`: rejected because the spec deliberately reuses existing
    `memory... := ...` syntax.
  - Let `ar_shell_session` own the map directly: rejected by the clarified requirement.

## Decision 8: Keep shell/session coordination message-based via a shell session protocol

- **Decision**: Define a message protocol between the `shell` method, `ar_shell_session`, the shell
  session owned by `ar_shell`, and the session-specific delegate for value lookup, assignment,
  acknowledgement, and reply-envelope flow. Capture that contract in
  [`contracts/shell-session-protocol.md`](./contracts/shell-session-protocol.md).
- **Rationale**: The spec requires shell-side state and the built-in shell method to exchange
  information via messages rather than hidden shared state. A documented protocol keeps the
  boundary explicit and testable and makes the delegate's wrap/unwrap role clear.
- **Alternatives considered**:
  - Share direct pointers or mutable references between the method runtime, shell module, session
    module, and delegate: rejected because it violates the clarified encapsulation boundary.

## Decision 9: Add `arsh` without removing the existing executable behavior

- **Decision**: Extend the executable/build path so `arsh` becomes the user-facing shell command
  while keeping the current demo-oriented executable flow available.
- **Rationale**: The spec fixes the public command name to `arsh`, but the repository already has
  useful demo behavior in `ar_executable`. Preserving that path reduces regression risk and keeps
  shell work additive.
- **Alternatives considered**:
  - Replace the existing executable entirely with shell behavior: rejected because the spec only
    requires an additive command.
  - Build a totally separate entry module with duplicated startup logic: rejected as unnecessary
    duplication until shared startup logic proves insufficient.
