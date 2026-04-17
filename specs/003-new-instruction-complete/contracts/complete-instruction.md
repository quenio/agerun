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
- substituting the stored values back into the original template preserves every literal segment exactly
- all target writes are applied atomically
- existing target values are overwritten together
- later AgeRun instructions can reuse the populated strings directly

### Success-path acceptance fixture set

The first implementation validates the success path with this documented fixture set:
- top-level write: `complete("The largest country in South America is {country}.")`
- literal-preservation variant with quoted literals: `complete("The \"largest\" country in South America is {country}.")`
- nested base-path write: `complete("The capital is {city}.", memory.location)`
- overwrite behavior: pre-populated `memory.location.city` is replaced only by a successful `complete(...)` call

## Failure Contract

On failure:
- the instruction returns `false`
- an actionable runtime error is recorded
- no partial target writes occur
- previously stored target values remain unchanged

Failure cases include:
- missing placeholder markers in the template
- invalid second-argument path
- local completion runtime unavailable
- timeout before a valid full result is ready
- incomplete placeholder coverage

## Non-Functional Contract

- the instruction uses only local completion resources
- the first release requires CPU-only execution support
- one instruction call must not block message handling indefinitely; over-limit execution is treated as failure
