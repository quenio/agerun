# Research: Completion Instruction

## Decision 1: Implement `complete(...)` as specialized Zig parser and evaluator modules

- **Decision**: Add `ar_complete_instruction_parser` and `ar_complete_instruction_evaluator` as new
  Zig C-ABI modules, then wire both into `ar_instruction_parser`, `ar_instruction_evaluator`, and
  `ar_instruction_ast`.
- **Rationale**: The latest planning input requires that all new modules be written in Zig. Using
  Zig C-ABI modules preserves the repository's stable C-facing interfaces while satisfying that
  constraint and keeping feature logic out of the existing facade modules.
- **Alternatives considered**:
  - Implement the parser in C and only the evaluator in Zig: rejected because it violates the new
    requirement that all new modules for this feature be written in Zig.
  - Implement parser logic directly inside `ar_instruction_parser.c`: rejected because AgeRun uses
    one specialized parser module per instruction type and inline parser growth would weaken
    separation of concerns.
  - Implement evaluator logic directly inside `ar_instruction_evaluator.zig`: rejected because the
    facade module should coordinate specialized evaluators, not absorb new instruction behavior.

## Decision 2: Hide llama.cpp integration behind a dedicated `ar_local_completion` Zig C-ABI module

- **Decision**: Add a new Zig C-ABI module, `ar_local_completion`, that owns local model loading,
  CPU-only inference, timeout handling, and normalization of backend output for the instruction
  evaluator.
- **Rationale**: The spec requires local-only completion while preserving portability and the
  project's separation-of-concerns rules. A dedicated adapter module keeps the evaluator focused on
  AgeRun semantics, not third-party runtime details, and leaves room to change the underlying model
  runtime later without rewriting the `complete(...)` evaluator contract.
- **Alternatives considered**:
  - Call llama.cpp directly from `ar_complete_instruction_evaluator`: rejected because it mixes
    instruction semantics with model-runtime lifecycle and makes testing harder.
  - Use a remote hosted completion service: rejected by the spec's local-only requirement.
  - Hard-code backend behavior into the instruction evaluator facade: rejected because it violates
    information hiding and would couple unrelated instruction types to one backend.

## Decision 3: Resolve placeholders through a structured placeholder-value response, not a completed sentence result

- **Decision**: Derive the placeholder list from the template, ask the local completion backend for
  a structured placeholder-to-string response, and apply those strings to AgeRun memory targets;
  do not treat a completed sentence as the primary instruction output.
- **Rationale**: The clarified spec says the interpolated memory variables are the primary output
  and the instruction result is boolean status only. Requesting structured values from the backend
  avoids brittle reparsing of a generated sentence, aligns directly with the memory-write contract,
  and makes atomic validation easier.
- **Alternatives considered**:
  - Generate a completed sentence and parse it back into variables: rejected because it is brittle,
    harder to validate, and conflicts with the clarified primary-output contract.
  - Return a map value from `complete(...)`: rejected because the spec requires boolean status plus
    direct writes into `memory...` targets.
  - Populate values incrementally as the model generates them: rejected because the spec requires
    atomic writes and no partial failure state.

## Decision 4: Load one local model runtime lazily per process and reuse it across `complete(...)` calls

- **Decision**: `ar_local_completion` will lazily initialize one process-local model/runtime from a
  configurable GGUF path, then reuse that loaded state across `complete(...)` evaluations.
- **Rationale**: Re-loading a local model for every instruction call would make the first-release
  latency target difficult to meet and would complicate memory management. A lazy shared runtime
  respects the local-only constraint while keeping the first release single-process and easier to
  test.
- **Alternatives considered**:
  - Load the model anew on every `complete(...)` call: rejected because it would add avoidable
    latency and resource churn.
  - Initialize the model eagerly at overall runtime startup: rejected because the feature may never
    be used in some runs and should not force model startup cost unconditionally.
  - Hard-code an unchangeable model file path: rejected because the project already uses
    environment-based path configuration patterns and portability requires an override mechanism.

## Decision 5: Resolve model path through an environment-based override with a documented default

- **Decision**: Resolve the GGUF model path from an environment override first, then fall back to a
  documented default relative path (`models/phi-3-mini-q4.gguf`) for local development and tests.
- **Rationale**: The repository already uses environment-variable configuration patterns
  (`AGERUN_METHODS_DIR`, `AGERUN_MEMORY_REPORT`) for runtime-resolved paths. Reusing that pattern
  keeps local model configuration portable without baking machine-specific paths into source code.
- **Alternatives considered**:
  - Hard-code an absolute filesystem path: rejected because it is non-portable.
  - Require a new CLI flag for all runtimes: rejected because `complete(...)` is a language feature,
    not a shell-only feature, and the broader runtime currently relies more on environment/file
    defaults than feature-specific CLI configuration.
  - Auto-discover arbitrary model files by scanning the filesystem: rejected because it increases
    ambiguity and reduces deterministic behavior.

## Decision 6: Stage and validate all generated strings before any memory mutation

- **Decision**: `ar_complete_instruction_evaluator` will buffer all generated placeholder strings in
  a temporary map, verify that every required placeholder is present, non-empty, and free of
  unresolved placeholder markers, then atomically write all target variables and return `true`.
  Any backend, validation, or timeout failure returns `false`, logs an actionable error, and leaves
  prior memory unchanged.
- **Rationale**: This directly matches the spec's overwrite-on-success, no-partial-write-on-failure,
  string-only output, and empty-string rejection requirements. It also preserves AgeRun's
  message-processing stability by making failure handling explicit and bounded.
- **Alternatives considered**:
  - Write values one by one as they become available: rejected because it violates atomicity and
    risks partial updates.
  - Permit partially complete writes with warnings: rejected because the spec requires full
    placeholder coverage or failure.
  - Infer numeric or structured types from generated strings: rejected because the clarified spec
    requires string-only storage in the first release.

## Decision 7: Measure warm-run and cold-start performance separately using the clarified short-template workload

- **Decision**: Validate performance with the spec-defined workload of 20 short completion
  templates, where each template has up to 2 placeholders and up to 120 total characters. Measure
  warm-run latency after runtime initialization and cold-start latency including the first model
  load, with end-to-end timing covering evaluator execution, validation, and atomic memory writes.
- **Rationale**: The clarified spec now distinguishes warm and cold timing and constrains the
  workload size. Capturing those boundaries in planning prevents benchmark drift and gives `/spec
  tasks` a concrete basis for validation work.
- **Alternatives considered**:
  - Use one mixed latency target for both cold and warm runs: rejected because model-load cost would
    blur steady-state behavior.
  - Measure only generation time: rejected because the user-facing instruction includes validation,
    logging, and atomic writes.
  - Leave workload size informal: rejected because the clarified spec makes it measurable.

## Decision 8: Require first-release validation evidence on both macOS and Linux

- **Decision**: Treat macOS and Linux as mandatory first-release validation targets for the feature,
  with the same user-facing grammar, failure semantics, and local-only CPU-only contract on both.
- **Rationale**: The clarified spec explicitly names both platforms. Planning must therefore call
  for evidence on both instead of treating portability as an implied future concern.
- **Alternatives considered**:
  - Validate one reference platform first and defer the second: rejected because it contradicts the
    clarified first-release scope.
  - Treat unsupported environments only as runtime failure cases: rejected because the spec now
    promises first-release support, not mere best effort.

## Decision 9: Build against a pinned, vendored, CPU-only `libllama` instead of a system package

- **Decision**: Treat direct `llama.cpp` integration as a project-controlled dependency by vendoring
  a pinned upstream source tree under the top-level `llama-cpp/` directory and building/installing
  a CPU-only `libllama` into a local repository prefix, rather than depending on Homebrew or
  another machine-specific system installation.
- **Rationale**: The feature contract requires CPU-only support on systems that may not have any GPU
  backend available. A vendored build keeps header/library versions aligned, avoids accidental
  coupling to a locally installed GPU-enabled package shape, gives the repository a specific and
  discoverable top-level dependency location, and gives the Makefile one portable dependency path
  on both macOS and Linux.
- **Alternatives considered**:
  - Link against Homebrew `llama.cpp`: rejected because it is machine-specific and may pull in
    platform-local backend dependencies that AgeRun should not assume.
  - Require users to preinstall `libllama` globally: rejected because it weakens reproducibility and
    makes the direct backend contract depend on host-specific package management.

## Decision 10: Use a C implementation for the local `llama.cpp` adapter while keeping stable AgeRun headers

- **Decision**: Keep the parser and evaluator feature modules in Zig, but implement the
  `ar_local_completion` runtime adapter in C behind the same stable `ar_local_completion.h` header.
- **Rationale**: The direct `libllama` path benefits from straightforward C-level dynamic loading,
  function-pointer binding, and low-level interop with the upstream C API. The user explicitly
  approved this exception because it reduces integration complexity without changing the exposed
  AgeRun contract.
- **Alternatives considered**:
  - Keep `ar_local_completion` in Zig despite the extra interop friction: rejected because it adds
    avoidable complexity for dynamic library loading and C API binding.
  - Move the whole feature out of Zig: rejected because the parser/evaluator modules remain a good
    fit for Zig and already integrate cleanly.

## Decision 11: Preserve upstream `llama.cpp` MIT license text in the vendored tree

- **Decision**: Require the vendored `llama-cpp/` tree to retain the upstream `LICENSE` file and
  treat that file as a packaging prerequisite for any local build/install workflow.
- **Rationale**: Upstream `llama.cpp` is MIT-licensed, and the MIT redistribution condition is to
  include the copyright notice and permission notice in copies or substantial portions of the
  software. Making the license file an explicit vendoring requirement prevents accidental
  non-compliant packaging.
- **Alternatives considered**:
  - Assume the source tree will always contain a license file without checking: rejected because it
    leaves compliance to chance.
  - Track only a note in planning docs: rejected because packaging compliance should also be guarded
    by the concrete build workflow.
  - Keep the external `llama-cli` runner boundary as the long-term integration shape: rejected by
    the explicit architecture decision to move toward direct in-process `libllama` integration.

## Decision 12: Record actionable failure diagnostics at both backend and evaluator boundaries

- **Decision**: Normalize `complete(...)` failures so the recorded error text includes
  `failure_category`, `cause`, and `recovery_hint`, while invalid templates and invalid base paths
  fail before local runtime initialization begins.
- **Rationale**: User Story 3 requires distinguishable, actionable failures without partial memory
  mutation. Recording the same diagnostic shape across runtime and evaluator layers makes failures
  testable, preserves the supported-environment/runtime-failure distinction, and keeps later method
  work runnable after a handled failure.
- **Alternatives considered**:
  - Log free-form failure strings only: rejected because they are harder to validate and compare
    across failure scenarios.
  - Initialize the backend before validating obvious request errors: rejected because the spec now
    requires invalid-before-generation fast failure.
