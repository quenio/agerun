# Sub-Agent Verification Pattern

## Learning
Using MCP sub-agents for step verification provides sophisticated verification capabilities that checkpoint scripts cannot offer, including evidence-based analysis, structured reporting, and automatic STOP instructions when critical failures are detected.

## Importance
- **Sophisticated verification**: Sub-agents can perform complex analysis that simple checkpoint scripts cannot
- **Evidence-based reporting**: Verification reports include specific evidence (file paths, line numbers, test results)
- **Evidence validation**: Sub-agents validate that all evidence actually exists and is valid (files exist, paths correct, git diff matches, outputs are real)
- **Automatic failure detection**: Sub-agents can detect missing elements, invalid evidence, and issue STOP instructions
- **Structured remediation**: Provides step-by-step recommendations for fixing issues
- **Separation of concerns**: Keeps verification logic separate from progress tracking

## Example

**Command integration pattern:**

```markdown
## STEP VERIFICATION ENFORCEMENT

**MANDATORY**: After completing each step, verify step completion using step-verifier sub-agent.

### Step Verification Process

After completing each step:

1. **Invoke Step Verifier Sub-Agent**
   - Use MCP to invoke the step-verifier sub-agent with:
     - Todo Item Description: Description of what the step accomplished
     - Command File Path: `.opencode/command/ar/command-name.md`
     - Step Number and Title: The step that was just completed

2. **Handle Verification Results**
   - If report contains "⚠️ STOP EXECUTION": STOP immediately, fix issues, re-verify
   - If verification PASSES: Mark step complete in session todo list, proceed to next step
   - If verification FAILS: Fix issues and re-verify before proceeding
   - If sub-agent unavailable: Stop and request user manual verification

3. **Only after verification passes**: Mark step complete in session todo list
```

**Sub-agent invocation:**

```bash
# Using mcp_sub-agents_run_agent tool
Agent: "step-verifier"
Prompt: "Verify Step N: [Step Title] completion for command-name command.

Todo Item: [Description of what the step accomplished]
Command File: .opencode/command/ar/command-name.md
Step: Step N: [Step Title]

Accomplishment Report:
[Report what was accomplished with evidence: files created/modified, commands executed, outputs produced, etc. The step-verifier will independently verify these claims by reading files, checking git status, etc.]"
```

**CRITICAL: Evidence-Based Reporting**

The top-level agent must report accomplishments with **concrete evidence**, NOT instructions or summaries. The step-verifier independently verifies claims by reading files, checking git status/diff, analyzing outputs, **AND VALIDATING THAT ALL EVIDENCE ACTUALLY EXISTS AND IS VALID** (files exist, paths correct, line numbers accurate, git diff matches, outputs are real). Report what was done with specific evidence, not what should be verified.

**Evidence Requirements:**

1. **File Changes**: Include actual file paths, line numbers, and content snippets
   - ✅ **GOOD**: "Fixed broken link in `.claude/step-verifier.md` line 41, changed from incorrect path to `mcp-sub-agent-integration-pattern.md`. Evidence: `git diff` shows line 41 changed from `[checkpoint-based-workflow-pattern.md](../kb/checkpoint-based-workflow-pattern.md)` to `[mcp-sub-agent-integration-pattern.md](../kb/mcp-sub-agent-integration-pattern.md)`"
   - ❌ **BAD**: "Fixed broken link in step-verifier.md"

2. **Command Execution**: Include full command output, not just success/failure
   - ✅ **GOOD**: "Ran `make clean build 2>&1`. Output: `[full build output showing compilation, linking, test execution]`. Exit code: 0. All tests passed: 47/47. Memory leaks: 0"
   - ❌ **BAD**: "Build completed successfully"

3. **Test Results**: Include test names, pass/fail status, and execution details
   - ✅ **GOOD**: "Executed `make ar_string_tests 2>&1`. Test output: `test_string__trim ... OK`, `test_string__split ... OK`. All 12 tests passed. Memory report: `bin/run-tests/memory_report_ar_string_tests.log` shows 0 leaks"
   - ❌ **BAD**: "Tests passed"

4. **Documentation Updates**: Include file paths, section names, and actual content
   - ✅ **GOOD**: "Updated `CHANGELOG.md` with new entry dated 2025-11-10. Entry added at line 3 under '## 2025-11-10' section. Content: `- **Remove Remaining Checkpoint Script References from Commands**` followed by implementation details. Evidence: `git diff CHANGELOG.md` shows 12 lines added"
   - ❌ **BAD**: "Updated CHANGELOG.md"

5. **Git Status**: Include actual git status/diff output when relevant
   - ✅ **GOOD**: "Staged files: `git status` shows `modified: AGENTS.md`, `modified: .opencode/command/ar/execute-plan.md`, `modified: CHANGELOG.md`. `git diff --cached` shows [actual diff output]"
   - ❌ **BAD**: "Files staged for commit"

6. **Grep/Verification Output**: Include actual command output proving claims
   - ✅ **GOOD**: "Verified no checkpoint references remain: `grep -i 'checkpoint' .opencode/command/ar/execute-plan.md .opencode/command/ar/commit.md` returned no matches (exit code 1)"
   - ❌ **BAD**: "Verified checkpoint references removed"

**Examples:**

✅ **GOOD Accomplishment Report:**
```
Accomplishment Report:
Updated `.opencode/command/ar/execute-plan.md` troubleshooting section to remove checkpoint script references.

Evidence:
- File modified: `.opencode/command/ar/execute-plan.md`
- Line changes: `git diff` shows lines 2356-2360 changed from:
  ```
  ### If progress tracking gets stuck:
  # Check current status
  ```
  to:
  ```
  ### If step tracking gets stuck:
  # Check session todo list status
  ```
- Verification: `grep -i 'checkpoint.*script' .opencode/command/ar/execute-plan.md` returns no matches
- Git status: File appears as modified in `git status`
```

❌ **BAD Accomplishment Report:**
```
Accomplishment Report:
Updated execute-plan.md to remove checkpoint references. Please verify the changes are correct.
```

**Key Principles:**
- Include **actual command outputs**, not summaries
- Include **file paths with line numbers** for code references
- Include **git diff output** showing exact changes
- Include **grep/search output** proving verification claims
- Include **test results** with specific test names and outcomes
- Include **build outputs** showing compilation and execution details
- **DO NOT** tell step-verifier what to verify - report what was done with evidence
- **DO NOT** use vague descriptions - provide specific details
- **CRITICAL**: All evidence will be validated by step-verifier - files must exist, paths must be correct, line numbers must be accurate, git diff must match actual changes, outputs must be real

**Evidence Validation by Step-Verifier:**

The step-verifier sub-agent validates ALL evidence provided in accomplishment reports:
- **File existence**: Verifies all mentioned files actually exist at the specified paths
- **Path correctness**: Validates file paths are accurate and accessible
- **Line number accuracy**: Checks that mentioned line numbers contain the claimed content
- **Git diff validation**: Executes `git diff` and `git status` to verify claims match actual changes
- **Command output validation**: Verifies command outputs are plausible and match actual execution
- **Test result validation**: Confirms test results match actual test files and execution
- **STOP on validation failure**: If evidence validation fails, step-verifier will STOP execution and require accurate evidence

**Verification report interpretation:**

```markdown
# Example verification report from sub-agent

## Step Verification Report

### ✅ Requirements Met
1. **File Created**
   - File: `modules/ar_module.h`
   - Evidence: File exists at expected path
   - Status: ✅ VERIFIED

2. **Documentation Updated**
   - File: `modules/ar_module.md`
   - Evidence: Documentation includes new API
   - Status: ✅ VERIFIED

### Overall Status
✅ **STEP VERIFIED: All requirements met. Safe to proceed to next step.**
```

**STOP instruction handling:**

```markdown
# Example STOP instruction from sub-agent

⚠️ STOP EXECUTION: Do not proceed to next step.

Critical failures detected:
- Missing required file: `modules/ar_module_tests.c`
- Documentation not updated: `modules/ar_module.md` unchanged
- Tests not executed: No test results found

The top-level agent must fix these issues before continuing.
```

## Generalization

**When to use sub-agent verification:**

1. **Complex verification requirements**: When verification needs analysis beyond simple file existence checks
2. **Evidence-based reporting**: When verification reports need to include specific evidence
3. **Structured remediation**: When failures need step-by-step fix recommendations
4. **Multi-aspect verification**: When steps require checking multiple aspects (files, tests, docs, quality)

**Sub-agent verification vs checkpoint scripts:**

| Aspect | Checkpoint Scripts | Sub-Agent Verification |
|--------|-------------------|----------------------|
| Purpose | Progress tracking | Step verification |
| Capabilities | Simple status tracking | Complex analysis |
| Reporting | Progress bars | Evidence-based reports |
| Failure handling | Exit codes | STOP instructions + remediation |
| Integration | Bash scripts | MCP sub-agents |

**Pattern structure:**

1. **Invocation**: Use `mcp_sub-agents_run_agent` tool with agent name and verification prompt
2. **Result interpretation**: Check for STOP instructions, verification status, missing elements
3. **Failure handling**: Fix issues, re-invoke sub-agent, only proceed after verification passes
4. **Fallback**: If MCP unavailable, stop and request user manual verification

## Implementation

**Command integration steps:**

1. **Add verification section** to command file:
   ```markdown
   ## STEP VERIFICATION ENFORCEMENT
   
   **MANDATORY**: After completing each step, verify via step-verifier sub-agent.
   ```

2. **Add verification checkpoint** after each step:
   ```markdown
   **⚠️ MANDATORY STEP VERIFICATION**
   
   Before proceeding to Step N+1, verify Step N completion:
   1. Invoke step-verifier sub-agent
   2. Check for STOP instructions
   3. Fix issues if verification fails
   4. Only proceed after verification passes
   ```

3. **Update session todo list** AFTER verification:
   ```markdown
   **Only after step-verifier verification passes**:
   
   1. **Mark step complete in session todo list**:
      - Update todo item: "Step N: [Title]"
      - Status: completed
   
   2. **Mark verification complete**:
      - Update todo item: "Verify Step N: [Title]"
      - Status: completed
   ```

**Sub-agent configuration:**

1. **MCP configuration** (`.cursor/mcp.json`):
   ```json
   {
     "mcpServers": {
       "sub-agents": {
         "command": "npx",
         "args": ["-y", "sub-agents-mcp"],
         "env": {
           "AGENTS_DIR": "/path/to/.claude/agents",
           "AGENT_TYPE": "cursor"
         }
       }
     }
   }
   ```

2. **Sub-agent specification** (`.claude/step-verifier.md`):
   - Defines verification responsibilities
   - Documents verification patterns
   - Specifies report format
   - Includes STOP instruction guidelines

## Related Patterns
- [Checkpoint Tracking Verification Separation](checkpoint-tracking-verification-separation.md) - Separating tracking from verification
- [Checkpoint-Based Workflow Pattern](checkpoint-based-workflow-pattern.md) - Progress tracking with checkpoint scripts
- [Checkpoint Work Verification Anti-Pattern](checkpoint-work-verification-antipattern.md) - Why verification is critical
- [Evidence Validation Requirements Pattern](evidence-validation-requirements-pattern.md) - Validating evidence existence and validity in verification systems
- [MCP Sub-Agent Integration Pattern](mcp-sub-agent-integration-pattern.md) - How to integrate sub-agents
- [Interleaved Todo Item Pattern](interleaved-todo-item-pattern.md) - Initializing step and verification todos together
- [Session Todo List Tracking Pattern](session-todo-list-tracking-pattern.md) - Tracking steps in session todo list

## Verification Questions

Before using sub-agent verification, ask:
- Does verification require complex analysis beyond simple checks?
- Do verification reports need to include specific evidence?
- Should failures trigger automatic STOP instructions?
- Does verification need structured remediation recommendations?

Sub-agent verification provides sophisticated capabilities for ensuring step completion quality, but requires MCP integration and proper result interpretation.

