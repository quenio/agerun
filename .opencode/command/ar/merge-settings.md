Safely merge local settings into main settings file with progress tracking.

**Note**: This workflow uses session todo tracking for safe multi-step integration with quality gates ([details](../../../kb/settings-file-merging-pattern.md)). The stats file management has been optimized with whitespace trimming to ensure reliable execution.

## Quick Start

```bash
# Initialize the merge workflow

# Follow workflow steps as indicated
```

# Merge Settings

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
   - **DO NOT** use vague summaries - provide specific details (see [kb/sub-agent-verification-pattern.md](../../../kb/sub-agent-verification-pattern.md) for examples)

2. **Invoke step-verifier sub-agent**
   - Use `mcp_sub-agents_run_agent` tool with:
     - Agent: `"step-verifier"`
     - Prompt: See format below
     - The step-verifier will independently verify your claims

3. **Handle Verification Results**
  
   **If verification PASSES** (report shows "✅ STEP VERIFIED" or "All requirements met"):
     - Proceed to next step
     -   
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
Prompt: "Verify Step N: [Step Title] completion for merge-settings command.

Todo Item: [Description of what the step accomplished]
Command File: .opencode/command/ar/merge-settings.md
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
- If accomplishment report lacks concrete evidence, step-verifier will STOP execution and require evidence "⚠️ STOP EXECUTION", you MUST fix issues before proceeding


## MANDATORY: Initialize All Todo Items

**CRITICAL**: Before executing ANY steps, add ALL step and verification todo items to the session todo list using `todo_write`:

**Step and Verification Todo Items:**
- Add todo item: "Step 1: Check Files" - Status: pending
- Add todo item: "Verify Step 1: Check Files" - Status: pending
- Add todo item: "Step 2: Read Settings" - Status: pending
- Add todo item: "Verify Step 2: Read Settings" - Status: pending
- Add todo item: "Step 3: Merge Permissions" - Status: pending
- Add todo item: "Verify Step 3: Merge Permissions" - Status: pending
- Add todo item: "Step 4: Validate Result" - Status: pending
- Add todo item: "Verify Step 4: Validate Result" - Status: pending
- Add todo item: "Step 5: Refactor Permissions" - Status: pending
- Add todo item: "Verify Step 5: Refactor Permissions" - Status: pending
- Add todo item: "Step 6: Commit and Cleanup" - Status: pending
- Add todo item: "Verify Step 6: Commit and Cleanup" - Status: pending
- Add todo item: "Verify Complete Workflow: merge-settings" - Status: pending

**Important**: All todo items are initialized as `pending` and will be updated to `in_progress` when their respective step/verification begins, then to `completed` after verification passes.

## Overview

This command uses session todo tracking to ensure safe merging of local settings into the main settings file. The process has 6 steps across 4 phases with verification gates.

The workflow:
1. **Discovery Phase** - Detect local settings file and plan merge strategy
2. **Merge Phase** - Read, merge, and validate settings with quality gates
3. **Optimization Phase** - Refactor permissions to generic patterns
4. **Cleanup Phase** - Commit and push all changes

## Session Todo Tracking

Track progress using the session todo list initialized above.

**Problem: Resume existing workflow**
```bash
# Check status

# Continue from next pending step
```

**Problem: Start fresh (discard previous progress)**
```bash
# Clean up old tracking

# Re-initialize
```

### Merge Issues

**Problem: Local settings file still exists after workflow completes**
```bash
# This indicates the merge and commit succeeded but the local file wasn't removed
# or the workflow didn't actually commit changes
#
# Check what happened:
git log --oneline -5

# If the workflow shows complete but local file exists:
# 1. Verify the main settings file has all merged permissions
grep '"Bash(' ./.claude/settings.json | wc -l

# 2. If numbers didn't increase, the merge didn't work - reset and retry

# 3. If the merge worked but file wasn't removed, manually clean up
rm ./.claude/settings.local.json
git add ./.claude/settings.json
git commit -m "chore: remove local settings file after successful merge"
git push
```

**Problem: Local settings file not found**
```bash
# This is normal - the workflow handles both cases
# With local file: performs merge
# Without local file: workflow reports nothing to merge and completes
```

**Problem: Permission conflicts during merge**
```bash
# The merge operation uses deduplication logic
# Examine the merge statistics output to see conflicts resolved
# Check .claude/settings.json for merged permissions
```

**Problem: Invalid JSON after merge**
```bash
# Solution: The validation step catches this
# Check the settings.json file for syntax errors:
python3 -m json.tool .claude/settings.json

# If validation fails, the workflow will stop at Step 4
# Fix the JSON file and re-run the workflow
```

**Problem: Stats file whitespace errors (line N: XX: command not found)**
```bash
# Solution: This was fixed in recent versions
# Update scripts if you see this error:
git pull origin main

# Clean and retry:
```

### Common Solutions

**View current workflow state:**
**Debug script execution:**
```bash
# Run individual step manually to see errors
./scripts/check-settings-local-file.sh
./scripts/read-settings-files.sh
./scripts/merge-permissions.sh
./scripts/validate-merged-settings.sh
```

**Verify merged settings:**
```bash
# Check merged permissions count
grep '"Bash(' ./.claude/settings.json | wc -l

# Validate JSON syntax
python3 -m json.tool ./.claude/settings.json > /dev/null && echo "✅ Valid JSON"

# Check for local file removal
[ -f ./.claude/settings.local.json ] && echo "❌ Local file still exists" || echo "✅ Local file removed"
```

## Related Documentation

- [Settings File Merging Pattern](../../../kb/settings-file-merging-pattern.md) - Design patterns for settings merging
- [Session Todo List Tracking Pattern](../../../kb/session-todo-list-tracking-pattern.md) - Workflow execution guarantees
- [Sub-Agent Verification Pattern](../../../kb/sub-agent-verification-pattern.md) - Step verification via sub-agents