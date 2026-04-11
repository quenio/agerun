# Implementation Plan: Command-Line Shell

**Branch**: `001-command-line-shell` | **Date**: 2026-04-11 | **Spec**: [spec.md](./spec.md)
**Input**: Feature specification from `specs/001-command-line-shell/spec.md`

**Note**: This plan is filled in by the native `/spec plan` workflow.

## Summary

Add a new user-facing `arsh` shell command that runs a transport-only stdio delegate, auto-creates
one session-scoped receiving agent from a built-in shell method, and routes each entered line into
AgeRun as an input envelope. The shell method interprets a restricted one-line instruction subset
(`spawn(...)`, `send(...)`, assignment, and assigned `spawn`/`send` forms), while a separate
instantiable shell session module owns shell session state in its own memory map and exchanges that
state with the shell method only through messages.

## Technical Context

**Language/Version**: C17 modules, Zig 0.14.1 support modules, and AgeRun method language
**Primary Dependencies**: `ar_system`, `ar_agency`, `ar_delegation`, `ar_delegate`, `ar_methodology`, `ar_executable`, Make, gcc-13/clang, Zig 0.14.1
**Storage**: In-memory system state, session-scoped shell session module memory map, existing persisted methods/agents where already supported by the runtime
**Testing**: `make ar_executable_tests 2>&1`, `make ar_system_tests 2>&1`, new `make ar_stdio_delegate_tests 2>&1`, new `make ar_shell_session_tests 2>&1`, new `make arsh_tests 2>&1`, `make sanitize-tests 2>&1`, `make check-docs`, `make clean build 2>&1`, `make check-logs`
**Target Platform**: macOS and Linux terminal CLI
**Project Type**: Message-driven runtime feature spanning executable, delegate, shell session module, built-in method, and documentation
**Performance Goals**: Human-interactive shell startup and per-line handoff consistent with [spec.md](./spec.md) SC-001 and SC-002; no bulk throughput target for the first release
**Constraints**: Transport-only delegate; built-in shell method owns `spawn`/`send`/assignment semantics; one-line restricted syntax only; no map literals; no nested function calls; shell session state separate from receiving agent memory; shell/session exchange only via messages; user-facing command name `arsh`; zero memory leaks; Make-target-only validation
**Scale/Scope**: Initial implementation supports one local shell session per process, one dedicated receiving agent and one shell session module per `arsh` invocation, and multiple spawned runtime agents reachable from that session

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

- [x] KB consultation recorded: `AGENTS.md`, `kb/README.md`, `kb/separation-of-concerns-principle.md`, `kb/design-for-change-principle.md`, `kb/system-message-flow-architecture.md`, `kb/shared-context-architecture-pattern.md`, `kb/agerun-method-language-nesting-constraint.md`
- [x] TDD approach defined: add failing executable, delegate, shell session module, and method tests before implementation tasks; keep RED -> GREEN -> REFACTOR sequencing in `/spec tasks`
- [x] Convention impact reviewed: new C modules follow `ar_<module>` naming, ownership prefixes remain mandatory, shell method uses real AgeRun syntax limits, and validation runs only through make targets
- [x] Specification and documentation impact identified: `SPEC.md`, `README.md`, `modules/ar_executable.md`, new module docs for shell transport/state, new method docs for `methods/arsh-1.0.0.md`, and feature docs under `specs/001-command-line-shell/`
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
├── ar_stdio_delegate.h           # new
├── ar_stdio_delegate.c           # new
├── ar_stdio_delegate_tests.c     # new
├── ar_stdio_delegate.md          # new
├── ar_shell_session.h            # new
├── ar_shell_session.c            # new
├── ar_shell_session_tests.c      # new
├── ar_shell_session.md           # new
├── ar_executable.h               # update for `arsh` entry handling
├── ar_executable.c               # update for shell startup path
├── ar_executable_tests.c         # update for `arsh` behavior
└── ar_system_tests.c             # update if shell session/delegate wiring needs system coverage

methods/
├── arsh-1.0.0.method             # new built-in shell method
├── arsh-1.0.0.md                 # new method documentation
└── arsh_tests.c                  # new method tests

README.md                         # update with `arsh` usage
SPEC.md                           # update with shell command/runtime syntax contract
Makefile                          # update if separate `arsh` binary/target is required
```

**Structure Decision**: Introduce one transport module (`ar_stdio_delegate`) and one session-state
module (`ar_shell_session`) so stdin/stdout handling, shell-session storage, and shell method
behavior evolve independently. Keep the user-facing shell semantics in a built-in method
(`methods/arsh-1.0.0.method`) executed by an auto-created receiving agent, and integrate `arsh`
startup through the executable/build path without removing the current demo executable behavior.

## Complexity Tracking

No constitution violations are anticipated at plan time.
