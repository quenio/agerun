# Implementation Plan: Workflow Coordinator

**Branch**: `002-let-build-workflow-coordinator` | **Date**: 2026-04-15 | **Spec**: [spec.md](./spec.md)
**Input**: Feature specification from `specs/002-let-build-workflow-coordinator/spec.md`

**Note**: This plan is filled in by the native `/spec plan` workflow.

## Summary

Replace the current boot-time chat-session demo with a methods-only workflow coordinator demo. Keep
all feature logic inside new AgeRun method assets loaded from `methods/`, have
`bootstrap-1.0.0.method` spawn a `workflow-coordinator` agent on fresh executable runs, and let the
coordinator spawn a generic workflow-definition agent, a workflow-item agent, and a
workflow-reporter agent. The active workflow definition lives in a YAML file read through the file
delegate rather than in a `.method` file, and tests swap the definition by pointing the same method
logic at alternate YAML files. The reusable workflow item method stays stateful, the generic
workflow-definition method owns YAML-backed transition validation and `advance` / `stay` / `reject`
decisions, and the reporter method emits the user-visible progress and final summary.

## Technical Context

**Language/Version**: AgeRun method language for feature logic; existing C17/Zig runtime only as the execution host and test harness
**Primary Dependencies**: Existing boot/executable flow in `modules/ar_executable.c`, existing method-loading from `methods/`, existing file-delegate runtime contract (`-100`), AgeRun method instructions (`parse`, `build`, `spawn`, `send`, `if`), `ar_method_fixture`, Make, gcc-13/clang, Zig 0.14.1
**Storage**: Agent memory maps for coordinator/item/reporter state, existing persisted methods/agents, and workflow-definition data read from YAML files via the file delegate
**Testing**: `make workflow_definition_default_tests 2>&1`, `make workflow_item_tests 2>&1`, `make workflow_coordinator_tests 2>&1`, `make workflow_reporter_tests 2>&1`, `make bootstrap_tests 2>&1`, `make ar_executable_tests 2>&1`, `make sanitize-tests 2>&1`, `make check-docs`, `make clean build 2>&1`, `make check-logs`
**Target Platform**: macOS and Linux CLI runtime through the existing `agerun` executable
**Project Type**: Message-driven runtime feature implemented entirely as method assets plus documentation and test updates
**Performance Goals**: Fresh boot demo completes in one `ar_system__process_all_messages()` run, reaches a terminal outcome in no more than ~16 processed messages for the bundled default workflow, and emits at least four visible lifecycle checkpoints before the final summary
**Constraints**: No shell-driven UX; no new C feature modules for coordinator logic; all workflow behavior lives in AgeRun methods; launch from `bootstrap`; workflow definition stays in YAML, not `.method`; agents read the definition through the file delegate; keep the existing persistence rule that restored agents skip fresh demo creation; no new runtime instructions; no map literals; no nested function calls; follow parse/build-friendly message shapes; log through the existing runtime log delegate path; zero memory leaks; make-target-only validation
**Scale/Scope**: One fresh-run demo item, one workflow coordinator agent, one generic workflow-definition agent, one workflow reporter agent, and one workflow item agent for the default demo; one bundled default YAML definition file plus one alternate YAML definition file for tests; updated executable tests verify the boot-driven demo and restored-agent skip behavior

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

- [x] KB consultation recorded: `AGENTS.md`, `kb/README.md`, `kb/kb-consultation-before-planning-requirement.md`, `kb/user-feedback-as-qa.md`, `kb/assumption-verification-before-action.md`, `kb/tdd-iteration-planning-pattern.md`, `kb/multi-session-tdd-planning.md`, `kb/plan-verification-checklist.md`, `kb/report-driven-task-planning.md`, `kb/agerun-method-language-nesting-constraint.md`, `kb/method-test-template.md`, `kb/message-processing-loop-pattern.md`, `kb/specification-consistency-maintenance.md`
- [x] TDD approach defined: add failing method tests for the workflow definition, item, coordinator, reporter, and bootstrap flow before changing any method asset or executable expectation; keep RED -> GREEN -> REFACTOR sequencing in `/spec tasks`
- [x] Convention impact reviewed: new method assets stay in `methods/` with `<name>-<version>.method` plus matching docs/tests, workflow YAML files stay separate from `.method` assets, boot behavior remains in `bootstrap`, feature logic stays out of new C modules, make targets remain the only validation entry points, and method bodies respect no-nested-function and no-map-literal constraints
- [x] Specification and documentation impact identified: `specs/002-let-build-workflow-coordinator/*`, `README.md`, `methods/README.md`, `methods/bootstrap.md`, new workflow method docs, YAML definition-file documentation, `CHANGELOG.md`, and `.specify/memory/pi-agent.md`; `SPEC.md` is not expected to change because no language/runtime syntax is being added
- [x] Required validation commands listed: `make clean build 2>&1`, `make check-logs`, `make check-docs`, targeted method tests, `make ar_executable_tests 2>&1`, and `make sanitize-tests 2>&1`

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
├── bootstrap_tests.c                      # update for workflow demo
├── workflow-coordinator-1.0.0.method      # new boot-time orchestrator
├── workflow-coordinator-1.0.0.md          # new
├── workflow_coordinator_tests.c           # new
├── workflow-item-1.0.0.method             # new per-item state machine
├── workflow-item-1.0.0.md                 # new
├── workflow_item_tests.c                  # new
├── workflow-reporter-1.0.0.method         # new log/report method
├── workflow-reporter-1.0.0.md             # new
├── workflow_reporter_tests.c              # new
├── workflow-definition-1.0.0.method       # new generic YAML reader/evaluator
├── workflow-definition-1.0.0.md           # new
└── workflow_definition_tests.c            # new

workflows/
├── default-workflow.yaml                  # bundled default workflow definition
└── test-workflow.yaml                     # alternate YAML definition for validation tests

modules/
└── ar_executable_tests.c                  # update boot-demo expectations only

README.md                                  # update executable demo description
CHANGELOG.md                               # required on implementation commit
.specify/memory/pi-agent.md                # updated with durable workflow-planning context
```

**Structure Decision**: Keep the executable boot path and method-loading behavior exactly where they
already live, but replace the old chat-session demo with a methods-only workflow pattern. The only
feature logic lives in the new workflow method assets plus the updated `bootstrap` method, while the
workflow schema itself lives in YAML files under the top-level `workflows/` directory and is read
through the file delegate. C files remain test/documentation touch points, not the implementation
home for the workflow feature.

## Complexity Tracking

No constitution violations are anticipated at plan time.
