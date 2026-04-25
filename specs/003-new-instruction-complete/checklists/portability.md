# Portability Checklist: Completion Instruction

**Purpose**: Validate whether the `complete(...)` specification defines portability requirements clearly, completely, and measurably enough for planning and implementation
**Created**: 2026-04-16
**Feature**: [spec.md](../spec.md)

**Note**: This checklist evaluates the quality of the written portability requirements. It does not test runtime portability.

## Requirement Completeness

- [x] CHK001 Are supported execution environments defined explicitly enough to know where `complete(...)` is expected to run? [Completeness, Gap, Spec §FR-009; FR-010; Assumptions & Dependencies]
- [x] CHK002 Does the spec define whether portability requirements apply equally to one-argument and two-argument forms of `complete(...)`? [Completeness, Spec §FR-002; FR-002a]
- [x] CHK003 Are portability requirements defined for both normal success paths and failure paths caused by unavailable local resources? [Completeness, Spec §FR-011; FR-013; SC-003]
- [x] CHK004 Does the spec define whether portability includes only runtime behavior or also setup/distribution expectations for local completion resources? [Gap, Spec §FR-009; Assumptions & Dependencies]
- [x] CHK005 Are portability expectations defined for both one-argument and values-map calls, or is that distinction intentionally irrelevant? [Coverage, Spec §FR-004a; FR-004b]

## Requirement Clarity

- [x] CHK006 Is “supported local runtime profile” defined clearly enough to avoid different interpretations across platforms? [Clarity, Ambiguity, Spec §FR-010; SC-002]
- [x] CHK007 Is “local completion resources” explicit enough to distinguish portable assumptions from machine-specific assumptions? [Clarity, Spec §FR-009; Key Entities]
- [x] CHK008 Is the phrase “CPU-only runtime profile” specific enough to judge portability without naming implementation details? [Clarity, Spec §FR-010]
- [x] CHK009 Is it clear whether portability means consistent behavior across environments or merely operation on at least one local CPU-only environment? [Clarity, Ambiguity, Spec §Scope Boundaries; FR-010]
- [x] CHK010 Are “actionable error” requirements clear enough to indicate whether platform-specific failure causes must remain distinguishable? [Clarity, Spec §FR-012]

## Requirement Consistency

- [x] CHK011 Do the local-only and CPU-only requirements align consistently with the success criteria and assumptions, without hidden environment-specific dependencies? [Consistency, Spec §FR-009; FR-010; SC-002; Assumptions & Dependencies]
- [x] CHK012 Are failure-handling requirements consistent with portability expectations, so unsupported environments do not imply contradictory behavior? [Consistency, Spec §FR-011; FR-013; SC-003]
- [x] CHK013 Do scope boundaries and assumptions consistently avoid promising platform reach that the requirements do not define? [Consistency, Spec §Scope Boundaries; Assumptions & Dependencies]

## Acceptance Criteria Quality

- [x] CHK014 Can the portability-related requirements be objectively reviewed from the spec without knowing the eventual implementation stack? [Measurability, Spec §FR-009; FR-010]
- [x] CHK015 Are portability success conditions measurable, or does the spec currently rely on implied assumptions about the target environment? [Acceptance Criteria, Gap, Spec §Success Criteria]
- [x] CHK016 Do the success criteria distinguish portability from performance well enough to avoid conflating “runs locally” with “runs fast enough”? [Consistency, Spec §SC-002; SC-003]

## Scenario Coverage

- [x] CHK017 Are requirements defined for running the feature on different classes of local environments, or is environment diversity left unspecified? [Coverage, Gap, Spec §FR-009; FR-010]
- [x] CHK018 Are exception scenarios defined for environments where the local completion resource is missing, incompatible, or unusable? [Coverage, Exception Flow, Spec §FR-011; Edge Cases]
- [x] CHK019 Are recovery expectations defined when a portability-related failure occurs, such as preserving runtime usability and prior values? [Coverage, Recovery Flow, Spec §FR-013; FR-016; User Story 3]

## Edge Case Coverage

- [x] CHK020 Does the spec define how portability requirements apply when path-like second arguments are valid in language terms but unsupported in an environment-specific configuration? [Edge Case Coverage, Spec §FR-002b; FR-014a]
- [x] CHK021 Are boundary cases addressed where the feature can start locally but cannot complete because the local completion resource behaves differently across environments? [Edge Case Coverage, Spec §FR-011; FR-018]
- [x] CHK022 Does the spec define whether portability expectations include consistent handling of templates with punctuation, quoted text, and repeated placeholders across environments? [Gap, Edge Case Coverage, Spec §Edge Cases; FR-006; FR-017]

## Non-Functional Requirements

- [x] CHK023 Are supported operating-system or environment families intentionally unspecified, or missing from the portability requirements? [Gap, Non-Functional, Spec §FR-009; FR-010]
- [x] CHK024 Are requirements defined for behavioral consistency across different CPU classes or architectures, or is portability limited to an unstated subset? [Gap, Non-Functional, Spec §FR-010]
- [x] CHK025 Are filesystem, packaging, or local resource-discovery assumptions documented strongly enough to support portable planning? [Dependency, Non-Functional, Spec §Assumptions & Dependencies]
- [x] CHK026 Are observability requirements sufficient to diagnose portability-related failures without embedding implementation details into the spec? [Non-Functional, Spec §FR-012; FR-013]

## Dependencies & Assumptions

- [x] CHK027 Are external dependencies documented in a way that separates mandatory portable behavior from environment-specific setup choices? [Dependency, Spec §Assumptions & Dependencies]
- [x] CHK028 Are assumptions about local availability of completion resources validated or bounded clearly enough for a portability review? [Assumption, Spec §Assumptions & Dependencies; FR-009]
- [x] CHK029 Does the spec define whether portability includes offline operation by design, or is that only implied by the local-only requirement? [Ambiguity, Assumption, Spec §FR-009]

## Ambiguities & Conflicts

- [x] CHK030 Is it unambiguous whether a platform that cannot satisfy the local completion dependency is considered unsupported or merely a failure case? [Ambiguity, Spec §FR-011; SC-003]
- [x] CHK031 Are terms such as “local,” “supported local runtime profile,” and “CPU-only” sufficiently defined to avoid conflicting interpretations during planning? [Ambiguity, Spec §FR-009; FR-010; SC-002]
- [x] CHK032 Does the spec avoid mixing portability requirements with deferred implementation choices, so future planning can make technical decisions without rewriting the feature contract? [Consistency, Spec §Documentation & Compatibility Impact; Assumptions & Dependencies]

## Notes

- Re-reviewed after later `/spec clarify` updates that defined the supported local runtime profile as a documented minimum hardware baseline per OS, clarified the minimum actionable error detail, distinguished supported environments from runtime-failure cases when local completion resources are missing or unusable, defined the resource-discovery contract boundary, clarified CPU-only semantics, and specified environment-neutral path validity, cross-environment template consistency, and baseline-based CPU-architecture scope.
- This checklist is intended for reviewer/PR use before `/spec plan`.
- It focuses on portability requirement quality only; concrete platform support strategy belongs in `/spec plan`.
