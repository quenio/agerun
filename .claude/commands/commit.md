Create a git commit following the exact workflow specified in CLAUDE.md.

**Important**: The build must pass all checks including command excellence scores ([details](../../kb/command-documentation-excellence-gate.md)). Always test with proper make targets ([details](../../kb/make-target-testing-discipline.md)).

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
========================================
   CHECKPOINT TRACKING INITIALIZED
========================================

Command: commit
Tracking file: /tmp/commit_progress.txt
Total steps: 9

Steps to complete:
  1. Run Tests
  2. Check Logs
  3. Update Docs
  4. Update TODO
  5. Update CHANGELOG
  6. Review Changes
  7. Stage Files
  8. Create Commit
  9. Push and Verify

Goal: Complete git commit with all quality checks
```

### Check Progress
```bash
make checkpoint-status CMD=commit
```

**Expected output (example at 56% completion):**
```
========================================
   CHECKPOINT STATUS: commit
========================================

Progress: 5/9 steps (56%)

[██████████░░░░░░░░] 56%

Current Phase: Documentation
Tests: Passed
Logs: Clean
Docs: Updated
TODO: Updated
CHANGELOG: In progress...

Next Action:
  → Step 6: Review Changes
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

Before starting the commit process, ensure you have completed ALL of these steps. This checklist acts as a quality gate to prevent incomplete commits ([details](../../kb/gate-enforcement-exit-codes-pattern.md)):

#### Checkpoint 1: Run Tests

```bash
# Run comprehensive build verification
echo "Running clean build..."
if ! make clean build 2>&1; then
  echo "❌ Build failed - fix errors before committing"
  exit 1
fi

echo "✅ Build completed successfully"
make checkpoint-update CMD=commit STEP=1
```

#### Checkpoint 2: Check Logs

```bash
# Verify no hidden issues in logs
echo "Checking build logs..."
if ! make check-logs; then
  echo "❌ Log issues found - fix before committing"
  exit 1
fi

echo "✅ Logs are clean - CI ready"
make checkpoint-update CMD=commit STEP=2
```

1. **Run Tests**: Use `make clean build` for comprehensive build verification (~20 lines output) to ensure all changes work correctly and no memory leaks. **ALWAYS follow with `make check-logs` to catch hidden issues!** **If clean build fails OR check-logs finds issues, STOP - do not proceed with commit until all issues are fixed.** ([details](../../kb/build-verification-before-commit.md), [CI requirement](../../kb/ci-check-logs-requirement.md))
   - **Note**: `make build` includes documentation validation (`make check-docs`). If docs fail, use `python3 scripts/batch_fix_docs.py` to fix automatically ([details](../../kb/batch-documentation-fix-enhancement.md))
#### Checkpoint 3: Update Docs

```bash
# Check if documentation needs updates
echo "Checking documentation status..."
DOCS_UPDATED=0

# Check for modified modules
if git diff --name-only | grep -q "modules/.*\.c\|modules/.*\.h"; then
  echo "Module changes detected - ensure .md files are updated"
  DOCS_UPDATED=1
fi

if [ $DOCS_UPDATED -eq 1 ]; then
  echo "✅ Documentation updates required and completed"
else
  echo "✅ No documentation updates needed"
fi

make checkpoint-update CMD=commit STEP=3
```

2. **Update Module Documentation**: If you changed a module's interface, update its .md file
3. **Update System Documentation**: For major refactoring, check modules/README.md and dependency trees ([details](../../kb/refactoring-phase-completion-checklist.md))

#### Checkpoint 4: Update TODO

```bash
# Verify TODO.md is updated
echo "Checking TODO.md updates..."
if ! git diff --cached TODO.md | grep -q "^+" && git diff TODO.md | grep -q "^+"; then
  echo "⚠️ TODO.md has changes but not staged"
  echo "Stage TODO.md updates before committing"
fi

echo "✅ TODO.md updated"
make checkpoint-update CMD=commit STEP=4
```

4. **Update TODO.md**: Mark completed tasks and add any new tasks identified

#### Checkpoint 5: Update CHANGELOG

```bash
# Verify CHANGELOG.md is updated
echo "Checking CHANGELOG.md updates..."
if ! git diff --cached CHANGELOG.md | grep -q "^+" && ! git diff CHANGELOG.md | grep -q "^+"; then
  echo "❌ CHANGELOG.md not updated - this is MANDATORY"
  exit 1
fi

echo "✅ CHANGELOG.md updated"
make checkpoint-update CMD=commit STEP=5
```

5. **Update CHANGELOG.md**: Document completed milestones and achievements (NON-NEGOTIABLE)
   - **IMPORTANT**: Include all documentation updates in the same commit as implementation ([details](../../kb/atomic-commit-documentation-pattern.md))
#### [BUILD GATE]
```bash
# Verify build and logs are clean before proceeding
make checkpoint-gate CMD=commit GATE="Build Quality" REQUIRED="1,2"
```

**Expected gate output:**
```
========================================
   GATE: Build Quality
========================================

✅ GATE PASSED: Build verified!

Build Status:
  ✓ Clean build completed
  ✓ All tests passed
  ✓ Zero memory leaks
  ✓ Logs are clean
  ✓ CI ready

Proceed to review phase.
```

## Phase 2: Review and Stage (Step 6-7)

#### [CHECKPOINT START - PHASE 2]

#### [CHECKPOINT END]

#### Checkpoint 6: Review Changes

```bash
# Review all changes
echo "Reviewing changes..."
git diff

# Check for backup files
if git status --porcelain | grep -E "\.(backup|bak|tmp)$"; then
  echo "❌ Backup files detected - remove before committing"
  exit 1
fi

echo "✅ All changes reviewed and intentional"
make checkpoint-update CMD=commit STEP=6
```

6. **Review Changes**: Use `git diff` to verify all changes are intentional
7. **Check for temporary/backup files**: NEVER commit backup files (*.backup, *.bak, *.tmp, etc.)
8. **Test Enhanced Scripts**: If you enhanced any scripts, verify they preserve original functionality ([details](../../kb/script-backward-compatibility-testing.md))
9. **Then Commit**: Only after completing steps 1-8

**IMPORTANT**: 
- Clean build MUST pass before committing - no exceptions
- CHANGELOG update is MANDATORY for every commit that completes tasks
- Never skip the checklist
- Report build time from output (e.g., "took 1m 3s") ([details](../../kb/build-time-reporting.md))

#### [DOCUMENTATION GATE]
```bash
# ⚠️ CRITICAL: Verify documentation is complete
make checkpoint-gate CMD=commit GATE="Documentation" REQUIRED="3,4,5"
```

**Expected gate output:**
```
========================================
   GATE: Documentation
========================================

⚠️ CRITICAL VERIFICATION

Documentation Status:
  ✓ Module docs updated (if needed)
  ✓ TODO.md updated
  ✓ CHANGELOG.md updated
  
✅ GATE PASSED: Documentation complete!

Ready for commit.
```

## Phase 3: Commit and Push (Steps 7-9)

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
   - Verify full scope of architectural changes ([details](../../kb/commit-scope-verification.md))

#### Checkpoint 8: Create Commit

```bash
# Create the commit
echo "Creating commit..."
git commit -m "$(cat <<'EOF'
[Your commit message here]

🤖 Generated with [Claude Code](https://claude.ai/code)

Co-Authored-By: Claude <noreply@anthropic.com>
EOF
)"

# Verify commit succeeded
if [ $? -eq 0 ]; then
  echo "✅ Commit created successfully"
else
  echo "❌ Commit failed - check pre-commit hooks"
  # Retry once for hook changes
  git add -A
  git commit -m "$(cat <<'EOF'
[Your commit message here]

🤖 Generated with [Claude Code](https://claude.ai/code)

Co-Authored-By: Claude <noreply@anthropic.com>
EOF
  )"
fi

make checkpoint-update CMD=commit STEP=8
```

3. **Execute these commands:**
   - Add all relevant files: `git add -A`
   - Create the commit using HEREDOC format:
   ```bash
   git commit -m "$(cat <<'EOF'
   [Your commit message here]

   🤖 Generated with [Claude Code](https://claude.ai/code)

   Co-Authored-By: Claude <noreply@anthropic.com>
   EOF
   )"
   ```
   - Run `git status` to verify the commit succeeded

4. **If the commit fails due to pre-commit hooks:**
   - Retry the commit ONCE to include any automated changes
   - If it fails again, report the error
   - If it succeeds but files were modified by hooks, amend the commit to include them

#### Checkpoint 9: Push and Verify

```bash
# Push to remote
echo "Pushing to remote..."
git push

# MANDATORY verification
echo "Verifying push..."
git status

if git status | grep -q "Your branch is up to date"; then
  echo "✅ Push completed successfully!"
  echo "Working tree is clean"
else
  echo "⚠️ Push may have issues - check git status output"
fi

make checkpoint-update CMD=commit STEP=9
```

5. **Push and verify:**
   - Run `git push`
   - **MANDATORY**: Run `git status` after push to verify it completed successfully ([details](../../kb/git-push-verification.md))
   - Confirm the working tree is clean and branch is up to date

**Important:** Never skip the final `git status` verification - this is a critical step per CLAUDE.md.

#### [CHECKPOINT COMPLETE]
```bash
# Show final summary
make checkpoint-status CMD=commit
```

**Expected completion output:**
```
========================================
   CHECKPOINT STATUS: commit
========================================

Progress: 9/9 steps (100%)

[████████████████████] 100%

✅ ALL CHECKPOINTS COMPLETE!

Commit Summary:
  Build: Clean
  Tests: All passing
  Logs: No issues
  Documentation: Updated
  TODO: Updated
  CHANGELOG: Updated
  Commit: Created
  Push: Verified

The commit has been successfully completed!
```

```bash
# Clean up tracking
make checkpoint-cleanup CMD=commit
```

## Custom Instructions

If available, the instructions below may override some of the instructions above.

$ARGUMENTS

