# Data Model: Command-Line Shell

## 1. Shell Module

### Description
An instantiable `ar_shell` module that implements the `arsh` executable and manages shell session
instances for the shell workflow. It creates, tracks, and destroys `ar_shell_session` instances,
while exposing shell orchestration logic to unit tests through a normal module API.

### Key Attributes
- `active_session_count`: number of shell sessions currently managed
- `default_mode`: normal or verbose acknowledgement mode, if configured
- `runtime_binding`: access path used to create receiving agents and process shell traffic
- `executable_name`: fixed executable name `arsh`

### Validation Rules
- The shell module is instantiable within the runtime architecture
- The shell module implements the `arsh` executable behavior
- The shell module manages shell session instances without owning each session's internal state directly
- The shell module remains directly unit testable without routing every behavior through unrelated executables

## 2. Shell Session Entry

### Description
A concrete session-tracking record held by `ar_shell` for one active or recently closing shell
session. It links the shell manager to the corresponding `ar_shell_session` instance, delegate,
and receiving agent for that session.

### Key Attributes
- `command_name`: fixed user-facing name `arsh`
- `mode`: normal or verbose acknowledgement mode
- `status`: `created`, `active`, `closing`, `closed`
- `agent_id`: identifier of the dedicated receiving agent for this entry
- `delegate_id`: identifier/handle for the session-specific shell delegate instance
- `session_id`: identifier/handle for the instantiable shell session module instance

### Validation Rules
- Exactly one dedicated receiving agent is linked to each shell session entry
- The linked receiving agent starts from the built-in `shell` method
- Exactly one shell delegate is linked to each shell session entry
- Exactly one shell session module instance is linked to each shell session entry
- The entry remains active after recoverable input and routing errors
- Entry cleanup is complete only after the agent, delegate, and session module are all cleaned up

### State Transitions
- `created -> active`: shell startup completes and agent/delegate/session-module links are available
- `active -> closing`: user exits shell
- `closing -> closed`: agent/delegate/session-module cleanup completes and `ar_shell` releases the entry

## 3. Shell Session Module

### Description
An instantiable `ar_shell_session` runtime module bound to one shell session. It owns the session's
state and lifecycle, including the shell session memory map, and mediates shell-session operations
for the built-in `shell` method through messages.

### Key Attributes
- `session_binding`: reference or identifier used by `ar_shell` to manage this shell session
- `status`: `created`, `active`, `closing`, `destroyed`
- `memory`: shell session values used by shell-mode assignments
- `pending_request_state`: tracks message-based set/get/ack operations, if needed

### Validation Rules
- The shell session module is instantiable within the runtime
- The shell session module exchanges state with the built-in `shell` method only through messages
- The shell session module owns and directly handles the session memory map for its shell session
- The shell session module owns the lifecycle state for its shell session while remaining managed by `ar_shell`

### Likely Protocol Operations
- `set`: request persistence of a session value
- `get`: request resolution of a previously stored session value
- `resolved`: return a resolved session value
- `ack`: confirm a state operation outcome
- `error`: report invalid or unresolved session-state request

## 4. Shell Session Delegate

### Description
The session-specific delegate bound to one shell session entry. It owns terminal I/O for that
session, wraps input strings into input envelopes, unwraps output envelopes back into display
strings, and holds the configured receiving-agent target.

### Key Attributes
- `agent_id`: agent targeted for wrapped shell input
- `input_transport`: stdin line reader
- `output_transport`: stdout writer
- `status`: `created`, `active`, `closing`, `closed`

### Validation Rules
- The delegate is session-specific, not generic across unrelated runtime features
- The delegate holds exactly one receiving-agent target for its session
- The delegate wraps accepted input before forwarding it
- The delegate unwraps returned output envelopes before display

## 5. Shell Input Envelope

### Description
The structured map created by the shell delegate for each accepted line of terminal input before it
is forwarded into the runtime.

### Key Attributes
- `text`: exact terminal input string entered by the user

### Validation Rules
- The initial envelope contains exactly one key-value pair
- The `text` field preserves the original input string verbatim
- One envelope is produced for each accepted input line

## 6. Shell Output Envelope

### Description
The structured map returned toward the shell delegate so it can display shell-visible output.

### Key Attributes
- `text`: string to display in the terminal session
- `sender_id`: runtime component identifier to attribute the reply

### Validation Rules
- The delegate can unwrap the envelope into a displayed string plus sender attribution
- Delayed output envelopes remain attributable to the correct sender
- Output envelope handling does not terminate or corrupt the active shell session

## 7. Receiving Agent

### Description
The session-scoped runtime agent that executes the built-in `shell` method and interprets shell
input.

### Key Attributes
- `agent_id`: positive runtime agent identifier
- `method_name`: `shell`
- `method_version`: `1.0.0` for the first release
- `status`: `created`, `active`, `destroyed`

### Validation Rules
- The receiving agent is created automatically when `arsh` starts
- The receiving agent is destroyed automatically when the shell session exits
- The receiving agent does not own shell assignment state directly

## 8. Built-in Shell Method

### Description
The built-in `shell` method executed by the receiving agent. It owns shell semantics for the
restricted instruction subset.

### Supported User-Facing Forms
- `spawn(...)`
- `send(...)`
- `memory... := ...`
- `memory... := spawn(...)`
- `memory... := send(...)`

### Validation Rules
- Exactly one input line is interpreted at a time
- The built-in shell method does not rely on nested function calls
- Session value assignment is redirected to the shell session memory map owned by `ar_shell_session`
  and kept separate from the receiving agent's memory map

## 9. Shell Acknowledgement

### Description
The shell-visible status reported to the terminal after an input line is handled.

### Key Attributes
- `mode`: normal or verbose
- `handoff_status`: whether the delegate successfully forwarded the envelope to the receiving agent
- `acceptance_status`: optional verbose-only receiving-agent acceptance state
- `action_status`: optional verbose-only runtime action outcome

### Validation Rules
- Normal mode always reports delegate-to-receiving-agent handoff status
- Verbose mode may additionally report receiving-agent acceptance and final action outcome

## 10. Runtime Reply

### Description
A message explicitly returned toward the shell delegate after a shell-driven interaction.

### Key Attributes
- `sender_id`: runtime component that sent the reply
- `payload`: returned AgeRun data value before delegate unwrapping, if still structured
- `arrival_timing`: immediate or delayed relative to newer shell input

### Validation Rules
- Replies remain attributable to the correct sender even when delayed
- Delayed replies do not terminate or corrupt the active shell session

## Relationships

- One **Shell Module** creates, tracks, and destroys many **Shell Session Entries** over time
- One **Shell Session Entry** links to one **Shell Session Module**
- One **Shell Session Entry** links to one **Shell Session Delegate**
- One **Shell Session Entry** links to one **Receiving Agent**
- One **Shell Session Entry** receives many **Shell Input Envelopes**
- One **Shell Session Delegate** targets one **Receiving Agent**
- One **Shell Session Delegate** displays many **Shell Acknowledgements** and **Runtime Replies**
- One **Receiving Agent** executes one **Built-in Shell Method**
- One **Built-in Shell Method** exchanges state messages with one **Shell Session Module**
- One **Shell Session Module** owns the state and lifecycle linked from one **Shell Session Entry**
