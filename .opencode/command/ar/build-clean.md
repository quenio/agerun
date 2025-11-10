Execute a clean build for comprehensive build verification with minimal output and check for hidden issues.

## ⚠️ CRITICAL: Let the script manage tracking

**DO NOT manually initialize before running this command.** The script handles all tracking initialization, execution, and cleanup automatically. Just run the script and let it complete.

## Quick Start

```bash
./scripts/run-build-clean.sh
```

That's it! The script will handle everything automatically.

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

1. **Report accomplishments with evidence**
   - Describe what was accomplished (files created/modified, commands executed, outputs produced)
   - Provide evidence (file paths, command outputs, git status/diff)
   - **DO NOT** tell step-verifier what to verify - report what was done

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
Prompt: "Verify Step N: [Step Title] completion for build-clean command.

Todo Item: [Description of what the step accomplished]
Command File: .opencode/command/ar/build-clean.md
Step: Step N: [Step Title]

Accomplishment Report:
[Report what was accomplished with evidence: files created/modified, commands executed, outputs produced, etc. The step-verifier will independently verify these claims by reading files, checking git status, etc.]"
```

**CRITICAL**: 
- Report accomplishments with evidence, NOT instructions
- The step-verifier independently verifies by reading command files, checking files, git status/diff, etc.
- If step-verifier reports "⚠️ STOP EXECUTION", you MUST fix issues before proceeding

## MANDATORY: Initialize All Todo Items

**CRITICAL**: Before executing ANY steps, add ALL step and verification todo items to the session todo list using `todo_write`:

**Step and Verification Todo Items:**
- Add todo item: "Step 1: Clean Artifacts" - Status: pending
- Add todo item: "Verify Step 1: Clean Artifacts" - Status: pending
- Add todo item: "Step 2: Compile Code" - Status: pending
- Add todo item: "Verify Step 2: Compile Code" - Status: pending
- Add todo item: "Step 3: Verify Build" - Status: pending
- Add todo item: "Verify Step 3: Verify Build" - Status: pending
- Add todo item: "Verify Complete Workflow: build-clean" - Status: pending

**Important**: All todo items are initialized as `pending` and will be updated to `in_progress` when their respective step/verification begins, then to `completed` after verification passes.

## Workflow Execution

Run the complete workflow:

```bash
./scripts/run-build-clean.sh
```

This script handles all stages of the clean build process:

### What the Script Does

1. **Clean Artifacts**: Removes all build artifacts and intermediate files
2. **Compile Code**: Compiles all modules and binaries from scratch
3. **Verify Build**: Confirms all artifacts were rebuilt correctly
4. **Completion**: Marks the workflow as complete

## Troubleshooting

If the script fails, simply rerun it:

```bash
./scripts/run-build-clean.sh
```

## Minimum Requirements

**MANDATORY for successful completion:**
- [ ] Command executes without errors
- [ ] Expected output is produced
- [ ] No unexpected warnings or issues

**MANDATORY**: Always run check-logs after clean build. CI will reject builds that fail log checks. ([details](../../../kb/build-verification-before-commit.md))

**IMPORTANT**: Clean builds are required when:
- Changing dependencies
- Major refactoring
- Suspicious incremental build behavior
- Before final commit

**Critical questions**:
- Did the previous incremental build behave unexpectedly?
- Have you changed any header files?
- Are you seeing strange linker errors?

For example, if you see "undefined reference" errors in incremental builds, a clean build often resolves them.

```bash
# Verify ready to execute
```

**Expected gate output:**
```
✅ GATE 'Ready' - PASSED
   Verified: Steps 1
```

## Command

```bash
make clean build 2>&1 && make check-logs

# Mark execution complete
```

## Expected Output

### Success State
```
Removing build artifacts...
Cleaning logs directory...
Building library...
Running all tests...
All 68 tests passed! (1126 assertions)
Build took 1m 24s
========================================
         BUILD SUCCESSFUL! ✓
========================================
Running log checks...
✅ No errors found in logs - CI ready!
```

### Failure States

**Clean Failure (rare):**
```
Removing build artifacts...
rm: cannot remove 'bin/': Permission denied
make: *** [clean] Error 1
```

**Build Failure After Clean:**
```
Removing build artifacts...
Cleaning logs directory...
Building library...
modules/ar_data.c:123: error: 'ar_data_t' undeclared
make: *** [build] Error 1
```

## Key Points

- **Clean build takes longer** (~1m 24s vs ~20s incremental)
- **Removes all artifacts** including bin/, logs/, and object files
- **Required for**: Major refactoring, dependency changes, CI verification
- **Always use `2>&1`** to capture stderr for complete error visibility
- **Always follow with `make check-logs`** to verify CI readiness

## Related Patterns
- [Build Verification Before Commit](../../../kb/build-verification-before-commit.md)
- [Script Debugging Through Isolation](../../../kb/script-debugging-through-isolation.md) - When build errors occur, isolate the failure
- [Cross-Platform Bash Script Patterns](../../../kb/cross-platform-bash-script-patterns.md) - Build scripts must work on macOS and Linux