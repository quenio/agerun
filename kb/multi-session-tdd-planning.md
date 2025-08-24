# Multi-Session TDD Planning

## Learning
Large architectural changes can be broken into detailed TDD plans with session tracking checkboxes, enabling systematic progress across multiple work sessions while maintaining context and momentum.

## Importance
Complex refactorings often span days or weeks. Session tracking prevents loss of context, ensures systematic progress, and allows different team members to continue work with clear understanding of what's been completed.

## Example
```markdown
### TDD Cycle 1: Remove wake messages from ar_agent
**Session tracking**: [ ] Started [ ] Completed

#### Iteration 1.1: Remove wake message sending from agent creation
- **RED**: Modify ar_agent_tests.c to expect no wake message → Test FAILS
- **GREEN**: Remove wake message sending from ar_agent__create (lines 65-71)
- **REFACTOR**: Clean up initialization flow

#### Iteration 1.2: Update ar_system_fixture
- **RED**: Tests using fixture fail because it expects wake messages
- **GREEN**: Update ar_system_fixture__process_wake_messages to be no-op
- **REFACTOR**: Consider renaming or removing function

**Success**: ar_agent_tests passes with zero memory leaks
**Documentation**: Update ar_agent.md

### TDD Cycle 2: Remove wake messages from ar_system
**Session tracking**: [ ] Started [ ] Completed
[continues...]
```

## Generalization
For multi-session work:
1. Break work into independently committable cycles
2. Add session tracking checkboxes for each cycle
3. Include specific line numbers and file references
4. Document success criteria for each cycle
5. Note documentation updates within cycles
6. Specify which tests verify success
7. Keep iterations small enough for single work sessions

## Implementation
```bash
# Starting a session - check progress
grep "Session tracking" TODO.md | grep -c "\[x\]"  # Completed cycles
grep "Session tracking" TODO.md | grep -c "\[ \]"  # Remaining cycles

# Mark cycle as started
# Update: **Session tracking**: [x] Started [ ] Completed

# After completing cycle
# Update: **Session tracking**: [x] Started [x] Completed

# Commit at cycle boundaries for clean history
git commit -m "feat: complete TDD Cycle N - [description]"
```

## Related Patterns
- [TDD Cycle Effort Estimation](tdd-cycle-effort-estimation.md)
- [TDD Advanced Large Refactoring](tdd-advanced-large-refactoring.md)
- [TDD Cycle Detailed Explanation](tdd-cycle-detailed-explanation.md)
- [Progressive System Decoupling TDD](progressive-system-decoupling-tdd.md)
- [Plan Verification and Review](plan-verification-and-review.md)