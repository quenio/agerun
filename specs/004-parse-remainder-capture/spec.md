# Feature Specification: Parse Remainder Capture

**Feature Branch**: `004-parse-remainder-capture`
**Created**: 2026-04-18
**Status**: Draft
**Input**: Enhancement proposal: "extend `parse(...)` with remainder capture so methods can decompose multiline text into head/tail chunks for iterative parsing"

## Clarifications

### Session 2026-04-18

- Q: What syntax should identify a remainder capture? → A: Use `{name...}` inside a `parse(...)` template.
- Q: How many remainder captures are allowed in one template? → A: At most one.
- Q: Where may the remainder capture appear? → A: It must be the final placeholder in the template.
- Q: Should remainder capture be allowed to match an empty string? → A: Yes.
- Q: What should happen when a template uses invalid remainder syntax or placement? → A: Parsing fails safely and `parse(...)` returns an empty map.
- Q: Should templates without remainder capture change behavior? → A: No. Existing `parse(...)` semantics remain unchanged.

## User Scenarios & Testing *(mandatory)*

### User Story 1 - Split one line from the remaining input (Priority: P1)

A method author can use `parse(...)` to extract the next logical chunk of input and preserve the
unconsumed suffix for later processing.

**Why this priority**: This is the minimum capability needed to process YAML one line at a time in
message-driven iterations.

**Independent Test**: Evaluate `parse("{line}\n{rest...}", input)` against multiline input and
verify that `line` contains the first line and `rest` contains the remaining suffix after the first
newline.

**Acceptance Scenarios**:

1. **Given** input `"name: Alice\nage: 30\n"`, **When** a method evaluates `parse("{line}\n{rest...}", input)`, **Then** the returned map contains `line = "name: Alice"` and `rest = "age: 30\n"`.
2. **Given** input `"single line"`, **When** a method evaluates `parse("{line...}", input)`, **Then** the returned map contains `line = "single line"`.
3. **Given** input `"first\n"`, **When** a method evaluates `parse("{line}\n{rest...}", input)`, **Then** the returned map contains `line = "first"` and `rest = ""`.

---

### User Story 2 - Use remainder capture in iterative self-messaging workflows (Priority: P2)

A method author can preserve the unconsumed suffix from one `parse(...)` step and pass it into a
later step or later message.

**Why this priority**: Head/tail decomposition is only useful if the remaining input can be fed
back into the next parser step without losing exact text.

**Independent Test**: Execute repeated `parse(...)` calls where the `rest` field from one call
becomes the input of the next call, and verify that the sequence preserves all original content in
order.

**Acceptance Scenarios**:

1. **Given** a first parse call returns `rest`, **When** a later parse call uses that `rest` as input, **Then** the remaining content is consumed in the same order as the original source text.
2. **Given** a remainder capture includes embedded newlines or indentation, **When** it is stored in memory and reused later, **Then** the exact suffix text is preserved byte-for-byte.

---

### User Story 3 - Fail safely on unsupported remainder templates (Priority: P3)

A runtime operator can rely on invalid remainder templates failing safely without changing existing
`parse(...)` stability guarantees.

**Why this priority**: `parse(...)` already has a simple failure contract. The new syntax must not
introduce crashes or ambiguous partial results.

**Independent Test**: Evaluate invalid templates such as multiple remainder captures or non-final
remainder captures and verify that `parse(...)` returns an empty map and leaves the runtime stable.

**Acceptance Scenarios**:

1. **Given** a template containing two remainder captures, **When** `parse(...)` is evaluated, **Then** the result is an empty map.
2. **Given** a template where `{rest...}` is followed by additional literal text or another placeholder, **When** `parse(...)` is evaluated, **Then** the result is an empty map.
3. **Given** an invalid remainder template, **When** later non-`parse(...)` instructions run, **Then** normal message processing continues.

## Edge Cases *(mandatory)*

- What happens when the input is an empty string and the template is `{rest...}`?
- What happens when the input ends immediately after the literal that precedes the remainder capture?
- What happens when the template contains `{...}` with no capture name before the ellipsis?
- What happens when the input does not match the literal prefix before the remainder capture?
- What happens when the remainder capture would otherwise shadow an existing capture name in the same template?

## Scope Boundaries *(mandatory)*

### In Scope

- Extending `parse(...)` template syntax with a remainder capture form `{name...}`
- Allowing one remainder capture per template
- Requiring the remainder capture to be the final placeholder in the template
- Preserving exact unmatched suffix text in the returned map
- Safe failure behavior for invalid remainder template usage
- Maintaining current `parse(...)` behavior for templates that do not use remainder capture

### Out of Scope

- Multiple remainder captures in one template
- Non-final remainder captures
- General regex or wildcard syntax in templates
- Automatic line splitting outside `parse(...)`
- Adding loop syntax to the method language

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: The system MUST extend `parse(template, input)` to support a remainder capture placeholder written as `{name...}`.
- **FR-002**: A template MUST permit at most one remainder capture placeholder.
- **FR-003**: If a remainder capture placeholder is present, it MUST be the final placeholder in the template.
- **FR-004**: The remainder capture MUST bind the entire unmatched suffix of the input after all preceding literal and placeholder matches succeed.
- **FR-005**: A remainder capture MUST be allowed to bind an empty string.
- **FR-006**: The returned map key for `{name...}` MUST be `name`, without the ellipsis.
- **FR-007**: If the template uses more than one remainder capture, `parse(...)` MUST fail safely and return an empty map.
- **FR-008**: If the remainder capture is not the final placeholder in the template, `parse(...)` MUST fail safely and return an empty map.
- **FR-009**: If the input does not match the non-remainder portions of the template, `parse(...)` MUST continue to use its existing mismatch behavior and return an empty map.
- **FR-010**: Templates that do not use `{name...}` MUST continue to behave exactly as they do before this enhancement.
- **FR-011**: Successful remainder capture MUST preserve the captured suffix text exactly, including embedded newlines and indentation.

### Key Entities *(include if feature involves data)*

- **Remainder Capture Placeholder**: A `parse(...)` placeholder written as `{name...}` that binds the remaining unmatched suffix of the input.
- **Prefix Match**: The portion of a `parse(...)` operation that must match before the remainder capture may consume the rest of the input.
- **Captured Suffix**: The exact unmatched input tail bound to the remainder capture key in the returned map.

## Assumptions & Dependencies *(mandatory)*

- **Assumptions**:
  - Existing `parse(...)` still returns a map and uses an empty map to represent parse failure.
  - Placeholder names continue to use the same identifier rules already documented for `parse(...)`.
  - Remainder capture is intended as a text-decomposition feature, not a general pattern language.
- **Dependencies**:
  - `parse(...)` template parsing and evaluation logic in the instruction evaluator
  - Existing map return semantics for `parse(...)`
- **Open Questions**: None

## Documentation & Compatibility Impact *(mandatory)*

- **Affected Documentation**: `SPEC.md`, language grammar and `parse(...)` semantics, parser/evaluator module docs, and any method examples that teach iterative parsing.
- **Affected Runtime Contracts**: `parse(...)` template semantics only.
- **Compatibility Notes**: Additive feature. Existing `parse(...)` templates remain unchanged unless they intentionally use `{name...}`.

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: In the documented acceptance fixture set for remainder capture, `100%` of valid templates return the expected head/tail values.
- **SC-002**: In the documented invalid-template fixture set, `100%` of invalid remainder templates return an empty map without crashing or aborting message processing.
- **SC-003**: In reuse testing, `100%` of captured remainder values preserve embedded newlines and indentation exactly when stored and reused in later method steps.
