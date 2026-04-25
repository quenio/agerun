# Data Model: Completion Instruction

**Ownership Prefix Note**: Reference-like attributes follow the repository ownership conventions from
`kb/ownership-naming-conventions.md`: `own_` = owned by this entity, `mut_` = mutable borrowed
reference, `ref_` = read-only borrowed reference. Plain value attributes such as enum-like status
fields, booleans, counts, and timeout values remain unprefixed.

## 1. Completion Template

### Description
A `complete(...)` input string that combines literal text with one or more `{placeholder}` markers.
It is the source from which the instruction derives placeholder names, literal context, and
result-map keys.

### Key Attributes
- `own_source_text`: full template string supplied to `complete(...)`
- `own_placeholder_names`: ordered collection of placeholder names derived from the template
- `own_literal_segments`: ordered collection of literal text segments between placeholders
- `placeholder_count`: number of placeholders found in the template

### Validation Rules
- Templates with placeholders derive result-map keys; templates without placeholders are allowed and require no local completion
- Placeholder names use the `{name}` syntax defined by the spec
- Repeated placeholder names are permitted, but all references resolve to one result-map value
- The template string remains unchanged while derived placeholder metadata is computed

## 2. Completion Values Context

### Description
The optional values-map context for one `complete(...)` evaluation. It maps placeholder names to
existing values that are copied into the result map and substituted into the template before missing
placeholder values are generated.

### Key Attributes
- `ref_values_map`: optional map value produced by the second instruction argument
- `own_result_keys`: mapping from placeholder name to result-map key
- `copy_mode`: fixed value `recursive_copy` for provided values
- `generation_mode`: generate only placeholder keys missing from the provided values map

### Validation Rules
- If the optional second argument is present, it evaluates to a map
- If no values map is present, `{country}` resolves to the `country` key in a new result map
- If a values map is present and contains `country`, that value is copied to the `country` result key
- Result keys are unique after placeholder de-duplication
- The context defines copied and missing values; it never mutates the provided map

## 3. Completion Operation

### Description
One execution of the `complete(...)` instruction. It combines the template, optional values-map
context, generated string values, validation status, and returned result map.

### Key Attributes
- `status`: `pending`, `succeeded`, `failed`, or `timed_out`
- `own_generated_values`: temporary map from placeholder name to generated string value
- `own_result_map`: map returned to AgeRun method execution
- `timeout_ms`: configured maximum wait time for the local completion attempt
- `failure_category`: optional machine-readable failure class for handled failures
- `own_failure_cause`: optional immediate-cause diagnostic string
- `own_recovery_hint`: optional recovery-hint diagnostic string

### Validation Rules
- `own_result_map` contains every required placeholder before success is allowed
- All generated values are stored as strings in the first release
- Provided values are recursively copied into the result map without mutating the input map
- Failed or timed-out completion exposes no partial generated values
- Successful generated values must not contain `{` or `}` characters

### State Transitions
- `pending -> succeeded`: all required placeholder values are available, validated, and present in the returned map
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
- Invalid requests such as missing template text or non-positive timeout values fail before backend initialization begins; empty placeholder sets return copied/empty maps without backend initialization
- A failed runtime initialization must surface actionable error information without corrupting agent memory or mutating provided values maps

## Relationships

- One **Completion Template** produces one **Completion Values Context** per instruction call
- One **Completion Template** drives one **Completion Operation**
- One **Completion Values Context** is consumed by one **Completion Operation**
- One **Completion Operation** requests generated values from one **Local Completion Runtime**
- One **Local Completion Runtime** may serve many **Completion Operations** over the lifetime of one process
