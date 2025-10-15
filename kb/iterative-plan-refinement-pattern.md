# Iterative Plan Refinement Pattern

**Note**: Examples reference TDD Cycle 6.5 which planned `ar_delegate` message queue functions (`send`, `take_message`, `has_messages`) - these are planned implementations, not yet existing code.  // EXAMPLE: TDD Cycle 6.5

## Learning
Complex TDD plans benefit from iterative refinement through user feedback. Present a draft plan, receive corrections, apply learnings from KB articles, refine the plan, and repeat until the methodology is correct. This process itself follows a cycle: Draft → Feedback → Learn → Refine → Repeat.

## Importance
Getting TDD methodology right requires nuanced understanding of RED-GREEN-REFACTOR principles, ownership semantics, BDD structure, and naming conventions. Attempting to create perfect plans on the first try leads to methodology errors that waste implementation time. Iterative refinement catches errors during planning when they're cheap to fix.

## Example

### Real Session: TDD Cycle 6.5 Plan Refinement (10 iterations)

```
Iteration 1: Terminology Error
Draft: "Cycle 1: Send returns success"
Feedback: "Just a correction in terminology: the cycle is 6.5;
           what you are outlining are iterations under the cycle 6.5"
Learning: Cycle = overall unit, Iteration = RED-GREEN-REFACTOR within cycle
Refinement: Changed all "Cycle N" to "Iteration N"

Iteration 2: Multiple Assertions per Iteration
Draft: Iteration 1 tests both return value AND queuing
Feedback: "Every assertion should have its own RED-GREEN-REFACTOR iteration"
Learning: One assertion = one iteration
Refinement: Expanded from 8 to 14 iterations

Iteration 3: Missing Explicit Assertion Failures
Draft: RED phase had no failure indicators
Feedback: "The RED phase of every iteration should state which assertion will fail"
Learning: Add inline comments: // ← FAILS (reason)
Refinement: Added explicit failure comments to all assertions

Iteration 4: GREEN Phase Not Minimal
Draft: Iteration 2 GREEN implemented full logic
Feedback: "Does not implement only what is necessary to pass the assertion"
Learning: Hardcoded returns are valid GREEN implementations
Refinement: Changed to return false;, added Iteration 3 to force proper impl

Iteration 5: RED Phase Wouldn't Actually Fail
Draft: Iteration 6 would pass immediately
Feedback: "The RED phase should undo enough implementation to show
           the newly introduced assertion fail"
Learning: Previous GREEN phases may need limiting to force RED failures
Refinement: Changed Iteration 5 to use ar_list__first() (peek),
           so Iteration 6 properly fails

Iteration 6: Using Non-Existent API
Draft: Plan used ar_list__get()
Feedback: "ar_list__get() is not implemented in ar_list"
Action: Read ar_list.h to verify available functions
Learning: Always verify API before planning
Refinement: Changed to ar_list__first() and ar_list__remove_first()

Iteration 7: Incorrect Function Naming
Draft: Used ar_delegate__get_message()  // EXAMPLE: TDD Cycle 6.5
Feedback: "Not a good name if the message will be removed;
           'get' is normally used to read attributes, not to change state"
Learning: Use "take_" for ownership transfer, "get_" for read-only
Refinement: Renamed to ar_delegate__take_message()  // EXAMPLE: TDD Cycle 6.5

Iteration 8: Missing BDD Structure
Draft: Tests lacked Given/When/Then/Cleanup comments
Feedback: "Not following BDD style in test, as instructed in the KB"
Action: Read bdd-test-structure.md
Learning: Mandatory Given/When/Then/Cleanup comments
Refinement: Restructured all test examples with proper comments

Iteration 9: Incorrect Ownership Semantics
Draft: Plan didn't show take_ownership/drop_ownership
Feedback: "Regarding the ownership of messages, the message should be
           owned by the delegate if not owned"
Action: Read ar_agent.c and KB ownership articles
Learning: take_ownership when queuing, drop_ownership when dequeuing
Refinement: Updated all code examples with proper ownership flow

Iteration 10: Message Creation in Wrong BDD Section
Draft: Created message in "Given" section
Feedback: "Should be part of the When since it is the message being sent"
Learning: Message creation is part of the action, not setup
Refinement: Moved message creation from Given to When

Plan Approved: After 10 iterations of refinement
```

## Generalization

### The Iterative Refinement Cycle

```
1. DRAFT Phase
   - Create initial plan based on current understanding
   - Don't aim for perfection - aim for reviewability
   - Include examples so feedback can be specific

2. PRESENT Phase
   - Share plan with reviewer/user
   - Highlight areas of uncertainty
   - Ask specific questions if needed

3. FEEDBACK Phase
   - Receive corrections and suggestions
   - Note recurring patterns in feedback
   - Identify knowledge gaps

4. LEARN Phase
   - Consult KB articles mentioned in feedback
   - Study existing implementations referenced
   - Verify API availability (grep *.h files)
   - Understand the "why" behind corrections

5. REFINE Phase
   - Apply learnings to the entire plan
   - Look for similar issues throughout
   - Update examples consistently
   - Document new understanding

6. REPEAT
   - Continue cycle until feedback is minimal
   - Plan is approved when no methodology issues remain
   - Save approved plan before implementation
```

### Signs You Need More Iterations

- Feedback mentions KB articles you haven't read
- User corrects the same type of issue multiple times
- Plan references functions that don't exist
- Terminology is inconsistent throughout plan
- Examples don't follow codebase patterns

### Signs Plan Is Ready

- User says "looks good" or "approved"
- Feedback is minor wording, not methodology
- All code examples use real AgeRun types/functions
- Proper BDD structure throughout
- Ownership semantics are correct
- Function names follow conventions

## Implementation

### Before First Draft
```bash
# 1. Consult relevant KB articles
grep -l "TDD\|RED.*GREEN\|iteration" kb/*.md

# 2. Study existing implementations
grep -r "similar_function" modules/*.c

# 3. Verify API availability
grep "function_name" modules/*.h
```

### After Each Feedback Round
```bash
# 1. Read any KB articles mentioned in feedback
# Example: If feedback mentions BDD, read:
# kb/bdd-test-structure.md

# 2. Study referenced implementations
# Example: If feedback mentions ar_agent pattern:
grep -A 20 "ar_agent__send" modules/ar_agent.c

# 3. Check for similar issues throughout plan
# Example: If one test lacks BDD structure, check all tests

# 4. Document new understanding in plan file
```

### Tracking Refinement Progress
Keep a log of refinements:
```markdown
## Plan History
- v1: Initial draft (8 iterations)
- v2: Fixed terminology (cycle vs iteration)
- v3: One assertion per iteration (expanded to 14)
- v4: Added explicit failure comments
- v5: Minimal GREEN implementations
- v6: Fixed API availability
- v7: Corrected function naming
- v8: Added BDD structure
- v9: Fixed ownership semantics
- v10: Message in When section (APPROVED)
```

## Related Patterns
- [KB Consultation Before Planning Requirement](kb-consultation-before-planning-requirement.md) - Consult KB first
- [TDD Iteration Planning Pattern](tdd-iteration-planning-pattern.md) - One assertion per iteration
- [TDD Plan Iteration Split Pattern](tdd-plan-iteration-split-pattern.md) - Splitting iterations following minimalism
- [TDD GREEN Phase Minimalism](tdd-green-phase-minimalism.md) - Minimal implementations
- [TDD RED Phase Assertion Requirement](tdd-red-phase-assertion-requirement.md) - Proper RED phases
- [Temporary Test Cleanup Pattern](temporary-test-cleanup-pattern.md) - Memory management in minimal implementations
- [Plan Review Status Tracking Pattern](plan-review-status-tracking.md) - Tracking multi-session reviews
- [Iterative Plan Review Protocol](iterative-plan-review-protocol.md) - Incremental plan approval
- [BDD Test Structure](bdd-test-structure.md) - Test organization
- [API Behavior Verification](api-behavior-verification.md) - Verify API before use
- [Assumption Verification Before Action](assumption-verification-before-action.md) - Check assumptions
- [User Feedback as QA](user-feedback-as-qa.md) - Learning from corrections
