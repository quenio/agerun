# AgeRun Development Guidelines

Auto-generated from the project constitution and repository context. Last updated: 2026-04-22

## Active Technologies

- C as the primary implementation language
- Zig 0.14.1 for integrated modules and build support
- GNU/BSD Make for builds, tests, sanitizers, and documentation validation
- Markdown-based project docs in AGENTS.md, SPEC.md, MMM.md, and kb/

## Project Structure

```text
modules/
methods/
kb/
reports/
scripts/
specs/
.specify/
```

## Commands

- `make build 2>&1`
- `make clean build 2>&1`
- `make check-logs`
- `make check-docs`
- `make sanitize-tests 2>&1`
- `make <module>_tests 2>&1`
- `/spec specify <feature description>`
- `/spec clarify [focus]`
- `/spec plan <technical context>`
- `/spec tasks [context]`
- `/spec implement [focus]`

## Code Style

- Search AGENTS.md and kb/README.md before planning or implementation
- Follow RED -> GREEN -> REFACTOR for behavior changes
- Use `own_`, `mut_`, and `ref_` ownership prefixes consistently
- Prefer `make` targets over direct compiler or script invocation
- Keep specs, docs, templates, and implementation synchronized

## Recent Changes

- Established the native pi `/spec constitution` for AgeRun as version 1.0.0
- Aligned spec templates with KB consultation, TDD, conventions, and evidence gates
- Standardized native `/spec` wording across the `.specify` workflow templates
- Re-planned `001-command-line-shell` so `arsh` is its own executable implemented by the instantiable `ar_shell` module, with `ar_shell` managing shell sessions, `ar_shell_session` owning per-session state and lifecycle, a session-specific `ar_shell_delegate`, and a built-in `shell` method

<!-- MANUAL ADDITIONS START -->
- `001-command-line-shell` implementation now has US1, US2, and US3 behavior slices in place: `arsh` uses `ar_shell`, shell-session state lives in `ar_shell_session`, replies render as `reply sender_id=<runtime-id> text=<reply>`, and EOF / Ctrl-D closes immediately while discarding later replies.
- `002-let-build-workflow-coordinator` is now implemented as a methods-only boot demo. `bootstrap` launches `workflow-coordinator` on fresh executable runs, builds a bundled `start` message, and emits an intake log line directly. The current `workflow-definition` implementation resolves supported workflow definitions by `definition_path` (`workflows/default-workflow.yaml`, `workflows/test-workflow.yaml`, and an invalid-schema fixture path) instead of parsing general YAML through the file delegate. It still uses `complete(...)` as the startup probe and transition-decision mechanism, requires `{outcome}` and `{reason}` placeholders, maps in-flight completion failures to retryable `stay`, and relies on deterministic `AGERUN_COMPLETE_RUNNER` overrides in tests. `workflow-reporter` emits visible summary/startup-failure output through the existing log delegate, coordinator tests use bounded step-by-step message processing instead of unbounded queue drains, and executable tests are now aligned with the 14-method workflow-demo runtime rather than the older 10-method chat-session bootstrap demo.
- `003-new-instruction-complete` now uses Zig C-ABI modules for parsing/evaluation (`ar_complete_instruction_parser.zig`, `ar_complete_instruction_evaluator.zig`) plus a user-approved C runtime adapter for direct llama.cpp interop (`ar_local_completion.c`) behind the stable `ar_local_completion.h` header. Existing facades only wire dispatch. `complete(...)` writes string values atomically into `memory...` targets, returns boolean status, rejects empty/generated-invalid values, fast-fails invalid templates and invalid base paths before backend initialization, records actionable failure fields (`failure_category`, `cause`, `recovery_hint`), preserves prior memory on failure, and uses separate warm-run (15s) and cold-start (30s) targets for short templates on both macOS and Linux.
<!-- MANUAL ADDITIONS END -->

## Feature Context

- `009-parameter-passed-executable` is now implemented as a startup-selection change in the main
  `agerun` executable only: operators can pass one combined boot identifier such as `echo-1.0.0`
  via `--boot-method`, and `make run-exec` exposes the same override through a single
  `BOOT_METHOD=<name-version>` variable. Persisted-agent restoration takes precedence over any
  override request, malformed identifiers fail clearly, and unavailable methods do not silently
  fall back to `bootstrap-1.0.0`.
- `010-command-line-option` is now implemented as a non-persistent executable run mode for the main
  `agerun` binary only: `agerun --no-persistence` and `make run-exec NO_PERSISTENCE=1` (the
  documented example; any non-empty `NO_PERSISTENCE` value triggers the same passthrough) skip
  both methodology and agency load/save behavior for a single run, preserve existing persisted
  files untouched, emit explicit no-persistence status output, and keep fresh-start boot behavior
  (including `--boot-method`) active when persistence is disabled.
