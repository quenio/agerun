# Task Verification Before Execution

## Learning
Before implementing any planned task from TODO.md or other documentation, always verify the task is actually needed by checking the current code state. Documentation can become outdated when implementation exceeds initial planning.

## Importance
Prevents wasted effort on unnecessary tasks. In this session, Phase 2c was planned to create 5 instance functions, but investigation revealed they were already removed in Phase 2b, saving significant implementation time.

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
4. Verify build/tests pass without the "missing" functionality
5. Check git history to understand what was actually done
6. Only proceed if verification confirms the task is needed

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

## Related Patterns
- [Documentation Completion Verification](documentation-completion-verification.md)
- [Retroactive Task Documentation](retroactive-task-documentation.md)
- [Commit Scope Verification](commit-scope-verification.md)