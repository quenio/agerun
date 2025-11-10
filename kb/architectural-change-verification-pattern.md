# Architectural Change Verification Pattern

## Learning
When making architectural changes (e.g., removing checkpoint tracking from commands), systematic verification is required to ensure all affected files reflect the change consistently. Without explicit verification steps, some files may retain outdated patterns, causing confusion and errors in future sessions.

## Importance
Architectural changes can leave inconsistent files if verification isn't systematic. Outdated references in KB articles and command files can mislead future sessions. Explicit verification steps ensure completeness and prevent technical debt from incomplete migrations.

## Example
```bash
# Architectural change: Remove checkpoint tracking from commands

# BAD: Update primary KB articles only, skip verification
# 1. Update kb/checkpoint-tracking-verification-separation.md ✓
# 2. Update 30 command files ✓
# 3. Commit and push ✓
# Missing: Verify no outdated references remain ✗

# GOOD: Update + systematic verification
# 1. Update primary KB articles ✓
# 2. Update all command files ✓
# 3. Verify KB article cross-references:
grep -i "checkpoint.*tracking" kb/*.md | grep -v "no longer\|deprecated"
# Found: 75 matches, some still reference old pattern
# 4. Verify command files:
grep -i "checkpoint" .opencode/command/ar/*.md
# Found: 0 matches (all updated correctly)
# 5. Update cross-references found in step 3 ✓
# 6. Document verification process in CHANGELOG ✓
```

## Generalization
**Architectural Change Verification Workflow:**

1. **Identify affected files**: List all files that might reference the old pattern
   ```bash
   # Find KB articles
   grep -r "old_pattern" kb/*.md --files-with-matches
   
   # Find command files
   grep -r "old_pattern" .opencode/command/ar/*.md --files-with-matches
   ```

2. **Update primary files**: Modify files that directly implement the change
   - Update KB articles documenting the change
   - Update command files using the old pattern
   - Update AGENTS.md if guidelines change

3. **Verify cross-references**: Check for outdated references in related files
   ```bash
   # Search for old pattern references
   grep -i "old_pattern" kb/*.md | grep -v "no longer\|deprecated\|UPDATED"
   
   # Review each match to determine if update needed
   ```

4. **Verify command files**: Ensure all commands reflect the change
   ```bash
   # Check for old pattern in commands
   grep -i "old_pattern" .opencode/command/ar/*.md
   
   # Should return 0 matches if all updated
   ```

5. **Update cross-references**: Fix outdated references found in verification
   - Update Related Patterns sections
   - Update command documentation
   - Update AGENTS.md references

6. **Document verification**: Record verification process in CHANGELOG
   - List verification commands executed
   - Document any issues found and fixed
   - Note verification completeness

## Implementation
```bash
#!/bin/bash
# Architectural change verification script

OLD_PATTERN="checkpoint.*tracking"
NEW_PATTERN="session.*todo.*tracking"

echo "=== Step 1: Identify Affected Files ==="
KB_FILES=$(grep -r "$OLD_PATTERN" kb/*.md --files-with-matches)
CMD_FILES=$(grep -r "$OLD_PATTERN" .opencode/command/ar/*.md --files-with-matches)

echo "KB articles: $(echo "$KB_FILES" | wc -l)"
echo "Command files: $(echo "$CMD_FILES" | wc -l)"

echo ""
echo "=== Step 2: Verify KB Article Cross-References ==="
OUTDATED_KB=$(grep -i "$OLD_PATTERN" kb/*.md | grep -v "no longer\|deprecated\|UPDATED")
if [ -n "$OUTDATED_KB" ]; then
    echo "⚠️ Found outdated KB references:"
    echo "$OUTDATED_KB"
else
    echo "✓ No outdated KB references found"
fi

echo ""
echo "=== Step 3: Verify Command Files ==="
OUTDATED_CMD=$(grep -i "$OLD_PATTERN" .opencode/command/ar/*.md)
if [ -n "$OUTDATED_CMD" ]; then
    echo "⚠️ Found outdated command references:"
    echo "$OUTDATED_CMD"
else
    echo "✓ No outdated command references found"
fi

echo ""
echo "=== Step 4: Verification Summary ==="
if [ -z "$OUTDATED_KB" ] && [ -z "$OUTDATED_CMD" ]; then
    echo "✅ All files verified: Architectural change complete"
    exit 0
else
    echo "❌ Verification incomplete: Update remaining references"
    exit 1
fi
```

**CHANGELOG documentation pattern:**
```markdown
## [Date] (Architectural Change Name)

- **Architectural Change Name**

  [Description of change]

  **Implementation**:
  - Updated primary KB articles: [list]
  - Updated command files: [count] files
  - Updated cross-references: [count] articles

  **Verification**:
  - KB article cross-references: Verified with `grep -i "old_pattern" kb/*.md` (0 outdated references)
  - Command files: Verified with `grep -i "old_pattern" .opencode/command/ar/*.md` (0 matches)
  - Cross-reference updates: [count] articles updated with new pattern references

  **Files Modified**:
  - [list of files]

  **Impact**:
  - [How change improves the project]
```

## Common Verification Failures
1. **Primary files only**: Updating main articles but missing cross-references
2. **No verification step**: Assuming all files updated without checking
3. **Incomplete grep patterns**: Using too narrow search patterns that miss variations
4. **Missing CHANGELOG documentation**: Not recording verification process
5. **One-time verification**: Not re-verifying after fixes

## Prevention Strategy
1. **Create verification checklist**: List all verification steps before starting
2. **Use systematic grep**: Search for old pattern with variations
3. **Review each match**: Don't assume all matches need updates
4. **Document verification**: Record commands and results in CHANGELOG
5. **Re-verify after fixes**: Check again after updating cross-references

## Quality Indicators
Signs of complete architectural change:
- Grep searches return 0 outdated references
- CHANGELOG documents verification process
- Cross-references updated in Related Patterns sections
- Command files reflect new pattern consistently
- No confusion in future sessions about which pattern to use

## Related Patterns
- [Architectural Documentation Consistency Pattern](architectural-documentation-consistency-pattern.md) - Systematic updates across all documentation layers for architectural changes
- [New Learnings Complete Integration Pattern](new-learnings-complete-integration-pattern.md) - Integration checklist including verification
- [Systematic Consistency Verification](systematic-consistency-verification.md) - Automated verification scripts
- [New Learnings Cross-Reference Requirement](new-learnings-cross-reference-requirement.md) - Cross-reference requirements
- [Command Continuous Improvement Pattern](command-continuous-improvement-pattern.md) - Command update patterns

