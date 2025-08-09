# Struggling Detection Pattern

## Learning
When making multiple failed attempts to solve the same problem (typically 3 or more), stop and ask for guidance instead of continuing to try variations. This "struggling detection" prevents wasted effort and often reveals fundamental misunderstandings or missing context that no amount of trial-and-error will resolve.

## Importance
- Prevents wasted time on incorrect approaches
- Reveals fundamental misunderstandings early
- Gets expert guidance when stuck in wrong mental model
- Avoids compounding errors through multiple failed attempts
- Maintains momentum by getting unstuck quickly

## Example
```c
// Scenario: Trying to fix failing test in TDD RED phase

// Attempt 1: Change method implementation
const char *method = "memory.count := 1";  // Test still fails

// Attempt 2: Try different field name
const char *method = "memory.ran := 1";  // Test still fails

// Attempt 3: Try another variation
const char *method = "memory.executed := 1";  // Test still fails

// User intervention: "When you are struggling, just stop and ask for guidance"

// RIGHT: After 2-3 failed attempts, ask for help
/*
"I'm struggling to get this test to fail properly. I've tried:
1. Setting memory.count := 1 - failed with frame creation error
2. Setting memory.ran := 1 - same error
3. Setting memory.executed := 1 - same error

The error 'Failed to create execution frame' suggests a deeper issue.
Could you provide guidance on the correct approach?"
*/

// Discovery after asking: The problem wasn't the method at all,
// but that ar_system was passing NULL context to agents
```

## Generalization
Struggling detection triggers:
- 3+ attempts at variations of same approach
- Cycling between previously tried solutions
- Making changes without understanding why
- Error messages don't change despite different attempts
- Feeling of "trying everything" without progress

Response protocol:
1. Stop making changes
2. Document what was tried and results
3. Identify the core problem
4. Ask for guidance with context
5. Listen for deeper insights in response

## Implementation
```c
// Pattern for detecting struggling:

typedef struct {
    int attempt_count;
    char *last_error;
    char *attempted_solutions[10];
} struggle_detector_t;  // EXAMPLE: Hypothetical struct for teaching

bool should_ask_for_help(struggle_detector_t *detector, const char *current_error) {  // EXAMPLE: Using hypothetical type
    // Same error after multiple attempts = struggling
    if (detector->attempt_count >= 3 && 
        strcmp(detector->last_error, current_error) == 0) {
        return true;
    }
    
    // Cycling through same solutions = struggling
    for (int i = 0; i < detector->attempt_count; i++) {
        if (solution_already_tried(detector->attempted_solutions[i])) {
            return true;
        }
    }
    
    return false;
}

void handle_error_with_struggle_detection(const char *error) {
    static struggle_detector_t detector = {0};  // EXAMPLE: Using hypothetical type
    
    if (should_ask_for_help(&detector, error)) {
        printf("I'm struggling with this problem after %d attempts.\n", 
               detector.attempt_count);
        printf("Consistent error: %s\n", error);
        printf("I should ask for guidance.\n");
        request_user_guidance();
    } else {
        detector.attempt_count++;
        try_next_approach();
    }
}
```

## Related Patterns
- [Evidence-Based Debugging](evidence-based-debugging.md)
- [User Feedback as QA](user-feedback-as-qa.md)
- [Architectural Review in Feedback](architectural-review-in-feedback.md)