Create a git commit following the exact workflow specified in AGENTS.md.

**Important**: The build must pass all checks including command excellence scores ([details](../../../kb/command-documentation-excellence-gate.md)). Always test with proper make targets ([details](../../../kb/make-target-testing-discipline.md)).

## MANDATORY KB Consultation

Before committing:
1. Search: `grep "commit\|pre-commit\|checklist" kb/README.md`
2. Must read:
   - pre-commit-checklist-detailed
   - atomic-commit-documentation-pattern
   - incomplete-commit-message-recovery-pattern
   - tdd-feature-completion-before-commit
3. Apply all pre-commit requirements
4. If commit message incomplete after push: use supplemental documentation commit ([details](../../../kb/incomplete-commit-message-recovery-pattern.md))

# Commit
## Checkpoint Tracking

This command uses checkpoint tracking to ensure thorough pre-commit verification and proper git workflow. The process has 9 checkpoints across 3 phases with critical quality gates.

### Initialize Tracking
```bash
# Start the commit process
make checkpoint-init CMD=commit STEPS='"Run Tests" "Check Logs" "Update Docs" "Update TODO" "Update CHANGELOG" "Review Changes" "Stage Files" "Create Commit" "Push and Verify"'
```

**Expected output:**
```
📍 Starting: commit (9 steps)
📁 Tracking: /tmp/commit_progress.txt
→ Run: make checkpoint-update CMD=commit STEP=1
```

### Check Progress
```bash
make checkpoint-status CMD=commit
```

**Expected output (example at 56% completion):**
```
📈 commit: 5/9 steps (56%)
   [███████████░░░░░░░░░] 56%
→ Next: make checkpoint-update CMD=commit STEP=6
```

## Minimum Requirements

**MANDATORY for successful commit:**
- [ ] Clean build passes (exit code 0)
- [ ] check-logs finds no issues
- [ ] Documentation validates
- [ ] TODO.md updated
- [ ] CHANGELOG.md updated
- [ ] All changes reviewed
- [ ] Git push verified
## Phase 1: Pre-Commit Verification (Steps 1-5)

#### [CHECKPOINT START - PHASE 1]

#### [CHECKPOINT END]

### Pre-Commit Checklist (MANDATORY - ALWAYS CHECK THIS)

Before starting the commit process, ensure you have completed ALL of these steps. This checklist acts as a quality gate to prevent incomplete commits ([details](../../../kb/gate-enforcement-exit-codes-pattern.md)):

#### Checkpoint 1: Run Tests

```bash
# Run comprehensive build verification
make clean build 2>&1
make checkpoint-update-verified CMD=commit STEP=1 SUMMARY="Clean build completed with all checks passed"
```

#### Checkpoint 2: Check Logs

```bash
# Verify no hidden issues in logs
make check-logs
make checkpoint-update-verified CMD=commit STEP=2 SUMMARY="Build logs verified clean - no hidden issues"
```

**Additional Notes**:
- Check-logs only works after full build, NOT after individual test runs ([details](../../../kb/build-logs-relationship-principle.md))
- If clean build fails OR check-logs finds issues, STOP - do not proceed ([details](../../../kb/build-verification-before-commit.md))
- `make build` includes documentation validation (`make check-docs`) ([details](../../../kb/batch-documentation-fix-enhancement.md))

#### Checkpoint 3: Update Docs

```bash
# Check if documentation needs updates (manual verification)
make checkpoint-update CMD=commit STEP=3
```

**Documentation Notes**:
- If you changed a module's interface, update its .md file
- For major refactoring, check modules/README.md and dependency trees ([details](../../../kb/refactoring-phase-completion-checklist.md))
- When removing global APIs, use systematic scripts ([details](../../../kb/global-function-removal-script-pattern.md))
- Let compiler errors guide refactoring completion ([details](../../../kb/compilation-driven-refactoring-pattern.md))

#### Checkpoint 4: Update TODO

```bash
# Verify TODO.md is updated (manual verification)
make checkpoint-update CMD=commit STEP=4
```

**TODO Note**: Mark completed tasks and add any new tasks identified

#### Checkpoint 5: Update CHANGELOG

```bash
# Verify CHANGELOG.md is updated
make checkpoint-update-verified CMD=commit STEP=5 SUMMARY="CHANGELOG.md updated with completed milestones"
```

**CHANGELOG Note**: Document completed milestones and achievements (NON-NEGOTIABLE)
- Include all documentation updates in the same commit as implementation ([details](../../../kb/atomic-commit-documentation-pattern.md))

#### [BUILD GATE]
```bash
# Verify build and logs are clean before proceeding
make checkpoint-gate CMD=commit GATE="Build Quality" REQUIRED="1,2"
```

**Expected gate output:**
```
✅ GATE 'Build Quality' - PASSED
   Verified: Steps 1,2
```

## Phase 2: Review and Stage (Steps 6-7)

#### [CHECKPOINT START - PHASE 2]

#### [CHECKPOINT END]

#### Checkpoint 6: Review Changes

```bash
# Review all changes
git diff
make checkpoint-update-verified CMD=commit STEP=6 SUMMARY="All changes reviewed and no backup files present"
```

**Review Notes**:
- Use `git diff` to verify all changes are intentional
- Check for temporary/backup files - NEVER commit backup files (*.backup, *.bak, *.tmp, etc.)
- If you enhanced any scripts, verify they preserve original functionality ([details](../../../kb/script-backward-compatibility-testing.md))

**IMPORTANT REMINDERS**: 
- Clean build MUST pass before committing - no exceptions
- CHANGELOG update is MANDATORY for every commit that completes tasks
- Report build time from output (e.g., "took 1m 3s") ([details](../../../kb/build-time-reporting.md))
- Ensure documentation stays in sync with implementation ([details](../../../kb/documentation-implementation-sync.md))

#### [DOCUMENTATION GATE]
```bash
# ⚠️ CRITICAL: Verify documentation is complete
make checkpoint-gate CMD=commit GATE="Documentation" REQUIRED="3,4,5"
```

**Expected gate output:**
```
✅ GATE 'Documentation' - PASSED
   Verified: Steps 3,4,5
```

## Phase 3: Commit and Push (Steps 8-9)

#### [CHECKPOINT START - PHASE 3]

#### [CHECKPOINT END]

### Commit Workflow

After completing the checklist above, follow these steps precisely:

#### Checkpoint 7: Stage Files

```bash
# Analyze and stage changes
echo "Analyzing current state..."
git status
git diff --cached
git log --oneline -10

# Stage all relevant files
echo "Staging files..."
git add -A

echo "✅ Files staged for commit"
make checkpoint-update CMD=commit STEP=7
```

1. **First, run these commands to analyze the current state:**
   - `git status` - to see all untracked files
   - `git diff` - to see both staged and unstaged changes
   - `git log --oneline -10` - to see recent commit messages and follow the repository's commit style

2. **Analyze all changes and draft a commit message:**
   - Summarize the nature of the changes (e.g., new feature, enhancement, bug fix, refactoring, test, docs)
   - Use proper verbs: "add" for new features, "update" for enhancements, "fix" for bug fixes
   - Focus on the "why" rather than the "what" 
   - Keep it concise (1-2 sentences)
   - Check for any sensitive information that shouldn't be committed
   - Ensure the message accurately reflects ALL changes
   - Verify full scope of architectural changes ([details](../../../kb/commit-scope-verification.md))

#### Checkpoint 8: Create Commit

```bash
# Create the commit
git commit -m "$(cat <<'EOF'
[Your commit message here]

🤖 Generated with [Claude Code](https://claude.ai/code)

Co-Authored-By: Claude <noreply@anthropic.com>
EOF
)" && make checkpoint-update-verified CMD=commit STEP=8 SUMMARY="Commit created successfully"
```

3. **Execute the commit:**
   - Create the commit using HEREDOC format with checkpoint verification
   - The `checkpoint-update-verified` will automatically verify:
     - Commit was created successfully
     - Working tree is clean after commit
     - Branch is ahead of remote (or warn if not)

#### Checkpoint 9: Push and Verify

```bash
# Push to remote and verify
git push
git status
make checkpoint-update-verified CMD=commit STEP=9 SUMMARY="Push completed successfully and working tree clean"
```

5. **Push and verify:**
   - Run `git push`
   - **MANDATORY**: Run `git status` after push to verify it completed successfully ([details](../../../kb/git-push-verification.md))
   - Confirm the working tree is clean and branch is up to date

**Important:** Never skip the final `git status` verification - this is a critical step per AGENTS.md.

#### [CHECKPOINT COMPLETE]
```bash
# Show final summary
make checkpoint-status CMD=commit
```

**Expected completion output:**
```
🎆 All 9 steps complete!
✓ Run: make checkpoint-cleanup CMD=commit
```

```bash
# Clean up tracking
make checkpoint-cleanup CMD=commit
```

## Related Documentation

### CI and Build Patterns
- [CI Check-Logs Requirement](../../../kb/ci-check-logs-requirement.md)
- [Check-Logs Deep Analysis Pattern](../../../kb/check-logs-deep-analysis-pattern.md)
- [Build Verification Before Commit](../../../kb/build-verification-before-commit.md)
- [Git Push Verification](../../../kb/git-push-verification.md)
- [Git Patch Preservation Workflow](../../../kb/git-patch-preservation-workflow.md)

## Custom Instructions

If available, the instructions below may override some of the instructions above.

$ARGUMENTS
