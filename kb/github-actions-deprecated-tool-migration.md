# GitHub Actions Deprecated Tool Migration

## Learning
When GitHub Actions fail with network timeouts (ETIMEDOUT), the root cause may be a deprecated or unmaintained action rather than a temporary network issue. The solution is to identify the deprecated action and migrate to its recommended replacement.

## Importance
CI pipeline failures due to deprecated dependencies can completely block development. Quick identification and migration to maintained alternatives is critical for keeping CI operational. Deprecated actions often lack proper error handling and retry logic, leading to mysterious timeout failures.

## Example
In this case, `goto-bus-stop/setup-zig@v2` was causing ETIMEDOUT errors:
```yaml
# Before - Deprecated action causing timeouts
- name: Install Zig 0.14.1
  uses: goto-bus-stop/setup-zig@v2
  with:
    version: 0.14.1

# After - Migrated to maintained alternative
- name: Install Zig 0.14.1
  uses: mlugg/setup-zig@v2
  with:
    version: 0.14.1
```

The error appeared as:
```
AggregateError [ETIMEDOUT]: 
    at internalConnectMultiple (node:net:1122:18)
```

## Generalization
When encountering network timeouts in GitHub Actions:
1. Check if the action is still maintained (look at last commit date)
2. Search for deprecation notices in the action's README
3. Identify recommended alternatives
4. Verify the alternative's maintainer credibility
5. Migrate to the latest stable version of the alternative

## Implementation
```bash
# Search for the failing action in workflow files
grep -r "uses:.*action-name" .github/workflows/

# Check the action's GitHub page for deprecation notices
# Look for phrases like "deprecated", "unmaintained", "use X instead"

# Update workflow file with sed or manual edit
sed -i 's/goto-bus-stop\/setup-zig/mlugg\/setup-zig/g' .github/workflows/ci.yml

# Verify the change
git diff .github/workflows/
```

## Related Patterns
- [GitHub Actions Debugging Workflow](github-actions-debugging-workflow.md)
- [CI Network Timeout Diagnosis](ci-network-timeout-diagnosis.md)
- [Tool Maintainer Verification Pattern](tool-maintainer-verification-pattern.md)
- [Local CI Discrepancy Investigation](local-ci-discrepancy-investigation.md)