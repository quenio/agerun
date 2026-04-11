# Data Model: Command-Line Shell

## 1. Shell Session

### Description
A shell session is the top-level runtime interaction created by invoking `arsh`. It coordinates the
session-specific shell delegate, the receiving agent, the shell session module, acknowledgement
behavior, and final shutdown.

### Key Attributes
- `command_name`: fixed user-facing name `arsh`
- `mode`: normal or verbose acknowledgement mode
- `lifecycle_state`: `created`, `active`, `closing`, `closed`
- `receiving_agent_id`: the dedicated agent created for the session
- `session_module_id`: identifier/handle for the shell session module instance
- `shell_delegate_id`: identifier/handle for the session-specific shell delegate instance

### Validation Rules
- Exactly one dedicated receiving agent is created per shell session
- The receiving agent starts from the built-in `shell` method
- Exactly one shell session module exists per shell session
- Exactly one shell delegate exists per shell session
- The session remains active after recoverable input and routing errors

### State Transitions
- `created -> active`: shell startup completes and receiving agent/session module/delegate are available
- `active -> closing`: user exits shell
- `closing -> closed`: receiving agent/session module/delegate cleanup completes

## 2. Shell Session Delegate

### Description
The session-specific delegate bound to one shell session. It owns terminal I/O for that session,
wraps input strings into input envelopes, unwraps output envelopes back into display strings, and
holds the configured receiving-agent target.

### Key Attributes
- `receiving_agent_id`: agent targeted for wrapped shell input
- `input_transport`: stdin line reader
- `output_transport`: stdout writer
- `lifecycle_state`: `created`, `active`, `closing`, `closed`

### Validation Rules
- The delegate is session-specific, not generic across unrelated runtime features
- The delegate holds exactly one receiving-agent target for its session
- The delegate wraps accepted input before forwarding it
- The delegate unwraps returned output envelopes before display

## 3. Shell Input Envelope

### Description
The structured map created by the shell delegate for each accepted line of terminal input before it
is forwarded into the runtime.

### Key Attributes
- `text`: exact terminal input string entered by the user

### Validation Rules
- The initial envelope contains exactly one key-value pair
- The `text` field preserves the original input string verbatim
- One envelope is produced for each accepted input line

## 4. Shell Output Envelope

### Description
The structured map returned toward the shell delegate so it can display shell-visible output.

### Key Attributes
- `text`: string to display in the terminal session
- `sender_id`: runtime component identifier to attribute the reply

### Validation Rules
- The delegate can unwrap the envelope into a displayed string plus sender attribution
- Delayed output envelopes remain attributable to the correct sender
- Output envelope handling does not terminate or corrupt the active shell session

## 5. Receiving Agent

### Description
The session-scoped runtime agent that executes the built-in `shell` method and interprets shell
input.

### Key Attributes
- `agent_id`: positive runtime agent identifier
- `method_name`: `shell`
- `method_version`: `1.0.0` for the first release
- `lifecycle_state`: `created`, `active`, `destroyed`

### Validation Rules
- The receiving agent is created automatically when `arsh` starts
- The receiving agent is destroyed automatically when the shell session exits
- The receiving agent does not own shell assignment state directly

## 6. Built-in Shell Method

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
- Session value assignment is redirected to the shell session module's memory map

## 7. Shell Session Module

### Description
An instantiable system module for one shell session. It is created by the `arsh` startup path and
creates/holds the shell session instance plus session-scoped values separately from the receiving
agent.

### Key Attributes
- `memory_map`: stores shell assignment values
- `lifecycle_state`: `created`, `active`, `destroyed`
- `pending_request_state`: tracks message-based set/get/ack operations, if needed

### Validation Rules
- The shell session module is separate from the receiving agent and its memory map
- The shell session module exchanges state with the built-in `shell` method only through messages
- `memory... := ...` in shell mode targets this module's memory map
- The shell session module owns creation/holding of the shell session instance

### Likely Protocol Operations
- `set`: persist a session value
- `get`: resolve a previously stored session value
- `ack`: confirm state operation outcome
- `error`: report invalid or unresolved session-state request

## 8. Shell Acknowledgement

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

## 9. Runtime Reply

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

- One **Shell Session** owns one **Shell Session Delegate**
- One **Shell Session** owns one **Receiving Agent**
- One **Shell Session** owns one **Shell Session Module**
- One **Shell Session** receives many **Shell Input Envelopes**
- One **Shell Session Delegate** targets one **Receiving Agent**
- One **Shell Session Delegate** displays many **Shell Acknowledgements** and **Runtime Replies**
- One **Receiving Agent** executes one **Built-in Shell Method**
- One **Built-in Shell Method** exchanges state messages with one **Shell Session Module**
- One **Shell Session Module** creates and holds one **Shell Session** instance
