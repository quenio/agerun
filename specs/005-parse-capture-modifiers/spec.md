# Feature Specification: Parse Capture Modifiers

**Feature Branch**: `005-parse-capture-modifiers`
**Created**: 2026-04-18
**Status**: Draft
**Input**: Enhancement proposal: "extend `parse(...)` placeholders with capture modifiers such as `trim` and `spaces` so methods can analyze indentation and normalize extracted text"

## Clarifications

### Session 2026-04-18

- Q: What modifier syntax should `parse(...)` use? → A: Use `{name|modifier}` inside a `parse(...)` template.
- Q: Which modifiers are in scope for the first release? → A: `trim` and `spaces` only.
- Q: How many modifiers may a placeholder use in the first release? → A: At most one.
- Q: What does `trim` do? → A: It removes leading and trailing ASCII whitespace from the captured text before normal parse type inference runs.
- Q: What does `spaces` do? → A: It captures a run of ASCII space characters and returns the count as an integer.
- Q: What should happen on unknown or invalid modifiers? → A: Parsing fails safely and `parse(...)` returns an empty map.

## User Scenarios & Testing *(mandatory)*

### User Story 1 - Normalize captured scalar text (Priority: P1)

A method author can trim surrounding whitespace from captured text without manually encoding
multiple whitespace variants into the template.

**Why this priority**: YAML parsing frequently needs normalized key and scalar text even when the
source contains alignment whitespace.

**Independent Test**: Evaluate `parse("{key|trim}: {value|trim}", input)` against input with
surrounding spaces and verify that the returned values contain no leading or trailing whitespace.

**Acceptance Scenarios**:

1. **Given** input `"name :  Alice  "`, **When** a method evaluates `parse("{key|trim}: {value|trim}", input)`, **Then** the returned map contains `key = "name"` and `value = "Alice"`.
2. **Given** input `"count : 42 "`, **When** a method evaluates `parse("{key|trim}: {value|trim}", input)`, **Then** `value` is normalized before normal type inference is applied.

---

### User Story 2 - Measure indentation as data (Priority: P2)

A method author can capture indentation width as an integer so later method logic can compare
nesting depth using normal arithmetic and `if(...)` expressions.

**Why this priority**: Indentation depth is the core structural signal for block-style YAML.

**Independent Test**: Evaluate `parse("{indent|spaces}{body...}", input)` or equivalent templates
against lines with leading spaces and verify that `indent` is returned as an integer count.

**Acceptance Scenarios**:

1. **Given** input `"  name: Alice"`, **When** a method evaluates `parse("{indent|spaces}{rest...}", input)`, **Then** the returned map contains `indent = 2`.
2. **Given** input `"name: Alice"`, **When** the same template is evaluated, **Then** the returned map contains `indent = 0`.
3. **Given** input beginning with non-space characters, **When** `{indent|spaces}` is used, **Then** the capture succeeds with a count of `0` if the remainder of the template matches.

---

### User Story 3 - Fail safely on invalid modifier usage (Priority: P3)

A runtime operator can rely on invalid modifier syntax failing safely without destabilizing
existing `parse(...)` behavior.

**Why this priority**: Capture modifiers extend template syntax and must preserve the simple failure
contract of `parse(...)`.

**Independent Test**: Evaluate templates with unknown modifiers, repeated modifiers, or invalid
`spaces` usage and verify that `parse(...)` returns an empty map.

**Acceptance Scenarios**:

1. **Given** a template containing `{key|unknown}`, **When** `parse(...)` is evaluated, **Then** the result is an empty map.
2. **Given** a template containing more than one modifier on a placeholder, **When** `parse(...)` is evaluated, **Then** the result is an empty map.
3. **Given** invalid modifier syntax, **When** later non-`parse(...)` instructions run, **Then** normal runtime behavior continues.

## Edge Cases *(mandatory)*

- What happens when `{value|trim}` captures only whitespace?
- What happens when `{indent|spaces}` is followed immediately by a literal and the input begins with tabs instead of spaces?
- What happens when a placeholder uses both `...` and a modifier, such as `{rest...|trim}`?
- What happens when the modifier name is empty, such as `{value|}`?
- What happens when the capture name is empty but a modifier is present?

## Scope Boundaries *(mandatory)*

### In Scope

- Extending `parse(...)` placeholder syntax with `{name|modifier}`
- Supporting `trim` and `spaces` modifiers in the first release
- Returning indentation width from `spaces` as an integer
- Applying `trim` before the normal post-capture type-inference path
- Safe failure behavior for unsupported modifier syntax

### Out of Scope

- Arbitrary user-defined modifiers
- Multiple modifiers on one placeholder in the first release
- Tab-width normalization or mixed-whitespace indentation policies
- YAML-specific scalar typing rules beyond existing parse type inference
- A general string transformation library beyond `trim` and `spaces`

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: The system MUST extend `parse(template, input)` to support one optional capture modifier written as `{name|modifier}`.
- **FR-002**: In the first release, the only supported modifiers MUST be `trim` and `spaces`.
- **FR-003**: A placeholder MUST permit at most one modifier in the first release.
- **FR-004**: `{name|trim}` MUST remove leading and trailing ASCII whitespace from the captured text before the normal parse post-processing path runs.
- **FR-005**: After `{name|trim}` normalization, existing parse type inference MUST continue to determine whether the stored result is an integer, double, or string.
- **FR-006**: `{name|spaces}` MUST capture a run of zero or more ASCII space characters and store the count as an integer result under `name`.
- **FR-007**: `{name|spaces}` MUST NOT count tabs or other non-space characters as spaces.
- **FR-008**: If a placeholder uses an unknown modifier, `parse(...)` MUST fail safely and return an empty map.
- **FR-009**: If a placeholder uses more than one modifier in the first release, `parse(...)` MUST fail safely and return an empty map.
- **FR-010**: Templates that do not use modifiers MUST continue to behave exactly as they do before this enhancement.
- **FR-011**: Modifier processing MUST apply only to the placeholder that declares the modifier.
- **FR-012**: Modifier usage MUST preserve the existing `parse(...)` failure contract: on failure, the instruction returns an empty map and does not crash the runtime.

### Key Entities *(include if feature involves data)*

- **Capture Modifier**: An optional placeholder suffix in `parse(...)` written as `|modifier` that changes how one capture is interpreted.
- **Trimmed Capture**: A capture whose surrounding ASCII whitespace is removed before standard parse post-processing.
- **Space Count Capture**: A capture that counts leading ASCII spaces and stores that count as an integer.

## Assumptions & Dependencies *(mandatory)*

- **Assumptions**:
  - Existing `parse(...)` returns an empty map on parse failure.
  - Existing parse type inference remains in place for captures other than the special integer result of `spaces`.
  - `trim` uses ASCII whitespace rules in the first release.
- **Dependencies**:
  - `parse(...)` template parser and evaluator
  - Existing number/string inference path for captured values
- **Open Questions**:
  - Whether a future release should support combining remainder capture and modifiers, such as `{rest...|trim}`

## Documentation & Compatibility Impact *(mandatory)*

- **Affected Documentation**: `SPEC.md`, `parse(...)` function documentation, parser/evaluator module docs, and any language examples that show indentation-aware parsing.
- **Affected Runtime Contracts**: `parse(...)` placeholder semantics only.
- **Compatibility Notes**: Additive feature. Existing templates are unchanged unless they intentionally use `|trim` or `|spaces`.

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: In the documented `trim` acceptance fixture set, `100%` of valid templates return captures with surrounding ASCII whitespace removed.
- **SC-002**: In the documented `spaces` acceptance fixture set, `100%` of valid templates return the correct integer indentation count.
- **SC-003**: In the invalid-modifier fixture set, `100%` of invalid templates return an empty map without runtime instability.
