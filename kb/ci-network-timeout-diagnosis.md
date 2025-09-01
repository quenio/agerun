# CI Network Timeout Diagnosis

## Learning
When CI builds fail with `AggregateError [ETIMEDOUT]` during action setup phases, the issue is typically with the action's download mechanism rather than temporary network problems. These timeouts often indicate deprecated or poorly maintained actions that lack proper retry logic.

## Importance
Correctly diagnosing network timeouts in CI prevents wasted time on the wrong fixes. Understanding that ETIMEDOUT during action setup usually means action problems (not network problems) leads to faster resolution by focusing on action replacement rather than network debugging.

## Example
The error pattern in GitHub Actions:
```
Run goto-bus-stop/setup-zig@v2
AggregateError [ETIMEDOUT]: 
    at internalConnectMultiple (node:net:1122:18)  // EXAMPLE: Node.js stack trace
    at internalConnectMultiple (node:net:1190:5)  // EXAMPLE: Node.js internal
    at Timeout.internalConnectMultipleTimeout (node:net:1716:5)  // EXAMPLE: Node.js timeout
```

This indicated the action itself was failing to download Zig, not a network issue.

## Generalization
CI network timeout diagnosis flow:
1. Check WHERE the timeout occurs (action setup vs build/test)
2. If in action setup: likely deprecated/unmaintained action
3. If during build/test: check for large downloads or slow endpoints
4. Look for retry logic in the failing component
5. Consider action alternatives before network solutions

## Implementation
For CI debugging:
```yaml
# Add timeout configurations to isolate issues
- name: Setup Tool
  uses: action-name@version
  timeout-minutes: 10  # Explicit timeout helps identify stuck actions

# Add debug output
- name: Debug Network
  run: |
    echo "Testing network connectivity"
    curl -I https://github.com
    ping -c 3 github.com || true
```

For AgeRun CI issues, check build logs:
```c
// When implementing file operations, check for errors
ar_file_result_t result = ar_io__open_file(
    "build.log",
    "r",
    &file_ptr
);

if (result.error) {
    // Handle file access errors
    ar_io__error("Failed to open build log: %s", 
                 ar_io__error_message(result));
}
```

## Related Patterns
- [GitHub Actions Deprecated Tool Migration](github-actions-deprecated-tool-migration.md)
- [Local CI Discrepancy Investigation](local-ci-discrepancy-investigation.md)
- [GitHub Actions Debugging Workflow](github-actions-debugging-workflow.md)
- [CI Check Logs Requirement](ci-check-logs-requirement.md)