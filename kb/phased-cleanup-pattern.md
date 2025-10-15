# Phased Cleanup Pattern

## Learning
After major structural changes, perform cleanup in multiple phases: first review to identify all issues, then fix only critical items immediately, and defer non-critical items to appropriate future cycles with proper tracking. This prevents scope creep while ensuring critical issues are addressed.

## Importance
- Prevents scope creep in current work
- Ensures critical issues don't get missed
- Maintains focus on current cycle's goals
- Provides systematic approach to technical debt
- Keeps commits focused and reviewable

## Example
```c
// Scenario: After removing wake messages from ar_system (TDD Cycle 2)

// Stage 1: Comprehensive Review
/*
Review found:
1. ✅ Inconsistent test comments in ar_system_tests.c 
2. ⚠️ Sleep messages still active in ar_agent.c
3. ⚠️ ar_agent__update_method still sends wake/sleep
4. ✅ RED phase comment outdated in test
*/

// User direction: "Ignore items 2 & 3, as they will be addressed in the next cycles"

// Stage 2: Fix Critical Items (1 & 4)
// ar_system_tests.c - Remove outdated comments
-    // Process the wake message that the agent sent to itself
-    ar_system__process_next_message(mut_system);
+    // No wake message to process (removed in TDD Cycle 1)

// Stage 3: Track Deferred Items in TODO.md
/*
##### TDD Cycle 3: Remove sleep messages from ar_agent and ar_agency
- Will address sleep messages (item 2)

##### TDD Cycle 4: Update all methods to remove wake/sleep detection  
- Will address ar_agent__update_method (item 3)
*/

// WRONG: Trying to fix everything immediately
// This would expand scope beyond current cycle
```

## Generalization
Phased cleanup process:
1. **Review Phase**: Identify ALL issues comprehensively
2. **Categorize Phase**: Mark items as critical vs. non-critical
3. **Fix Phase**: Address only critical items for current scope
4. **Track Phase**: Document deferred items in TODO/tracking system
5. **Verify Phase**: Confirm deferred items have proper cycle assignment

Critical vs. Non-critical:
- Critical: Breaks current functionality, inconsistent with current changes
- Non-critical: Future work, different subsystem, planned for later cycle

## Implementation
```c
// Pattern for phased cleanup:

typedef enum {
    PRIORITY_CRITICAL,    // Must fix now
    PRIORITY_DEFERRED,   // Fix in planned future cycle
    PRIORITY_TECHNICAL_DEBT  // Track but no immediate plan
} cleanup_priority_t;  // EXAMPLE: Hypothetical enum for teaching

typedef struct {
    char *description;
    char *location;
    cleanup_priority_t priority;  // EXAMPLE: Using hypothetical enum
    int target_cycle;  // Which TDD cycle will address
} cleanup_item_t;  // EXAMPLE: Hypothetical struct for teaching

void perform_phased_cleanup(cleanup_item_t *items, int count) {  // EXAMPLE: Using hypothetical types
    // Stage 1: Review and categorize
    printf("=== Cleanup Review ===\n");
    for (int i = 0; i < count; i++) {
        printf("%d. %s at %s [%s]\n", 
               i+1, items[i].description, items[i].location,
               priority_to_string(items[i].priority));
    }
    
    // Stage 2: Fix critical only
    printf("\n=== Fixing Critical Items ===\n");
    for (int i = 0; i < count; i++) {
        if (items[i].priority == PRIORITY_CRITICAL) {
            fix_issue(&items[i]);
        }
    }
    
    // Stage 3: Track deferred
    printf("\n=== Tracking Deferred Items ===\n");
    for (int i = 0; i < count; i++) {
        if (items[i].priority == PRIORITY_DEFERRED) {
            add_to_todo(items[i].description, items[i].target_cycle);
        }
    }
}
```

## Related Patterns
- [Comprehensive Impact Analysis](comprehensive-impact-analysis.md)
- [Selective Compaction Pattern](selective-compaction-pattern.md)
- [Multi-Session TDD Planning](multi-session-tdd-planning.md)