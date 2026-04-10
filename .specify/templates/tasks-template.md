---

description: "Task list template for feature implementation"
---

# Tasks: [FEATURE NAME]

**Input**: Design documents from `specs/[###-feature-name]/`
**Prerequisites**: plan.md (required), spec.md (required), research.md, data-model.md, contracts/

**Tests**: Tests and validation tasks are REQUIRED for any behavior, interface, parsing,
persistence, or ownership change. Documentation-only work may substitute doc validation
instead of code tests when explicitly justified in the plan.

**Organization**: Tasks are grouped by user story so each story can be implemented,
validated, and demonstrated independently.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no unmet dependencies)
- **[Story]**: Which user story this task belongs to (e.g., US1, US2, US3)
- Include exact file paths in descriptions
- Reference AgeRun paths directly (`modules/`, `methods/`, `kb/`, `specs/`, `.specify/`)

## Path Conventions

- **Modules**: `modules/ar_<module>.h`, `modules/ar_<module>.c`,
  `modules/ar_<module>_tests.c`, `modules/ar_<module>.md`
- **Methods**: `methods/<name>-<version>.method`, `methods/<name>.md`,
  `methods/<name>_tests.c`
- **Specs**: `specs/[###-feature-name]/...`
- **Workflow context**: `.specify/memory/pi-agent.md`

<!--
  IMPORTANT: The tasks below are sample tasks. `/spec tasks` MUST replace them
  with actual tasks derived from spec.md, plan.md, research.md, data-model.md,
  and contracts/.
-->

## Phase 1: Setup (Shared Context)

**Purpose**: Establish scope, touched files, and baseline validation context.

- [ ] T001 Record impacted files and modules in specs/[###-feature-name]/plan.md
- [ ] T002 Run baseline validation for touched areas using the relevant `make` targets
- [ ] T003 [P] Identify documentation/spec updates required in README.md, SPEC.md,
      module docs, method docs, or `.specify/` templates

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Create the failing validation and supporting infrastructure that every
story depends on.

**⚠️ CRITICAL**: No user story implementation starts before this phase is complete.

- [ ] T004 Add or update failing tests in the affected `*_tests.c` file(s)
- [ ] T005 [P] Add failing documentation or contract validation updates where applicable
- [ ] T006 [P] Prepare shared fixtures, helper functions, or method test assets required
      by multiple stories
- [ ] T007 Confirm the RED phase fails for the intended reason and capture the evidence

**Checkpoint**: Failing validation exists and the implementation target is proven.

---

## Phase 3: User Story 1 - [Title] (Priority: P1) 🎯 MVP

**Goal**: [Brief description of what this story delivers]

**Independent Test**: [How to verify this story works on its own]

### Validation for User Story 1

- [ ] T008 [P] [US1] Run the targeted failing test(s) for this story first
- [ ] T009 [P] [US1] Add or update story-specific doc/spec assertions if required

### Implementation for User Story 1

- [ ] T010 [P] [US1] Update the primary implementation file in modules/[file] or methods/[file]
- [ ] T011 [US1] Update supporting interfaces, helpers, or method assets in [file path]
- [ ] T012 [US1] Re-run the targeted tests until they pass
- [ ] T013 [US1] Refactor while preserving green tests and conventions

**Checkpoint**: User Story 1 is functional and independently validated.

---

## Phase 4: User Story 2 - [Title] (Priority: P2)

**Goal**: [Brief description of what this story delivers]

**Independent Test**: [How to verify this story works on its own]

### Validation for User Story 2

- [ ] T014 [P] [US2] Add or update failing tests for this story in [file path]
- [ ] T015 [P] [US2] Prove the RED phase with targeted execution output

### Implementation for User Story 2

- [ ] T016 [P] [US2] Update implementation in [file path]
- [ ] T017 [US2] Update dependent docs, contracts, or examples in [file path]
- [ ] T018 [US2] Re-run targeted tests and relevant regression coverage

**Checkpoint**: User Stories 1 and 2 both work independently.

---

## Phase 5: User Story 3 - [Title] (Priority: P3)

**Goal**: [Brief description of what this story delivers]

**Independent Test**: [How to verify this story works on its own]

### Validation for User Story 3

- [ ] T019 [P] [US3] Add or update failing tests for this story in [file path]
- [ ] T020 [P] [US3] Prove the RED phase with targeted execution output

### Implementation for User Story 3

- [ ] T021 [P] [US3] Update implementation in [file path]
- [ ] T022 [US3] Update dependent docs, contracts, or examples in [file path]
- [ ] T023 [US3] Re-run targeted tests and relevant regression coverage

**Checkpoint**: All requested user stories are independently functional.

---

## Final Phase: Polish & Cross-Cutting Concerns

**Purpose**: Complete repo-wide validation and synchronization work.

- [ ] T024 [P] Update affected documentation and specifications
- [ ] T025 Run `make clean build 2>&1` and then `make check-logs`
- [ ] T026 Run `make check-docs`
- [ ] T027 Run `make sanitize-tests 2>&1` when memory or ownership behavior changed
- [ ] T028 Update `.specify/memory/pi-agent.md` if active technologies or workflow context changed

---

## Dependencies & Execution Order

### Phase Dependencies

- **Setup (Phase 1)**: Starts immediately
- **Foundational (Phase 2)**: Depends on Setup and establishes the RED phase
- **User Stories (Phase 3+)**: Depend on Foundational completion
- **Final Phase**: Depends on all desired user stories being complete

### Within Each User Story

- Failing validation first
- Minimal implementation second
- Green verification third
- Refactor and documentation synchronization last

### Parallel Opportunities

- Tasks marked `[P]` may run in parallel when they touch different files
- Different user stories may proceed in parallel only after shared foundations are done
- Final documentation updates may run in parallel with non-overlapping validation prep

---

## Implementation Strategy

### MVP First (User Story 1 Only)

1. Complete Setup
2. Complete Foundational RED-phase work
3. Complete User Story 1
4. Validate User Story 1 independently
5. Stop if MVP scope is sufficient

### Incremental Delivery

1. Finish foundations once
2. Deliver User Story 1 with proof
3. Add User Story 2 with independent validation
4. Add User Story 3 with independent validation
5. Finish with repo-wide quality gates and documentation sync

---

## Notes

- Prefer exact file paths over generic descriptions
- Use `make` targets instead of direct compiler or binary execution
- Keep tasks small enough to show a clear RED -> GREEN -> REFACTOR sequence
- Do not mark a task complete until the corresponding evidence exists
