# Data Model: Command-Line Shell

## 1. Shell Session

### Description
A shell session is the top-level runtime interaction created by invoking `arsh`. It coordinates the
stdio delegate, the receiving agent, the shell session module, acknowledgement behavior, and final
shutdown.

### Key Attributes
- `command_name`: fixed user-facing name `arsh`
- `mode`: normal or verbose acknowledgement mode
- `lifecycle_state`: `created`, `active`, `closing`, `closed`
- `receiving_agent_id`: the dedicated agent created for the session
- `session_module_id`: identifier/handle for the shell session module instance
- `stdio_delegate_id`: identifier/handle for the stdio delegate instance

### Validation Rules
- Exactly one dedicated receiving agent is created per shell session
- The receiving agent starts from one built-in shell method
- Exactly one shell session module exists per shell session
- The session remains active after recoverable input and routing errors

### State Transitions
- `created -> active`: shell startup completes and receiving agent/session module are available
- `active -> closing`: user exits shell
- `closing -> closed`: receiving agent/session module cleanup completes

## 2. Shell Input Envelope

### Description
The structured map created by the stdio delegate for each accepted line of terminal input before it
is forwarded into the runtime.

### Key Attributes
- `text`: exact terminal input string entered by the user

### Validation Rules
- The initial envelope contains exactly one key-value pair
- The `text` field preserves the original input string verbatim
- One envelope is produced for each accepted input line

## 3. Receiving Agent

### Description
The session-scoped runtime agent that executes the built-in shell method and interprets shell input.

### Key Attributes
- `agent_id`: positive runtime agent identifier
- `method_name`: `arsh`
- `method_version`: `1.0.0` for the first release
- `lifecycle_state`: `created`, `active`, `destroyed`

### Validation Rules
- The receiving agent is created automatically when `arsh` starts
- The receiving agent is destroyed automatically when the shell session exits
- The receiving agent does not own shell assignment state directly

## 4. Built-in Shell Method

### Description
The built-in method executed by the receiving agent. It owns shell semantics for the restricted
instruction subset.

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

## 5. Shell Session Module

### Description
An instantiable system module for one shell session. It owns session-scoped values separately from
the receiving agent.

### Key Attributes
- `memory_map`: stores shell assignment values
- `lifecycle_state`: `created`, `active`, `destroyed`
- `pending_request_state`: tracks message-based set/get/ack operations, if needed

### Validation Rules
- The shell session module is separate from the receiving agent and its memory map
- The shell session module exchanges state with the built-in shell method only through messages
- `memory... := ...` in shell mode targets this module's memory map

### Likely Protocol Operations
- `set`: persist a session value
- `get`: resolve a previously stored session value
- `ack`: confirm state operation outcome
- `error`: report invalid or unresolved session-state request

## 6. Shell Acknowledgement

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

## 7. Runtime Reply

### Description
A message explicitly returned to the stdio delegate after a shell-driven interaction.

### Key Attributes
- `sender_id`: runtime component that sent the reply
- `payload`: returned AgeRun data value
- `arrival_timing`: immediate or delayed relative to newer shell input

### Validation Rules
- Replies remain attributable to the correct sender even when delayed
- Delayed replies do not terminate or corrupt the active shell session

## Relationships

- One **Shell Session** owns one **Receiving Agent**
- One **Shell Session** owns one **Shell Session Module**
- One **Shell Session** owns one **Stdio Delegate** interaction endpoint
- One **Shell Session** receives many **Shell Input Envelopes**
- One **Receiving Agent** executes one **Built-in Shell Method**
- One **Built-in Shell Method** exchanges state messages with one **Shell Session Module**
- One **Shell Session** displays many **Shell Acknowledgements** and **Runtime Replies**
