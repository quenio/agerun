# AgeRun Development Guidelines

Auto-generated from the project constitution and repository context. Last updated: 2026-04-11

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
- Planned `001-command-line-shell` around a transport-only stdio delegate, a separate shell session module, and a built-in `arsh` shell method

<!-- MANUAL ADDITIONS START -->
<!-- MANUAL ADDITIONS END -->
