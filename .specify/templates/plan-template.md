# Implementation Plan: [FEATURE]

**Branch**: `[###-feature-name]` | **Date**: [DATE] | **Spec**: [link]
**Input**: Feature specification from `specs/[###-feature-name]/spec.md`

**Note**: This template is filled in by the `/spec plan` command.

## Summary

[Extract from feature spec: primary requirement + technical approach from research]

## Technical Context

<!--
  ACTION REQUIRED: Replace the content in this section with the technical details
  for the project. Mark unknowns as NEEDS CLARIFICATION and resolve them during
  research before implementation begins.
-->

**Language/Version**: [e.g., C17 with Zig 0.14.1 interop or NEEDS CLARIFICATION]
**Primary Dependencies**: [e.g., Make, gcc-13/clang, Zig 0.14.1 or NEEDS CLARIFICATION]
**Storage**: [e.g., in-memory runtime state, persisted methods/agents, files, or N/A]
**Testing**: [e.g., `make <module>_tests 2>&1`, `make sanitize-tests 2>&1`, `make check-docs`]
**Target Platform**: [e.g., macOS + Linux CLI/runtime]
**Project Type**: [e.g., message-driven runtime, module, method, documentation update]
**Performance Goals**: [domain-specific measurable target or NEEDS CLARIFICATION]
**Constraints**: [e.g., zero leaks, relative markdown links, make-target-only workflow]
**Scale/Scope**: [domain-specific scope or NEEDS CLARIFICATION]

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

- [ ] KB consultation recorded: AGENTS.md, `kb/README.md`, and relevant KB articles cited
- [ ] TDD approach defined: failing test/validation listed before implementation tasks
- [ ] Convention impact reviewed: naming, ownership, file layout, and make-target usage
- [ ] Specification and documentation impact identified: SPEC.md, module docs, README,
      `.specify` templates, or other affected references
- [ ] Required validation commands listed: `make clean build 2>&1`, `make check-logs`,
      `make check-docs`, relevant module tests, and sanitizer runs when applicable

## Project Structure

### Documentation (this feature)

```text
specs/[###-feature-name]/
├── plan.md
├── research.md
├── data-model.md
├── quickstart.md
├── contracts/
└── tasks.md
```

### Source Code (repository root)

```text
modules/
├── ar_<module>.h
├── ar_<module>.c
├── ar_<module>_tests.c
└── ar_<module>.md

methods/
├── <name>-<version>.method
├── <name>.md
└── <name>_tests.c

kb/
reports/
scripts/
.specify/
└── memory/pi-agent.md
```

**Structure Decision**: [Document the real files and directories this feature will touch.]

## Complexity Tracking

> **Fill ONLY if Constitution Check has violations that must be justified**

| Violation | Why Needed | Simpler Alternative Rejected Because |
|-----------|------------|-------------------------------------|
| [e.g., temporary deviation from naming rule] | [current need] | [why standard approach is insufficient] |
