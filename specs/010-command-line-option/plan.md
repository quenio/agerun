# Implementation Plan: Executable Persistence Disable Option

**Branch**: `010-command-line-option` | **Date**: 2026-04-22 | **Spec**: [spec.md](./spec.md)
**Input**: Feature specification from `specs/010-command-line-option/spec.md`

**Note**: This plan is filled in by the native `/spec plan` workflow.

## Summary

Add an optional persistence-disable mode to the main `agerun` executable so operators can run a
fresh, non-persistent session without manually deleting `agerun.methodology` or `agerun.agency`.
Implement the feature as one explicit executable flag that suppresses both persisted methodology and
persisted agency loading/saving for the current run, treat the run as a fresh-start startup path,
leave existing persisted files untouched, and expose the same behavior through `make run-exec` so
repository workflows can validate it without bypassing Make.

## Technical Context

**Language/Version**: C11 runtime code in `modules/ar_executable.c`, `modules/ar_executable_fixture.c`,
`modules/ar_executable_tests.c`, and the existing Zig-backed runtime modules already linked by the
project
**Primary Dependencies**: Existing AgeRun system startup flow (`ar_system__create`,
`ar_agency__load_agents`, `ar_methodology__load_methods`, `ar_system__init`,
`ar_agency__save_agents`, `ar_methodology__save_methods`), Make, gcc-13/clang, Zig 0.14.1,
persisted `agerun.methodology` / `agerun.agency` files, and the executable test fixture
**Storage**: In-memory startup selection state during process launch, persisted methods in
`agerun.methodology`, persisted agents in `agerun.agency`, and one boolean operator intent carried
from `make run-exec` into the executable CLI
**Testing**: `make ar_executable_tests 2>&1`, `make sanitize-tests 2>&1`, `make check-docs`,
`make check-naming 2>&1`, `make clean build 2>&1`, `make check-logs`
**Target Platform**: macOS and Linux command-line runtime through the existing `agerun` executable
**Project Type**: Runtime startup/shutdown behavior enhancement plus top-level build-file interface
and operator-facing documentation update
**Performance Goals**: No new throughput target. Persistence-disabled runs must still execute a
single startup/shutdown path, skip disabled load/save file I/O entirely, and avoid introducing extra
message-processing passes or retries.
**Constraints**: Zero memory leaks; Make-target-only validation; no helper shell scripts; preserve
existing persistence-enabled behavior when the option is omitted; preserve current persisted files
instead of deleting them; keep boot override behavior consistent with fresh-start semantics when
persistence is disabled; use repository-relative markdown links; update docs/specs in sync with
runtime behavior
**Scale/Scope**: One executable CLI flag, one `make run-exec` passthrough variable, one executable
fixture interface update, targeted executable integration tests for startup/save/interaction cases,
and documentation/spec updates across README/SPEC/module docs and `.specify` planning artifacts; no
method-language changes and no changes to `arsh`

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

- [x] KB consultation recorded: `AGENTS.md`, `kb/README.md`,
  `kb/kb-consultation-before-planning-requirement.md`, `kb/make-only-test-execution.md`,
  `kb/specification-consistency-maintenance.md`,
  `kb/documentation-update-cascade-pattern.md`, and
  `kb/persistence-simplification-through-instantiation.md`
- [x] TDD approach defined: add failing executable integration tests first for skipped methodology
  load, skipped agency load, skipped save behavior, default behavior without the option, and fresh
  startup interaction with existing boot selection; keep RED -> GREEN -> REFACTOR sequencing in
  `/spec tasks`
- [x] Convention impact reviewed: keep startup option parsing and persistence branching in
  `modules/ar_executable.c`; preserve ownership prefixes and cleanup rules in any fixture/helper
  changes; use Make targets for validation; expose one explicit Make passthrough variable instead of
  a parallel target; keep docs/specs aligned with the module -> project -> spec cascade
- [x] Specification and documentation impact identified: `specs/010-command-line-option/*`,
  `README.md`, `SPEC.md`, `modules/ar_executable.md`, `modules/ar_executable_fixture.h`,
  `modules/ar_executable_tests.c`, the top-level build file, and `.specify/memory/pi-agent.md`
- [x] Required validation commands listed: `make ar_executable_tests 2>&1`,
  `make sanitize-tests 2>&1`, `make check-docs`, `make check-naming 2>&1`,
  `make clean build 2>&1`, and `make check-logs`

**Post-design re-check**: Pass. Phase 0 research and Phase 1 design resolved the CLI/Make naming,
interaction, and persistence-scope choices without introducing constitution violations.

## Project Structure

### Documentation (this feature)

```text
specs/010-command-line-option/
├── plan.md
├── research.md
├── data-model.md
├── quickstart.md
├── contracts/
│   ├── README.md
│   ├── executable-persistence-cli.md
│   └── run-exec-persistence-mode.md
└── tasks.md
```

### Source Code (repository root)

```text
modules/
├── ar_executable.c                 # parse no-persistence flag and gate load/save behavior
├── ar_executable.md                # document persistence-disabled startup/shutdown behavior
├── ar_executable_fixture.c         # pass optional no-persistence intent through make run-exec
├── ar_executable_fixture.h         # expose fixture support for no-persistence runs
└── ar_executable_tests.c           # RED/GREEN coverage for skipped load/save and interactions

top-level build file               # allow `make run-exec NO_PERSISTENCE=1`
README.md                           # document operator-facing no-persistence usage
SPEC.md                             # document executable startup/shutdown persistence contract
.specify/memory/pi-agent.md         # preserve durable planning context
CHANGELOG.md                        # required when implementation is committed
```

**Structure Decision**: Keep the feature entirely inside the existing executable layer. Add one
explicit persistence-disable flag to the `agerun` CLI parser, derive a single in-memory execution
mode that suppresses both methodology and agency load/save branches, and continue using the current
fresh-start boot path when persisted agents are not restored because persistence was intentionally
disabled. Expose the same intent through one `run-exec` Make variable so repository workflows stay
Make-driven, and extend the executable fixture rather than adding ad hoc scripts or direct binary
launches in tests.

## Complexity Tracking

No constitution violations are anticipated at plan time.
