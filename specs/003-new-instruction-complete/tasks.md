# Tasks: Completion Instruction

**Input**: Design documents from `specs/003-new-instruction-complete/`
**Prerequisites**: `plan.md` (required), `spec.md` (required), `research.md`, `data-model.md`, `contracts/`

**Tests**: Tests and validation tasks are REQUIRED for this feature because it changes the AgeRun
language grammar, instruction parsing, AST dispatch, instruction evaluation, local runtime
integration, failure handling, and ownership-sensitive memory updates.

**Organization**: Tasks are grouped by user story so each story can be implemented, validated, and
demonstrated independently.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no unmet dependencies)
- **[Story]**: Which user story this belongs to (`[US1]`, `[US2]`, `[US3]`)
- Include exact repository paths in every task
- Use `make` targets instead of direct compiler or binary execution

## Phase 1: Setup (Shared Context)

**Purpose**: Establish baseline evidence, confirm the Zig-only module plan, and record the exact
contracts and documentation that must stay synchronized.

- [x] T001 Run baseline validation for `modules/ar_instruction_ast_tests.c`, `modules/ar_instruction_parser_tests.c`, `modules/ar_instruction_evaluator_tests.c`, `modules/ar_parse_instruction_evaluator_tests.c`, and `modules/ar_build_instruction_evaluator_tests.c` with `make ar_instruction_ast_tests 2>&1`, `make ar_instruction_parser_tests 2>&1`, `make ar_instruction_evaluator_tests 2>&1`, `make ar_parse_instruction_evaluator_tests 2>&1`, and `make ar_build_instruction_evaluator_tests 2>&1`
- [x] T002 [P] Reconcile feature scope across `specs/003-new-instruction-complete/spec.md`, `specs/003-new-instruction-complete/plan.md`, `specs/003-new-instruction-complete/research.md`, `specs/003-new-instruction-complete/data-model.md`, `specs/003-new-instruction-complete/contracts/complete-instruction.md`, `specs/003-new-instruction-complete/contracts/local-completion-runtime.md`, and `specs/003-new-instruction-complete/quickstart.md`
- [x] T003 [P] Confirm documentation and contract sync targets in `SPEC.md`, `README.md`, `modules/ar_instruction_ast.md`, `modules/ar_instruction_parser.md`, `modules/ar_instruction_evaluator.md`, `modules/README.md`, `specs/003-new-instruction-complete/quickstart.md`, and `.specify/memory/pi-agent.md`

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Create the RED-phase evidence and Zig scaffolding required by every completion story.

**⚠️ CRITICAL**: No user story implementation starts before this phase is complete.

- [x] T004 Add failing AST and parser-facade coverage for `complete(...)` in `modules/ar_instruction_ast_tests.c` and `modules/ar_instruction_parser_tests.c`
- [x] T005 [P] Add failing evaluator-facade coverage for `complete(...)` in `modules/ar_instruction_evaluator_tests.c`
- [x] T006 [P] Add failing specialized parser tests in new `modules/ar_complete_instruction_parser_tests.c` for one-argument parsing, two-argument parsing, rejection of non-direct `memory...` access paths as the second argument, rejection of placeholder names that do not follow the existing `build(...)`/`parse(...)` identifier syntax, and rejection of unsupported interpolation syntax beyond `{name}`
- [x] T007 [P] Add failing specialized evaluator tests in new `modules/ar_complete_instruction_evaluator_tests.c` for boolean success, atomic staging, top-level versus nested target resolution, literal-segment preservation after substitution, rejection of leading/trailing whitespace, and rejection of generated `{` or `}` characters
- [x] T008 [P] Add failing local-runtime adapter tests in new `modules/ar_local_completion_tests.c` for environment override handling, documented default path handling, success payload normalization, timeout responses, and unavailable-runtime failures
- [x] T009 [P] Create compile-only Zig scaffolding in new `modules/ar_complete_instruction_parser.h`, new `modules/ar_complete_instruction_parser.zig`, new `modules/ar_complete_instruction_parser.md`, new `modules/ar_complete_instruction_evaluator.h`, new `modules/ar_complete_instruction_evaluator.zig`, new `modules/ar_complete_instruction_evaluator.md`, new `modules/ar_local_completion.h`, new `modules/ar_local_completion.zig`, and new `modules/ar_local_completion.md` so the RED phase fails on assertions instead of compilation
- [x] T010 Confirm the RED phase for `modules/ar_instruction_ast_tests.c`, `modules/ar_instruction_parser_tests.c`, `modules/ar_instruction_evaluator_tests.c`, `modules/ar_complete_instruction_parser_tests.c`, `modules/ar_complete_instruction_evaluator_tests.c`, and `modules/ar_local_completion_tests.c` with `make ar_instruction_ast_tests 2>&1`, `make ar_instruction_parser_tests 2>&1`, `make ar_instruction_evaluator_tests 2>&1`, `make ar_complete_instruction_parser_tests 2>&1`, `make ar_complete_instruction_evaluator_tests 2>&1`, and `make ar_local_completion_tests 2>&1`

**Checkpoint**: Failing validation exists for AST support, parser dispatch, evaluator dispatch, and local completion runtime behavior.

---

## Phase 3: User Story 1 - Populate memory from a completion template (Priority: P1) 🎯 MVP

**Goal**: Add the core `complete(...)` instruction so a method can submit a template with one or
more placeholders and have AgeRun populate top-level or base-path `memory...` targets with
non-empty string values, overwrite existing targets atomically, and return boolean status.

**Independent Test**: Execute `complete("The largest country in South America is {country}.")` and
`complete("The capital is {city}.", memory.location)` through the instruction runtime and verify
that `memory.country` or `memory.location.city` is populated with text generated by the chosen
local language model, that substituting the stored values back into the original template preserves
every literal segment exactly, that the stored values contain no leading/trailing whitespace and no
`{` or `}` characters, that the boolean result reports success, and that pre-existing targets are
overwritten only through one atomic successful write.

### Validation for User Story 1

- [x] T011 [P] [US1] Run the targeted failing tests for core completion parsing and evaluation in `modules/ar_complete_instruction_parser_tests.c`, `modules/ar_complete_instruction_evaluator_tests.c`, `modules/ar_instruction_parser_tests.c`, `modules/ar_instruction_evaluator_tests.c`, and `modules/ar_instruction_ast_tests.c`

### Implementation for User Story 1

- [x] T012 [P] [US1] Define the public parser API and ownership documentation in new `modules/ar_complete_instruction_parser.h` and new `modules/ar_complete_instruction_parser.md`
- [x] T013 [P] [US1] Define the public evaluator and local-runtime APIs plus ownership documentation in new `modules/ar_complete_instruction_evaluator.h`, new `modules/ar_complete_instruction_evaluator.md`, new `modules/ar_local_completion.h`, and new `modules/ar_local_completion.md`
- [x] T014 [US1] Extend instruction type support for `complete(...)` in `modules/ar_instruction_ast.h`, `modules/ar_instruction_ast.zig`, `modules/ar_instruction_ast.md`, and `modules/ar_instruction_ast_tests.c`
- [x] T015 [US1] Implement one-argument and two-argument `complete(...)` parsing plus facade dispatch wiring in new `modules/ar_complete_instruction_parser.zig`, `modules/ar_instruction_parser.c`, `modules/ar_instruction_parser.md`, `modules/ar_instruction_parser_tests.c`, and `modules/ar_complete_instruction_parser_tests.c`, enforcing that the optional second argument is a direct `memory...` access path, that placeholder names follow the same identifier syntax used by `build(...)` and `parse(...)`, and that unsupported interpolation syntaxes beyond `{name}` are rejected
- [x] T016 [P] [US1] Implement local model-path resolution, lazy runtime initialization, direct `libllama` loading with explicit runner override support, and structured placeholder-value success responses in new `modules/ar_local_completion.c`, new `modules/ar_local_completion_tests.c`, and new `modules/ar_local_completion.md`
- [x] T016a [P] [US1] Add vendored CPU-only `libllama` build and link scaffolding in the repository build file, `.gitignore`, and new `llama-cpp/README.agerun.md` so direct in-process integration can use a project-controlled dependency path instead of Homebrew or another machine-specific system package, with an explicit upstream-license-preservation requirement for vendored packaging
- [x] T017 [US1] Implement target-path resolution, string-only staging, boolean result handling, model-generated value acceptance, literal-segment preservation checks, rejection of leading/trailing whitespace, rejection of generated `{` or `}` characters, and atomic overwrite-on-success behavior in new `modules/ar_complete_instruction_evaluator.zig`, new `modules/ar_complete_instruction_evaluator_tests.c`, `modules/ar_instruction_evaluator.zig`, `modules/ar_instruction_evaluator.md`, and `modules/ar_instruction_evaluator_tests.c`
- [x] T018 [P] [US1] Update happy-path instruction examples, success-contract notes, and the documented acceptance fixture set used for success-path validation in `specs/003-new-instruction-complete/contracts/complete-instruction.md` and `specs/003-new-instruction-complete/quickstart.md`
- [x] T019 [US1] Re-run validation for `modules/ar_complete_instruction_parser_tests.c`, `modules/ar_complete_instruction_evaluator_tests.c`, `modules/ar_local_completion_tests.c`, `modules/ar_instruction_parser_tests.c`, `modules/ar_instruction_evaluator_tests.c`, and `modules/ar_instruction_ast_tests.c` with `make ar_complete_instruction_parser_tests 2>&1`, `make ar_complete_instruction_evaluator_tests 2>&1`, `make ar_local_completion_tests 2>&1`, `make ar_instruction_parser_tests 2>&1`, `make ar_instruction_evaluator_tests 2>&1`, and `make ar_instruction_ast_tests 2>&1` until User Story 1 passes
- [x] T020 [US1] Refactor shared template, placeholder, and target-resolution helpers in `modules/ar_complete_instruction_parser.zig`, `modules/ar_complete_instruction_evaluator.zig`, and `modules/ar_local_completion.c` while preserving green User Story 1 tests

**Checkpoint**: `complete(...)` can parse, dispatch, populate `memory...` targets, and return boolean success for the core happy path.

---

## Phase 4: User Story 2 - Reuse populated variables in later method logic (Priority: P2)

**Goal**: Ensure populated values behave like normal AgeRun string memory so later assignments,
builds, and sends can reuse them directly, including repeated-placeholder consistency within one
template.

**Independent Test**: Execute a completion template followed by later `build(...)` or `send(...)`
reuse through the instruction runtime and verify that repeated placeholders resolve consistently and
the populated strings are reusable without conversion.

### Validation for User Story 2

- [x] T021 [P] [US2] Add or update failing reuse and repeated-placeholder tests in `modules/ar_complete_instruction_evaluator_tests.c` and `modules/ar_instruction_evaluator_tests.c`
- [x] T022 [P] [US2] Run the targeted failing reuse tests in `modules/ar_complete_instruction_evaluator_tests.c` and `modules/ar_instruction_evaluator_tests.c`

### Implementation for User Story 2

- [x] T023 [US2] Implement repeated-placeholder de-duplication and one-value-per-placeholder consistency in `modules/ar_complete_instruction_evaluator.zig` and `modules/ar_complete_instruction_evaluator_tests.c`
- [x] T024 [US2] Implement downstream reuse coverage for later `build(...)`, message-flow consumption, and normal string reads in `modules/ar_instruction_evaluator_tests.c` and `modules/ar_complete_instruction_evaluator_tests.c`
- [x] T025 [P] [US2] Update developer-facing reuse examples and document the acceptance fixture-set coverage used for SC-004 in `specs/003-new-instruction-complete/quickstart.md` and `specs/003-new-instruction-complete/contracts/complete-instruction.md`
- [x] T026 [US2] Re-run validation for `modules/ar_complete_instruction_evaluator_tests.c` and `modules/ar_instruction_evaluator_tests.c` with `make ar_complete_instruction_evaluator_tests 2>&1` and `make ar_instruction_evaluator_tests 2>&1` until User Story 2 passes
- [x] T027 [US2] Refactor repeated-placeholder and reuse helpers in `modules/ar_complete_instruction_evaluator.zig` while preserving green User Story 2 tests

**Checkpoint**: Populated completion values behave like normal string memory and can be reused directly in later AgeRun logic.

---

## Phase 5: User Story 3 - Fail safely when completion is unavailable (Priority: P3)

**Goal**: Ensure unavailable local resources, invalid templates, invalid second arguments,
incomplete placeholder coverage, empty generated values, unresolved markers, and timeout
conditions fail safely with actionable errors and no partial memory mutation.

**Independent Test**: Execute failing `complete(...)` scenarios through the instruction runtime and
verify that the boolean result is `false`, actionable errors are recorded, prior memory values
remain unchanged, empty generated values are rejected, and later non-completion work still
succeeds.

### Validation for User Story 3

- [x] T028 [P] [US3] Add or update failing failure-path tests in `modules/ar_local_completion_tests.c`, `modules/ar_complete_instruction_evaluator_tests.c`, and `modules/ar_instruction_evaluator_tests.c` for invalid templates, invalid-before-generation rejection without local completion invocation, non-direct or otherwise invalid second arguments, placeholder names that violate the supported identifier syntax, unsupported interpolation syntax beyond `{name}`, missing or unusable local completion resources on otherwise supported environments, environment-specific resource behavior differences that prevent completion, incomplete placeholder coverage, empty generated values, leading/trailing whitespace, generated `{` or `}` characters, unresolved markers, partial-generation failure, and timeout behavior
- [x] T029 [P] [US3] Run the targeted failing failure-path tests in `modules/ar_local_completion_tests.c`, `modules/ar_complete_instruction_evaluator_tests.c`, and `modules/ar_instruction_evaluator_tests.c`

### Implementation for User Story 3

- [x] T030 [P] [US3] Implement local-runtime failure diagnostics so recorded errors include failure category, immediate cause, and recovery hint, plus environment/default-path configuration errors, supported-environment runtime-failure handling for missing or unusable local completion resources, environment-specific resource-behavior failure normalization, unavailable-model handling, invalid-before-generation fast-failure handling, and timeout response normalization in `modules/ar_local_completion.c`, `modules/ar_local_completion.md`, and `modules/ar_local_completion_tests.c`
- [x] T031 [US3] Implement evaluator-side invalid-template, invalid-base-path, invalid-placeholder-name rejection, environment-neutral direct-path validity, incomplete-placeholder, partial-generation failure handling, empty-string rejection, leading/trailing-whitespace rejection, generated-brace rejection, unresolved-marker rejection, actionable-error recording with failure category, immediate cause, and recovery hint, timeout, next-queued-message blocking semantics, immediate post-failure scheduling readiness, and no-partial-write behavior in `modules/ar_complete_instruction_evaluator.zig`, `modules/ar_complete_instruction_evaluator.md`, `modules/ar_complete_instruction_evaluator_tests.c`, `modules/ar_instruction_evaluator.zig`, and `modules/ar_instruction_evaluator_tests.c`
- [x] T032 [P] [US3] Update failure-path contracts and quickstart expectations, including required actionable-error fields (failure category, immediate cause, recovery hint), the supported-environment runtime-failure distinction, invalid-before-generation rejection, partial-generation failure handling, immediate post-failure scheduling readiness, and the documented validation fixture set covering all failure cases, in `specs/003-new-instruction-complete/contracts/local-completion-runtime.md`, `specs/003-new-instruction-complete/contracts/complete-instruction.md`, and `specs/003-new-instruction-complete/quickstart.md`
- [x] T033 [US3] Re-run validation for `modules/ar_local_completion_tests.c`, `modules/ar_complete_instruction_evaluator_tests.c`, and `modules/ar_instruction_evaluator_tests.c` with `make ar_local_completion_tests 2>&1`, `make ar_complete_instruction_evaluator_tests 2>&1`, and `make ar_instruction_evaluator_tests 2>&1` until User Story 3 passes
- [x] T034 [US3] Refactor failure-path cleanup and logging helpers in `modules/ar_local_completion.c` and `modules/ar_complete_instruction_evaluator.zig` while preserving green User Story 3 tests

**Checkpoint**: `complete(...)` fails safely with actionable diagnostics and no partial memory corruption.

---

## Final Phase: Polish & Cross-Cutting Concerns

**Purpose**: Finish documentation synchronization, quantitative validation, portability evidence, and repo-wide quality gates.

- [x] T035 [P] Update public language documentation in `SPEC.md`, `README.md`, and `specs/003-new-instruction-complete/quickstart.md`
- [x] T036 [P] Update technical module documentation in `modules/ar_complete_instruction_parser.md`, `modules/ar_complete_instruction_evaluator.md`, `modules/ar_local_completion.md`, `modules/ar_instruction_ast.md`, `modules/ar_instruction_parser.md`, `modules/ar_instruction_evaluator.md`, and `modules/README.md`
- [x] T037 [P] Sync plan artifacts and workflow context in `specs/003-new-instruction-complete/research.md`, `specs/003-new-instruction-complete/data-model.md`, `specs/003-new-instruction-complete/contracts/README.md`, and `.specify/memory/pi-agent.md` if implementation details materially change the planned contract
- [x] T038 Add or update short-template performance validation coverage in `modules/ar_local_completion_tests.c` and `modules/ar_complete_instruction_evaluator_tests.c`, and document the exact 20-template fixture set, repeated-placeholder latency treatment, invalid-before-generation fast-failure behavior, partial-generation waiting-limit treatment, warm-run timing procedure, cold-start timing procedure, next-queued-message timing boundary, single-active-`complete(...)` assumption, below-baseline/heavier-runtime no-guarantee rule, and minimum hardware baseline per OS in `specs/003-new-instruction-complete/contracts/local-completion-runtime.md` and `specs/003-new-instruction-complete/quickstart.md`
- [x] T039 Execute the documented 20-template warm-run and cold-start validation procedure on macOS using the fixture set, repeated-placeholder latency treatment, invalid-before-generation fast-failure behavior, next-queued-message timing boundary, single-active-`complete(...)` assumption, below-baseline/heavier-runtime no-guarantee rule, and minimum hardware baseline from `specs/003-new-instruction-complete/quickstart.md`, then record the baseline, timing method, and results in `specs/003-new-instruction-complete/contracts/local-completion-runtime.md` and `specs/003-new-instruction-complete/quickstart.md`
- [ ] T040 Execute the documented 20-template warm-run and cold-start validation procedure on Linux using the fixture set, repeated-placeholder latency treatment, invalid-before-generation fast-failure behavior, next-queued-message timing boundary, single-active-`complete(...)` assumption, below-baseline/heavier-runtime no-guarantee rule, and minimum hardware baseline from `specs/003-new-instruction-complete/quickstart.md`, then record the baseline, timing method, and results in `specs/003-new-instruction-complete/contracts/local-completion-runtime.md` and `specs/003-new-instruction-complete/quickstart.md`
- [x] T041 Run full targeted regression for `modules/ar_instruction_ast_tests.c`, `modules/ar_instruction_parser_tests.c`, `modules/ar_instruction_evaluator_tests.c`, `modules/ar_complete_instruction_parser_tests.c`, `modules/ar_complete_instruction_evaluator_tests.c`, and `modules/ar_local_completion_tests.c` with `make ar_instruction_ast_tests 2>&1`, `make ar_instruction_parser_tests 2>&1`, `make ar_instruction_evaluator_tests 2>&1`, `make ar_complete_instruction_parser_tests 2>&1`, `make ar_complete_instruction_evaluator_tests 2>&1`, and `make ar_local_completion_tests 2>&1`
- [x] T042 Run documentation validation for `SPEC.md`, `README.md`, `modules/ar_complete_instruction_parser.md`, `modules/ar_complete_instruction_evaluator.md`, `modules/ar_local_completion.md`, `modules/ar_instruction_ast.md`, `modules/ar_instruction_parser.md`, `modules/ar_instruction_evaluator.md`, and `specs/003-new-instruction-complete/quickstart.md` with `make check-docs`
- [x] T043 Run sanitizer coverage for `modules/ar_complete_instruction_parser.zig`, `modules/ar_complete_instruction_evaluator.zig`, `modules/ar_local_completion.c`, `modules/ar_instruction_parser.c`, `modules/ar_instruction_evaluator.zig`, and `modules/ar_instruction_ast.zig` with `make sanitize-tests 2>&1`
- [x] T044 Run final repo gates for `modules/ar_complete_instruction_parser.zig`, `modules/ar_complete_instruction_evaluator.zig`, `modules/ar_local_completion.c`, `SPEC.md`, and `README.md` with `make clean build 2>&1` and then `make check-logs`

---

## Dependencies & Execution Order

### Phase Dependencies

- **Phase 1: Setup** starts immediately
- **Phase 2: Foundational** depends on Setup and establishes the RED phase for AST support, parser dispatch, evaluator dispatch, and local runtime integration
- **Phase 3: User Story 1** depends on Foundational completion
- **Phase 4: User Story 2** depends on User Story 1 because variable reuse requires working completion writes first
- **Phase 5: User Story 3** depends on User Story 1 because failure handling builds on the core parser/evaluator/backend path; it may begin after User Story 1 even if User Story 2 is still in progress when file overlap permits
- **Final Phase** depends on all desired stories being complete

### Within Each User Story

- Add or run the failing tests first
- Implement the minimal change that makes the story pass
- Re-run targeted tests until green
- Refactor only after the story is green
- Update story-specific docs/contracts after behavior stabilizes

### Parallel Opportunities

- **Setup**: `T002` and `T003` can run in parallel after `T001`
- **Foundational**: `T005`, `T006`, `T007`, `T008`, and `T009` can run in parallel after scope review when they touch different files
- **User Story 1**: `T012` and `T013` can proceed in parallel after `T011`; `T016` can proceed in parallel with parser work when the backend adapter stays isolated
- **User Story 2**: `T025` can proceed in parallel with `T023` and `T024` once the reuse semantics are clear
- **User Story 3**: `T030` and `T032` can proceed in parallel after `T029` exposes the failures
- **Polish**: `T035`, `T036`, and `T037` can run in parallel after behavior stabilizes

---

## Implementation Strategy

### MVP First (User Story 1 Only)

1. Complete Setup
2. Complete Foundational RED-phase work
3. Deliver User Story 1 (`complete(...)` parsing, local runtime happy path, AST/evaluator dispatch, atomic memory writes)
4. Validate User Story 1 independently with the targeted `make` tests
5. Stop if core completion population is sufficient for the current milestone

### Incremental Delivery

1. Finish shared RED-phase and scaffolding once
2. Deliver User Story 1 for core completion population
3. Deliver User Story 2 for repeated-placeholder consistency and downstream reuse
4. Deliver User Story 3 for safe failure handling, diagnostics, and no-partial-write guarantees
5. Finish with quantitative performance evidence, macOS/Linux validation evidence, documentation sync, and full repository quality gates

---

## Notes

- Keep every RED phase at assertion level; use scaffolding tasks only to avoid compilation failures
- Preserve the separation of concerns from `research.md`: parsing in `ar_complete_instruction_parser`, AgeRun semantics in `ar_complete_instruction_evaluator`, backend lifecycle in `ar_local_completion`, and facade dispatch in the existing instruction parser/evaluator modules
- All new feature-specific modules for this feature are Zig C-ABI modules with stable `.h` headers; only existing facades remain in C where already present
- Use ownership-prefixed names and repository-standard memory management patterns in every new C and Zig implementation file
- Do not mark any task complete until the corresponding `make` output, file diff, or documentation update exists
