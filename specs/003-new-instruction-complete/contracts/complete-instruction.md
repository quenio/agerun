# Contract: `complete(...)` Instruction

## Purpose

Define the user-facing AgeRun language contract for the `complete(...)` instruction.

## Invocation Contract

### One-argument form

```text
memory.result := complete("The largest country in South America is {country}.")
```

- accepts one required template string
- derives result-map keys from `{name}` markers
- returns generated values in a new map, e.g. `memory.result.country` after assignment

### Two-argument form

```text
memory.result := complete("The capital of {country} is {city}.", memory.values)
```

- accepts the same required template string plus one optional values map expression
- the optional second argument must evaluate to a map
- copies provided values into a new result map, uses them for build-style substitution, and generates only missing placeholder keys

## Template Contract

- the template contains one or more `{placeholder}` markers
- repeated placeholder names are allowed
- repeated placeholder names resolve to one consistent stored string value
- templates with no placeholders return a copied provided values map, or an empty map if no values map is provided, without invoking local completion

## Output Contract

### Primary output

- the instruction returns a new map containing generated string values for missing placeholders and copied provided values
- the completed sentence itself is not the primary output of the instruction

### Instruction result

- the instruction returns a completion result map when assigned
- handled failures assign an empty map when a result path is present

## Success Contract

On success:
- every required placeholder has a value in the returned map
- every generated value is stored as a string suitable for direct AgeRun memory reuse
- provided values are copied into the returned map without mutating the input map
- generated values are validated directly before being stored in the result map
- later AgeRun instructions can reuse the returned values directly

### Success-path acceptance fixture set

The first implementation validates the success path with this documented fixture set:
- generated value: `memory.result := complete("The largest country in South America is {country}.")`
- quoted-literal prompt variant: `memory.result := complete("The \"largest\" country in South America is {country}.")`
- provided value map: `memory.result := complete("The capital of {country} is {city}.", memory.values)` where `memory.values.country` is preserved
- input preservation: pre-populated `memory.values.city` is copied to `memory.result.city` and remains unchanged in `memory.values`

### Reuse-path acceptance fixture set for SC-004

The first implementation validates reuse with this documented fixture set:
- repeated-placeholder consistency: `complete("{country} is in {continent}. {country} remains consistent.")`
- downstream build reuse: `memory.reply := build("reply={country}|{continent}", memory.result)`
- downstream send reuse: `send(message.sender, memory.reply)` or an equivalent delegate/agent send using the built reply

## Failure Contract

On failure:
- the instruction returns an empty map when assigned
- an actionable runtime error is recorded
- no partial generated values are exposed
- the provided input map remains unchanged

Failure cases include:
- non-map second argument (`failure_category=invalid_values_map` or equivalent actionable error)
- invalid-before-generation request errors such as missing template content or invalid placeholder syntax
- local completion runtime unavailable on an otherwise supported environment (`failure_category=runtime_unavailable`)
- timeout before a valid full result is ready (`failure_category=timeout`)
- incomplete placeholder coverage or other partial-generation failures (`failure_category=incomplete_placeholder` or another actionable runtime class)
- generated values that are empty, outer-whitespace-padded, or brace-containing

Failure-path diagnostics must include:
- `failure_category=...`
- `cause=...`
- `recovery_hint=...`

Failure-path acceptance fixture set:
- no-placeholder template: `complete("No placeholders here.")` returns an empty map or copied values map without local completion
- non-map second-argument fast-failure: `complete("The capital is {city}.", "not-a-map")` at the evaluator layer
- incomplete placeholder coverage: request `{country}` and `{language}` when the backend only returns `country`
- generated-value rejection: empty values, leading/trailing whitespace, or `{` / `}` in a returned placeholder value
- timeout/unavailable runtime: non-positive timeout or missing runner/runtime/model configuration
- post-failure continuation: a later non-`complete(...)` instruction still succeeds after a failed `complete(...)` call and the input map remains unchanged

## Non-Functional Contract

- the instruction uses only local completion resources
- the first release requires CPU-only execution support
- one instruction call must not block message handling indefinitely; over-limit execution is treated as failure
