# Data Model: Command-Line Shell

**Ownership Prefix Note**: Reference-like attributes follow the repository ownership conventions from
`kb/ownership-naming-conventions.md`: `own_` = owned by this entity, `mut_` = mutable borrowed
reference, `ref_` = read-only borrowed reference. Plain value attributes such as IDs, enum-like
status fields, and mode fields remain unprefixed.

## 1. Shell

### Description
An instantiable `ar_shell` module that implements the `arsh` executable, wraps the AgeRun system,
and manages shell sessions for the shell workflow. It creates, tracks, and destroys
`ar_shell_session` instances while exposing shell orchestration logic to unit tests through a
normal module API.

### Key Attributes
- `own_sessions`: collection of managed shell sessions keyed or indexed by `session_id`
- `default_mode`: enum value `normal` or `verbose` for the default acknowledgement mode, if configured
- `own_system`: owned AgeRun system instance used to create receiving agents and process shell traffic
- `ref_executable_name`: borrowed executable name `arsh`

### Validation Rules
- The shell is instantiable within the runtime architecture
- The shell implements the `arsh` executable behavior
- The shell owns the wrapped AgeRun system lifecycle through `own_system`
- The shell manages shell sessions without owning each session's internal state directly
- The shell remains directly unit testable without routing every behavior through unrelated executables

### Protocol Operations
- `ar_shell__create_session`: create a new shell session together with its delegate and receiving agent // EXAMPLE: planned protocol operation label
- `ar_shell__register_session`: insert a newly created session into `own_sessions` // EXAMPLE: planned protocol operation label
- `ar_shell__process_system_turn`: process one unit of AgeRun system work for active shell traffic // EXAMPLE: planned protocol operation label
- `ar_shell__release_session`: remove a closed session from `own_sessions` and release its remaining resources // EXAMPLE: planned protocol operation label

## 2. Shell Session

### Description
An instantiable `ar_shell_session` runtime entity bound to one shell invocation and managed by
`ar_shell`. It owns per-session state and lifecycle, including shell session memory, links the
receiving agent and delegate for that session, and mediates shell-session operations for the built-
in `shell` method through messages.

### Key Attributes
- `session_id`: identifier/handle for this shell session
- `ref_command_name`: borrowed user-facing name `arsh`
- `mode`: enum value `normal` or `verbose` for the session acknowledgement mode
- `status`: `created`, `active`, `closing`, `closed`
- `agent_id`: identifier of the dedicated receiving agent for this session
- `delegate_id`: identifier/handle for the session-specific shell delegate instance
- `own_memory`: shell session values used by shell-mode assignments

### Validation Rules
- The shell session is instantiable within the runtime
- Exactly one dedicated receiving agent is linked to each shell session
- The linked receiving agent starts from the built-in `shell` method
- Exactly one shell delegate is linked to each shell session
- The shell session exchanges state with the built-in `shell` method only through messages
- The shell session owns and directly handles `own_memory`
- The shell session remains active after recoverable input and routing errors
- Session cleanup is complete only after the agent and delegate are cleaned up and `ar_shell` releases the session

### State Transitions
- `created -> active`: shell startup completes and agent/delegate links are available
- `active -> closing`: user exits shell
- `closing -> closed`: agent/delegate cleanup completes and `ar_shell` releases the session

### Protocol Operations
- `ar_shell_session__bind_delegate`: associate the created delegate with this shell session // EXAMPLE: planned protocol operation label
- `ar_shell_session__bind_agent`: associate the created receiving agent with this shell session // EXAMPLE: planned protocol operation label
- `ar_shell_session__store_value`: persist a shell value into `own_memory` for a requested path // EXAMPLE: planned protocol operation label
- `ar_shell_session__load_value`: resolve a shell value from `own_memory` for a requested path // EXAMPLE: planned protocol operation label
- `ar_shell_session__return_loaded_value`: return a successfully resolved shell value to the requester // EXAMPLE: planned protocol operation label
- `ar_shell_session__report_operation_failure`: return a failed shell-session operation with a reason // EXAMPLE: planned protocol operation label
- `ar_shell_session__begin_shutdown`: transition the shell session from `active` to `closing` // EXAMPLE: planned protocol operation label
- `ar_shell_session__finish_shutdown`: complete cleanup and transition the shell session to `closed` // EXAMPLE: planned protocol operation label

## 3. Shell Delegate

### Description
The session-specific delegate bound to one shell session. It owns terminal I/O for that session,
wraps input strings into input envelopes, unwraps output envelopes back into display strings, and
holds the configured receiving-agent target.

### Key Attributes
- `agent_id`: agent targeted for wrapped shell input
- `own_input_transport`: owned stdin line reader
- `own_output_transport`: owned stdout writer
- `status`: `created`, `active`, `closing`, `closed`

### Validation Rules
- The delegate is session-specific, not generic across unrelated runtime features
- The delegate holds exactly one receiving-agent target for its session
- The delegate wraps accepted input before forwarding it
- The delegate unwraps returned output envelopes before display

### Protocol Operations
- `ar_shell_delegate__read_line`: read one line of terminal input from `own_input_transport` // EXAMPLE: planned protocol operation label
- `ar_shell_delegate__wrap_input`: create a shell input envelope from the accepted terminal line // EXAMPLE: planned protocol operation label
- `ar_shell_delegate__send_input`: forward the created input envelope to `agent_id` // EXAMPLE: planned protocol operation label
- `ar_shell_delegate__render_output`: unwrap a shell output envelope and write it to `own_output_transport` // EXAMPLE: planned protocol operation label
- `ar_shell_delegate__close`: close delegate transports during shell session shutdown // EXAMPLE: planned protocol operation label

## 4. Shell Input Envelope

### Description
The structured map created by the shell delegate for each accepted line of terminal input before it
is forwarded into the runtime.

### Key Attributes
- `own_text`: exact terminal input string entered by the user

### Validation Rules
- The initial envelope contains exactly one key-value pair
- The `text` field preserves the original input string verbatim
- One envelope is produced for each accepted input line

### Protocol Operations
- `ar_shell_input__create`: construct an input envelope that stores the accepted terminal line in `own_text` // EXAMPLE: planned protocol operation label
- `ar_shell_input__read_text`: expose `own_text` to the receiving agent for shell-method interpretation // EXAMPLE: planned protocol operation label

## 5. Shell Output Envelope

### Description
The structured map returned toward the shell delegate so it can display shell-visible output.

### Key Attributes
- `own_text`: string to display in the terminal session
- `sender_id`: runtime component identifier to attribute the reply

### Validation Rules
- The delegate can unwrap the envelope into a displayed string plus sender attribution
- Delayed output envelopes remain attributable to the correct sender
- Output envelope handling does not terminate or corrupt the active shell session

### Protocol Operations
- `ar_shell_output__create`: construct an output envelope from display text and sender attribution // EXAMPLE: planned protocol operation label
- `ar_shell_output__read_text`: expose `own_text` for terminal rendering // EXAMPLE: planned protocol operation label
- `ar_shell_output__read_sender`: expose `sender_id` for sender attribution during rendering // EXAMPLE: planned protocol operation label

## 6. Receiving Agent

### Description
The session-scoped runtime agent that executes the built-in `shell` method and interprets shell
input.

### Key Attributes
- `agent_id`: positive runtime agent identifier
- `ref_method_name`: borrowed method name `shell`
- `ref_method_version`: borrowed method version `1.0.0`
- `status`: `created`, `active`, `destroyed`

### Validation Rules
- The receiving agent is created automatically when `arsh` starts
- The receiving agent is destroyed automatically when the shell session exits
- The receiving agent does not own shell assignment state directly

### Protocol Operations
- `ar_shell_agent__receive_input`: accept one shell input envelope from the delegate // EXAMPLE: planned protocol operation label
- `ar_shell_agent__execute_method`: invoke the built-in `shell` method for the received input // EXAMPLE: planned protocol operation label
- `ar_shell_agent__send_output`: return shell-visible output toward the delegate // EXAMPLE: planned protocol operation label
- `ar_shell_agent__destroy`: terminate the session-scoped receiving agent during shell shutdown // EXAMPLE: planned protocol operation label

## 7. Built-in Shell Method

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
- Session value assignment is redirected to the shell session `own_memory` owned by `ar_shell_session`
  and kept separate from the receiving agent's memory map

### Protocol Operations
- `ar_shell_method__interpret_line`: parse and execute one shell input line from the received envelope // EXAMPLE: planned protocol operation label
- `ar_shell_method__spawn_agent`: execute a supported `spawn(...)` form and return its result // EXAMPLE: planned protocol operation label
- `ar_shell_method__send_message`: execute a supported `send(...)` form and return its result // EXAMPLE: planned protocol operation label
- `ar_shell_method__store_session_value`: execute a supported `memory... := ...` assignment into `own_memory` // EXAMPLE: planned protocol operation label
- `ar_shell_method__load_session_value`: resolve `memory...` references from the active shell session // EXAMPLE: planned protocol operation label

## 8. Shell Acknowledgement

### Description
The shell-visible status reported to the terminal after an input line is handled.

### Key Attributes
- `mode`: enum value `normal` or `verbose`
- `handoff_status`: whether the delegate successfully forwarded the envelope to the receiving agent
- `acceptance_status`: optional verbose-only receiving-agent acceptance state
- `action_status`: optional verbose-only runtime action outcome

### Validation Rules
- Normal mode always reports delegate-to-receiving-agent handoff status
- Verbose mode may additionally report receiving-agent acceptance and final action outcome

### Protocol Operations
- `ar_shell_ack__report_handoff`: report whether the delegate handed the input envelope to the receiving agent // EXAMPLE: planned protocol operation label
- `ar_shell_ack__report_acceptance`: report whether the receiving agent accepted the shell input for processing // EXAMPLE: planned protocol operation label
- `ar_shell_ack__report_action_outcome`: report the final runtime action result when verbose mode requests it // EXAMPLE: planned protocol operation label

## 9. Runtime Reply

### Description
A message explicitly returned toward the shell delegate after a shell-driven interaction.

### Key Attributes
- `sender_id`: runtime component that sent the reply
- `own_payload`: returned AgeRun data value before delegate unwrapping, if still structured
- `arrival_timing`: immediate or delayed relative to newer shell input

### Validation Rules
- Replies remain attributable to the correct sender even when delayed
- Delayed replies do not terminate or corrupt the active shell session

### Protocol Operations
- `ar_shell_reply__create`: construct a shell-visible reply payload from a runtime result // EXAMPLE: planned protocol operation label
- `ar_shell_reply__attach_sender`: attach sender attribution to the reply payload // EXAMPLE: planned protocol operation label
- `ar_shell_reply__route_to_delegate`: deliver the reply payload to the active shell delegate // EXAMPLE: planned protocol operation label

## Relationships

- One **Shell** creates, tracks, and destroys many **Shell Sessions** over time
- One **Shell Session** links to one **Shell Delegate**
- One **Shell Session** links to one **Receiving Agent**
- One **Shell Session** receives many **Shell Input Envelopes**
- One **Shell Delegate** targets one **Receiving Agent**
- One **Shell Delegate** displays many **Shell Acknowledgements** and **Runtime Replies**
- One **Receiving Agent** executes one **Built-in Shell Method**
- One **Built-in Shell Method** exchanges state messages with one **Shell Session** through its `own_memory`
