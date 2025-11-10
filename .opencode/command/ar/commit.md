Create a git commit following the exact workflow specified in AGENTS.md.

## STEP VERIFICATION ENFORCEMENT

**MANDATORY**: After completing each step, you MUST verify step completion using the **step-verifier sub-agent** before proceeding to the next step ([details](../../../kb/sub-agent-verification-pattern.md)).

### About the step-verifier Sub-Agent

The **step-verifier** is a specialized sub-agent that independently verifies step completion:

- **Reads command files** to understand step requirements
- **Checks files, git status/diff, test results, build outputs** to verify accomplishments
- **Compares accomplishments against requirements** systematically
- **Reports verification results with evidence** (what was verified, what's missing)
- **Provides STOP instructions** when failures are detected (blocks execution until fixed)
- **Read-only agent**: Never modifies files, commits changes, or makes autonomous decisions

**CRITICAL**: The step-verifier independently verifies your claims. You report accomplishments with evidence; the step-verifier verifies by reading files and checking outputs.

### Step Verification Process

After completing each step, you MUST:

1. **Report accomplishments with concrete evidence**
   - Describe what was accomplished (files created/modified, commands executed, outputs produced)
   - Provide **concrete evidence**: actual file paths with line numbers, full command outputs, git diff output, test results with specific test names, grep/search output proving claims
   - **DO NOT** tell step-verifier what to verify - report what was done with evidence
   - **DO NOT** use vague summaries - provide specific details (see [kb/sub-agent-verification-pattern.md](../../../kb/sub-agent-verification-pattern.md) and [kb/evidence-validation-requirements-pattern.md](../../../kb/evidence-validation-requirements-pattern.md) for examples)

2. **Invoke step-verifier sub-agent**
   - Use `mcp_sub-agents_run_agent` tool with:
     - Agent: `"step-verifier"`
     - Prompt: See format below
     - The step-verifier will independently verify your claims

3. **Handle Verification Results**
  
   **If verification PASSES** (report shows "✅ STEP VERIFIED" or "All requirements met"):
     - Proceed to next step
  
   **If verification FAILS** (report shows "⚠️ STOP EXECUTION" or missing elements):
     - **STOP execution immediately** - do not proceed to next step
     - Fix all reported issues from verification report
     - Re-invoke step-verifier with updated evidence after fixes
     - Only proceed after verification report shows "✅ STEP VERIFIED"
  
   **If sub-agent CANNOT be executed** (MCP unavailable or tool error):
     - STOP execution immediately
     - Inform user: "⚠️ Step verification sub-agent unavailable. Please manually verify Step N completion before proceeding."
     - Wait for explicit user confirmation before proceeding

### How to Invoke step-verifier

Use the `mcp_sub-agents_run_agent` tool:

```
Agent: "step-verifier"
Prompt: "Verify Step N: [Step Title] completion for commit command.

Todo Item: [Description of what the step accomplished]
Command File: .opencode/command/ar/commit.md
Step: Step N: [Step Title]

Accomplishment Report:
[Report what was accomplished with CONCRETE EVIDENCE. The step-verifier will independently verify these claims by reading files, checking git status, etc.

**MANDATORY Evidence Requirements:**
- **File Changes**: Include actual file paths, line numbers, and git diff output showing exact changes
- **Command Execution**: Include full command output, exit codes, test results with specific test names
- **Documentation Updates**: Include file paths, section names, actual content snippets, git diff output
- **Git Status**: Include actual `git status` and `git diff` output showing what changed
- **Verification Output**: Include actual grep/search command output proving claims
- **Build/Test Results**: Include full output showing compilation, test execution, memory leak reports

**Examples:**
✅ GOOD: "Updated `.opencode/command/ar/execute-plan.md` line 2356: `git diff` shows lines changed from `### If progress tracking` to `### If step tracking`. Verification: `grep -i 'checkpoint' file.md` returned no matches (exit code 1)"
❌ BAD: "Updated execute-plan.md to remove checkpoint references"

See [kb/sub-agent-verification-pattern.md](../../../kb/sub-agent-verification-pattern.md) for complete evidence requirements and examples.]"
```

**CRITICAL**: 
- Report accomplishments with **concrete evidence** (file paths, line numbers, command outputs, git diff, test results), NOT instructions or vague summaries
- The step-verifier independently verifies by reading command files, checking files, git status/diff, etc.
- If step-verifier reports "⚠️ STOP EXECUTION", you MUST fix issues before proceeding
- If accomplishment report lacks concrete evidence, step-verifier will STOP execution and require evidence

**MANDATORY: Session Todo List Tracking**

Each step MUST be added to the session todo list before execution begins ([details](../../../kb/session-todo-list-tracking-pattern.md)):
- Use `todo_write` to add each step as a todo item with status `in_progress` before starting the step
- Use `todo_write` to mark each step as `completed` after step-verifier verification passes
- This ensures the session maintains track of all steps to be executed

**Important**: The build must pass all checks including command excellence scores ([details](../../../kb/command-documentation-excellence-gate.md)). Always test with proper make targets ([details](../../../kb/make-target-testing-discipline.md)).

## MANDATORY KB Consultation

Before committing, you MUST:
1. Search: `grep "commit\|pre-commit\|checklist" kb/README.md`
2. Read these KB articles IN FULL using the Read tool:
   - `kb/pre-commit-checklist-detailed.md`
   - `kb/atomic-commit-documentation-pattern.md`
   - `kb/incomplete-commit-message-recovery-pattern.md`
   - `kb/tdd-feature-completion-before-commit.md`
   - `kb/tdd-cycle-completion-verification-pattern.md` (if TDD cycle work)
   - `kb/plan-document-completion-status-pattern.md` (if following a plan)
   - `kb/documentation-index-consistency-pattern.md` (if module integration involved)
3. Check Related Patterns sections in each article and read any additional relevant articles found there
4. In your response, quote these specific items from the KB:
   - The complete pre-commit checklist steps
   - Requirements for CHANGELOG.md updates (NON-NEGOTIABLE)
   - When to use supplemental documentation commits
   - Module integration documentation requirements
5. Apply ALL pre-commit requirements from the KB

**Example of proper KB consultation:**
```
I've read pre-commit-checklist-detailed.md which requires:

"Pre-Commit Checklist (MANDATORY):
1. make clean build 2>&1 → verify exit 0 → make check-logs
2. make check-docs → validate all documentation
3. Update docs for API changes
4. Update TODO.md & CHANGELOG.md in same commit"

The CHANGELOG.md update is NON-NEGOTIABLE per atomic-commit-documentation-pattern.md.
After module integration, modules/README.md must be updated per documentation-index-consistency-pattern.md.
```

**CRITICAL**: If you skip reading these KB articles, you risk incomplete commits that may fail CI or violate project standards.

# Commit

## MANDATORY: Initialize All Todo Items

**CRITICAL**: Before executing ANY steps, add ALL step and verification todo items to the session todo list using `todo_write`:

**Step and Verification Todo Items:**
- Add todo item: "Step 1: Run Tests" - Status: pending
- Add todo item: "Verify Step 1: Run Tests" - Status: pending
- Add todo item: "Step 2: Check Logs" - Status: pending
- Add todo item: "Verify Step 2: Check Logs" - Status: pending
- Add todo item: "Step 3: Update Docs" - Status: pending
- Add todo item: "Verify Step 3: Update Docs" - Status: pending
- Add todo item: "Step 4: Update TODO" - Status: pending
- Add todo item: "Verify Step 4: Update TODO" - Status: pending
- Add todo item: "Step 5: Update CHANGELOG" - Status: pending
- Add todo item: "Verify Step 5: Update CHANGELOG" - Status: pending
- Add todo item: "Step 6: Review Changes" - Status: pending
- Add todo item: "Verify Step 6: Review Changes" - Status: pending
- Add todo item: "Step 7: Stage Files" - Status: pending
- Add todo item: "Verify Step 7: Stage Files" - Status: pending
- Add todo item: "Step 8: Create Commit" - Status: pending
- Add todo item: "Verify Step 8: Create Commit" - Status: pending
- Add todo item: "Step 9: Push and Verify" - Status: pending
- Add todo item: "Verify Step 9: Push and Verify" - Status: pending
- Add todo item: "Verify Complete Workflow: commit" - Status: pending

**Important**: All todo items are initialized as `pending` and will be updated to `in_progress` when their respective step/verification begins, then to `completed` after verification passes.

## Minimum Requirements

**MANDATORY for successful commit:**
- [ ] Clean build passes (exit code 0)
- [ ] check-logs finds no issues
- [ ] Documentation validates
- [ ] TODO.md updated
- [ ] CHANGELOG.md updated
- [ ] All changes reviewed
- [ ] Git push verified
### Stage 1: Pre-Commit Verification (Steps 1-5)



### Pre-Commit Checklist (MANDATORY - ALWAYS CHECK THIS)

Before starting the commit process, ensure you have completed ALL of these steps. This checklist acts as a quality gate to prevent incomplete commits ([details](../../../kb/gate-enforcement-exit-codes-pattern.md)):

#### Step 1: Run Tests

**MANDATORY: Update step todo item status**

Before starting this step, update the step todo item status to `in_progress`:
- Update todo item: "Step 1: Run Tests"
- Status: in_progress

```bash
# Run comprehensive build verification
make clean build 2>&1
```

**⚠️ MANDATORY STEP VERIFICATION**

**MANDATORY: Update verification todo item status**

Before proceeding to Step 2, update the verification todo item status to `in_progress`:
- Update todo item: "Verify Step 1: Run Tests"
- Status: in_progress

Before proceeding to Step 2, you MUST verify Step 1 completion via step-verifier sub-agent:

1. **Invoke step-verifier sub-agent** to verify:
   - Clean build completed successfully
   - All tests passed
   - No compilation errors
   - Step objectives were met

2. **If verification fails**: Fix issues and re-verify before proceeding

3. **If sub-agent unavailable**: Stop and request user manual verification

**Only after step-verifier verification passes**:

1. **Mark verification complete in session todo list** using `todo_write`:
   - Update todo item: "Verify Step 1: Run Tests"
   - Status: completed

2. **Mark step complete in session todo list** using `todo_write`:
   - Update todo item: "Step 1: Run Tests"
   - Status: completed

#### Step 2: Check Logs

**MANDATORY: Update step todo item status**

Before starting this step, update the step todo item status to `in_progress`:
- Update todo item: "Step 2: Check Logs"
- Status: in_progress

```bash
# Verify no hidden issues in logs
make check-logs
```

**⚠️ MANDATORY STEP VERIFICATION**

**MANDATORY: Update verification todo item status**

Before proceeding to Step 3, update the verification todo item status to `in_progress`:
- Update todo item: "Verify Step 2: Check Logs"
- Status: in_progress

Before proceeding to Step 3, you MUST verify Step 2 completion via step-verifier sub-agent:

1. **Invoke step-verifier sub-agent** to verify:
   - Build logs verified clean
   - No hidden issues found
   - Step objectives were met

2. **If verification fails**: Fix issues and re-verify before proceeding

3. **If sub-agent unavailable**: Stop and request user manual verification

**Only after step-verifier verification passes**:

1. **Mark verification complete in session todo list** using `todo_write`:
   - Update todo item: "Verify Step 2: Check Logs"
   - Status: completed

2. **Mark step complete in session todo list** using `todo_write`:
   - Update todo item: "Step 2: Check Logs"
   - Status: completed

**Additional Notes**:
- Check-logs only works after full build, NOT after individual test runs ([details](../../../kb/build-logs-relationship-principle.md))
- If clean build fails OR check-logs finds issues, STOP - do not proceed ([details](../../../kb/build-verification-before-commit.md))
- `make build` includes documentation validation (`make check-docs`) ([details](../../../kb/batch-documentation-fix-enhancement.md))

#### Step 3: Update Docs

**MANDATORY: Update step todo item status**

Before starting this step, update the step todo item status to `in_progress`:
- Update todo item: "Step 3: Update Docs"
- Status: in_progress

**Documentation Notes**:
- If you changed a module's interface, update its .md file
- For major refactoring, check modules/README.md and dependency trees ([details](../../../kb/refactoring-phase-completion-checklist.md))
- When removing global APIs, use systematic scripts ([details](../../../kb/global-function-removal-script-pattern.md))
- Let compiler errors guide refactoring completion ([details](../../../kb/compilation-driven-refactoring-pattern.md))

**⚠️ MANDATORY STEP VERIFICATION**

**MANDATORY: Update verification todo item status**

Before proceeding to Step 4, update the verification todo item status to `in_progress`:
- Update todo item: "Verify Step 3: Update Docs"
- Status: in_progress

Before proceeding to Step 4, you MUST verify Step 3 completion via step-verifier sub-agent:

1. **Invoke step-verifier sub-agent** to verify:
   - Documentation was updated if module interfaces changed
   - All relevant .md files were checked
   - Step objectives were met

2. **If verification fails**: Fix issues and re-verify before proceeding

3. **If sub-agent unavailable**: Stop and request user manual verification

**Only after step-verifier verification passes**:

1. **Mark verification complete in session todo list** using `todo_write`:
   - Update todo item: "Verify Step 3: Update Docs"
   - Status: completed

2. **Mark step complete in session todo list** using `todo_write`:
   - Update todo item: "Step 3: Update Docs"
   - Status: completed

#### Step 4: Update TODO

**MANDATORY: Update step todo item status**

Before starting this step, update the step todo item status to `in_progress`:
- Update todo item: "Step 4: Update TODO"
- Status: in_progress

**TODO Note**: Mark completed tasks and add any new tasks identified

**⚠️ MANDATORY STEP VERIFICATION**

**MANDATORY: Update verification todo item status**

Before proceeding to Step 5, update the verification todo item status to `in_progress`:
- Update todo item: "Verify Step 4: Update TODO"
- Status: in_progress

Before proceeding to Step 5, you MUST verify Step 4 completion via step-verifier sub-agent:

1. **Invoke step-verifier sub-agent** to verify:
   - TODO.md was updated with completed tasks
   - New tasks were added if identified
   - Step objectives were met

2. **If verification fails**: Fix issues and re-verify before proceeding

3. **If sub-agent unavailable**: Stop and request user manual verification

**Only after step-verifier verification passes**:

1. **Mark verification complete in session todo list** using `todo_write`:
   - Update todo item: "Verify Step 4: Update TODO"
   - Status: completed

2. **Mark step complete in session todo list** using `todo_write`:
   - Update todo item: "Step 4: Update TODO"
   - Status: completed

#### Step 5: Update CHANGELOG

**MANDATORY: Update step todo item status**

Before starting this step, update the step todo item status to `in_progress`:
- Update todo item: "Step 5: Update CHANGELOG"
- Status: in_progress

**CHANGELOG Note**: Document completed milestones and achievements (NON-NEGOTIABLE)
- Include all documentation updates in the same commit as implementation ([details](../../../kb/atomic-commit-documentation-pattern.md))

**⚠️ MANDATORY STEP VERIFICATION**

**MANDATORY: Update verification todo item status**

Before proceeding to Step 6, update the verification todo item status to `in_progress`:
- Update todo item: "Verify Step 5: Update CHANGELOG"
- Status: in_progress

Before proceeding to Step 6, you MUST verify Step 5 completion via step-verifier sub-agent:

1. **Invoke step-verifier sub-agent** to verify:
   - CHANGELOG.md was updated with completed milestones
   - All achievements were documented
   - Step objectives were met

2. **If verification fails**: Fix issues and re-verify before proceeding

3. **If sub-agent unavailable**: Stop and request user manual verification

**Only after step-verifier verification passes**:

1. **Mark verification complete in session todo list** using `todo_write`:
   - Update todo item: "Verify Step 5: Update CHANGELOG"
   - Status: completed

2. **Mark step complete in session todo list** using `todo_write`:
   - Update todo item: "Step 5: Update CHANGELOG"
   - Status: completed

#### [BUILD GATE]

**Verify build and logs are clean before proceeding**

**Expected gate output:**
```
✅ BUILD GATE - PASSED
   Verified: Steps 1,2
```

### Stage 2: Review and Stage (Steps 6-7)



#### Step 6: Review Changes

**MANDATORY: Update step todo item status**

Before starting this step, update the step todo item status to `in_progress`:
- Update todo item: "Step 6: Review Changes"
- Status: in_progress

```bash
# Review all changes
git diff
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

**⚠️ MANDATORY STEP VERIFICATION**

**MANDATORY: Update verification todo item status**

Before proceeding to Step 7, update the verification todo item status to `in_progress`:
- Update todo item: "Verify Step 6: Review Changes"
- Status: in_progress

Before proceeding to Step 7, you MUST verify Step 6 completion via step-verifier sub-agent:

1. **Invoke step-verifier sub-agent** to verify:
   - All changes were reviewed
   - No backup files present
   - All changes are intentional
   - Step objectives were met

2. **If verification fails**: Fix issues and re-verify before proceeding

3. **If sub-agent unavailable**: Stop and request user manual verification

**Only after step-verifier verification passes**:

1. **Mark verification complete in session todo list** using `todo_write`:
   - Update todo item: "Verify Step 6: Review Changes"
   - Status: completed

2. **Mark step complete in session todo list** using `todo_write`:
   - Update todo item: "Step 6: Review Changes"
   - Status: completed

#### [DOCUMENTATION GATE]

**⚠️ CRITICAL: Verify documentation is complete**

**Expected gate output:**
```
✅ DOCUMENTATION GATE - PASSED
   Verified: Steps 3,4,5
```

### Stage 3: Commit and Push (Steps 8-9)



### Commit Workflow

After completing the checklist above, follow these steps precisely:

#### Step 7: Stage Files

**MANDATORY: Update step todo item status**

Before starting this step, update the step todo item status to `in_progress`:
- Update todo item: "Step 7: Stage Files"
- Status: in_progress

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
   - Verify full scope of architectural changes ([details](../../../kb/commit-scope-verification.md), [verification pattern](../../../kb/architectural-change-verification-pattern.md))

**⚠️ MANDATORY STEP VERIFICATION**

**MANDATORY: Update verification todo item status**

Before proceeding to Step 8, update the verification todo item status to `in_progress`:
- Update todo item: "Verify Step 7: Stage Files"
- Status: in_progress

Before proceeding to Step 8, you MUST verify Step 7 completion via step-verifier sub-agent:

1. **Invoke step-verifier sub-agent** to verify:
   - Files were staged successfully
   - All relevant files were included
   - Commit message was drafted
   - Step objectives were met

2. **If verification fails**: Fix issues and re-verify before proceeding

3. **If sub-agent unavailable**: Stop and request user manual verification

**Only after step-verifier verification passes**:

1. **Mark verification complete in session todo list** using `todo_write`:
   - Update todo item: "Verify Step 7: Stage Files"
   - Status: completed

2. **Mark step complete in session todo list** using `todo_write`:
   - Update todo item: "Step 7: Stage Files"
   - Status: completed

#### Step 8: Create Commit

**MANDATORY: Update step todo item status**

Before starting this step, update the step todo item status to `in_progress`:
- Update todo item: "Step 8: Create Commit"
- Status: in_progress

```bash
# Create the commit
git commit -m "$(cat <<'EOF'
[Your commit message here]
EOF
)"
```

3. **Execute the commit:**
   - Create the commit using HEREDOC format for proper multi-line message formatting

**⚠️ MANDATORY STEP VERIFICATION**

**MANDATORY: Update verification todo item status**

Before proceeding to Step 9, update the verification todo item status to `in_progress`:
- Update todo item: "Verify Step 8: Create Commit"
- Status: in_progress

Before proceeding to Step 9, you MUST verify Step 8 completion via step-verifier sub-agent:

1. **Invoke step-verifier sub-agent** to verify:
   - Commit was created successfully
   - Working tree is clean after commit
   - Step objectives were met

2. **If verification fails**: Fix issues and re-verify before proceeding

3. **If sub-agent unavailable**: Stop and request user manual verification

**Only after step-verifier verification passes**:

1. **Mark verification complete in session todo list** using `todo_write`:
   - Update todo item: "Verify Step 8: Create Commit"
   - Status: completed

2. **Mark step complete in session todo list** using `todo_write`:
   - Update todo item: "Step 8: Create Commit"
   - Status: completed

#### Step 9: Push and Verify

**MANDATORY: Update step todo item status**

Before starting this step, update the step todo item status to `in_progress`:
- Update todo item: "Step 9: Push and Verify"
- Status: in_progress

```bash
# Push to remote and verify
git push
git status
```

5. **Push and verify:**
   - Run `git push`
   - **MANDATORY**: Run `git status` after push to verify it completed successfully ([details](../../../kb/git-push-verification.md))
   - Confirm the working tree is clean and branch is up to date

**Important:** Never skip the final `git status` verification - this is a critical step per AGENTS.md.

**⚠️ MANDATORY STEP VERIFICATION**

**MANDATORY: Update verification todo item status**

Before completing the workflow, update the verification todo item status to `in_progress`:
- Update todo item: "Verify Step 9: Push and Verify"
- Status: in_progress

Before completing the workflow, you MUST verify Step 9 completion via step-verifier sub-agent:

1. **Invoke step-verifier sub-agent** to verify:
   - Push completed successfully
   - Working tree is clean after push
   - Branch is up to date with remote
   - Step objectives were met

2. **If verification fails**: Fix issues and re-verify before proceeding

3. **If sub-agent unavailable**: Stop and request user manual verification

**⚠️ MANDATORY FINAL VERIFICATION**

**MANDATORY: Update final verification todo item status**

Before completing the workflow, update the final verification todo item status to `in_progress`:
- Update todo item: "Verify Complete Workflow: commit"
- Status: in_progress

Before completing the workflow, you MUST verify ALL steps were completed correctly:

1. **Invoke step-verifier sub-agent** to verify complete workflow:
   - Verify all 9 steps were completed correctly
   - Verify all step objectives were met
   - Verify commit was created and pushed successfully

2. **If verification fails**: Fix issues and re-verify before completing

3. **If sub-agent unavailable**: Stop and request user manual verification

**Only after ALL steps verified:**

1. **Mark final verification complete in session todo list** using `todo_write`:
   - Update todo item: "Verify Complete Workflow: commit"
   - Status: completed

2. **Mark verification complete in session todo list** using `todo_write`:
   - Update todo item: "Verify Step 9: Push and Verify"
   - Status: completed

3. **Mark step complete in session todo list** using `todo_write`:
   - Update todo item: "Step 9: Push and Verify"
   - Status: completed

## Related Documentation

### CI and Build Patterns
- [CI Check-Logs Requirement](../../../kb/ci-check-logs-requirement.md)
- [Check-Logs Deep Analysis Pattern](../../../kb/check-logs-deep-analysis-pattern.md)
- [Build Verification Before Commit](../../../kb/build-verification-before-commit.md)
- [Git Push Verification](../../../kb/git-push-verification.md)
- [Git Patch Preservation Workflow](../../../kb/git-patch-preservation-workflow.md)

### TDD and Planning Patterns
- [TDD Plan Iteration Split Pattern](../../../kb/tdd-plan-iteration-split-pattern.md)
- [TDD GREEN Phase Minimalism](../../../kb/tdd-green-phase-minimalism.md)
- [TDD Iteration Planning Pattern](../../../kb/tdd-iteration-planning-pattern.md)
- [Temporary Test Cleanup Pattern](../../../kb/temporary-test-cleanup-pattern.md)
- [Plan Review Status Tracking Pattern](../../../kb/plan-review-status-tracking.md)
- [Iterative Plan Review Protocol](../../../kb/iterative-plan-review-protocol.md)
- [Iterative Plan Refinement Pattern](../../../kb/iterative-plan-refinement-pattern.md)

### Refactoring Patterns
- [Dead Code After Mandatory Parameters](../../../kb/dead-code-after-mandatory-parameters.md)
- [API Suffix Cleanup After Consolidation](../../../kb/api-suffix-cleanup-after-consolidation.md)
- [Compilation-Driven Refactoring Pattern](../../../kb/compilation-driven-refactoring-pattern.md)

### Workflow and Session Patterns
- [Context Preservation Across Sessions](../../../kb/context-preservation-across-sessions.md)
- [Session Resumption Without Prompting](../../../kb/session-resumption-without-prompting.md)
- [Claude Code Commit Attribution](../../../kb/claude-code-commit-attribution.md)

## Custom Instructions

If available, the instructions below may override some of the instructions above.

$ARGUMENTS
