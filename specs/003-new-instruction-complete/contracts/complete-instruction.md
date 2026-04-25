# Contract: `complete(...)` Instruction

## Purpose

Define the user-facing AgeRun language contract for the `complete(...)` instruction.

## Invocation Contract

### One-argument form

```text
complete("The largest country in South America is {country}.")
```

- accepts one required template string
- derives placeholder targets from `{name}` markers
- writes generated values to top-level `memory.<name>` targets

### Two-argument form

```text
complete("The capital is {city}.", memory.location)
```

- accepts the same required template string plus one optional base path
- the optional second argument must be a `memory...` path
- writes generated values under the supplied base path, so `{city}` targets `memory.location.city`

## Template Contract

- the template contains one or more `{placeholder}` markers
- repeated placeholder names are allowed
- repeated placeholder names resolve to one consistent stored string value
- templates with no placeholders are invalid

## Output Contract

### Primary output

- the instruction writes generated string values into interpolated `memory...` targets
- the completed sentence itself is not the primary output of the instruction

### Instruction result

- the instruction returns boolean success/failure status
- if the result is assigned, the receiving variable stores that boolean status only

## Success Contract

On success:
- every required placeholder has one generated string value
- every generated value is stored as a string suitable for direct AgeRun memory reuse
- generated values are validated directly before being stored in the result map
- all target writes are applied atomically
- existing target values are overwritten together
- later AgeRun instructions can reuse the populated strings directly

### Success-path acceptance fixture set

The first implementation validates the success path with this documented fixture set:
- top-level write: `complete("The largest country in South America is {country}.")`
- literal-preservation variant with quoted literals: `complete("The \"largest\" country in South America is {country}.")`
- nested base-path write: `complete("The capital is {city}.", memory.location)`
- overwrite behavior: pre-populated `memory.location.city` is replaced only by a successful `complete(...)` call

### Reuse-path acceptance fixture set for SC-004

The first implementation validates reuse with this documented fixture set:
- repeated-placeholder consistency: `complete("{country} is in {continent}. {country} remains consistent.")`
- downstream build reuse: `memory.reply := build("reply={country}|{continent}", memory)`
- downstream send reuse: `send(message.sender, memory.reply)` or an equivalent delegate/agent send using the built reply

## Failure Contract

On failure:
- the instruction returns `false`
- an actionable runtime error is recorded
- no partial target writes occur
- previously stored target values remain unchanged

Failure cases include:
- missing placeholder markers in the template (`failure_category=invalid_template`)
- invalid second-argument path (`failure_category=invalid_base_path`)
- invalid-before-generation request errors such as missing template content or empty placeholder sets
- local completion runtime unavailable on an otherwise supported environment (`failure_category=runtime_unavailable`)
- timeout before a valid full result is ready (`failure_category=timeout`)
- incomplete placeholder coverage or other partial-generation failures (`failure_category=incomplete_placeholder` or another actionable runtime class)
- generated values that are empty, outer-whitespace-padded, or brace-containing

Failure-path diagnostics must include:
- `failure_category=...`
- `cause=...`
- `recovery_hint=...`

Failure-path acceptance fixture set:
- invalid template fast-failure: `complete("No placeholders here.")`
- invalid base-path fast-failure: `complete("The capital is {city}.", "not-a-memory-path")` at the AST/evaluator layer
- incomplete placeholder coverage: request `{country}` and `{language}` when the backend only returns `country`
- generated-value rejection: empty values, leading/trailing whitespace, or `{` / `}` in a returned placeholder value
- timeout/unavailable runtime: non-positive timeout or missing runner/runtime/model configuration
- post-failure continuation: a later non-`complete(...)` instruction still succeeds after a failed `complete(...)` call

## Non-Functional Contract

- the instruction uses only local completion resources
- the first release requires CPU-only execution support
- one instruction call must not block message handling indefinitely; over-limit execution is treated as failure
