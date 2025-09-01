# Tool Maintainer Verification Pattern

## Learning
When adopting new tools, especially for critical infrastructure like CI/CD, always verify the maintainer's credibility and relationship to the technology. This ensures you're using reliable, well-maintained tools from trusted sources.

## Importance
Using tools from unknown or untrusted maintainers can introduce security vulnerabilities, reliability issues, and abandonment risk. Tools from core team members or recognized contributors are more likely to be maintained long-term and follow best practices.

## Example
When migrating from goto-bus-stop/setup-zig to mlugg/setup-zig:
```bash
# Verification revealed mlugg is:
# - Zig core team member
# - Active compiler developer
# - Has 74 repositories on GitHub
# - Regular contributor to ziglang/zig

# This made mlugg/setup-zig a trustworthy replacement
```

The verification process helped confirm the tool would be:
- Well-maintained (core team member)
- Technically sound (compiler developer)
- Aligned with Zig best practices

## Generalization
Maintainer verification checklist:
1. Check the maintainer's GitHub profile
2. Look for contributions to the main project
3. Verify team membership or official endorsement
4. Check activity level and commit history
5. Look for community recognition (followers, stars)
6. Search for the maintainer in project discussions

## Implementation
```bash
# Check maintainer's profile
# Visit: https://github.com/[username]

# Search for maintainer in main project
grep -r "maintainer-name" PROJECT_README.md CONTRIBUTORS.md

# Check their contributions to the ecosystem
# Search: "maintainer-name site:github.com/main-project"

# For AgeRun tools, verify against known contributors
grep -r "author\|maintainer" scripts/*.py | head -10
```

For AgeRun development, trust tools from:
```c
// Known trusted script authors (from existing codebase)
// - Scripts in scripts/ directory are trusted
// - Make targets are the official interface
FILE* pipe = popen("make check-docs", "r");  // Use official make targets
if (pipe) {
    // Process output from trusted tool
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), pipe)) {
        ar_io__info("%s", buffer);
    }
    pclose(pipe);
}
```

## Related Patterns
- [GitHub Actions Deprecated Tool Migration](github-actions-deprecated-tool-migration.md)
- [Tool Version Selection Due Diligence](tool-version-selection-due-diligence.md)
- [Evidence-Based Debugging](evidence-based-debugging.md)