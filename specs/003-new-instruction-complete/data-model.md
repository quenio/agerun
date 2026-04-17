# Data Model: Completion Instruction

**Ownership Prefix Note**: Reference-like attributes follow the repository ownership conventions from
`kb/ownership-naming-conventions.md`: `own_` = owned by this entity, `mut_` = mutable borrowed
reference, `ref_` = read-only borrowed reference. Plain value attributes such as enum-like status
fields, booleans, counts, and timeout values remain unprefixed.

## 1. Completion Template

### Description
A `complete(...)` input string that combines literal text with one or more `{placeholder}` markers.
It is the source from which the instruction derives placeholder names, literal context, and target
memory writes.

### Key Attributes
- `own_source_text`: full template string supplied to `complete(...)`
- `own_placeholder_names`: ordered collection of placeholder names derived from the template
- `own_literal_segments`: ordered collection of literal text segments between placeholders
- `placeholder_count`: number of placeholders found in the template

### Validation Rules
- The template contains at least one placeholder
- Placeholder names use the `{name}` syntax defined by the spec
- Repeated placeholder names are permitted, but all references resolve to one stored value
- The template string remains unchanged while derived placeholder metadata is computed

## 2. Completion Target Context

### Description
The resolved memory-write context for one `complete(...)` evaluation. It maps placeholder names to
actual `memory...` destinations using either the default top-level mapping or the optional base
memory path argument.

### Key Attributes
- `ref_base_memory_path`: optional `memory...` path provided as the second instruction argument
- `own_target_paths`: mapping from placeholder name to final memory destination path
- `write_mode`: fixed value `atomic_overwrite`
- `uses_default_root`: boolean indicating whether targets resolve under top-level `memory`

### Validation Rules
- If `ref_base_memory_path` is present, it begins with `memory`
- If no base path is present, `{country}` resolves to `memory.country`
- If a base path is present, `{country}` resolves to `<base path>.country`
- All target paths are unique after placeholder de-duplication
- The context defines write targets only; it does not own generated values

## 3. Completion Operation

### Description
One execution of the `complete(...)` instruction. It combines the template, resolved target
context, generated string values, validation status, and final boolean instruction result.

### Key Attributes
- `status`: `pending`, `succeeded`, `failed`, or `timed_out`
- `own_generated_values`: temporary map from placeholder name to generated string value
- `result`: boolean instruction result returned to AgeRun method execution
- `timeout_ms`: configured maximum wait time for the local completion attempt
- `own_error_reason`: optional failure reason for logging/diagnostics

### Validation Rules
- `own_generated_values` contains every required placeholder before success is allowed
- All generated values are stored as strings in the first release
- Successful completion writes all target values atomically
- Failed or timed-out completion writes no target values
- Successful values must not contain unresolved placeholder markers from the submitted template

### State Transitions
- `pending -> succeeded`: all required placeholder values are available, validated, and written atomically
- `pending -> failed`: validation, backend, or configuration error prevents full completion
- `pending -> timed_out`: timeout threshold is exceeded before a valid full result is ready

## 4. Local Completion Runtime

### Description
The process-local adapter that encapsulates the local completion backend used by
`ar_complete_instruction_evaluator`. It hides model loading, backend configuration, CPU-only
execution, and structured placeholder-value generation behind a stable AgeRun-facing interface.

### Key Attributes
- `status`: `uninitialized`, `ready`, or `failed`
- `own_model_path`: resolved filesystem path to the GGUF model asset
- `backend_name`: fixed value `llama.cpp`
- `model_name`: fixed first-release value `phi-3-mini-q4.gguf`
- `execution_target`: fixed first-release value `cpu`
- `request_count`: number of completion requests served by the loaded runtime instance

### Validation Rules
- The runtime operates locally and does not require network access
- The runtime loads one configured GGUF model asset for the process
- The runtime accepts placeholder-oriented requests and returns structured string values
- The runtime may remain shared across multiple instruction evaluations in one process
- A failed runtime initialization must surface actionable error information without corrupting agent memory

## Relationships

- One **Completion Template** produces one **Completion Target Context** per instruction call
- One **Completion Template** drives one **Completion Operation**
- One **Completion Target Context** is consumed by one **Completion Operation**
- One **Completion Operation** requests generated values from one **Local Completion Runtime**
- One **Local Completion Runtime** may serve many **Completion Operations** over the lifetime of one process
