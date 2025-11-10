# MCP Sub-Agent Integration Pattern

## Learning
Integrating MCP sub-agents into command workflows requires clear invocation patterns, fallback handling for MCP unavailability, and proper result interpretation including STOP instruction handling.

## Importance
- **Sophisticated capabilities**: Sub-agents provide verification and analysis capabilities beyond simple scripts
- **Reliability**: Proper fallback handling ensures workflows continue even if MCP is unavailable
- **Clear integration**: Well-defined patterns make sub-agent integration consistent across commands
- **Error handling**: Proper result interpretation prevents workflow failures from sub-agent issues

## Example

**MCP configuration** (`.cursor/mcp.json`):

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

**Sub-agent invocation pattern:**

```markdown
### Step Verifier Invocation

**Preferred method** (via MCP sub-agent):
Use the `mcp_sub-agents_run_agent` tool to invoke the step-verifier:

```
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

**Fallback** (if MCP unavailable):
1. **STOP execution immediately**
2. Inform user: "⚠️ Step verification sub-agent unavailable (MCP not accessible)."
3. Read `.claude/step-verifier.md` for verification criteria
4. Request user to manually verify step completion
5. Wait for explicit user confirmation before proceeding
6. Document manual verification in checkpoint notes
```

**Result interpretation:**

```markdown
2. **Handle Verification Results**
   
   **Check verification report for STOP instructions:**
   - If report contains "⚠️ STOP EXECUTION" or "STOP:" instruction:
     - **STOP execution immediately**
     - Do NOT proceed to next step
     - Fix all reported issues from the verification report
     - Re-invoke step-verifier with updated evidence after fixes
     - Only proceed after verification report shows "✅ STEP VERIFIED" or "All requirements met"
   
   **If verification PASSES** (report shows "✅ STEP VERIFIED" or "All requirements met"):
     - Proceed to next step
     - Mark checkpoint step as complete (for progress tracking only - verification already done by step-verifier)
   
   **If verification FAILS** (missing elements or incomplete work):
     - STOP execution immediately
     - Fix all reported issues from verification report
     - Re-invoke step-verifier to confirm fixes
     - Only proceed after verification passes
   
   **If sub-agent CANNOT be executed** (MCP unavailable or tool error):
     - STOP execution immediately
     - Inform user: "⚠️ Step verification sub-agent unavailable. Please manually verify Step N completion before proceeding."
     - Provide step description and expected outcomes
     - Wait for explicit user confirmation before continuing
     - Document manual verification in checkpoint notes
```

## Generalization

**Integration components:**

1. **MCP configuration**: Configure MCP server in `.cursor/mcp.json`
2. **Sub-agent specification**: Define sub-agent behavior in `.claude/[agent-name].md`
3. **Invocation pattern**: Use `mcp_sub-agents_run_agent` tool with agent name and prompt
4. **Result interpretation**: Check for STOP instructions, verification status, missing elements
5. **Fallback handling**: Handle MCP unavailability gracefully

**Invocation pattern structure:**

```
Agent: "[agent-name]"
Prompt: "[Verification request with context]

[Context information]:
- Todo Item: [What was accomplished]
- Command File: [Command file path]
- Step: [Step number and title]

[Verification requirements]:
- [Specific verification points]
- [Expected outcomes]

[Request]:
Provide verification report with evidence."
```

**Result interpretation patterns:**

| Result Type | Action |
|------------|--------|
| STOP instruction present | Stop execution, fix issues, re-verify |
| Verification PASSES | Proceed to next step |
| Verification FAILS | Stop execution, fix issues, re-verify |
| MCP unavailable | Stop execution, request manual verification |

**Fallback strategy:**

1. **Detect unavailability**: Tool error or MCP server not responding
2. **Stop execution**: Do not proceed without verification
3. **Inform user**: Clear message about unavailability
4. **Provide context**: Step description and expected outcomes
5. **Request confirmation**: Wait for explicit user approval
6. **Document**: Record manual verification in checkpoint notes

## Implementation

**Command integration steps:**

1. **Add MCP configuration** (if not already present):
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

2. **Create sub-agent specification** (`.claude/[agent-name].md`):
   - Define agent responsibilities
   - Document verification patterns
   - Specify report format
   - Include STOP instruction guidelines

3. **Add invocation section** to command:
   ```markdown
   ### Step Verifier Invocation
   
   **Preferred method** (via MCP sub-agent):
   Use the `mcp_sub-agents_run_agent` tool to invoke the step-verifier:
   [Invocation pattern]
   
   **Fallback** (if MCP unavailable):
   [Fallback steps]
   ```

4. **Add result interpretation** to command:
   ```markdown
   2. **Handle Verification Results**
      [Result interpretation patterns]
   ```

5. **Add fallback handling** to each step:
   ```markdown
   3. **If sub-agent unavailable**: Stop and request user manual verification
   ```

**Error handling:**

```markdown
**If sub-agent CANNOT be executed** (MCP unavailable or tool error):
- STOP execution immediately
- Inform user with clear message
- Provide step description and expected outcomes
- Wait for explicit user confirmation
- Document manual verification
```

## Related Patterns
- [Sub-Agent Verification Pattern](sub-agent-verification-pattern.md) - Using sub-agents for verification
- [Evidence Validation Requirements Pattern](evidence-validation-requirements-pattern.md) - Validating evidence existence and validity in verification systems
- [Checkpoint Tracking Verification Separation](checkpoint-tracking-verification-separation.md) - Separating tracking from verification
- [Session Todo List Tracking Pattern](session-todo-list-tracking-pattern.md) - Tracking steps in session todo list
- [Interleaved Todo Item Pattern](interleaved-todo-item-pattern.md) - Initializing step and verification todos together at workflow start

## Verification Questions

Before integrating MCP sub-agents, ask:
- What verification capabilities are needed? (Determine if sub-agent is appropriate)
- Is MCP configured and available? (Check configuration before relying on it)
- What happens if MCP is unavailable? (Define fallback strategy)
- How are sub-agent results interpreted? (Define result interpretation patterns)

MCP sub-agent integration enables sophisticated verification capabilities but requires proper configuration, invocation patterns, and fallback handling.

