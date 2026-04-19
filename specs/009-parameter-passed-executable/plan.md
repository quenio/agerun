# Implementation Plan: Executable Boot Method Override

**Branch**: `009-parameter-passed-executable` | **Date**: 2026-04-18 | **Spec**: [spec.md](./spec.md)
**Input**: Feature specification from `specs/009-parameter-passed-executable/spec.md`

**Note**: This plan is filled in by the native `/spec plan` workflow.

## Summary

Add an optional boot-method override to the main `agerun` executable so a fresh runtime session can
start with a requested method instead of the default `bootstrap` method. Implement the override as a
single named executable parameter whose value uses the same combined method identifier convention as
the `methods/` folder (for example, `echo-1.0.0`), keep the existing `"__boot__"` startup message
contract, treat restored agents as higher priority than any fresh-start override, and expose the
same override through the existing `make run-exec` target so repository workflows can request
alternate boot methods without running the binary manually.

## Technical Context

**Language/Version**: C11 runtime code in `modules/ar_executable.c` and related fixture/tests,
plus the existing Zig-backed runtime modules already linked by the project
**Primary Dependencies**: Existing AgeRun system startup flow (`ar_system__init`,
`ar_agency__load_agents`, `ar_agency__send_to_agent`), Make, gcc-13/clang, Zig 0.14.1, persisted
`agerun.methodology` / `agerun.agency` files, and the existing executable test fixture
**Storage**: In-memory startup selection state during process launch, persisted methods in
`agerun.methodology`, persisted agents in `agerun.agency`, and a single combined boot-method
identifier expanded from `run-exec` into executable CLI arguments
**Testing**: `make ar_executable_tests 2>&1`, `make sanitize-tests 2>&1`, `make check-docs`,
`make clean build 2>&1`, `make check-logs`
**Target Platform**: macOS and Linux command-line runtime through the existing `agerun` executable
**Project Type**: Runtime startup behavior enhancement plus Makefile interface/documentation update
**Performance Goals**: No new throughput target. Startup must remain a single-boot path: exactly one
fresh boot agent on clean startup, zero fresh boot agents when persisted agents are restored, and no
extra startup passes introduced by the override path.
**Constraints**: Zero memory leaks; Make-target-only validation; no shell-script helpers; preserve
existing default startup behavior when no override is supplied; keep restored-agent precedence; do
not silently fall back on invalid overrides; use repository-relative markdown links; update docs and
specs in sync with runtime behavior; the override value must follow the `methods/` filename-stem
convention `<method-name>-<version>`
**Scale/Scope**: One executable CLI surface (`agerun`), one Makefile passthrough (`run-exec` with a
single `BOOT_METHOD` variable), one executable fixture abstraction update, targeted executable
integration tests, and startup documentation/spec updates; no new method language features and no
changes to `arsh`

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

- [x] KB consultation recorded: `AGENTS.md`, `kb/README.md`,
  `kb/kb-consultation-before-planning-requirement.md`,
  `kb/tdd-red-phase-assertion-requirement.md`, `kb/make-only-test-execution.md`,
  `kb/specification-consistency-maintenance.md`,
  `kb/documentation-update-cascade-pattern.md`, `kb/generic-make-targets-pattern.md`,
  `kb/requirement-precision-in-tdd.md`, `kb/user-feedback-as-qa.md`, and
  `kb/assumption-verification-before-action.md`
- [x] TDD approach defined: add failing executable integration tests first for valid override,
  default behavior without override, invalid-override failure, and restored-agent override skip;
  keep RED -> GREEN -> REFACTOR sequencing in `/spec tasks`; ensure RED failures are assertion
  failures rather than compilation errors when new helper interfaces are introduced
- [x] Convention impact reviewed: keep startup logic in `modules/ar_executable.c`; maintain
  ownership prefixes and explicit cleanup in any new helpers; use Make targets instead of direct
  binary execution for validation; expose `run-exec` override through one generic `BOOT_METHOD`
  variable rather than a duplicate target or second version variable; preserve repository-relative
  markdown links and current file layout
- [x] Specification and documentation impact identified: `specs/009-parameter-passed-executable/*`,
  `README.md`, `SPEC.md`, `modules/ar_executable.md`, the top-level build file help text, and
  `.specify/memory/pi-agent.md`; no method docs are expected to change because the feature only
  selects which existing method becomes the initial boot agent
- [x] Required validation commands listed: `make ar_executable_tests 2>&1`,
  `make sanitize-tests 2>&1`, `make check-docs`, `make clean build 2>&1`, and `make check-logs`

**Post-design re-check**: Pass. Phase 0 research and Phase 1 design resolved the CLI/Make
interface choices without introducing constitution violations.

## Project Structure

### Documentation (this feature)

```text
specs/009-parameter-passed-executable/
├── plan.md
├── research.md
├── data-model.md
├── quickstart.md
├── contracts/
│   ├── README.md
│   ├── executable-startup-cli.md
│   └── run-exec-target.md
└── tasks.md
```

### Source Code (repository root)

```text
modules/
├── ar_executable.c                 # parse/select combined boot identifier and report startup outcome
├── ar_executable.h                 # align executable entry-point interface if argc/argv are exposed
├── ar_executable.md                # document startup override behavior
├── ar_executable_fixture.c         # pass optional override identifier through make run-exec
├── ar_executable_fixture.h         # expose fixture support for override-aware runs
└── ar_executable_tests.c           # add RED/GREEN coverage for override cases

Makefile                            # allow `make run-exec BOOT_METHOD=<name-version>`
README.md                           # document operator-facing startup usage
SPEC.md                             # document executable startup contract
.specify/memory/pi-agent.md         # preserve durable planning context
CHANGELOG.md                        # required when implementation is committed
```

**Structure Decision**: Keep all runtime-selection logic inside the existing executable layer rather
than introducing a new startup module. Parse the optional combined identifier in
`modules/ar_executable.c`, split it into method name and version using the same final-hyphen
convention already implied by `methods/<name>-<version>.method`, resolve the selected boot
method/version before `ar_system__init`, and continue using the existing boot-message queueing flow
after the boot agent is created. Use one named CLI flag for the executable contract and one Make
variable that expands into that CLI flag for `run-exec`, so the external contract stays minimal and
matches the user's requirement to specify name and version together. Extend the executable fixture
instead of adding new one-off test shell commands, and update documentation in the module → project
→ spec/user-facing cascade required by project guidance.

## Complexity Tracking

No constitution violations are anticipated at plan time.
