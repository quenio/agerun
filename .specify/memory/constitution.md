<!--
Sync Impact Report
- Version change: template -> 1.0.0
- Modified principles:
  - Template Principle 1 -> I. Knowledge Base Before Action
  - Template Principle 2 -> II. Test-Driven Development Is Mandatory
  - Template Principle 3 -> III. Conventions Are Part Of The Design
  - Template Principle 4 -> IV. Specifications, Documentation, And Code Stay In Sync
  - Template Principle 5 -> V. Evidence-Based Quality Gates
- Added sections:
  - Additional Constraints
  - Development Workflow
- Removed sections:
  - None
- Templates requiring updates:
  - ✅ .specify/templates/plan-template.md
  - ✅ .specify/templates/spec-template.md
  - ✅ .specify/templates/tasks-template.md
  - ✅ .specify/templates/checklist-template.md
  - ✅ .specify/templates/commands/*.md
  - ✅ .specify/memory/pi-agent.md
- Deferred follow-up:
  - None
-->
# AgeRun Constitution

## Core Principles

### I. Knowledge Base Before Action
Contributors MUST search AGENTS.md and kb/README.md before planning, implementing,
debugging, or refactoring work, read the relevant KB articles, and apply the cited
guidance to the task at hand. When user feedback challenges an approach, contributors
MUST pause and repeat that search before continuing. Rationale: AgeRun relies on
accumulated project knowledge; skipping that knowledge causes avoidable regressions and
repeated mistakes.

### II. Test-Driven Development Is Mandatory
Behavior changes MUST follow a RED -> GREEN -> REFACTOR cycle. A failing test or other
intentional validation signal MUST exist before implementation begins, and the minimal
passing change MUST be verified before refactoring. Documentation-only changes may skip
code TDD, but any behavior, interface, parsing, persistence, or ownership change MUST
add or update executable validation. Rationale: AgeRun quality depends on proving
behavior, not asserting it.

### III. Conventions Are Part Of The Design
AgeRun naming, ownership, module, and command conventions defined in AGENTS.md are
mandatory design constraints, not optional style guidance. Contributors MUST use real
AgeRun terminology, ownership prefixes (`own_`, `mut_`, `ref_`), state-change naming
(`get_`, `take_`, `peek_`), repository-relative documentation links, and Makefile entry
points instead of ad hoc structure or commands. Rationale: shared conventions protect
memory safety, readability, and maintainability.

### IV. Specifications, Documentation, And Code Stay In Sync
Contributors MUST check relevant specifications before implementing and MUST update
specifications, documentation, templates, and user guidance whenever behavior or
workflow changes. Examples and command descriptions MUST reflect real AgeRun APIs,
actual repository structure, and native `/spec` workflow behavior. Rationale: stale
specs or templates create incorrect implementations and broken workflows.

### V. Evidence-Based Quality Gates
No work is complete until required evidence has been gathered and reviewed. Contributors
MUST validate claims with concrete outputs such as failing tests, passing tests,
`make`-based verification, documentation checks, file paths, and diffs. If a mandatory
gate cannot be completed, the gap MUST be recorded explicitly with follow-up guidance.
Rationale: evidence is the only reliable basis for change in a memory-sensitive,
message-driven runtime.

## Additional Constraints

- Use repository Make targets for builds, tests, sanitizers, and documentation checks;
  do not substitute direct compiler or script invocation when a Make target exists.
- Apply the memory-management rules from MMM.md and AGENTS.md to every code change,
  including explicit ownership transfer, leak prevention, and cleanup validation.
- Keep `.specify` artifacts aligned with the real repository layout: `modules/`,
  `methods/`, `kb/`, `reports/`, `scripts/`, and `specs/`.
- Use `.specify/memory/pi-agent.md` as the maintained native pi context file instead of
  agent-specific generated context outputs.

## Development Workflow

1. `/spec specify` captures user value, scope boundaries, measurable outcomes, and
   documentation impact.
2. `/spec clarify` resolves high-impact ambiguities that would change implementation,
   testing, or validation strategy.
3. `/spec plan` MUST record constitution gates, KB consultation, repo-specific structure,
   required validations, and documentation/spec impacts.
4. `/spec tasks` MUST create dependency-ordered, file-specific tasks that place tests and
   validation before implementation work.
5. `/spec implement` MUST execute tasks sequentially, keep task state current, and stop
   when required quality gates fail unless the user explicitly accepts the risk.

## Governance

This constitution supersedes conflicting `.specify` workflow guidance and informs review
of every spec, plan, task list, and implementation change. Amendments MUST update this
file, any affected templates under `.specify/templates/`, and `.specify/memory/pi-agent.md`
in the same change set. Semantic versioning governs amendments: MAJOR for incompatible
principle changes or removals, MINOR for new principles or materially expanded sections,
and PATCH for clarifications that do not change governance intent. Compliance review
MUST occur during `/spec plan`, `/spec tasks`, and final implementation validation, with
AGENTS.md serving as the operational reference for day-to-day execution.

**Version**: 1.0.0 | **Ratified**: 2026-04-10 | **Last Amended**: 2026-04-10
