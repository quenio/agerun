# Local-CI Discrepancy Investigation

## Learning

When local validation tools pass but CI reportedly fails, the discrepancy itself is the most important clue. Rather than blindly "fixing" code to match CI errors, investigate why the environments disagree.

## Importance

Discrepancies between local and CI environments usually indicate:
- The CI error is from an old or cached run
- Environment or version differences between local and CI
- The error has already been fixed in the current codebase
- Different configuration or flags being used

Trusting local validation when it uses the same tools prevents unnecessary changes.

## Example

```c
// Local validation with check_docs.py
ar_data_t* own_result = ar_data__create_string("test");
ar_data__destroy(own_result);  // Proper cleanup

// Local: make check-docs → PASSED ✓
// CI: "Invalid reference to message_field_t"  // EXAMPLE: Type that doesn't exist

// Investigation reveals:
// 1. Local script is current version
// 2. CI error is from old run
// 3. Both use same check_docs.py
// 4. Conclusion: Trust local validation
```

## Generalization

When local and CI validation disagree:
1. Verify both are running the same version of validation tools
2. Check if CI error is from a current run
3. Compare the exact commands being executed
4. Look for environment-specific configuration
5. Trust local validation if tools and versions match
6. Investigate the source of discrepancy before making changes

## Implementation

```bash
# Compare local and CI validation
make check-docs  # Run locally
echo "Exit code: $?"

# Check what CI is running
gh run view <run_id> --log | grep "check-docs"

# Verify script versions match
git log -1 --format="%H %ai" -- scripts/check_docs.py

# Check for configuration differences
diff .github/workflows/ci.yml Makefile
```

## Related Patterns

- [Issue Currency Verification Pattern](issue-currency-verification-pattern.md)
- [Evidence-Based Debugging](evidence-based-debugging.md)
- [CI Network Timeout Diagnosis](ci-network-timeout-diagnosis.md)
- [GitHub Actions Deprecated Tool Migration](github-actions-deprecated-tool-migration.md)