# RED Phase Dual Goals Pattern

## Learning

The RED phase serves TWO independent goals that must both be completed for every iteration, regardless of whether implementation already exists.

## Importance

Understanding that these are separate, independent goals prevents skipping the RED phase for verification iterations, which would create "always-passing tests" that don't prove they catch bugs.

## The Two Goals

### Goal 1: Prove Test Validity (ALWAYS REQUIRED)

**Purpose**: Prove this specific test can actually catch bugs

**Method**:
1. Apply temporary corruption to break the feature
2. Run the test
3. Verify the test FAILS with expected message
4. Document the failure evidence

**Applies to**:
- ✅ New implementation iterations
- ✅ Verification iterations
- ✅ Regression test iterations
- ✅ Refactoring iterations
- ✅ **EVERY iteration without exception**

**Evidence required**:
- Test output showing FAILURE
- Failure message matches plan's prediction
- Temporary corruption applied as documented in plan

**Example** (Verification iteration testing existing agent routing):
```zig
// TEMPORARY: Break agent routing to prove test catches it
} else if (agent_id > 0) {
    send_result = c.ar_delegation__send_to_delegate(...);  // WRONG destination
}
// Run test → MUST FAIL with "Agent should have received message"
```

### Goal 2: Identify What to Implement (CONDITIONAL)

**Purpose**: Determine what code needs to be written to make test pass

**Method**:
1. Observe what the failing test expects
2. Identify minimal implementation needed
3. Write code to satisfy the test

**Applies to**:
- ✅ New implementation iterations (Goal 2 needs satisfaction)
- ⚠️ Verification iterations (Goal 2 already satisfied by existing code)

**Status depends on iteration type**:
- **New implementation**: Goal 2 not yet satisfied → implement minimal code
- **Verification**: Goal 2 already satisfied → just remove corruption from Goal 1

## Critical Mistake Pattern

### ❌ WRONG Mental Model
```
RED phase = "Create failing test to guide implementation"

Therefore:
  If implementation exists → Skip RED phase
```

**Why wrong**: This conflates Goal 1 and Goal 2, making Goal 1 conditional on Goal 2.

### ✅ CORRECT Mental Model
```
RED phase has TWO independent goals:
  1. Prove test validity (ALWAYS required)
  2. Identify what to implement (CONDITIONAL on implementation status)

Therefore:
  Goal 1: ALWAYS execute, regardless of implementation
  Goal 2: Only needed if implementation doesn't exist
```

## Iteration Type Patterns

### New Implementation Iteration

**Example**: Iteration 1.1 - Implement send to delegate

**RED Phase**:
- **Goal 1**: ✅ REQUIRED - Prove test catches routing bugs
  - Add test
  - Add temporary stub: `return false;`
  - Verify test FAILS
- **Goal 2**: ✅ REQUIRED - Determine what to implement
  - Observe: Test expects routing to delegation
  - Plan: Need to call `ar_delegation__send_to_delegate()`

**GREEN Phase**:
- Satisfy Goal 2: Implement routing to delegation
- Test PASSES

### Verification Iteration

**Example**: Iteration 2 - Verify agent routing still works

**RED Phase**:
- **Goal 1**: ✅ REQUIRED - Prove test catches routing bugs
  - Add test
  - Corrupt existing implementation: route to wrong destination
  - Verify test FAILS
- **Goal 2**: ✅ ALREADY SATISFIED - Implementation exists from Iteration 1
  - No new code needed
  - Just need to remove corruption

**GREEN Phase**:
- Goal 2 already satisfied, just remove corruption
- Test PASSES

**CRITICAL**: Even though Goal 2 is satisfied, Goal 1 is STILL MANDATORY. The test must fail before it passes.

## Enforcement Mechanism

Before marking any RED phase complete, verify ALL of these:

**Goal 1 Evidence** (ALWAYS required):
- [ ] Did I see test output showing FAILURE?
- [ ] Does failure message match plan's prediction?
- [ ] Did I apply exact temporary corruption from plan?
- [ ] Can I show git diff of corruption applied?

**Goal 2 Status** (Context-dependent):
- [ ] For new implementation: Have I identified what to implement?
- [ ] For verification: Have I confirmed implementation already exists?

**Both must be YES** before proceeding to GREEN phase.

## Common Scenarios

### Scenario 1: "Implementation already exists, so RED phase is optional"

**Wrong**: This assumes Goal 1 depends on Goal 2
**Correct**: Goal 1 is independent - must prove test validity regardless

### Scenario 2: "I'll just write the test and run it - if it fails, that's my RED phase"

**Wrong**: This only satisfies Goal 2 (seeing what needs implementation)
**Correct**: Goal 1 requires INTENTIONAL corruption to prove test catches bugs

### Scenario 3: "The test passed immediately, so implementation is correct"

**Wrong**: Test might be "always-passing" - haven't proven it can catch bugs
**Correct**: Apply corruption first, verify test fails, THEN remove corruption

## Example Plan Structure

```markdown
### Iteration X: [Description] (RED-GREEN-REFACTOR)

**Iteration Type**: VERIFICATION (or NEW IMPLEMENTATION)

#### RED Phase

**RED Phase has TWO goals (both must be completed):**

**GOAL 1: Prove Test Validity** (ALWAYS REQUIRED)
- Purpose: Prove this test can catch bugs
- Method: Apply temporary corruption, verify FAILS
- Status: [Implementation exists, but validity must still be proven]

**GOAL 2: Identify What to Implement** (CONDITIONAL)
- Purpose: Determine what code needs writing
- Method: Observe failing test expectations
- Status: [✅ ALREADY SATISFIED - Implemented in Iteration Y] OR [Needs implementation]

**Step 1: Write the Test**
[Test code...]

**Step 2: Prove Test Validity (GOAL 1 - MANDATORY)**
[Temporary corruption code...]
Expected: Test FAILS with [specific message]

#### GREEN Phase

**For verification iterations**: Remove corruption (Goal 2 already satisfied)
**For new implementations**: Implement minimal code (satisfy Goal 2)

[Implementation code...]
```

## Related Patterns

- [TDD RED Phase Assertion Requirement](tdd-red-phase-assertion-requirement.md) - Why assertions must fail first
- [TDD Cycle Detailed Explanation](tdd-cycle-detailed-explanation.md) - Complete RED-GREEN-REFACTOR cycle
- [Test Assertion Strength Patterns](test-assertion-strength-patterns.md) - What makes strong assertions
- [TDD Plan Review Checklist](tdd-plan-review-checklist.md) - Lesson 7: Assertion validity verification

## Key Insight

**The RED phase isn't about "does implementation exist?"**

**The RED phase is about "can this test catch bugs?"**

That question must be answered for EVERY iteration, and the only way to answer it is to break something and watch the test catch it.

**Goal 1 and Goal 2 are independent. One does not imply the other.**
