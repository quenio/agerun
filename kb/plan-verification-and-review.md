# Plan Verification and Review

## Overview

Plan verification and review is a critical practice that ensures development plans are complete, accurate, and likely to succeed before execution begins. This practice prevents wasted effort, catches missing steps early, and leverages collaborative review to improve plan quality.

## Core Principles

### 1. Single Task Focus
Create plans for one todo item at a time, not multi-task plans. This ensures each plan receives proper attention and detail.

**Bad**: "Plan for refactoring all modules and adding new features"  
**Good**: "Plan for extracting ownership handling functions from evaluators"

### 2. Include Critical Verification Steps
Every plan must include concrete verification steps to ensure correctness.

**Essential verifications**:
- Diff verification for code movement
- Test execution for behavior preservation  
- Memory leak checking for resource management
- Documentation updates for interface changes

### 3. Embrace User Feedback
User questions and corrections are valuable quality assurance, not criticism.

**Example feedback that improves plans**:
- "We're missing the comparison of previous and new implementation"
- "Did you consider the impact on existing tests?"
- "What about updating the documentation?"

### 4. Iterative Refinement Through Multiple Rounds
Plans often require multiple refinement iterations based on user feedback to reach optimal form.

**Common refinement patterns**:
- Combining iterations that test the same behavior
- Ensuring each iteration has a real failing test in RED phase
- Placing fixture updates within dependent cycles, not separately
- Separating module documentation from project documentation
- Removing redundant build verification steps when user handles commits

**Example refinement progression**:
- Initial: "Iterations 1.1 and 1.2 both test wake message removal"
- Feedback: "Iterations 1.1 and 1.2 should be combined"
- Refined: Single iteration testing complete wake removal
- Result: Clearer, more focused plan

## Plan Structure Template

A well-structured plan should include:

### 1. Objective Statement
Clear, single-sentence description of what will be accomplished.

### 2. Preconditions
What must be true before starting:
- Tests currently passing
- No memory leaks present
- Documentation up to date

### 3. Detailed Steps
Numbered list with specific actions:
1. Read relevant KB articles
2. Run baseline tests
3. Make specific code changes
4. Verify with tests
5. Check for memory leaks
6. Update documentation

### 4. Verification Criteria
How to confirm success:
- All tests pass
- Zero memory leaks
- Documentation reflects changes
- No behavioral changes (unless intended)

### 5. Rollback Strategy
How to recover if things go wrong:
- Git stash/reset commands
- Backup file locations
- Clean build instructions

## Common Plan Elements

### For Refactoring Tasks
```markdown
1. Run baseline tests to ensure clean starting state
2. Create backup of current implementation
3. Extract common pattern to new location
4. Update all call sites to use new pattern
5. Verify with diff that code was moved, not rewritten
6. Run tests to ensure behavior unchanged
7. Check memory reports for any new leaks
8. Update module documentation
9. Remove old implementation
```

### For Bug Fixes
```markdown
1. Reproduce the bug with minimal test case
2. Add failing test that demonstrates the bug
3. Investigate root cause with debugging tools
4. Implement fix with minimal changes
5. Verify test now passes
6. Run full test suite for regressions
7. Check memory reports
8. Update relevant documentation
```

### For New Features
```markdown
1. Write failing test for new behavior (TDD Red phase)
2. Implement minimal code to pass test (TDD Green phase)
3. Refactor for clarity (TDD Refactor phase)
4. Add comprehensive test coverage
5. Update interface documentation
6. Run full test suite
7. Verify zero memory leaks
8. Update user-facing documentation
```

## Verification Checklist

Before approving a plan, verify:

### Completeness
- [ ] All necessary steps included
- [ ] Verification steps specified
- [ ] Documentation updates mentioned
- [ ] Error handling considered
- [ ] File paths and line numbers specific

### Accuracy  
- [ ] Technical approach is sound
- [ ] Dependencies identified
- [ ] Correct modules/files referenced
- [ ] Appropriate tools selected

### Risk Management
- [ ] Backup/rollback strategy exists
- [ ] Impact on other modules considered
- [ ] Breaking changes identified
- [ ] Performance implications noted

## Review Process

### Self-Review Questions
Before presenting a plan:

1. **Is this the simplest approach?**
   - Could steps be eliminated?
   - Is there unnecessary complexity?

2. **What could go wrong?**
   - What assumptions am I making?
   - What edge cases exist?

3. **Is verification adequate?**
   - How will I know it worked?
   - What specific outputs confirm success?

4. **Are all impacts considered?**
   - What else might break?
   - Who else is affected?

### Collaborative Review
When reviewing plans with others:

1. **State assumptions explicitly**
   - "This assumes the current tests are comprehensive"
   - "This requires the build system to support X"

2. **Welcome challenges**
   - "What am I missing?"
   - "Do you see any potential issues?"

3. **Incorporate feedback immediately**
   - Update the plan during review
   - Thank reviewers for improvements

## Common Planning Mistakes

### 1. Overly Ambitious Scope
**Problem**: Trying to accomplish too much in one plan  
**Solution**: Break into smaller, focused plans

### 2. Missing Verification Steps
**Problem**: No way to confirm success  
**Solution**: Add specific checks after each major step

### 3. Vague Descriptions
**Problem**: "Update the code" without specifics  
**Solution**: Include file names, function names, line numbers

### 4. Ignoring Dependencies
**Problem**: Not considering impact on other modules  
**Solution**: Check uses with grep, review module hierarchy

### 5. No Rollback Plan
**Problem**: No recovery strategy if things go wrong  
**Solution**: Include git commands, backup steps

## Plan Improvement Examples

### Before: Vague Plan
```
1. Refactor the evaluators
2. Extract common code
3. Test everything
4. Update docs
```

### After: Specific Plan
```
1. Read kb/refactoring-key-patterns.md for guidance
2. Run `make ar_expression_evaluator_tests` to establish baseline
3. Search for _store_result pattern: `grep -n "_store_result" modules/*evaluator.c`
4. Create new function in ar_instruction_ast.c at line 145
5. Replace pattern in ar_spawn_evaluator.c (lines 78-102) with new function
6. Verify code moved correctly: `diff -u <(sed -n '78,102p' old.c) new_location.c`
7. Run `make ar_spawn_evaluator_tests` to verify behavior
8. Check memory: `grep "Actual memory leaks:" bin/memory_report_spawn.log`
9. Repeat for other evaluators: build, parse, compile, exit, deprecate
10. Update ar_instruction_ast.md with new function documentation
11. Run `make build` for full verification
```

## Integration with Development Workflow

### Before Starting Work
1. Create detailed plan
2. Review against checklist
3. Get feedback if uncertain
4. Update based on feedback

### During Execution
1. Follow plan step-by-step
2. Note any deviations
3. Update plan if approach changes
4. Verify at checkpoints

### After Completion
1. Confirm all steps completed
2. Verify success criteria met
3. Document any lessons learned
4. Update KB if new patterns discovered

## Related Patterns
- [User Feedback as QA](user-feedback-as-qa.md)
- [Evidence-Based Debugging](evidence-based-debugging.md)
- [TDD Red-Green-Refactor Cycle](red-green-refactor-cycle.md)
- [TDD Cycle Completion Verification Pattern](tdd-cycle-completion-verification-pattern.md)
- [Plan Document Completion Status Pattern](plan-document-completion-status-pattern.md)
- [Plan Review Status Tracking Pattern](plan-review-status-tracking.md)
- [Section-by-Section Review Protocol](section-by-section-review-protocol.md)
- [TDD Plan Iteration Split Pattern](tdd-plan-iteration-split-pattern.md)
- [Iterative Plan Refinement Pattern](iterative-plan-refinement-pattern.md)
- [Build Verification Before Commit](build-verification-before-commit.md)
- [Task Authorization Pattern](task-authorization-pattern.md)
- [Multi-Session TDD Planning](multi-session-tdd-planning.md)
- [Fixture Dependency Mapping](fixture-dependency-mapping.md)