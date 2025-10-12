# User Feedback as Architecture Gate

## Learning
User domain knowledge serves as a critical architecture quality gate during planning phases. Mid-implementation insights from users can reveal missing infrastructure that systematic analysis overlooked. Explicitly pausing after planning to ask users "Is anything missing?" catches gaps before they block implementation.

## Importance
Even thorough systematic planning can miss critical components. Users bring domain expertise that reveals requirements  not captured in code analysis. In this session, user insight identified missing delegate message queue infrastructure that would have blocked TDD Cycle 7 (routing). Catching this during planning prevented implementing a broken architecture.

## Example
```c
// Scenario: Planning delegate system implementation

// SYSTEMATIC ANALYSIS identified:
// - ar_delegate_t structure
// - ar_delegate__create/destroy
// - ar_delegation_t wrapper
// - ar_delegate_registry for tracking

// Implementation plan looked complete...

// USER INSIGHT (during planning phase):
// "I realized that we need to implement a message queue for each delegate,
//  like it was previously done for each agent. We need to add this task
//  to the implementation plan of the delegate system."

// This revealed MISSING INFRASTRUCTURE:
struct ar_delegate_s {
    ar_log_t *ref_log;
    const char *type;
    // MISSING: ar_list_t *own_message_queue  // EXAMPLE: Infrastructure to be added
};

// Without this, TDD Cycle 7 (routing) would fail:
bool ar_send_evaluator__evaluate(...) {
    if (agent_id < 0) {
        // Route to delegation
        return delegation_send(...);  // EXAMPLE: Would fail - no queue exists!
    }
}

// With queue infrastructure added:
// - delegate_send() function  // EXAMPLE: To be implemented
// - delegate_has_messages() function  // EXAMPLE: To be implemented
// - delegate_get_message() function  // EXAMPLE: To be implemented
// - delegation_send_to_delegate() routing  // EXAMPLE: To be implemented
// Now routing can deliver messages successfully
```

## Generalization
**User Feedback as Quality Gate Pattern**:

### Planning Phase Checkpoints
After completing systematic planning, explicitly ask:

1. **Architecture Completeness**:
   - "Does this mirror the pattern completely, or are we missing components?"
   - "What infrastructure pieces are we assuming exist?"

2. **Integration Points**:
   - "Will this work with the existing system as planned?"
   - "Are we creating an incomplete implementation?"

3. **Sister Module Symmetry**:
   - "If module X has feature Y, does sister module Z need it too?"
   - "What worked well in the original that we might be skipping?"

### When to Engage User
- After completing initial implementation plan
- Before starting TDD cycles
- When creating sister modules to existing ones
- After API refactorings that change interfaces
- Before committing to multi-cycle implementation sequences

### Response to User Insights
```markdown
# User says: "We need X infrastructure before Y"

1. **Acknowledge**: "You're right - that's critical infrastructure"
2. **Verify scope**: Understand what X entails completely
3. **Insert prerequisite**: Add as cycle X.5 before Y
4. **Document rationale**: Explain why X is needed for Y
5. **Update plan**: Show X as dependency in updated TODO.md
```

## Implementation
Systematic user engagement for architecture validation:

```bash
#!/bin/bash
# Architecture review checklist before implementation

echo "=== Architecture Review Checklist ==="
echo ""
echo "1. Sister Module Symmetry:"
echo "   - Compare structures side-by-side"
echo "   - List functions in both modules"
echo "   - Identify missing functions"
echo ""
echo "2. Infrastructure Completeness:"
echo "   - Message queues for entities that receive messages"
echo "   - Registries for entities that need lookup"
echo "   - State management for entities with lifecycle"
echo ""
echo "3. Integration Point Verification:"
echo "   - How will new code be called?"
echo "   - What does calling code expect to exist?"
echo "   - Are we providing complete interfaces?"
echo ""
echo "PAUSE HERE: Present plan to user with question:"
echo "'Have we missed any critical infrastructure?'"
```

Real example from session:
```markdown
# Planned Implementation:
- ✓ TDD Cycle 6: Propagate delegation parameter
- ✗ TDD Cycle 7: Implement routing (BLOCKED - missing infrastructure!)

# User Insight:
"We need message queue infrastructure for delegates"

# Updated Plan:
- ✓ TDD Cycle 6: Propagate delegation parameter
- → TDD Cycle 6.5: Implement message queue infrastructure (PREREQUISITE)
- → TDD Cycle 7: Implement routing (now unblocked)
```

## Related Patterns
- [Symmetric Module Architecture Pattern](symmetric-module-architecture-pattern.md)
- [Architectural Review in Feedback](architectural-review-in-feedback.md)
- [Implementation Plan Line References](implementation-plan-line-references.md)
- [Assumption Verification Before Action](assumption-verification-before-action.md)
