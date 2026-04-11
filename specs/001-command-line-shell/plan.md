# Implementation Plan: Command-Line Shell

**Branch**: `001-command-line-shell` | **Date**: 2026-04-11 | **Spec**: [spec.md](./spec.md)
**Input**: Feature specification from `specs/001-command-line-shell/spec.md`

**Note**: This plan is filled in by the native `/spec plan` workflow.

## Summary

Add a new user-facing `arsh` shell command whose startup path instantiates a shell session module
and one session-specific shell delegate. The delegate wraps each entered input string into an input
envelope map, unwraps returned output envelopes back into terminal strings, and forwards traffic to
one session-scoped receiving agent started from the built-in `shell` method. The `shell` method
interprets the restricted one-line instruction subset (`spawn(...)`, `send(...)`, assignment, and
assigned `spawn`/`send` forms), while the instantiable shell session module creates and holds the
shell session instance plus its separate memory map.

## Technical Context

**Language/Version**: C17 modules, Zig 0.14.1 support modules, and AgeRun method language
**Primary Dependencies**: `ar_system`, `ar_agency`, `ar_delegation`, `ar_delegate`, `ar_methodology`, `ar_executable`, Make, gcc-13/clang, Zig 0.14.1
**Storage**: In-memory system state, session-scoped shell session module memory map, existing persisted methods/agents where already supported by the runtime
**Testing**: `make ar_executable_tests 2>&1`, `make ar_system_tests 2>&1`, new `make ar_shell_delegate_tests 2>&1`, new `make ar_shell_session_tests 2>&1`, new `make shell_tests 2>&1`, `make sanitize-tests 2>&1`, `make check-docs`, `make clean build 2>&1`, `make check-logs`
**Target Platform**: macOS and Linux terminal CLI
**Project Type**: Message-driven runtime feature spanning executable startup, a session-specific shell delegate, a shell session module, a built-in shell method, and documentation
**Performance Goals**: Human-interactive shell startup and per-line handoff consistent with [spec.md](./spec.md) SC-001 and SC-002; no bulk throughput target for the first release
**Constraints**: No generic stdio delegate; one session-specific shell delegate per shell session owns envelope wrap/unwrap and receiving-agent targeting; built-in `shell` method owns `spawn`/`send`/assignment semantics; one-line restricted syntax only; no map literals; no nested function calls; shell session state separate from receiving agent memory; shell/session exchange only via messages; user-facing command name `arsh`; zero memory leaks; Make-target-only validation
**Scale/Scope**: Initial implementation supports one local shell session per process, one session-specific shell delegate, one dedicated receiving agent running `shell-1.0.0`, and one shell session module instance per `arsh` invocation, with multiple spawned runtime agents reachable from that session

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

- [x] KB consultation recorded: `AGENTS.md`, `kb/README.md`, `kb/separation-of-concerns-principle.md`, `kb/design-for-change-principle.md`, `kb/system-message-flow-architecture.md`, `kb/shared-context-architecture-pattern.md`, `kb/specification-consistency-maintenance.md`, `kb/agerun-method-language-nesting-constraint.md`
- [x] TDD approach defined: add failing executable, shell delegate, shell session module, and `shell` method tests before implementation tasks; keep RED -> GREEN -> REFACTOR sequencing in `/spec tasks`
- [x] Convention impact reviewed: new C modules follow `ar_<module>` naming, ownership prefixes remain mandatory, shell method uses real AgeRun syntax limits, and validation runs only through make targets
- [x] Specification and documentation impact identified: `SPEC.md`, `README.md`, `modules/ar_executable.md`, new module docs for shell transport/state, new method docs for `methods/shell-1.0.0.md`, and feature docs under `specs/001-command-line-shell/`
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
├── ar_shell_delegate.h           # new session-specific shell delegate
├── ar_shell_delegate.c           # new
├── ar_shell_delegate_tests.c     # new
├── ar_shell_delegate.md          # new
├── ar_shell_session.h            # new
├── ar_shell_session.c            # new
├── ar_shell_session_tests.c      # new
├── ar_shell_session.md           # new
├── ar_executable.h               # update for `arsh` entry handling
├── ar_executable.c               # update for shell startup path
├── ar_executable_tests.c         # update for `arsh` behavior
└── ar_system_tests.c             # update if shell session/delegate wiring needs system coverage

methods/
├── shell-1.0.0.method            # new built-in shell method
├── shell-1.0.0.md                # new method documentation
└── shell_tests.c                 # new method tests

README.md                         # update with `arsh` usage
SPEC.md                           # update with shell command/runtime syntax contract
Makefile                          # update if separate `arsh` binary/target is required
```

**Structure Decision**: Introduce one session-specific delegate module (`ar_shell_delegate`) and
one session-state module (`ar_shell_session`) so envelope transport, shell-session lifecycle/state,
and shell method behavior evolve independently. Keep the user-facing shell semantics in a built-in
method (`methods/shell-1.0.0.method`) executed by an auto-created receiving agent, and let the
`arsh` executable startup path instantiate the shell session module that creates and holds the
shell session instance without removing the current demo executable behavior.

## Complexity Tracking

No constitution violations are anticipated at plan time.
