# Contract: Local Completion Runtime

## Purpose

Define the backend-facing contract between `ar_complete_instruction_evaluator` and the dedicated
local completion adapter module used for `complete(...)`.

## Role Boundary

- `ar_complete_instruction_evaluator` owns AgeRun instruction semantics, target-path resolution,
  validation, and atomic memory writes
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

## Non-Goals

- no remote inference service contract
- no streaming partial values into AgeRun memory
- no requirement to return the fully completed sentence text as the evaluator/backend interface
