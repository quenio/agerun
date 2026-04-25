# Contract: Local Completion Runtime

## Purpose

Define the backend-facing contract between `ar_complete_instruction_evaluator` and the dedicated
local completion adapter module used for `complete(...)`.

## Role Boundary

- `ar_complete_instruction_evaluator` owns AgeRun instruction semantics, values-map handling,
  validation, and result-map creation
- the local completion runtime owns model-path resolution, local backend lifecycle, CPU-only model
  execution, timeout enforcement support, and normalization of backend output into placeholder
  values
- the evaluator does not embed llama.cpp lifecycle logic directly

## Request Contract

Each completion request supplies:

```text
template = <original template string>
placeholders = <ordered unique placeholder name list>
timeout_ms = <maximum wait time>
```

Expected request properties:
- the template is the same string provided to `complete(...)`
- the placeholder list contains every required placeholder exactly once
- the request is local-only and does not depend on network access

## Response Contract

The local completion runtime returns either:

### Success response

```text
status = success
values.<placeholder> = <string value>
```

Success requirements:
- every requested placeholder key is present
- every returned value is a string
- values are normalized for AgeRun string storage

### Failure response

```text
status = failure
reason = <diagnostic string>
failure_category = <machine-readable failure class>
cause = <immediate cause>
recovery_hint = <next operator/developer action>
```

Failure requirements:
- the reason is specific enough for actionable logging
- the logged failure text includes `failure_category=...`, `cause=...`, and `recovery_hint=...`
- invalid requests such as missing template text, missing placeholder lists, empty placeholder lists, or non-positive `timeout_ms` fail before backend initialization begins
- failure does not mutate AgeRun memory directly
- timeout, incomplete-placeholder coverage, invalid request, runtime-unavailable, and other runtime-failure cases are distinguishable at the diagnostic level
- supported-environment resource problems remain runtime failures, not unsupported-platform cases

## Lifecycle Contract

- the runtime resolves a local GGUF model path before serving requests
- the first release uses llama.cpp with `phi-3-mini-q4.gguf` on CPU
- the primary backend boundary is direct in-process `libllama` loading from the project-controlled vendored build under `.deps/llama.cpp-install/`
- the runtime initializes lazily on the first completion request that needs local execution
- the runtime loads the GGUF model with CPU-only settings and reuses that loaded model for the runtime instance
- runtime initialization failure is reported as a normal instruction failure path, not a process crash
- partial-generation failures, including missing placeholder coverage after some values were produced, are normalized into the same handled failure contract and do not leak partial values into AgeRun memory

## Configuration Contract

- the runtime supports an environment-based model-path override
- the runtime also supports a documented default local model path for development
- the runtime supports an optional environment-based explicit library override via `AGERUN_COMPLETE_LIBLLAMA`
- the runtime supports an environment-based runner override via `AGERUN_COMPLETE_RUNNER`
- when no runner override is supplied, the runtime uses the vendored direct `libllama` path instead of shelling out to `llama-cli`
- the runner override exists for controlled fallback/testing scenarios and does not change the primary first-release architecture decision
- configuration and runtime-discovery errors are surfaced as actionable runtime failures
- if `AGERUN_COMPLETE_RUNNER` is set, runner spawn/read/exit failures are normalized into the same actionable failure-shape used by the direct backend

## Performance Validation Fixture Set

The first-release short-template validation fixture set contains exactly these 20 templates:

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

Fixture rules:
- every template is at most 120 characters
- every template uses at most 2 placeholder occurrences
- odd/even execution alternates one-argument and two-argument evaluator forms so no-values and provided-values calls both remain in scope
- fixture 20 is the repeated-placeholder latency case for SC-002f

## Validation Procedure

Use `make complete-performance-validation 2>&1` on the local host, or
`make complete-performance-validation-linux-container 2>&1` for the documented Linux containerized
validation path, with no other active `complete(...)` evaluation.

The documented procedure is:
- **Runtime warm support check**: run `ar_local_completion_tests` with
  `AGERUN_LOCAL_COMPLETION_SUBTEST=real_phi3_fixture_set_warm_run_support` to confirm the 20
  templates all receive structured placeholder values from an already initialized runtime
- **Evaluator cold-start check**: run `ar_complete_instruction_evaluator_tests` in 20 separate
  cold-start subprocesses using `AGERUN_COMPLETE_EVALUATOR_SUBTEST=performance_cold_fixture` plus
  `AGERUN_COMPLETE_EVALUATOR_FIXTURE_INDEX=<0..19>` so each measured call includes a fresh
  evaluator/runtime instance and the first model load for that instance
- **Evaluator warm-run check**: run `ar_complete_instruction_evaluator_tests` with
  `AGERUN_COMPLETE_EVALUATOR_SUBTEST=performance_warm_fixture_set` so the runtime is warmed once
  and then the same evaluator executes the full 20-template fixture set across the complete
  instruction path (generation, validation, error reporting, and result-map creation)
- **Linux containerized execution path**: when using
  `make complete-performance-validation-linux-container 2>&1`, execute the same runtime warm,
  evaluator cold-start, and evaluator warm-run procedure inside the project-controlled Docker image,
  using isolated Linux-specific build directories (`.deps/linux-container-llama.cpp-*` and
  `bin/run-tests-linux-container`) plus `LD_LIBRARY_PATH` pointed at the Linux vendored
  `libllama.so`
- **Invalid-before-generation fast-fail**: verify separately with
  `test_complete_instruction_evaluator__invalid_template_fast_failure_does_not_initialize_runtime`
  and `test_local_completion__invalid_before_generation_rejects_without_runtime_initialization`
- **Partial-generation waiting-limit treatment**: verify separately with
  `test_local_completion__partial_generation_missing_placeholder_failure_is_actionable` and
  `test_complete_instruction_evaluator__missing_placeholder_response_keeps_memory_clean`
- **Immediate post-failure scheduling readiness**: verify separately with
  `test_instruction_evaluator__normal_work_continues_after_complete_failure`

Interpretation rules:
- evaluator timings are the authoritative SC-002 / SC-002a measurements because they cover the full
  instruction path and stop before the runtime begins the next queued message
- runtime-only timings are diagnostic support for backend behavior, not the final acceptance metric
- the validation assumes at most one active `complete(...)` evaluation at a time
- environments below the documented minimum baseline, or heavier local-runtime conditions than this
  procedure, retain failure-safety guarantees but do not retain SC-002 / SC-002a timing guarantees

## Minimum Validation Baseline

The documented first-release validation baseline is:
- **macOS**: Apple silicon, 14 logical CPU cores, 36 GiB RAM, local SSD-backed model/runtime files
- **Linux containerized**: CPU-only `linux/arm64` Docker execution using the project-controlled
  validation image, Linux-specific vendored `libllama` build directories, bind-mounted local model
  and runtime files, and a container allocation of 14 logical CPU cores with approximately 8 GiB
  visible RAM

## Recorded Results

### macOS validation (executed 2026-04-17)

Environment:
- macOS 26.4.1 (`arm64`)
- Apple M3 Max
- 14 logical CPU cores
- 36 GiB RAM
- vendored CPU-only `libllama`
- local `models/phi-3-mini-q4.gguf`

Observed results from `make complete-performance-validation 2>&1`:
- runtime warm support summary: `fixtures=20 success=20 under_15000ms=20 avg=2682 ms max=10061 ms`
- evaluator cold-start summary: `fixtures=20 success=20 under_30000ms=20 avg=3573 ms max=10969 ms`
- evaluator warm-run summary: `fixtures=20 success=20 under_15000ms=20 avg=2760 ms max=10421 ms`

### Linux containerized validation (executed 2026-04-17)

Environment:
- Docker Desktop Linux container on macOS host
- `linux/arm64`
- kernel `6.12.54-linuxkit`
- 14 logical CPU cores visible in container
- `MemTotal: 8024304 kB` visible in container
- bind-mounted vendored CPU-only `libllama` from `.deps/linux-container-llama.cpp-install/`
- bind-mounted local `models/phi-3-mini-q4.gguf`

Observed results from `make complete-performance-validation-linux-container 2>&1`:
- runtime warm support summary: `fixtures=20 success=20 under_15000ms=20 avg=2109 ms max=10899 ms`
- evaluator cold-start summary: `fixtures=20 success=20 under_30000ms=20 avg=3611 ms max=12394 ms`
- evaluator warm-run summary: `fixtures=20 success=20 under_15000ms=20 avg=2118 ms max=10776 ms`

## Non-Goals

- no remote inference service contract
- no streaming partial values into AgeRun memory
- no requirement to return the fully completed sentence text as the evaluator/backend interface
