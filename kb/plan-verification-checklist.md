# Plan Verification Checklist

## Learning

This pattern provides comprehensive checklists and examples for creating and verifying development plans, ensuring critical steps aren't missed and user feedback is incorporated.

## Importance

Incomplete plans lead to bugs, missing documentation, and rework. A systematic checklist ensures all aspects are considered upfront, and user feedback during planning prevents implementation errors.

## Example

### Incomplete Plan (Missing Critical Steps)
```
Plan: Implement new feature X
1. Write the code
2. Test it
3. Commit
```

### Complete Plan (With Verification)
```
Plan: Implement new feature X
1. Search KB for similar patterns
2. Run existing tests first (baseline)
3. TDD Cycle 1: Red (write failing test)
4. TDD Cycle 1: Green (implement minimal code)
5. TDD Cycle 1: Refactor if needed
6. Run full test suite
7. Check memory leaks: grep "Actual memory leaks:" bin/run-tests/memory_report_*.log
8. Update documentation if API changed
9. Verify with diff: old vs new implementation
10. Update TODO.md and CHANGELOG.md
11. Final build verification: make clean build 2>&1
12. Single atomic commit
```

## Generalization

### Master Plan Completeness Checklist

**Essential Verification Steps**:
- [ ] Diff verification for moved/modified code
- [ ] Test running (individual then suite)
- [ ] Memory leak checking
- [ ] Build verification
- [ ] Documentation updates for interface changes

**File/Location Specificity**:
- [ ] File paths specified with line numbers
- [ ] Exact function/method names listed
- [ ] Module dependencies identified

**Success Criteria**:
- [ ] Define what "working" means
- [ ] Specify expected test outcomes
- [ ] List performance requirements
- [ ] Memory leak tolerance (must be 0)

**Error Handling**:
- [ ] Rollback strategy if tests fail
- [ ] Debug approach for failures
- [ ] User communication plan

### Module-Specific Considerations

Different modules have unique constraints that plans must account for:

**Parser Modules**:
- Method parser: All 3 args must be quoted strings
- Build parser: Second arg must be expression
- Instruction parser: Check existing tests for patterns

**Generic vs Specific Design**:
- Consider if a generic solution serves multiple uses
- Example: ar_path instead of ar_memory_path
- Benefits: Less duplication, broader utility

## Implementation

### Plan Template for Feature Implementation

```markdown
## Plan: [Feature Name]

### Pre-Implementation
1. Search KB: grep -r "[feature]" kb/*.md
2. Read existing tests in [module]_tests.c
3. Check SPEC.md for language constraints
4. Baseline: make [module]_tests 2>&1

### TDD Cycles
For each behavior:
1. Write failing test (RED)
2. Minimal implementation (GREEN)  
3. Refactor if needed
4. Verify: make [test_name] 2>&1

### Verification
1. Full suite: make run-tests 2>&1
2. Memory: grep "Actual memory leaks:" bin/run-tests/memory_report_*.log
3. Sanitizer: make sanitize-tests 2>&1
4. Documentation: make check-docs

### Integration
1. Update module documentation
2. Update TODO.md with completion
3. Update CHANGELOG.md
4. Final build: make clean build 2>&1

### Commit
Single atomic commit with all changes
```

### User Feedback Integration

**When User Points Out Missing Steps**:
1. Immediately update the plan
2. Thank them for the catch (it prevents bugs)
3. Add the step to your checklist
4. Example: "You're right, I missed the diff verification step"

**Common User Corrections**:
- Missing documentation updates
- Forgot to verify old vs new behavior
- No memory leak checking
- Missing error handling consideration

### Plan Scope Management

**Single Task Focus**:
- One TODO item = one plan
- Don't combine multiple features
- Break large tasks into sub-plans

**Time Estimation**:
- Estimate in TDD cycles, not hours
- Account for debugging time
- Include documentation time

## Related Patterns

- [TDD Cycle Detailed Explanation](tdd-cycle-detailed-explanation.md)
- [Pre-Commit Checklist Detailed](pre-commit-checklist-detailed.md)
- [User Feedback as QA](user-feedback-as-qa.md)
- [Task Authorization Pattern](task-authorization-pattern.md)
- [Plan Verification and Review](plan-verification-and-review.md)