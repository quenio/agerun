# Assumption Verification Before Action

## Learning

Before making any changes based on reported issues, verify all assumptions with concrete evidence. User feedback that challenges assumptions often prevents unnecessary work and reveals the real problem.

## Importance

Verifying assumptions:
- Prevents fixing non-existent problems
- Saves time by avoiding unnecessary changes
- Reduces risk of introducing new bugs
- Reveals the actual root cause of issues

The question "Why would we do X if Y doesn't show the problem?" is a powerful verification tool.

## Example

```c
// Assumption: CI errors mean code needs fixing
ar_data_t* own_assumption = ar_data__create_string("CI failed, must fix docs");

// User challenges: "Why make changes if check-docs passes locally?"
// This prompts verification:

// 1. Check if error is current
// Result: Error from old CI run

// 2. Verify local validation
ar_data_t* own_validation = ar_data__create_integer(1);  // Passes (1 = true)

// 3. Evidence contradicts assumption
// Conclusion: No changes needed

ar_data__destroy(own_assumption);
ar_data__destroy(own_validation);
```

## Generalization

Before taking action based on assumptions:
1. State the assumption explicitly
2. Identify what evidence would confirm/refute it
3. Gather that evidence systematically
4. Challenge the assumption: "Why would we X if Y?"
5. Only proceed if evidence supports the assumption
6. Thank users who challenge your assumptions

## Implementation

```bash
# Verification checklist before making changes

# 1. Can I reproduce the issue?
make check-docs  # or relevant validation

# 2. Is the issue current?
git log --since="1 day ago"  # Recent changes?
gh run list --limit=5  # Recent CI status?

# 3. Does the fix make sense?
# If local validation passes, why would CI fail?
# What's different between environments?

# 4. What evidence supports the change?
# Concrete error messages
# Reproducible failures
# Clear root cause

# 5. What evidence contradicts it?
# Passing tests
# Working functionality
# Historical fixes
```

## Related Patterns

- [Evidence-Based Debugging](evidence-based-debugging.md)
- [User Feedback as QA](user-feedback-as-qa.md)
- [Local-CI Discrepancy Investigation](local-ci-discrepancy-investigation.md)
- [Tool Version Selection Due Diligence](tool-version-selection-due-diligence.md)