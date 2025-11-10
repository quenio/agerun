Migrate a C module to Zig while maintaining C API compatibility.

## MANDATORY KB Consultation

Before starting migration:
1. Search: `grep "zig\|migration\|c-to-zig" kb/README.md`
2. Must read these articles:
   - c-to-zig-module-migration
   - zig-integration-comprehensive
   - zig-module-development-guide
3. Apply migration patterns and best practices

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
Prompt: "Verify Step N: [Step Title] completion for migrate-module-to-zig-abi command.

Todo Item: [Description of what the step accomplished]
Command File: .opencode/command/ar/migrate-module-to-zig-abi.md
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
- Add todo item: "Step 1: Check Existing" - Status: pending
- Add todo item: "Verify Step 1: Check Existing" - Status: pending
- Add todo item: "Step 2: Analyze Dependencies" - Status: pending
- Add todo item: "Verify Step 2: Analyze Dependencies" - Status: pending
- Add todo item: "Step 3: Identify Challenges" - Status: pending
- Add todo item: "Verify Step 3: Identify Challenges" - Status: pending
- Add todo item: "Step 4: Review API" - Status: pending
- Add todo item: "Verify Step 4: Review API" - Status: pending
- Add todo item: "Step 5: Create Zig File" - Status: pending
- Add todo item: "Verify Step 5: Create Zig File" - Status: pending
- Add todo item: "Step 6: Map Types" - Status: pending
- Add todo item: "Verify Step 6: Map Types" - Status: pending
- Add todo item: "Step 7: Implement Functions" - Status: pending
- Add todo item: "Verify Step 7: Implement Functions" - Status: pending
- Add todo item: "Step 8: Run Tests" - Status: pending
- Add todo item: "Verify Step 8: Run Tests" - Status: pending
- Add todo item: "Step 9: Verify Memory" - Status: pending
- Add todo item: "Verify Step 9: Verify Memory" - Status: pending
- Add todo item: "Step 10: Cleanup and Document" - Status: pending
- Add todo item: "Verify Step 10: Cleanup and Document" - Status: pending
- Add todo item: "Verify Complete Workflow: migrate-module-to-zig-abi" - Status: pending

**Important**: All todo items are initialized as `pending` and will be updated to `in_progress` when their respective step/verification begins, then to `completed` after verification passes.

### In-Progress Workflow Detection

If a `/migrate-module-to-zig-abi` workflow is already in progress:

### First-Time Initialization Check


# Migrate Module to Zig ABI

This command uses session todo tracking to ensure systematic C-to-Zig migration while maintaining C API compatibility. The process has 10 steps across 4 phases with critical verification gates.

### Initialize Tracking
```bash
# Start the migration process (replace MODULE with actual module name)
MODULE={{1}}  # Set the module name
```

## Related Documentation

### Tracking Patterns
- [Session Todo List Tracking Pattern](../../../kb/session-todo-list-tracking-pattern.md)
- [Gate Enforcement Exit Codes Pattern](../../../kb/gate-enforcement-exit-codes-pattern.md)
- [Command Thoroughness Requirements Pattern](../../../kb/command-thoroughness-requirements-pattern.md)

### Zig Migration Patterns
- [C to Zig Module Migration](../../../kb/c-to-zig-module-migration.md)
- [Zig Integration Comprehensive](../../../kb/zig-integration-comprehensive.md)
- [Zig Migration Memory Debugging](../../../kb/zig-migration-memory-debugging.md)
- [Zig Memory Allocation with ar_allocator](../../../kb/zig-memory-allocation-with-ar-allocator.md)
- [Zig C Memory Tracking Consistency](../../../kb/zig-c-memory-tracking-consistency.md)
- [Zig Build Flag Configuration](../../../kb/zig-build-flag-configuration.md)
- [Zig Error Path Testing Pattern](../../../kb/zig-error-path-testing-pattern.md)
- [Zig Errdefer Value Capture Pattern](../../../kb/zig-errdefer-value-capture-pattern.md)