# Sub-Agent Verification Pattern

## Learning
Using MCP sub-agents for step verification provides sophisticated verification capabilities that checkpoint scripts cannot offer, including evidence-based analysis, structured reporting, and automatic STOP instructions when critical failures are detected.

## Importance
- **Sophisticated verification**: Sub-agents can perform complex analysis that simple checkpoint scripts cannot
- **Evidence-based reporting**: Verification reports include specific evidence (file paths, line numbers, test results)
- **Automatic failure detection**: Sub-agents can detect missing elements and issue STOP instructions
- **Structured remediation**: Provides step-by-step recommendations for fixing issues
- **Separation of concerns**: Keeps verification logic separate from progress tracking

## Example

**Command integration pattern:**

```markdown
## STEP VERIFICATION ENFORCEMENT

**MANDATORY**: After completing each step, verify step completion using step-verifier sub-agent.

### Step Verification Process

After completing each step (before calling checkpoint-update.sh):

1. **Invoke Step Verifier Sub-Agent**
   - Use MCP to invoke the step-verifier sub-agent with:
     - Todo Item Description: Description of what the step accomplished
     - Command File Path: `.opencode/command/ar/command-name.md`
     - Step Number and Title: The step that was just completed

2. **Handle Verification Results**
   - If report contains "⚠️ STOP EXECUTION": STOP immediately, fix issues, re-verify
   - If verification PASSES: Proceed to next step
   - If verification FAILS: Fix issues and re-verify before proceeding
   - If sub-agent unavailable: Stop and request user manual verification

3. **Only after verification passes**: Mark checkpoint step as complete
```

**Sub-agent invocation:**

```bash
# Using mcp_sub-agents_run_agent tool
Agent: "step-verifier"
Prompt: "Verify Step N: [Step Title] completion for command-name command.

Todo Item: [Description of what the step accomplished]
Command File: .opencode/command/ar/command-name.md
Step: Step N: [Step Title]

Please verify that:
- [Step-specific verification points]
- All step objectives were met
- Required outputs were produced

Provide verification report with evidence."
```

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

3. **Update checkpoint call** to happen AFTER verification:
   ```bash
   # Verification happens first
   # Then checkpoint-update (for tracking only)
   ./scripts/checkpoint-update.sh command-name N
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
- [MCP Sub-Agent Integration Pattern](mcp-sub-agent-integration-pattern.md) - How to integrate sub-agents

## Verification Questions

Before using sub-agent verification, ask:
- Does verification require complex analysis beyond simple checks?
- Do verification reports need to include specific evidence?
- Should failures trigger automatic STOP instructions?
- Does verification need structured remediation recommendations?

Sub-agent verification provides sophisticated capabilities for ensuring step completion quality, but requires MCP integration and proper result interpretation.

