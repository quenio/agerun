# TDD Plan Review Checklist

## Learning
This checklist captures critical TDD plan design lessons to catch issues on first review pass, preventing rework during implementation. Use on every TDD cycle plan before marking as REVIEWED.

## Importance
TDD rigor depends on test quality, not just test quantity. Poor plan design reveals itself during implementation, wasting time and undermining confidence. Early review prevents:
- Assertions that don't actually catch bugs
- Iteration dependencies causing confusion
- Over-implementation in GREEN phases
- Integration-level bugs slipping through
- Resource leaks in minimal implementations

## Core Checklist

### Section 1: Cycle Organization (Organization Quality)

- [ ] **Clear cycle separation**: Each cycle has distinct purpose (e.g., "implement feature" vs "verify integration" vs "error handling")
- [ ] **Consistent numbering**: Cycles increment logically (1.x, 2.x, 3.x, not 1.1, 1.4, 2.x)
- [ ] **Purpose labels**: Each cycle clearly states what problem it solves
- [ ] **No ambiguous transitions**: Reader can clearly see when one cycle ends and next begins

**Example of GOOD numbering:**
```
Cycle 1 (1.1-1.3): Implement delegation routing
Cycle 2 (2.x): Verify agent routing still works
Cycle 3 (3.x): Error handling
Cycle 4 (4.x): Documentation
```

**Example of BAD numbering:**
```
Cycle 1 (1.1, 1.4, 1.7): ← confusing gaps
Cycle 2 (1.x again?): ← numbering regression
```

### Section 2: Iteration Structure (TDD Minimalism)

For EACH iteration, verify:

- [ ] **One assertion per iteration**: Does RED phase test exactly ONE behavior?
  - RED: "Does send return true?"
  - Not: "Does send return true AND is message queued?"

- [ ] **Hardcoded returns acceptable**: Is minimal GREEN allowed to use hardcoded values?
  - ✓ GOOD: `return true;` if that's all the assertion tests
  - ✗ BAD: "I need real implementation here" (that's next iteration's job)

- [ ] **Next iteration forces real implementation**: Does iteration N+1's RED fail if iteration N implements minimally?
  - Iteration 1.1 GREEN: hardcoded `true`
  - Iteration 1.2 RED: tests message delivery → FAILS with hardcoded return
  - Iteration 1.2 GREEN: forced to implement real routing

- [ ] **Resource cleanup in minimal implementations**: Even minimal GREEN cleans up resources?
  - Example: 1.1 destroys message (prevents leak) even though 1.2 will route it
  - ✗ BAD: "I'll just leak memory in the minimal version"

### Section 3: Assertion Quality (TEST VALIDITY) ⭐ CRITICAL

For EACH assertion in each iteration, verify:

- [ ] **RED phase will actually fail**: If I run the test RIGHT NOW (before GREEN code), does it fail?
  - ✗ RED ALERT: "Well, it will pass because iteration N already implemented it"
  - If yes → This iteration's assertion is redundant/validation-only (acceptable but document it)

- [ ] **Assertion validity verified via temporary corruption**: ⭐ MUST DO
  - In RED phase description: Include temporary code that BREAKS the feature
  - Example 1.3.1: "Don't queue the message (destroy it instead) so take_delegate_message returns NULL"
  - Example 1.3.2: "Corrupt type field to INT so assertion fails"
  - This PROVES the assertion catches real bugs

- [ ] **Temporary corruption is documented**: Does RED phase explain:
  - What code is temporarily broken?
  - Why it's broken?
  - What failure the test should show?
  - When to remove it (GREEN phase)?

**Template for verified assertion:**
```markdown
#### RED Phase
Add assertion: ...
Temporary corruption: [Describe code break]
Expected RED: "Test FAILS at line X because [broken thing]"
Verify: `make test` → assertion fails

#### GREEN Phase
Remove temporary corruption: [Describe removal]
Expected GREEN: "Test PASSES because [now works]"
Verify: `make test` → assertion passes
```

### Section 4: Integration-Level Testing (Seam Quality)

- [ ] **Module seam testing included**: Are there assertions that test how modules connect?
  - Example: Test that send_instruction_evaluator→delegation integration preserves message type
  - Not just "does delegation preserve type" but "does send→delegation→retrieve flow preserve type"

- [ ] **Three properties validated for data flow**:
  - [ ] **Persistence**: Message reaches destination (not lost)
  - [ ] **Type integrity**: Message metadata preserved (not corrupted)
  - [ ] **Data integrity**: Message payload preserved (not corrupted)
  - Each should be independent assertions that could fail separately

- [ ] **Redundancy is acceptable**: Is it OK if delegation tests AND send_instruction_evaluator tests check the same thing?
  - ✓ YES - integration assertions catch bugs at module boundaries
  - These are valuable even when underlying modules have their own tests

### Section 5: Test Type Clarity (Semantic Clarity)

- [ ] **TDD vs Validation iterations distinguished**:
  - [ ] True TDD: RED fails → GREEN passes (test drives code)
  - [ ] Validation: Tests pass after feature is complete (regression prevention)
  - [ ] HYBRID: Validation with verified assertions (1.3.1-1.3.3 pattern)

- [ ] **HYBRID iterations documented**: If using validation+verification pattern, clearly explain:
  - What will be validated (message properties, error cases)
  - That GREEN code already exists from earlier iterations
  - How assertions verify integration (temporary corruption in RED)

Example of clear documentation:
```markdown
**Objective**: Verify that the message is actually stored in the delegate's queue.
This iteration proves the assertion catches real failures by temporarily breaking the implementation.
```

### Section 6: Implementation Integrity (Code Quality)

- [ ] **GREEN phase genuinely minimal**: Does GREEN implement ONLY what RED tests?
  - ✓ GOOD: Test checks "return true" → GREEN returns hardcoded true
  - ✗ BAD: Test checks "return true" → GREEN implements full routing (over-implementation)

- [ ] **No implementation leakage**: Does GREEN avoid implementing future iteration behaviors?
  - ✗ BAD: 1.1 GREEN implements message queueing (1.2's job)
  - ✓ GOOD: 1.1 GREEN destroys message, 1.2 GREEN routes it

- [ ] **Resource ownership clear**: Is it documented who owns resources at each stage?
  - Example: "Fixture owns delegate" vs "Test temporarily destroys for cleanup"
  - Uses naming conventions (own_, ref_, mut_)

### Section 7: Interdependency Management (Dependency Quality)

- [ ] **Clear dependency chain**: Can reader trace which iterations build on which?
  - 1.1 hardcoded return → 1.2 tests message delivery → 1.3.x test properties
  - Each iteration's RED depends on previous iterations being REVIEWED/PASSED

- [ ] **No circular dependencies**: Can you read iterations in order without forward references?
  - ✗ BAD: "1.3 will test what 1.2 implements" (forward reference)
  - ✓ GOOD: "1.3 validates what 1.2 implemented" (can understand in sequence)

- [ ] **REVIEWED iterations are stable**: Once marked REVIEWED, do later iterations assume its behavior?
  - Yes, and that's the point - create stable foundation for next cycles

### Section 8: Documentation Quality (Communication Quality)

- [ ] **Commit messages document methodology**: Does commit message explain NOT JUST what changed but why it matters?
  - Example: "Verified assertions via temporary corruption pattern"
  - Not just: "Updated iterations 1.3.1-1.3.3"

- [ ] **Temporary corruption is explainable**: If someone reads the plan 6 months later, can they understand:
  - Why corruption code exists?
  - That it's temporary (removed in GREEN)?
  - How it validates the assertion?

- [ ] **RED/GREEN expectations are explicit**: Does each phase clearly state:
  - What SHOULD happen (PASS/FAIL)?
  - How to VERIFY it (exact make commands)?
  - Why the outcome occurs?

## Verification Template

Use this template for each cycle:

```markdown
## Pre-Review Checklist

### Cycle Organization
- [ ] Clear cycle separation from previous cycles
- [ ] Numbering is sequential and consistent
- [ ] Cycle purpose is one-sentence clear

### Each Iteration
- [ ] One assertion per iteration (count: ___)
- [ ] Hardcoded returns acceptable for minimalism level
- [ ] Next iteration would force real implementation
- [ ] Resource cleanup prevents leaks

### Assertion Validity (⭐ CRITICAL)
- [ ] RED phase describes temporary corruption
- [ ] Corruption makes assertion fail
- [ ] GREEN phase removes corruption explicitly
- [ ] Expectation docs say "Test FAILS/PASSES" with reason

### Integration Testing
- [ ] Seam tests included (module boundaries)
- [ ] Three independent properties tested (if data flow)
- [ ] Documentation explains why seam testing matters

### Test Type Clarity
- [ ] TDD vs Validation vs Hybrid clearly labeled
- [ ] Hybrid iterations explain validation+verification approach

### Implementation Integrity
- [ ] GREEN is genuinely minimal (count assertions, verify GREEN size)
- [ ] No implementation leakage to future iterations
- [ ] Resource ownership semantics clear

### Documentation
- [ ] Commit message documents methodology
- [ ] Temporary corruption is clearly marked/explained
- [ ] RED/GREEN expectations explicit (PASS/FAIL + reason)

## Common Anti-Patterns (Red Flags)

🚩 **RED FLAG: "This test will pass because iteration N already implements it"**
→ Add temporary corruption to RED phase to prove assertion catches bugs

🚩 **RED FLAG: GREEN implementation is 5x larger than RED test**
→ Over-implementation detected - split iteration or simplify GREEN

🚩 **RED FLAG: "Hardcoded return seems wrong here"**
→ It's not - if that's all the assertion tests, it's perfect minimalism

🚩 **RED FLAG: No resource cleanup in minimal GREEN**
→ Even hardcoded returns must clean up ownership

🚩 **RED FLAG: Iteration numbering has gaps or restarts**
→ Renumber for clarity (1.1, 1.2, 1.3 not 1.1, 1.4, 1.7)

🚩 **RED FLAG: Same thing tested by multiple assertions in one iteration**
→ Split into .1/.2 sub-iterations per [TDD Plan Iteration Split Pattern](tdd-plan-iteration-split-pattern.md)

🚩 **RED FLAG: No mention of temporary code/corruption in RED phase**
→ Assertion validity unproven - add temporary corruption pattern

## Implementation Workflow

**When reviewing a TDD plan:**

1. Print this checklist
2. Read plan end-to-end first (get overview)
3. Go back to start, check each section of checklist
4. For assertions: Find where temporary corruption is documented
5. If not there: Stop and request additions
6. If there: Mark as ready for review

**When creating a TDD plan:**

1. Draft initial plan (cycles + iterations)
2. Run through "Section 1: Cycle Organization" only
3. For each iteration, draft RED/GREEN/REFACTOR
4. Run through "Section 3: Assertion Quality"
5. **CRITICAL**: Add temporary corruption descriptions to RED phases
6. Run full checklist before marking PENDING REVIEW

## Related Patterns

- [Review-Plan Command Improvements](review-plan-command-improvements.md) - Session 2025-10-18 learnings: interactive review, NULL validation
- [Iterative Plan Review Protocol](iterative-plan-review-protocol.md) - Multi-session review approach
- [TDD Plan Iteration Split Pattern](tdd-plan-iteration-split-pattern.md) - When to split iterations
- [TDD GREEN Phase Minimalism](tdd-green-phase-minimalism.md) - What minimal means
- [Temporary Test Cleanup Pattern](temporary-test-cleanup-pattern.md) - Resource cleanup in minimal implementations
- [Plan Review Status Tracking Pattern](plan-review-status-tracking.md) - Status markers for multi-session review

## Lessons Embedded in This Checklist

This checklist encodes 14 critical lessons from authentic TDD practice:

1. ✓ Iteration numbering clarity prevents confusion
2. ✓ One assertion per iteration creates reliable pace
3. ✓ Hardcoded returns are acceptable for minimalism
4. ✓ Minimal implementations must still clean up resources
5. ✓ Tests drive implementation through dependency chains
6. ✓ Integration tests catch module seam bugs
7. ✓ **Assertion validity MUST be verified via temporary corruption**
8. ✓ Temporary code in tests is valid TDD technique (not a smell)
9. ✓ Property validation through independent assertions
10. ✓ Clear test type distinction (TDD vs Validation vs Hybrid)
11. ✓ Over-implementation in GREEN violates minimalism
12. ✓ Commit messages should document methodology
13. ✓ Forward dependencies disable sequential reading
14. ✓ Resource ownership naming conventions matter
