# Implementation Plan: Command-Line Shell

**Branch**: `001-command-line-shell` | **Date**: 2026-04-11 | **Spec**: [spec.md](./spec.md)
**Input**: Feature specification from `specs/001-command-line-shell/spec.md`

**Note**: This plan is filled in by the native `/spec plan` workflow.

## Summary

Add a new user-facing `arsh` executable implemented by the `ar_shell` module. The instantiable
`ar_shell` module wraps the AgeRun system and manages shell session instances, while an
instantiable `ar_shell_session` module owns the per-session state and lifecycle used by the
built-in `shell` method. The session-specific `ar_shell_delegate` reads terminal input into the
required input map instances, routes them to one session-scoped agent running the built-in `shell`
method, and calls back into `ar_shell_session` when a message is returned by that agent so the
session can render shell-visible output. The `shell` method interprets the restricted one-line
instruction subset (`spawn(...)`, `send(...)`, assignment, and assigned `spawn`/`send` forms),
while shell session state remains outside the running agent and is owned by `ar_shell_session`
under `ar_shell` management.

## Technical Context

**Language/Version**: C17 modules, Zig 0.14.1 support modules, and AgeRun method language
**Primary Dependencies**: `ar_shell`, `ar_shell_session`, `ar_shell_delegate`, `ar_system`, `ar_agency`, `ar_delegation`, `ar_delegate`, `ar_methodology`, Make, gcc-13/clang, Zig 0.14.1
**Storage**: In-memory system state, shell session instances managed by instantiable `ar_shell`, per-session memory maps and lifecycle owned by `ar_shell_session`, existing persisted methods/agents where already supported by the runtime
**Testing**: `make ar_shell_tests 2>&1`, `make ar_shell_session_tests 2>&1`, `make ar_shell_delegate_tests 2>&1`, `make shell_tests 2>&1`, `make ar_system_tests 2>&1`, `make sanitize-tests 2>&1`, `make check-docs`, `make clean build 2>&1`, `make check-logs`
**Target Platform**: macOS and Linux terminal CLI
**Project Type**: Message-driven runtime feature spanning a dedicated shell executable module, an instantiable shell module, an instantiable shell session module, a session-specific shell delegate, a built-in shell method, and documentation
**Performance Goals**: Human-interactive shell startup and per-line handoff consistent with [spec.md](./spec.md) SC-001 and SC-002; no bulk throughput target for the first release
**Constraints**: No generic stdio delegate; do not implement `arsh` in `ar_executable`; implement the `arsh` executable in `ar_shell`; provide a unit-testable instantiable `ar_shell` module that manages shell sessions and owns the wrapped system; retain an instantiable `ar_shell_session` runtime module as the shell method's message boundary and per-session state/lifecycle owner; one session-specific shell delegate per shell session reads input into the required map shape, targets the shell agent, and calls back into the session when messages return; built-in `shell` method owns `spawn`/`send`/assignment semantics; one-line restricted syntax only; no map literals; no nested function calls; shell session state separate from agent memory; shell/session exchange only via messages; user-facing command name `arsh`; zero memory leaks; Make-target-only validation
**Scale/Scope**: Initial implementation supports one local shell session per process, one `ar_shell` instance backing the `arsh` executable and managing one active shell session, one `ar_shell_session` instance owning that session's state and lifecycle, one session-specific shell delegate, one dedicated agent instance running `shell-1.0.0`, and multiple spawned runtime agents reachable from that session

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

- [x] KB consultation recorded: `AGENTS.md`, `kb/README.md`, `kb/separation-of-concerns-principle.md`, `kb/design-for-change-principle.md`, `kb/system-message-flow-architecture.md`, `kb/shared-context-architecture-pattern.md`, `kb/specification-consistency-maintenance.md`, `kb/module-development-patterns.md`, `kb/agerun-method-language-nesting-constraint.md`
- [x] TDD approach defined: add failing `ar_shell`, `ar_shell_session`, `ar_shell_delegate`, and `shell` method tests before implementation tasks; keep RED -> GREEN -> REFACTOR sequencing in `/spec tasks`
- [x] Convention impact reviewed: new C modules follow `ar_<module>` naming, ownership prefixes remain mandatory, shell method uses real AgeRun syntax limits, and validation runs only through make targets
- [x] Specification and documentation impact identified: `SPEC.md`, `README.md`, `modules/ar_shell.md`, `modules/ar_shell_session.md`, `modules/ar_shell_delegate.md`, new method docs for `methods/shell-1.0.0.md`, and feature docs under `specs/001-command-line-shell/`
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
├── ar_shell.h                    # shell executable module backing `arsh`
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
└── ar_system_tests.c             # update if shell/system wiring needs coverage

methods/
├── shell-1.0.0.method            # new built-in shell method
├── shell-1.0.0.md                # new method documentation
└── shell_tests.c                 # new method tests

README.md                         # update with `arsh` usage
SPEC.md                           # update with shell command/runtime syntax contract
Makefile                          # update for the `arsh` executable target
```

**Structure Decision**: Implement the dedicated `arsh` executable in `ar_shell` instead of
routing shell behavior through `ar_executable`. Keep `ar_shell` as the executable-owning module for
shell session management, keep `ar_shell_session` as the instantiable runtime module that owns
per-session state and lifecycle while mediating shell access through messages, and keep
`ar_shell_delegate` as the session-specific input transport plus callback bridge back into the
session. User-facing shell semantics remain in the built-in method `methods/shell-1.0.0.method`
executed by an auto-created agent instance.

## Complexity Tracking

No constitution violations are anticipated at plan time.
