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
- previously stored target values remain unchanged because the failed call performs no partial write

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
