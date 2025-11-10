# Command Documentation Excellence Gate

## Learning
All Claude Code command documentation must achieve 90%+ structure scores or the CI build fails. This enforces documentation quality at the build level, preventing degradation over time.

## Importance
Documentation quality directly impacts developer productivity. By enforcing excellence scores (90%+) as a hard CI gate, we ensure all commands maintain comprehensive structure with checkpoints, gates, examples, and quality indicators.

## Generalization
Quality gates should be enforced programmatically in CI pipelines. Set measurable criteria and fail builds when standards aren't met. This prevents gradual quality degradation.

## Implementation
Command structure validation is now done through:
- Documentation checks (`make check-docs`)
- Manual review
- KB article cross-references

## Quality Enhancement Pattern

Commands should extract complex embedded bash logic to helper scripts for better maintainability and testability:

```markdown
## Bad: Embedded 30-line verification logic
\`\`\`bash
for file in modules/*.c; do
  # ... complex logic ...
done
\`\`\`

## Good: Extract to helper script
**Helper script available:**
\`\`\`bash
./scripts/verify-module-quality.sh [path]
\`\`\`
```

This improves command documentation scores by:
- Reducing command file size and complexity
- Making verification logic testable
- Enabling reuse across commands

See [Command Helper Script Extraction Pattern](command-helper-script-extraction-pattern.md).

## Related Patterns
- [Command Helper Script Extraction Pattern](command-helper-script-extraction-pattern.md) - Extract embedded bash to standalone scripts
- [Build Verification Before Commit](build-verification-before-commit.md)
- [Build System Exit Code Verification](build-system-exit-code-verification.md)
- [CI Check-Logs Requirement](ci-check-logs-requirement.md)