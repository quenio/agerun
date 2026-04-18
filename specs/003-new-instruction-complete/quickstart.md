# Quickstart: Completion Instruction (`complete(...)`)

This document describes the intended first-release developer flow for the `complete(...)`
instruction.

## Prerequisites

- AgeRun builds successfully with `make clean build 2>&1`
- A local GGUF model file is available for CPU-only execution
- The model path is configured for the runtime before testing `complete(...)`
- The `complete(...)` instruction is exposed through the normal AgeRun instruction parser/evaluator

## 1. Build the vendored runtime and configure the model path

Example setup:

```bash
make vendor-llama-cpu
export AGERUN_COMPLETE_MODEL=/absolute/path/to/phi-3-mini-q4.gguf
# Optional: override the default vendored library path for experiments
export AGERUN_COMPLETE_LIBLLAMA=/absolute/path/to/libllama.dylib
# Optional fallback/testing override: use an explicit local runner instead of direct libllama
export AGERUN_COMPLETE_RUNNER=/absolute/path/to/llama-cli
```

Expected behavior:
- the local completion runtime resolves the configured model path
- when `AGERUN_COMPLETE_RUNNER` is not set, the runtime loads the vendored direct `libllama` runtime from `.deps/llama.cpp-install/`
- when `AGERUN_COMPLETE_RUNNER` is set, the runtime uses that explicit local runner override instead
- the runtime does not require a network service
- the first `complete(...)` call can lazily initialize the local model runtime for the process

## 2. Use `complete(...)` with top-level memory targets

Example method instructions:

```text
memory.ok := complete("The largest country in South America is {country}.")
memory.reply := build("ok={ok} country={country}", memory)
send(message.sender, memory.reply)
```

Expected outcome:
- `complete(...)` derives the placeholder `country`
- the instruction populates `memory.country` with a non-empty string
- the instruction returns boolean status in `memory.ok`
- `memory.reply` can reuse both values without extra conversion

## 2a. Reuse one stored value across repeated placeholders

Example method instructions:

```text
memory.ok := complete("{country} is in {continent}. {country} remains consistent.")
memory.reply := build("reply={country}|{continent}", memory)
send(message.sender, memory.reply)
```

Expected outcome:
- repeated `{country}` references resolve to the same stored string value
- later `build(...)` reads `memory.country` and `memory.continent` directly
- later `send(...)` can forward the built reply without extra conversion

## 3. Use `complete(...)` with a nested base memory path

Example method instructions:

```text
memory.ok := complete("The capital is {city}.", memory.location)
memory.reply := build("ok={ok} city={city}", memory.location)
send(message.sender, memory.reply)
```

Expected outcome:
- the second argument redirects writes under `memory.location`
- the instruction populates `memory.location.city`
- the instruction returns boolean status in `memory.ok`
- later instructions can read the populated string directly from `memory.location.city`

## 4. Observe failure behavior

Example method instructions:

```text
memory.ok := complete("No placeholders here.")
memory.reply := build("ok={ok}", memory)
send(message.sender, memory.reply)
```

Expected outcome:
- the instruction returns `false`
- an actionable error is recorded through the runtime logging path
- the logged error text includes `failure_category=...`, `cause=...`, and `recovery_hint=...`
- invalid templates and invalid base paths fail before local completion initialization begins
- previously stored target values remain unchanged because the failed call performs no partial write
- later non-`complete(...)` work can continue normally after the failed call

## 5. Success-path acceptance fixture set

Use the following documented fixture set when validating the first implementation success path:
- top-level target write: `complete("The largest country in South America is {country}.")`
- quoted-literal preservation: `complete("The \"largest\" country in South America is {country}.")`
- nested target write: `complete("The capital is {city}.", memory.location)`
- overwrite behavior: start with a pre-populated `memory.location.city`, then verify that only a successful `complete(...)` call replaces it

Expected validation evidence:
- `memory.ok` stores boolean status only
- successful calls store generated values as strings
- reconstructing the sentence from the stored values preserves the literal text exactly
- nested writes update the requested `memory...` base path
- pre-existing values are replaced only on successful completion

## 5a. Reuse-path acceptance fixture set for SC-004

Use the following documented fixture set when validating direct reuse after a successful completion call:
- repeated-placeholder consistency: `complete("{country} is in {continent}. {country} remains consistent.")`
- downstream build reuse: `memory.reply := build("reply={country}|{continent}", memory)`
- downstream send reuse: `send(message.sender, memory.reply)` or an equivalent delegate/agent send using the built reply

Expected validation evidence:
- repeated placeholders resolve to one stored string value per placeholder name
- later `build(...)` reads the populated completion outputs as normal string memory
- later `send(...)` can forward the reused string value without additional conversion or translation

## 5b. Failure-path acceptance fixture set

Use the following documented fixture set when validating User Story 3 failure handling:
- invalid template fast-failure: `complete("No placeholders here.")`
- invalid base-path fast-failure at the evaluator layer: `complete("The capital is {city}.", <non-memory AST>)`
- incomplete placeholder coverage: request `{country}` and `{language}` when the backend only returns `country`
- generated-value rejection: empty placeholder values, leading/trailing whitespace, or returned `{` / `}` characters
- timeout/unavailable runtime: non-positive timeout, missing runner override, missing model file, or unusable GGUF model
- post-failure continuation: execute a later assignment/build/send after a failed `complete(...)` call and verify it still succeeds

Expected validation evidence:
- invalid-before-generation cases leave `ar_local_completion` uninitialized
- actionable errors include `failure_category`, `cause`, and `recovery_hint`
- prior memory values survive every failure case unchanged
- no partial generated values are written when placeholder coverage is incomplete
- later non-completion instructions still succeed after a handled failure

## 6. Run targeted validation during implementation

```bash
make ar_complete_instruction_parser_tests 2>&1
make ar_complete_instruction_evaluator_tests 2>&1
make ar_local_completion_tests 2>&1
make ar_instruction_parser_tests 2>&1
make ar_instruction_evaluator_tests 2>&1
make sanitize-tests 2>&1
make check-docs
make clean build 2>&1
make check-logs
```

## Notes for the first implementation

- `complete(...)` writes generated values into `memory...` targets; it does not return a completed sentence
- if the instruction result is assigned, it is a boolean success/failure value
- all successful generated values are stored as strings in the first release
- repeated placeholders resolve to one consistent stored value
- all writes from one successful call are applied atomically
- the first release is local-only, CPU-only, and intended for short factual or structured template completion
- the primary backend path is direct vendored `libllama`; `AGERUN_COMPLETE_RUNNER` is an explicit override for controlled fallback/testing scenarios
- first-release performance guarantees apply only to short templates with up to 2 placeholders and 120 total characters
- warm-run and cold-start timing are validated separately in the implementation plan
- public language documentation for the instruction is synchronized across `SPEC.md`, `README.md`, and this quickstart so user-facing semantics match the implemented parser/evaluator behavior
