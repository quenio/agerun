# Implementation Plan: Workflow Coordinator

**Branch**: `002-let-build-workflow-coordinator` | **Date**: 2026-04-18 | **Spec**: [spec.md](./spec.md)
**Input**: Feature specification from `specs/002-let-build-workflow-coordinator/spec.md`

**Note**: This plan is filled in by the native `/spec plan` workflow.

## Summary

Replace the current boot-time chat-session demo with a methods-only workflow coordinator demo that
uses `complete(...)` as a core transition-decision mechanism. Keep all feature logic inside new
AgeRun method assets loaded from `methods/`, have `bootstrap-1.0.0.method` spawn a
`workflow-coordinator` agent on fresh executable runs, and let the coordinator stage the run in two
steps: first confirm that the active YAML workflow definition is valid and that any required local
`complete(...)` dependency is available, then seed exactly one workflow item and drive it through a
single-pass lifecycle. The active workflow definition still lives in a YAML file read through the
file delegate rather than in a `.method` file, but first-release YAML is intentionally shaped for
reliable extraction by AgeRun methods. The reusable workflow-definition method owns both YAML-backed
transition metadata and `complete(...)` prompts whose templates must include `{outcome}` and
`{reason}` placeholders. The MVP includes enough bundled-default transition evaluation to let a
supported fresh run reach a completed or rejected terminal outcome, while broader alternate-
definition reuse remains follow-on story work. Transition-level `complete(...)` failures become
retryable `stay` responses with visible reasons, while bundled-startup dependency failures stop the
workflow before item creation and emit a clear startup error rather than a fake work-item summary.

## Technical Context

**Language/Version**: AgeRun method language for feature logic; existing C17/Zig runtime only as
the execution host and test harness
**Primary Dependencies**: Existing boot/executable flow in `modules/ar_executable.c`, existing
method-loading from `methods/`, existing file-delegate runtime contract (`-100`), existing
`complete(...)` instruction support through `ar_complete_instruction_evaluator` and
`ar_local_completion`, AgeRun method instructions (`parse`, `build`, `spawn`, `send`, `if`,
`complete`), `ar_method_fixture`, Make, gcc-13/clang, Zig 0.14.1
**Storage**: Agent memory maps for coordinator/item/reporter/definition state, existing persisted
methods/agents, workflow-definition data read from YAML files via the file delegate, and local
completion runtime/model configuration through `AGERUN_COMPLETE_MODEL`, `AGERUN_COMPLETE_RUNNER`,
and related `complete(...)` environment support
**Testing**: `make workflow_definition_tests 2>&1`, `make workflow_item_tests 2>&1`,
`make workflow_coordinator_tests 2>&1`, `make workflow_reporter_tests 2>&1`,
`make bootstrap_tests 2>&1`, `make ar_executable_tests 2>&1`,
`make ar_complete_instruction_evaluator_tests 2>&1`, `make ar_local_completion_tests 2>&1`,
`make sanitize-tests 2>&1`, `make check-docs`, `make clean build 2>&1`, `make check-logs`
**Target Platform**: macOS and Linux CLI runtime through the existing `agerun` executable
**Project Type**: Message-driven runtime feature implemented entirely as method assets plus
documentation and test updates
**Performance Goals**: No new standalone latency target beyond the existing `complete(...)`
contract. At the feature level, a successful fresh boot run must either (a) reach a terminal outcome
automatically and emit at least four visible lifecycle checkpoints with reasons as required by the
spec, or (b) emit a clear startup dependency failure before item progression begins.
**Constraints**: No shell-driven UX; no new C feature modules for coordinator logic; all workflow
behavior lives in AgeRun methods; launch from `bootstrap`; workflow definition stays in YAML, not
`.method`; bundled default validation depends on local `complete(...)`; duplicate boot triggers in
one runtime must be ignored after the first accepted launch; invalid YAML definitions and reporting
failures must fail visibly without corrupting workflow state; `complete(...)` decision templates
must include `{outcome}` and `{reason}`; in-flight `complete(...)` failures map to retryable
`stay`; tests may use `AGERUN_COMPLETE_RUNNER` fake-runner overrides for determinism; no new runtime
instructions; no map literals; no nested function calls; keep YAML parse-friendly for method-level
extraction; follow parse/build-friendly message shapes; log through the existing runtime log
delegate path; zero memory leaks; make-target-only validation
**Scale/Scope**: One fresh-run demo item, one workflow coordinator agent, one generic
workflow-definition agent, one workflow reporter agent, and one workflow item agent for the default
demo; one bundled default YAML definition file plus one alternate YAML definition file for tests;
startup dependency probing before item creation; updated executable tests verify boot-driven demo,
startup failure, visible reasons, and restored-agent skip behavior

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

- [x] KB consultation recorded: `AGENTS.md`, `kb/README.md`,
  `kb/kb-consultation-before-planning-requirement.md`, `kb/user-feedback-as-qa.md`,
  `kb/assumption-verification-before-action.md`, `kb/tdd-iteration-planning-pattern.md`,
  `kb/multi-session-tdd-planning.md`, `kb/plan-verification-checklist.md`,
  `kb/report-driven-task-planning.md`, `kb/agerun-method-language-nesting-constraint.md`,
  `kb/method-test-template.md`, `kb/message-processing-loop-pattern.md`, and
  `kb/specification-consistency-maintenance.md`
- [x] Existing `complete(...)` contract reviewed: `SPEC.md` (`complete(...)` semantics),
  `specs/003-new-instruction-complete/contracts/complete-instruction.md`,
  `specs/003-new-instruction-complete/contracts/local-completion-runtime.md`, and
  `modules/ar_local_completion.md`
- [x] TDD approach defined: add failing method tests for workflow-definition startup probing,
  YAML-schema rejection, `complete(...)` decision extraction, retryable `stay` handling, visible
  reason reporting, coordinator startup failure behavior, bootstrap flow, and executable output
  before changing any method asset or executable expectation; keep RED -> GREEN -> REFACTOR
  sequencing in `/spec tasks`
- [x] Convention impact reviewed: new method assets stay in `methods/` with
  `<name>-<version>.method` plus matching docs/tests, workflow YAML files stay separate from
  `.method` assets, boot behavior remains in `bootstrap`, feature logic stays out of new C modules,
  make targets remain the only validation entry points, method bodies respect no-nested-function and
  no-map-literal constraints, and fake completion runners should follow the existing temporary-test
  pattern already used by complete-instruction tests
- [x] Specification and documentation impact identified: `specs/002-let-build-workflow-coordinator/*`,
  `README.md`, `methods/README.md`, `methods/bootstrap.md`, new workflow method docs,
  YAML definition-file documentation, `CHANGELOG.md`, and `.specify/memory/pi-agent.md`; `SPEC.md`
  is not expected to change because `complete(...)` already exists and this feature only consumes
  the existing instruction contract
- [x] Required validation commands listed: `make clean build 2>&1`, `make check-logs`,
  `make check-docs`, targeted workflow method tests, `make ar_executable_tests 2>&1`,
  `make ar_complete_instruction_evaluator_tests 2>&1`, `make ar_local_completion_tests 2>&1`, and
  `make sanitize-tests 2>&1`

## Project Structure

### Documentation (this feature)

```text
specs/002-let-build-workflow-coordinator/
├── plan.md
├── research.md
├── data-model.md
├── quickstart.md
├── contracts/
│   ├── README.md
│   ├── workflow-definition-schema.md
│   └── workflow-runtime-messages.md
└── tasks.md
```

### Source Code (repository root)

```text
methods/
├── README.md                              # update bundled method inventory
├── bootstrap-1.0.0.method                 # update boot demo orchestration
├── bootstrap.md                           # update boot demo docs
├── bootstrap_tests.c                      # update for workflow demo + startup failure cases
├── workflow-coordinator-1.0.0.method      # new boot-time orchestrator
├── workflow-coordinator-1.0.0.md          # new
├── workflow_coordinator_tests.c           # new
├── workflow-item-1.0.0.method             # new per-item state machine
├── workflow-item-1.0.0.md                 # new
├── workflow_item_tests.c                  # new
├── workflow-reporter-1.0.0.method         # new log/report method
├── workflow-reporter-1.0.0.md             # new
├── workflow_reporter_tests.c              # new
├── workflow-definition-1.0.0.method       # new YAML reader / `complete(...)` evaluator
├── workflow-definition-1.0.0.md           # new
└── workflow_definition_tests.c            # new

workflows/
├── default-workflow.yaml                  # bundled default workflow definition with `complete(...)`
└── test-workflow.yaml                     # alternate YAML definition for deterministic validation

modules/
└── ar_executable_tests.c                  # update boot-demo expectations only

README.md                                  # update executable demo description
CHANGELOG.md                               # required on implementation commit
.specify/memory/pi-agent.md                # updated with durable workflow-planning context
```

**Structure Decision**: Keep the executable boot path and method-loading behavior exactly where they
already live, but replace the old chat-session demo with a methods-only workflow pattern whose
workflow-definition agent both reads the YAML schema and owns `complete(...)`-driven transition
logic. The YAML contract is first-release parse-friendly rather than fully general, so methods can
extract required values without a new parser module. The coordinator explicitly waits for
"definition ready / dependency ready" before seeding the item, the default-workflow path reaches a
terminal outcome during the MVP story when dependencies are ready, startup dependency failures are
reported without work-item summary fields, and method tests use the existing
`AGERUN_COMPLETE_RUNNER` override pattern for deterministic outcome/reason fixtures.

## Complexity Tracking

No constitution violations are anticipated at plan time.
