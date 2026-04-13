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
- `loop`: drive the shell runtime by creating, advancing, and releasing shell sessions while processing system work

## 2. Shell Session

### Description
An instantiable `ar_shell_session` runtime entity bound to one shell invocation and managed by
`ar_shell`. It owns per-session state and lifecycle, including shell session memory, links the
receiving agent and delegate for that session, mediates shell-session operations for the built-in
`shell` method through messages, and reports shell-visible acknowledgement state.

### Key Attributes
- `session_id`: identifier/handle for this shell session
- `mode`: enum value `normal` or `verbose` for the session acknowledgement mode
- `status`: `created`, `active`, `closing`, `closed`
- `agent_id`: identifier of the dedicated receiving agent for this session
- `own_memory`: shell session values used by shell-mode assignments

### Validation Rules
- The shell session is instantiable within the runtime
- Exactly one dedicated agent is linked to each shell session
- The linked agent starts from the built-in `shell` method definition
- Exactly one shell delegate is linked to each shell session
- The shell session exchanges state with the running `shell` method only through messages
- The shell session owns and directly handles `own_memory`
- The shell session remains active after recoverable input and routing errors
- Session cleanup is complete only after the agent and delegate are cleaned up and `ar_shell` releases the session

### State Transitions
- `created -> active`: shell startup completes and agent/delegate links are available
- `active -> closing`: user exits shell
- `closing -> closed`: agent/delegate cleanup completes and `ar_shell` releases the session

### Protocol Operations
- `activate`: complete startup by linking the delegate and the agent running the `shell` method, then transition the session to `active`
- `advance`: process one shell-session turn while active, including message exchange with the running `shell` method
- `report_acknowledgement`: report shell-visible acknowledgement state for the current input interaction
- `render_output`: render shell-visible output when the delegate calls back with a message received from the agent
- `close`: complete shutdown, clean up session-linked resources, and transition the session to `closed`

## 3. Shell Delegate

### Description
The session-specific delegate bound to one shell session. It reads terminal input, wraps each line
into the required input map instance, routes that map to the receiving agent, and calls back into
the session when agent output arrives.

### Key Attributes
- `agent_id`: agent targeted for wrapped shell input and running the `shell` method
- `ref_session`: borrowed reference to the shell session used for callback-based output rendering

### Validation Rules
- The delegate is session-specific, not generic across unrelated runtime features
- The delegate holds exactly one receiving-agent target for its session
- The delegate wraps accepted input into the required input map instance before forwarding it
- The delegate holds `ref_session` so it can call back into its shell session when agent output is received

### Protocol Operations
- `read_input`: read one line of terminal input, create the corresponding input map instance, and deliver it to `agent_id`

## 4. Runtime Reply

### Description
A message explicitly returned toward the shell delegate after a shell-driven interaction. Its
payload may still be a structured map instance before the session renders it.

### Key Attributes
- `sender_id`: runtime component that sent the reply
- `own_payload`: returned AgeRun data value before delegate unwrapping, if still structured
- `arrival_timing`: immediate or delayed relative to newer shell input

### Validation Rules
- Replies remain attributable to the correct sender even when delayed
- Delayed replies do not terminate or corrupt the active shell session

### Protocol Operations
- `create`: construct a shell-visible reply payload from a runtime result
- `attach_sender`: attach sender attribution to the reply payload
- `route_to_delegate`: deliver the reply payload to the active shell delegate

## Relationships

- One **Shell** creates, tracks, and destroys many **Shell Sessions** over time
- One **Shell Session** links to one **Shell Delegate**
- One **Shell Session** links to one agent instance running the `shell` method
- One **Shell Session** receives many shell input map instances over time
- One **Shell Delegate** targets one agent instance running the `shell` method
- One **Shell Session** reports many shell-visible acknowledgements and renders many **Runtime Replies** via delegate callbacks
- One running `shell` method exchanges state messages with one **Shell Session** through its `own_memory`
