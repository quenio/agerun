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

## 2. Use `complete(...)` without provided values

Example method instructions:

```text
memory.result := complete("The largest country in South America is {country}.")
memory.reply := build("country={country}", memory.result)
send(message.sender, memory.reply)
```

Expected outcome:
- `complete(...)` derives the placeholder `country`
- the instruction returns a new map in `memory.result`
- `memory.result.country` contains a non-empty generated string
- `memory.reply` can reuse the returned map without extra conversion

## 2a. Reuse one returned value across repeated placeholders

Example method instructions:

```text
memory.result := complete("{country} is in {continent}. {country} remains consistent.")
memory.reply := build("reply={country}|{continent}", memory.result)
send(message.sender, memory.reply)
```

Expected outcome:
- repeated `{country}` references resolve to the same returned string value
- later `build(...)` reads `memory.result.country` and `memory.result.continent` directly
- later `send(...)` can forward the built reply without extra conversion

## 3. Use `complete(...)` with provided values

Example method instructions:

```text
memory.values.country := "Brazil"
memory.result := complete("The capital of {country} is {city}.", memory.values)
memory.reply := build("country={country} city={city}", memory.result)
send(message.sender, memory.reply)
```

Expected outcome:
- the second argument supplies a values map
- `memory.values.country` is substituted into the prompt and copied into `memory.result.country`
- only the missing `city` placeholder is sent to local completion
- the input `memory.values` map is not mutated

## 4. Observe failure behavior

Example method instructions:

```text
memory.result := complete("The capital is {city}.", "not-a-map")
memory.reply := build("result_count={count}", memory)
send(message.sender, memory.reply)
```

Expected outcome:
- the assigned instruction result is an empty map
- an actionable error is recorded through the runtime logging path
- the logged error text includes `failure_category=...`, `cause=...`, and `recovery_hint=...`
- invalid values-map arguments fail before local completion initialization begins
- any provided input map remains unchanged because the failed call performs no partial mutation
- later non-`complete(...)` work can continue normally after the failed call

## 5. Success-path acceptance fixture set

Use the following documented fixture set when validating the first implementation success path:
- generated result map: `memory.result := complete("The largest country in South America is {country}.")`
- quoted-literal prompt variant: `memory.result := complete("The \"largest\" country in South America is {country}.")`
- provided values map: `memory.result := complete("The capital of {country} is {city}.", memory.values)`
- input preservation: start with a pre-populated `memory.values.city`, then verify the returned map copies it and `memory.values` remains unchanged

Expected validation evidence:
- `memory.result` stores a returned map
- successful calls return generated values as strings in a result map
- generated values are non-empty and contain no leading/trailing whitespace or braces
- values-map inputs seed known placeholders without mutating the provided map
- pre-existing values supplied in the values map are preserved rather than overwritten

## 5a. Reuse-path acceptance fixture set for SC-004

Use the following documented fixture set when validating direct reuse after a successful completion call:
- repeated-placeholder consistency: `complete("{country} is in {continent}. {country} remains consistent.")`
- downstream build reuse: `memory.reply := build("reply={country}|{continent}", memory.result)`
- downstream send reuse: `send(message.sender, memory.reply)` or an equivalent delegate/agent send using the built reply

Expected validation evidence:
- repeated placeholders resolve to one stored string value per placeholder name
- later `build(...)` reads the returned completion outputs as normal string data
- later `send(...)` can forward the reused string value without additional conversion or translation

## 5b. Failure-path acceptance fixture set

Use the following documented fixture set when validating User Story 3 failure handling:
- non-map second-argument fast-failure at the evaluator layer: `complete("The capital is {city}.", "not-a-map")`
- no-placeholder template returns a copied values map or empty map without local completion
- incomplete placeholder coverage: request `{country}` and `{language}` when the backend only returns `country`
- generated-value rejection: empty placeholder values, leading/trailing whitespace, or returned `{` / `}` characters
- timeout/unavailable runtime: non-positive timeout, missing runner override, missing model file, or unusable GGUF model
- post-failure continuation: execute a later assignment/build/send after a failed `complete(...)` call and verify it still succeeds

Expected validation evidence:
- invalid-before-generation cases leave `ar_local_completion` uninitialized
- actionable errors include `failure_category`, `cause`, and `recovery_hint`
- provided input maps survive every failure case unchanged
- no partial generated values are exposed when placeholder coverage is incomplete
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

## 7. Run the documented short-template performance validation

```bash
make complete-performance-validation 2>&1
```

For the documented Linux containerized validation path:

```bash
make complete-performance-validation-linux-container 2>&1
```

The documented 20-template fixture set is:
1. `The largest country in South America is {country}.`
2. `The capital of Brazil is {city}.`
3. `The capital of Argentina is {city}.`
4. `The capital of Chile is {city}.`
5. `The capital of Peru is {city}.`
6. `The capital of Colombia is {city}.`
7. `The capital of Uruguay is {city}.`
8. `The capital of Paraguay is {city}.`
9. `The capital of Japan is {city}.`
10. `The capital of Canada is {city}.`
11. `The capital of Australia is {city}.`
12. `The official language of Brazil is {language}.`
13. `The official language of Argentina is {language}.`
14. `The Amazon rainforest is in {continent}.`
15. `The Nile river is in {continent}.`
16. `France is in {continent}.`
17. `Egypt is in {continent}.`
18. `Brasilia is the capital of {country}.`
19. `Brasilia is the capital of {country} in {continent}.`
20. `The capital of Brazil is {city}. {city} remains the capital.`

Execution notes:
- every template stays within the short-template class (≤120 characters and ≤2 placeholder occurrences)
- odd/even evaluator executions alternate one-argument and two-argument forms so no-values and provided-values calls are both measured
- fixture 20 is the repeated-placeholder latency case
- invalid-before-generation fast-failure, partial-generation waiting-limit behavior, and immediate
  post-failure readiness are verified by the dedicated failure-path tests rather than by the timed
  fixture loop
- the timing boundary ends when the `complete(...)` instruction returns, before any next queued
  message begins processing
- the first-release timing procedure assumes only one active `complete(...)` evaluation at a time
- below-baseline or heavier-than-documented runtime conditions retain failure-safety guarantees but
  do not retain the documented timing guarantees

### macOS results recorded on 2026-04-17

Validation baseline used:
- macOS 26.4.1 (`arm64`)
- Apple M3 Max
- 14 logical CPU cores
- 36 GiB RAM
- local SSD-backed vendored runtime/model files

Observed results:
- runtime warm support: `20/20` success, `20/20` under `15000 ms`, `avg=2682 ms`, `max=10061 ms`
- evaluator cold-start: `20/20` success, `20/20` under `30000 ms`, `avg=3573 ms`, `max=10969 ms`
- evaluator warm-run: `20/20` success, `20/20` under `15000 ms`, `avg=2760 ms`, `max=10421 ms`

### Linux containerized results recorded on 2026-04-17

Validation baseline used:
- Docker Desktop Linux container on macOS host
- `linux/arm64`
- kernel `6.12.54-linuxkit`
- 14 logical CPU cores visible in container
- `MemTotal: 8024304 kB` visible in container
- bind-mounted `.deps/linux-container-llama.cpp-install/lib/libllama.so`
- bind-mounted `models/phi-3-mini-q4.gguf`

Observed results:
- runtime warm support: `20/20` success, `20/20` under `15000 ms`, `avg=2109 ms`, `max=10899 ms`
- evaluator cold-start: `20/20` success, `20/20` under `30000 ms`, `avg=3611 ms`, `max=12394 ms`
- evaluator warm-run: `20/20` success, `20/20` under `15000 ms`, `avg=2118 ms`, `max=10776 ms`

## Notes for the first implementation

- `complete(...)` returns a map of copied provided values plus generated values; it does not return a completed sentence
- if the instruction result is assigned, it receives the completion result map
- all successful generated values are stored as strings in the first release
- repeated placeholders resolve to one consistent returned value
- provided input maps are never mutated
- the first release is local-only, CPU-only, and intended for short factual or structured template completion
- the primary backend path is direct vendored `libllama`; `AGERUN_COMPLETE_RUNNER` is an explicit override for controlled fallback/testing scenarios
- first-release performance guarantees apply only to short templates with up to 2 placeholders and 120 total characters
- warm-run and cold-start timing are validated separately in the implementation plan
- public language documentation for the instruction is synchronized across `SPEC.md`, `README.md`, and this quickstart so user-facing semantics match the implemented parser/evaluator behavior
