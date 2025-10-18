# Command Thoroughness Requirements Pattern

## Learning
Commands should define and enforce minimum requirements for quality-critical steps to prevent superficial execution. This includes specifying minimum counts (e.g., "update 3-5 articles"), verification scripts, and clear success criteria.

## Importance
Without explicit requirements, users tend to do the minimum necessary to proceed, missing important integration work. This leads to incomplete implementations, missing cross-references, and technical debt from rushed execution.

## Example
```bash
# Step definition with explicit minimums
echo "## Step 6: Update Existing KB Articles (THOROUGH EXECUTION REQUIRED)"
echo ""
echo "**MINIMUM REQUIREMENT**: Update at least 3-5 existing KB articles."
echo "**SUCCESS CRITERIA**: More articles modified than created."

# Verification script with concrete metrics
echo "=== Integration Verification ==="
MODIFIED_KB=$(git diff --name-only | grep "kb.*\.md" | wc -l)
MODIFIED_CMDS=$(git diff --name-only | grep ".claude/commands" | wc -l)

if [ $MODIFIED_KB -lt 3 ]; then
    echo "⚠️ WARNING: Only $MODIFIED_KB KB articles modified - need at least 3-5!"
    READY=false
else
    echo "✓ Good: $MODIFIED_KB KB articles have cross-references"
fi

if [ $MODIFIED_CMDS -lt 3 ]; then
    echo "⚠️ WARNING: Only $MODIFIED_CMDS commands updated - need at least 3-4!"
    READY=false
else
    echo "✓ Good: $MODIFIED_CMDS commands updated"
fi

if [ "$READY" = "false" ]; then
    echo "⚠️ NOT READY: Need more cross-references and command updates"
    exit 1
fi

echo "✓ READY TO COMMIT: Thorough integration completed"
```

## Generalization
1. **Define measurable minimums** - "3-5 articles" not "several articles"
2. **Explain why minimums exist** - Connect to quality outcomes
3. **Provide verification scripts** - Automated checking removes ambiguity
4. **Block progression if not met** - Use gates to enforce requirements
5. **Show current vs required** - "2/5 completed" helps users gauge effort

## Implementation
```markdown
## Thoroughness Requirements

**MANDATORY MINIMUMS for this execution:**
- [ ] Create 2-3 new KB articles with real code examples
- [ ] Update 3-5 existing KB articles with cross-references  
- [ ] Update 3-4 commands with new KB references
- [ ] All articles pass `make check-docs` validation
- [ ] Verification script shows "READY TO COMMIT"

**Quality Indicators:**
- More KB articles modified than created (shows integration)
- Multiple commands updated (shows broad impact)
- No validation errors (shows attention to detail)

**Verification Command:**
\`\`\`bash
bash scripts/verify_thoroughness.sh
\`\`\`

**Note**: Extract verification scripts from commands to `scripts/` directory for testability and reuse. See [Command Helper Script Extraction Pattern](command-helper-script-extraction-pattern.md).
```

## Related Patterns
- [Command Helper Script Extraction Pattern](command-helper-script-extraction-pattern.md) - Extract verification scripts for reusability
- [Comprehensive Learning Extraction Pattern](comprehensive-learning-extraction-pattern.md)
- [Multi-Step Checkpoint Tracking Pattern](multi-step-checkpoint-tracking-pattern.md)
- [Gate Enforcement Exit Codes Pattern](gate-enforcement-exit-codes-pattern.md)
- [Search Result Completeness Verification](search-result-completeness-verification.md) - Enforce complete data gathering
- [KB Target Compliance Enforcement](kb-target-compliance-enforcement.md) - Enforce quantitative targets with gates