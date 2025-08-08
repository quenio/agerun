# Task Authorization Pattern

## Learning
Never proceed with tasks shown as "in_progress" or "pending" in TODO lists without explicit user instruction, even when continuing from a previous session. Task state indicates planning status, not authorization to proceed.

## Importance
- Respects user control over session priorities and direction
- Prevents unwanted work that may conflict with user's current goals
- Ensures alignment between assistant actions and user intent
- Avoids wasted effort on tasks that may have changed priority
- Maintains clear boundaries between sessions

## Example
```c
// Session context showed this in TODO:
// "TDD Cycle 6: Add bootstrap agent creation in executable" - status: in_progress

// WRONG: Automatically continuing without instruction
static void test_bootstrap_agent_creation(void) {
    // Started implementing test without being asked
    AR_ASSERT(found_bootstrap_creation, "Should see bootstrap agent");
}

// RIGHT: Wait for explicit instruction
// User: "Let's continue with TDD Cycle 6"
// Assistant: "I'll now implement TDD Cycle 6 - adding bootstrap agent creation"
static void test_bootstrap_agent_creation(void) {
    // Implementation begins only after explicit request
}
```

## Generalization
Apply task authorization discipline consistently:
1. **Session boundaries**: Each new session requires fresh instructions
2. **TODO status**: "in_progress" or "pending" indicates planning, not permission
3. **Explicit requests**: Wait for clear user direction like "continue with X" or "implement Y"
4. **Clarification first**: When unsure, ask "What would you like me to work on?"
5. **Previous context**: Use it for reference, not as automatic instruction

## Implementation
```c
// Check pattern when resuming work:
ar_methodology_t* mut_methodology = ar_methodology__create();

// Before starting any task:
// 1. Review TODO/context for awareness
// 2. Wait for user instruction
// 3. Confirm understanding: "I'll work on [specific task]"
// 4. Only then proceed with implementation

// Example workflow:
if (user_instruction_received) {
    // Proceed with requested task
    ar_method_t* own_method = ar_methodology__get_method_with_instance(
        mut_methodology, "requested_task", "1.0.0"
    );
} else {
    // Wait for direction, don't assume
    // Review context but don't act
}
```

## Related Patterns
- [User Feedback as QA](user-feedback-as-qa.md)
- [Plan Verification and Review](plan-verification-and-review.md)
- [Frank Communication Principle](frank-communication-principle.md)