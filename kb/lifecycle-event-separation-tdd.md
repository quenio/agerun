# Lifecycle Event Separation in TDD

## Learning
When removing or modifying lifecycle events in a system, separate changes to different lifecycle phases (creation, destruction, updates) into distinct TDD cycles to maintain clarity about what's changing at each step.

## Importance
Lifecycle events occur at different points in an object's lifetime and affect different parts of the codebase. Mixing changes to multiple lifecycle phases in one cycle creates confusion and makes it harder to identify which change caused a failure.

## Example
```c
// Wrong: Combining wake (creation) and sleep (destruction) in one cycle
// TDD Cycle 1: Remove all lifecycle messages
//   - Remove wake from ar_agent__create
//   - Remove sleep from ar_agent__destroy
//   - Remove both constants
// Problem: Too many changes, unclear failure sources

// Right: Separate cycles for different lifecycle events
// TDD Cycle 1: Remove wake messages (creation phase)
//   - Remove wake from ar_agent__create (lines 65-71)
//   - Update fixtures that expect wake
//   - Focus only on creation behavior

// TDD Cycle 2: Remove wake from system
//   - Remove duplicate wake from ar_system__init
//   - Remove g_wake_message constant

// TDD Cycle 3: Remove sleep messages (destruction phase)
//   - Remove sleep from ar_agent__destroy (lines 96-106)
//   - Remove sleep from ar_agent__update_method (lines 287-303)
//   - Remove g_sleep_message constant
//   - Focus only on destruction behavior
```

## Generalization
When modifying lifecycle events:
1. Identify all lifecycle phases involved (create, update, destroy)
2. Create separate TDD cycles for each phase
3. Complete all changes for one phase before moving to next
4. Test each phase independently
5. Document which phase each cycle addresses
6. Consider dependencies between phases

## Implementation
```bash
# Identify lifecycle functions
grep -n "create\|destroy\|update\|init\|shutdown" modules/ar_agent.c

# Plan separate cycles
echo "Cycle 1: Creation phase - wake messages"
grep -n "wake" modules/ar_agent.c | grep create

echo "Cycle 2: Destruction phase - sleep messages"  
grep -n "sleep" modules/ar_agent.c | grep destroy

echo "Cycle 3: Update phase - lifecycle transitions"
grep -n "sleep\|wake" modules/ar_agent.c | grep update

# Verify each phase independently
make ar_agent_tests  # After each cycle
```

## Related Patterns
- [Multi-Session TDD Planning](multi-session-tdd-planning.md)
- [TDD Cycle Detailed Explanation](tdd-cycle-detailed-explanation.md)
- [Progressive System Decoupling TDD](progressive-system-decoupling-tdd.md)
- [Separation of Concerns Principle](separation-of-concerns-principle.md)