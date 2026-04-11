# Feature Specification: Command-Line Shell

**Feature Branch**: `001-command-line-shell`
**Created**: 2026-04-10
**Status**: Draft
**Input**: User description: "A command-line shell for AgeRun, where one can launch methods, and send messages to them, and receive back their messages."

## Clarifications

### Session 2026-04-10

- Q: How should replies be routed back to the shell? → A: a session-specific shell delegate over stdio
- Q: Should shell transport and shell interpretation be coupled? → A: no; the delegate transports input/output and the receiving agent interprets messages
- Q: How should the shell handle replies that do not arrive immediately? → A: report delivery first and display replies asynchronously
- Q: Should the shell delegate enforce a special input syntax? → A: no; input is just text and interpretation belongs to the receiving agent
- Q: How should the shell delegate package stdin input? → A: always wrap it in a structured envelope map that initially contains exactly one key-value pair: `text = input string`
- Q: How should shell output be surfaced? → A: the session-specific shell delegate unwraps returned envelope maps into terminal output strings and sender attribution
- Q: How is the receiving agent established for a shell session? → A: the shell session automatically creates a dedicated receiving agent at startup
- Q: What should the automatically created receiving agent start from? → A: one dedicated built-in `shell` method
- Q: What should delivery acknowledgement mean in the shell? → A: in normal mode it means delegate-to-receiving-agent handoff succeeded; in verbose mode the shell may also surface receiving-agent acceptance and requested runtime action outcome
- Q: What should happen to the dedicated receiving agent when the shell exits? → A: destroy it automatically when the shell session ends
- Q: Does the shell need a minimal interpreted syntax? → A: yes; the receiving agent must interpret exactly one input line at a time as a restricted subset of AgeRun instruction syntax: `spawn(...)`, `send(...)`, or assignment
- Q: Where are launch, send, and assignment capabilities implemented? → A: in the built-in `shell` method executed by the session's receiving agent
- Q: What is the user-facing shell command name? → A: `arsh` (AgeRun SHell)
- Q: Where should assignment lines store shell session values? → A: in a shell session owned by a dedicated instantiable shell module, with its own memory map and message-based exchange with the shell method
- Q: How should assignment syntax refer to the shell session memory map? → A: keep existing `memory... := ...` syntax; in shell mode it targets the shell session memory map owned by the shell module
- Q: Should assigned function-call forms be allowed in shell mode? → A: yes; forms such as `memory.x := spawn(...)` and `memory.ok := send(...)` are necessary and allowed

### Session 2026-04-11

- Q: Do we need a generic stdio delegate for this feature? → A: no; we need a session-specific shell delegate that owns one shell session's envelope wrapping/unwrapping responsibilities
- Q: Which method should the shell session's receiving agent run? → A: the dedicated built-in `shell` method, not an `arsh` method
- Q: Does the shell module replace the shell session module? → A: no; both are required
- Q: Where should shell session creation and ownership live? → A: in a dedicated instantiable shell module used by the `arsh` executable; that module creates and holds the shell session instance and remains unit-testable
- Q: What remains the role of the shell session module? → A: it remains an instantiable runtime module that mediates shell-session interactions, but it does not replace the shell module
- Q: Who should directly handle the session map? → A: the session map is owned by the shell session held by the shell module, not directly by the shell session module
- Q: Should `arsh` be implemented inside `ar_executable`? → A: no; `arsh` is a separate executable whose module is `ar_shell`

## User Scenarios & Testing *(mandatory)*

### User Story 1 - Send Terminal Input Into AgeRun (Priority: P1)

A user starts an interactive AgeRun shell, has the runtime create a dedicated receiving agent for
that session automatically from one built-in shell method, types text into the terminal, and has
that input forwarded into the runtime as a structured message for that receiving agent.

**Why this priority**: Without reliable terminal-to-runtime input transport, the shell cannot do
anything useful.

**Independent Test**: Start the shell, enter a line of text, and confirm the shell delegate wraps
that line into the required envelope and forwards it to the configured receiving agent while keeping
the session open.

**Acceptance Scenarios**:

1. **Given** the shell session is running, **When** the session starts, **Then** the runtime
   creates a dedicated receiving agent for that shell session from one built-in shell method before
   normal interaction begins.
2. **Given** the shell session is running, **When** the user enters a line of text, **Then** the
   shell delegate wraps that input into a structured envelope map with `text` set to the exact input
   string and forwards it to the session's receiving agent.
3. **Given** the shell session is running, **When** the user enters another line of text, **Then**
   the shell remains open for repeated input without requiring a restart.

---

### User Story 2 - Use the Built-in Shell Method to Launch and Message Agents (Priority: P2)

A user uses the shell together with a receiving agent that executes the built-in shell method,
which interprets shell-delivered messages one line at a time through a restricted subset of AgeRun
instruction syntax so methods can be launched, runtime agents can be messaged, and session values
can be stored through direct assignment or assigned function-call forms, allowing AgeRun behavior to
be explored interactively.

**Why this priority**: The feature is valuable because users want to launch methods and send
messages through an interactive terminal workflow.

**Independent Test**: Start the shell with a receiving agent running the built-in shell method,
enter one line that the shell method interprets to launch a runtime agent, send a message, or store
a session value, and confirm the runtime accepts the requested operation.

**Acceptance Scenarios**:

1. **Given** a shell session is connected to a receiving agent running the built-in shell method,
   **When** the user enters one line that the shell method interprets as `spawn(...)` or
   `memory.x := spawn(...)`, **Then** the runtime launches the requested agent and the shell session
   remains usable.
2. **Given** a shell session is connected to a receiving agent running the built-in shell method,
   **When** the user enters one line that the shell method interprets as `send(...)` or
   `memory.ok := send(...)`, **Then** the runtime accepts or rejects that requested delivery and the
   result is surfaced back to the shell session, with additional status detail available in verbose
   mode.
3. **Given** a shell session is connected to a receiving agent running the built-in shell method,
   **When** the user enters one line using `memory... := ...` assignment syntax, **Then** the shell
   method interprets that assignment as targeting the shell session memory map owned by the shell
   module, accessed through the shell session module, and the assigned value remains available for
   later shell-driven interactions through message-based exchange.

---

### User Story 3 - Observe Replies in the Terminal Session (Priority: P3)

A user sees messages returned to the shell delegate asynchronously, with enough context to tell what
runtime component sent each reply.

**Why this priority**: A command-line shell is only useful if users can observe what comes back from
runtime interactions.

**Independent Test**: Trigger a runtime reply through a shell-driven interaction, continue the
session, and confirm the returned message later appears in the same terminal session with sender
attribution.

**Acceptance Scenarios**:

1. **Given** the shell session has already forwarded input into the runtime, **When** a runtime
   agent later sends a reply back to the shell delegate, **Then** the shell displays that reply
   asynchronously and identifies the sender.
2. **Given** the shell is still running after later input has already been entered, **When** a late
   reply arrives, **Then** the shell still displays it without losing session continuity.

## Edge Cases *(mandatory)*

- What happens when the shell delegate cannot forward wrapped input to the receiving agent?
- What happens when the receiving agent rejects or cannot interpret the wrapped input message?
- What happens when the user input is empty?
- What happens when a requested launch refers to a method that is not available?
- What happens when a requested message targets an agent that does not exist or has already exited?
- What happens when a reply arrives after newer user input has already been entered?
- What happens when input is accepted for delivery but no reply ever arrives?
- What happens if receiving-agent cleanup fails during shell shutdown?

## Scope Boundaries *(mandatory)*

### In Scope

- An interactive AgeRun shell mode exposed through the `arsh` command, using a session-specific shell delegate over stdio
- Wrapping each line of terminal input into a structured message envelope
- Automatically creating a dedicated receiving agent for each shell session from one built-in shell
  method
- Forwarding that envelope to that receiving agent inside the AgeRun runtime
- Leaving interpretation of the wrapped input to the built-in shell method executed by the
  receiving agent
- Supporting the built-in shell method interpreting one line at a time using a restricted subset of
  AgeRun instruction syntax for launching methods, sending runtime messages, and requesting storage
  of session values in a shell session owned by a dedicated instantiable shell module, with the
  shell session module mediating runtime access to that session memory map
- Displaying asynchronously returned messages sent back to the shell delegate
- Clear error reporting, help, and clean shell exit behavior
- Automatic cleanup of the session-specific receiving agent when the shell exits

### Out of Scope

- A rich or arbitrary shell language enforced by the delegate
- Delegate-side parsing of shell semantics beyond envelope construction
- Mutating a shell agent method definition as the core shell behavior
- Remote multi-user shell access over the network
- Batch scripting, macros, or non-interactive automation workflows
- Graphical interfaces
- Inferring replies by scraping log files or polling agent memory directly

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: The system MUST provide an interactive AgeRun shell mode using a session-specific
  shell delegate over stdio that stays open for repeated input until the user explicitly exits.
- **FR-001a**: The user-facing command that starts this shell mode MUST be named `arsh`.
- **FR-001b**: The `arsh` executable MUST be implemented by the `ar_shell` module rather than by
  `ar_executable`.
- **FR-002**: For each accepted line of terminal input, the shell delegate MUST create a structured
  envelope map before forwarding the input into the runtime.
- **FR-003**: Initially, the shell delegate input envelope MUST contain exactly one key-value pair:
  `text = input string`.
- **FR-004**: The `text` value in the envelope MUST preserve the exact input string entered by the
  user.
- **FR-005**: When a shell session starts, the runtime MUST automatically create a dedicated
  receiving agent for that session.
- **FR-005a**: The automatically created receiving agent MUST start from one dedicated built-in
  `shell` method.
- **FR-006**: The session-specific shell delegate MUST hold that session's receiving-agent target
  and MUST forward each wrapped input envelope to it in the runtime.
- **FR-007**: The shell workflow MUST interpret exactly one input line at a time using a
  restricted subset of existing AgeRun instruction syntax.
- **FR-007a**: The allowed interpreted line forms MUST be limited to `spawn(...)`, `send(...)`,
  assigned function-call forms based on `spawn(...)` or `send(...)`, and assignment forms.
- **FR-007c**: In shell mode, allowed assignment forms MUST keep the existing `memory... := ...`
  syntax.
- **FR-007d**: Allowed assigned function-call forms MUST include `memory... := spawn(...)` and
  `memory... := send(...)`.
- **FR-007b**: The shell delegate MUST NOT interpret that restricted syntax beyond transporting text
  and wrapping it in the required envelope.
- **FR-008**: Interpretation of shell-delivered input MUST be performed by the session's receiving
  agent through the built-in shell method, not by the shell delegate.
- **FR-009**: The built-in shell method MUST support interpreting allowed `spawn(...)` lines and
  allowed `memory... := spawn(...)` lines that launch runtime agents from available methods.
- **FR-010**: The built-in shell method MUST support interpreting allowed `send(...)` lines and
  allowed `memory... := send(...)` lines that cause messages to be sent to runtime agents.
- **FR-010a**: The built-in shell method MUST support interpreting allowed assignment lines that
  store and later reuse session values.
- **FR-010aa**: In shell mode, `memory... := ...` assignment lines MUST target the shell session
  memory map owned by the shell module rather than the receiving agent's memory map.
- **FR-010b**: Assignment lines MUST store session values in shell session state owned by a
  dedicated shell module, separate from the receiving agent and separate from the receiving agent's
  memory map.
- **FR-010c**: The system MUST provide a dedicated instantiable shell module that creates,
  holds, and cleans up shell session instances and their memory maps for shell session values.
- **FR-010ca**: The system MUST also provide an instantiable shell session module that mediates
  runtime shell-session access for the built-in shell method.
- **FR-010cb**: The shell session module MUST NOT directly own or directly handle the shell session
  memory map; it MUST mediate access to shell session state held by the shell module.
- **FR-010d**: Access between the shell session module and the built-in shell method MUST occur via
  messages rather than shared internal state.
- **FR-011**: In normal mode, the shell session MUST acknowledge whether a wrapped input interaction
  was successfully forwarded from the shell delegate to the receiving agent before any later reply
  arrives.
- **FR-011a**: In verbose mode, the shell MAY additionally surface whether the receiving agent
  accepted the interaction for its own processing and whether the requested runtime action later
  succeeded or failed.
- **FR-012**: The shell MUST display messages explicitly returned to the shell delegate session and
  identify the sending runtime component for each displayed reply.
- **FR-012a**: The session-specific shell delegate MUST unwrap returned output envelope maps into
  terminal output strings while preserving sender attribution for display.
- **FR-013**: The shell MUST display returned replies asynchronously if they arrive after later user
  input has already been accepted.
- **FR-014**: The shell MUST keep the session alive after forwarding failures, interpretation
  failures, unavailable methods, invalid runtime targets, and malformed user input, while providing
  actionable feedback.
- **FR-015**: The shell MUST let the user end the shell session cleanly.
- **FR-016**: When the shell session ends, the runtime MUST automatically destroy the dedicated
  receiving agent created for that session.

### Key Entities *(include if feature involves data)*

- **Shell Session**: The user’s interactive AgeRun terminal workspace, including the session-
  specific shell delegate, its receiving agent connection, the shell module that owns it, the shell
  session module that mediates access to it, displayed replies, and shutdown lifecycle.
- **Shell Input Envelope**: The structured map created by the shell delegate for each accepted line
  of terminal input. Initially it contains exactly one entry: `text`.
- **Shell Session Delegate**: The session-specific delegate bound to one shell session. It wraps
  terminal input strings into envelope maps, unwraps returned output envelopes back into terminal
  strings, and holds the configured receiving-agent target for that session.
- **Receiving Agent**: The dedicated runtime agent created automatically from the built-in `shell`
  method for one shell session that receives shell input envelopes and executes the shell method's
  interpretation behavior.
- **Built-in Shell Method**: The built-in `shell` method executed by the session's receiving agent
  that implements the shell's interpreted `spawn(...)`, `send(...)`, and assignment capabilities.
- **Shell Module**: A dedicated instantiable `ar_shell` module that implements the `arsh`
  executable, creates and holds shell session instances, and cleans them up. It owns the shell
  session memory map used for shell assignment values and coordinates shell lifecycle for the shell
  executable.
- **Shell Session Module**: An instantiable runtime module for one shell session that mediates
  shell-session operations for the built-in shell method through messages. It does not directly own
  the shell session memory map.
- **Minimal Shell Syntax**: A restricted subset of AgeRun instruction syntax interpreted one line
  at a time by the receiving agent. Allowed forms are limited to `spawn(...)`, `send(...)`,
  `memory... := spawn(...)`, `memory... := send(...)`, and assignment forms. In shell mode,
  assignment forms keep the existing `memory... := ...` syntax and target the shell session memory
  map owned by the shell module through the shell session module.
- **Runtime Reply**: A message explicitly sent back to the shell delegate session by a runtime
  component after an earlier shell-driven interaction.
- **Shell Acknowledgement**: The shell-visible status reported for a wrapped input interaction.
  In normal mode it confirms delegate-to-receiving-agent handoff; in verbose mode it may also show
  receiving-agent acceptance and requested runtime action outcome.

## Assumptions & Dependencies *(mandatory)*

- **Assumptions**:
  - The session-specific shell delegate is responsible for stdio transport plus envelope
    wrap/unwrap, not shell semantics.
  - The receiving agent executes the built-in `shell` method, which decides what a wrapped shell
    input message means.
  - The shell remains unusable without a minimal receiving-agent syntax for launch, send, and value
    storage.
  - The receiving agent interprets exactly one input line at a time using only the restricted shell
    instruction subset.
  - Assigned function-call forms such as `memory... := spawn(...)` and `memory... := send(...)`
    are allowed within that restricted subset because shell interactions need to capture results.
  - In shell mode, `memory... := ...` continues to be the assignment syntax, but it targets the
    shell session memory map owned by the shell module instead of the receiving agent's memory map.
  - Launching methods and sending runtime messages happen because of receiving-agent behavior, not
    because the delegate understands that syntax directly.
  - Assignment values are stored outside the receiving agent's memory map in a shell session owned
    by a separate shell module.
  - The shell module is instantiable at the runtime contract level and owns the shell session
    lifecycle plus its memory map.
  - The shell session module remains instantiable and mediates runtime access to the shell session
    without directly handling the session map.
  - The shell session module and the built-in shell method exchange shell state through messages
    rather than shared internal state.
  - Replies intended for the shell session are delivered as messages to the shell delegate rather
    than being inferred from logs or agent memory.
  - The shell exposes at least one normal acknowledgement state for delegate-to-receiving-agent
    handoff and may expose deeper staged acknowledgement details in verbose mode.
  - The dedicated receiving agent is session-scoped and is destroyed automatically when the shell
    session exits.
- **External Dependencies**:
  - A local terminal environment capable of running the AgeRun executable
  - The AgeRun runtime with the ability to create a dedicated receiving agent from the built-in
    shell method when the shell starts
  - Registered methods and reply-capable runtime agents for interaction validation
- **Open Questions**: None

## Documentation & Compatibility Impact *(mandatory)*

- **Affected Documentation**: README.md, SPEC.md, executable/runtime documentation, `arsh` usage
  walkthroughs, and any shell guidance added for AgeRun users
- **Affected Runtime Contracts**: CLI behavior, the `arsh` command name, the dedicated `arsh`
  executable implemented by `ar_shell`, session-specific shell delegate messaging, shell
  input/output envelope handling, restricted one-line receiving-agent shell syntax, the built-in
  `shell` method contract, shell-mode `memory... := ...` redirection to shell session state owned
  by the shell module and mediated by the shell session module, shell module behavior, shell
  session module behavior, receiving-agent expectations, and reply display behavior for shell-
  directed interactions
- **Compatibility Notes**: This feature is intended as an additive capability; existing non-shell
  runtime entry points should remain available unless explicitly replaced in a later specification

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: A new user can start the shell, obtain a dedicated receiving agent automatically, and
  successfully forward a line of terminal input into the runtime in under 2 minutes using built-in
  guidance alone.
- **SC-002**: In acceptance testing, users can complete a shell interaction that produces normal
  handoff acknowledgement without restarting the shell in at least 95% of attempts.
- **SC-002a**: In verbose-mode validation scenarios, layered acknowledgement states are presented
  consistently for transport, receiving-agent acceptance, and requested runtime action outcome in
  100% of reviewed cases where those stages occur.
- **SC-003**: In transcript checks involving delayed replies, returned messages remain attributable
  to the correct runtime sender in 100% of reviewed scenarios.
- **SC-004**: Forwarding failures, interpretation failures, unavailable methods, invalid runtime
  targets, and malformed input all leave the shell session usable and provide corrective guidance in
  100% of validation scenarios.
- **SC-005**: In shell shutdown validation scenarios, the session-specific receiving agent is
  cleaned up automatically in 100% of reviewed normal exits.
