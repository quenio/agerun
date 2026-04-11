# Research: Command-Line Shell

## Decision 1: Use a dedicated stdio delegate module for terminal transport

- **Decision**: Add a new `ar_stdio_delegate` module that owns stdin/stdout interaction and wraps
  each accepted input line into the required envelope map (`text = input string`).
- **Rationale**: This follows separation of concerns and keeps terminal transport independent from
  shell semantics. It also fits AgeRun's delegate architecture, where external I/O is mediated
  through delegates instead of direct agent console access.
- **Alternatives considered**:
  - Put stdin/stdout logic directly into `ar_executable.c` only: simpler initially, but mixes CLI
    transport with shell workflow logic.
  - Let the receiving agent print/read directly: conflicts with delegate-mediated I/O.

## Decision 2: Model shell session state as its own instantiable module

- **Decision**: Add a new `ar_shell_session` module with an opaque instance and its own memory map.
  It is session-scoped, separate from the receiving agent's memory, and exchanges data with the
  shell method only through messages.
- **Rationale**: The specification now requires shell-session values to live outside the receiving
  agent for encapsulation. A dedicated module localizes shell state, keeps the receiving agent's
  memory free of shell-session storage, and supports future changes to session persistence or state
  inspection without changing delegate transport.
- **Alternatives considered**:
  - Store shell variables in the receiving agent's memory: rejected by the spec and user feedback.
  - Keep shell state in delegate-owned ad hoc structs: rejected because it blurs transport and
    session-state responsibilities.

## Decision 3: Implement shell behavior in a built-in `arsh` method executed by a receiving agent

- **Decision**: Create a new built-in method file `methods/arsh-1.0.0.method` and a dedicated
  receiving agent that starts from it automatically when `arsh` starts.
- **Rationale**: The spec explicitly attributes launch/send/assignment capabilities to the built-in
  shell method. Keeping behavior in a method preserves AgeRun's message-driven model and provides a
  real method contract that can be tested and documented like other runtime methods.
- **Alternatives considered**:
  - Hard-code shell commands in C only: rejected because it would bypass the method-level behavior
    contract now required by the spec.
  - Reuse `bootstrap` or `chat-session`: rejected because shell behavior is a distinct concern.

## Decision 4: Restrict shell input to a canonical one-line AgeRun subset

- **Decision**: Support a bounded one-line syntax subset: `spawn(...)`, `send(...)`,
  `memory... := ...`, `memory... := spawn(...)`, and `memory... := send(...)`.
- **Rationale**: This satisfies the spec while staying implementable within AgeRun's existing method
  language constraints. The subset is intentionally narrow and avoids inventing a broader shell
  language. Because function calls cannot be nested and map literals are not supported, the shell
  method and session module will need multi-step internal message exchanges for some cases, but the
  user-facing syntax remains small and stable.
- **Alternatives considered**:
  - Invent a separate shell-only command language: rejected because the clarified spec now prefers
    existing AgeRun instruction syntax.
  - Support the full AgeRun instruction grammar immediately: rejected as too broad for a first
    shell release and harder to validate.

## Decision 5: Redirect shell-mode `memory... := ...` to shell session state, not agent memory

- **Decision**: In shell mode, `memory... := ...` writes to the `ar_shell_session` memory map rather
  than the receiving agent's memory map.
- **Rationale**: This preserves syntax consistency with method definitions while still honoring the
  encapsulation requirement. It also keeps shell session variables stable across multiple shell
  interactions without exposing or overloading the receiving agent's own runtime memory.
- **Alternatives considered**:
  - Add a new root like `session... := ...`: rejected because the current spec deliberately reuses
    existing `memory... := ...` syntax.
  - Delay assignment support: rejected because the shell would be too limited to use.

## Decision 6: Keep shell/session coordination message-based via a shell session protocol

- **Decision**: Define a message protocol between the shell method and `ar_shell_session` for value
  lookup, assignment, acknowledgement, and reply routing. Capture this in
  [`contracts/shell-session-protocol.md`](./contracts/shell-session-protocol.md).
- **Rationale**: The spec requires the shell session module and built-in shell method to exchange
  information only via messages. A documented protocol keeps that boundary explicit and testable.
- **Alternatives considered**:
  - Share direct pointers or mutable references between the method runtime and session module:
    rejected because it violates the clarified encapsulation boundary.

## Decision 7: Add `arsh` without removing the existing executable behavior

- **Decision**: Extend the executable/build path so `arsh` becomes the user-facing shell command
  while keeping the current demo-oriented executable flow available.
- **Rationale**: The spec now fixes the public command name to `arsh`, but the repository already
  has useful demo behavior in `ar_executable`. Preserving that path reduces regression risk and
  keeps shell work additive.
- **Alternatives considered**:
  - Replace the existing executable entirely with shell behavior: rejected because the spec only
    requires an additive command.
  - Build a totally separate entry module with duplicated startup logic: rejected as unnecessary
    duplication until shared startup logic proves insufficient.
