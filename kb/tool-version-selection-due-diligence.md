# Tool Version Selection Due Diligence

## Learning
When updating dependencies or migrating to new tools, always verify you're selecting the latest stable version rather than defaulting to v1 or the first version found. Check the releases page and documentation for the most recent recommended version.

## Importance
Using outdated versions can miss critical bug fixes, performance improvements, and security patches. In CI/CD contexts, older versions may lack essential features or compatibility with current infrastructure. Taking an extra moment to verify the latest version prevents future migration work.

## Example
When migrating from a deprecated GitHub Action, the initial selection was v1:
```yaml
# Initial selection (outdated)
- uses: mlugg/setup-zig@v1

# After checking releases page (current)
- uses: mlugg/setup-zig@v2
```

The v2 version included important improvements:
- Better Zig cache management
- Support for build.zig.zon version detection
- Fixes for identifying Zig cache directory on recent builds

## Generalization
Version selection process:
1. Check the tool's releases page on GitHub
2. Read the latest release notes for breaking changes
3. Look for explicit upgrade recommendations
4. Verify compatibility with your use case
5. Prefer the latest stable version unless there's a specific reason not to

## Implementation
```bash
# Check available versions for a GitHub Action
# Visit: https://github.com/[owner]/[repo]/releases

# For make targets and build tools
make help | grep -i version

# For package managers
npm view package-name versions --json | tail -5

# Verify current version in use
grep -r "@v[0-9]" .github/workflows/
```

For AgeRun development, always check method versions:
```c
// When loading methods, verify version compatibility
ar_method_t* method = ar_methodology__get_method_with_instance(
    "method_name", 
    "2.0.0"  // Always use explicit version strings
);

// Or with instance
ar_method_t* method2 = ar_methodology__get_method_with_instance(
    methodology,
    "another_method",
    "1.0.0"
);
```

## Related Patterns
- [Assumption Verification Before Action](assumption-verification-before-action.md)
- [User Feedback as QA](user-feedback-as-qa.md)
- [GitHub Actions Deprecated Tool Migration](github-actions-deprecated-tool-migration.md)