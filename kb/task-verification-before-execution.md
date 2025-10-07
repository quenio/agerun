# Task Verification Before Execution

## Learning
Before implementing any planned task from TODO.md or other documentation, always verify the task is actually needed by checking the current code state. Documentation can become outdated when implementation exceeds initial planning.

## Importance
Prevents wasted effort on unnecessary tasks. Examples:
- Phase 2c was planned to create 5 instance functions, but they were already removed in Phase 2b
- TDD Cycle 10 was listed as incomplete, but implementation and tests already existed
- YAML parser bug blocking Cycle 9 was already fixed in commit 5ef1ce6

Verification can reveal work is 50-75% more complete than documentation indicates.

## Example
Verifying if functions need to be created:
```bash
# Check if functions exist in current code
grep "is_agent_active" modules/ar_agency.c
# No results means function doesn't exist

# Check git history to see if they were removed
git diff HEAD~1 HEAD -- modules/ar_agency.c | grep "^-.*is_agent_active"
# Shows: -bool is_agent_active(int64_t agent_id) {

# Verify build passes without them
make clean build 2>&1 | grep -i "undefined"
# No undefined function errors means they're not needed
```

When checking ar_agency module tasks:
```c
// TODO said to create these functions:
// EXAMPLE: Functions that were planned but not needed
// bool is_agent_active_with_instance(void *ref_agency, int64_t agent_id);
// bool set_agent_active_with_instance(void *mut_agency, int64_t agent_id, bool active);

// But checking showed they were already removed and not needed
// The ar_agent module already has these functions:
bool ar_agent__is_active(const ar_agent_t *ref_agent);
void ar_agent__set_active(ar_agent_t *mut_agent, bool is_active);
```

## Generalization
1. Read the task description from TODO/documentation
2. Check if the code already handles this case
3. Search for existing implementations
4. **Run tests to verify current state**
5. Check git history to understand what was actually done
6. Verify build/tests pass without the "missing" functionality
7. Only proceed if verification confirms the task is needed

**Discovery pattern**: Tasks marked incomplete often have implementation that's further along than documented

## Implementation
```bash
# Task verification workflow
TASK="Create ar_agency__is_agent_active_with_instance"

# 1. Check current state
grep -r "is_agent_active" modules/ --include="*.h" --include="*.c"

# 2. Check if already removed
git log --oneline -S "is_agent_active" | head -5

# 3. Verify build status
make build 2>&1 | tee build.log
grep -i "error\|undefined" build.log

# 4. Update TODO if task not needed
# Mark as [x] with explanation of why not needed
```

**Discovering already-complete work**:
```bash
# Example: Verifying TDD Cycle status
make module_tests 2>&1 | tail -20  # Check if tests pass

# Check implementation exists
grep -A 30 "function_name" modules/module.c

# Verify in memory report
grep "Actual memory leaks:" bin/run-tests/memory_report_module_tests.log

# If all pass: Work is complete, update TODO.md
```

## Related Patterns
- [Systematic Task Analysis Protocol](systematic-task-analysis-protocol.md)
- [Evidence-Based Debugging](evidence-based-debugging.md)
- [Documentation Completion Verification](documentation-completion-verification.md)
- [Retroactive Task Documentation](retroactive-task-documentation.md)
- [Commit Scope Verification](commit-scope-verification.md)