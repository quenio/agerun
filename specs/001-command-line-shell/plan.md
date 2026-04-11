# Implementation Plan: Command-Line Shell

**Branch**: `001-command-line-shell` | **Date**: 2026-04-11 | **Spec**: [spec.md](./spec.md)
**Input**: Feature specification from `specs/001-command-line-shell/spec.md`

**Note**: This plan is filled in by the native `/spec plan` workflow.

## Summary

Add a new user-facing `arsh` shell command with a thin executable entrypoint that delegates shell
startup and session lifecycle to a dedicated non-instantiable `ar_shell` module. That module owns
shell session instances and their session memory maps, while an instantiable `ar_shell_session`
module remains the runtime-facing message boundary used by the built-in `shell` method. The
session-specific `ar_shell_delegate` wraps each entered input string into an input envelope map,
unwraps returned output envelopes back into terminal strings, and forwards traffic to one
session-scoped receiving agent started from the built-in `shell` method. The `shell` method
interprets the restricted one-line instruction subset (`spawn(...)`, `send(...)`, assignment, and
assigned `spawn`/`send` forms), while shell session state remains outside the receiving agent and
is owned by `ar_shell` rather than being directly handled by `ar_shell_session`.

## Technical Context

**Language/Version**: C17 modules, Zig 0.14.1 support modules, and AgeRun method language
**Primary Dependencies**: `ar_system`, `ar_agency`, `ar_delegation`, `ar_delegate`, `ar_methodology`, `ar_executable`, Make, gcc-13/clang, Zig 0.14.1
**Storage**: In-memory system state, shell session instances and memory maps owned by `ar_shell`, runtime mediation through `ar_shell_session`, existing persisted methods/agents where already supported by the runtime
**Testing**: `make ar_executable_tests 2>&1`, `make ar_system_tests 2>&1`, new `make ar_shell_tests 2>&1`, new `make ar_shell_session_tests 2>&1`, new `make ar_shell_delegate_tests 2>&1`, new `make shell_tests 2>&1`, `make sanitize-tests 2>&1`, `make check-docs`, `make clean build 2>&1`, `make check-logs`
**Target Platform**: macOS and Linux terminal CLI
**Project Type**: Message-driven runtime feature spanning a thin executable entrypoint, a non-instantiable shell module, an instantiable shell session module, a session-specific shell delegate, a built-in shell method, and documentation
**Performance Goals**: Human-interactive shell startup and per-line handoff consistent with [spec.md](./spec.md) SC-001 and SC-002; no bulk throughput target for the first release
**Constraints**: No generic stdio delegate; keep `arsh` entrypoint thin; provide a unit-testable non-instantiable `ar_shell` module that owns shell session lifecycle and memory; retain an instantiable `ar_shell_session` runtime module as the shell method's message boundary; the session map must not be directly handled by `ar_shell_session`; one session-specific shell delegate per shell session owns envelope wrap/unwrap and receiving-agent targeting; built-in `shell` method owns `spawn`/`send`/assignment semantics; one-line restricted syntax only; no map literals; no nested function calls; shell session state separate from receiving agent memory; shell/session exchange only via messages; user-facing command name `arsh`; zero memory leaks; Make-target-only validation
**Scale/Scope**: Initial implementation supports one local shell session per process, one `ar_shell` module managing one active shell session for `arsh`, one `ar_shell_session` instance mediating runtime access to that session, one session-specific shell delegate, one dedicated receiving agent running `shell-1.0.0`, and multiple spawned runtime agents reachable from that session

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

- [x] KB consultation recorded: `AGENTS.md`, `kb/README.md`, `kb/separation-of-concerns-principle.md`, `kb/design-for-change-principle.md`, `kb/system-message-flow-architecture.md`, `kb/shared-context-architecture-pattern.md`, `kb/specification-consistency-maintenance.md`, `kb/module-development-patterns.md`, `kb/agerun-method-language-nesting-constraint.md`
- [x] TDD approach defined: add failing executable, `ar_shell`, `ar_shell_session`, `ar_shell_delegate`, and `shell` method tests before implementation tasks; keep RED -> GREEN -> REFACTOR sequencing in `/spec tasks`
- [x] Convention impact reviewed: new C modules follow `ar_<module>` naming, ownership prefixes remain mandatory, shell method uses real AgeRun syntax limits, and validation runs only through make targets
- [x] Specification and documentation impact identified: `SPEC.md`, `README.md`, `modules/ar_executable.md`, new module docs for shell lifecycle/session/transport, new method docs for `methods/shell-1.0.0.md`, and feature docs under `specs/001-command-line-shell/`
- [x] Required validation commands listed: `make clean build 2>&1`, `make check-logs`, `make check-docs`, targeted module/method tests, and `make sanitize-tests 2>&1` when behavior changes land

## Project Structure

### Documentation (this feature)

```text
specs/001-command-line-shell/
├── plan.md
├── research.md
├── data-model.md
├── quickstart.md
├── contracts/
│   ├── arsh-cli.md
│   └── shell-session-protocol.md
└── tasks.md
```

### Source Code (repository root)

```text
modules/
├── ar_shell.h                    # new non-instantiable shell module
├── ar_shell.c                    # new
├── ar_shell_tests.c              # new
├── ar_shell.md                   # new
├── ar_shell_session.h            # new instantiable shell session module
├── ar_shell_session.c            # new
├── ar_shell_session_tests.c      # new
├── ar_shell_session.md           # new
├── ar_shell_delegate.h           # new session-specific shell delegate
├── ar_shell_delegate.c           # new
├── ar_shell_delegate_tests.c     # new
├── ar_shell_delegate.md          # new
├── ar_executable.h               # update for `arsh` entry handling
├── ar_executable.c               # update for thin shell startup path
├── ar_executable_tests.c         # update for `arsh` behavior
└── ar_system_tests.c             # update if shell/system wiring needs coverage

methods/
├── shell-1.0.0.method            # new built-in shell method
├── shell-1.0.0.md                # new method documentation
└── shell_tests.c                 # new method tests

README.md                         # update with `arsh` usage
SPEC.md                           # update with shell command/runtime syntax contract
Makefile                          # update if separate `arsh` binary/target is required
```

**Structure Decision**: Introduce one non-instantiable shell module (`ar_shell`) that owns shell
session lifecycle/state, one instantiable shell session module (`ar_shell_session`) that mediates
runtime access to shell sessions without directly handling the session map, and one session-
specific delegate module (`ar_shell_delegate`) for envelope transport. Keep the user-facing shell
semantics in a built-in method (`methods/shell-1.0.0.method`) executed by an auto-created
receiving agent, and let the `arsh` executable startup path call into `ar_shell` rather than hold
session logic directly.

## Complexity Tracking

No constitution violations are anticipated at plan time.
