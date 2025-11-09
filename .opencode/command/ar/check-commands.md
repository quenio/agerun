Validate that all Claude Code commands follow comprehensive structure standards.

**Excellence Gate**: Commands must achieve 90%+ average score for quality documentation. The checker validates checkpoint tracking, expected outputs, troubleshooting sections, and minimum requirements ([details](../../../kb/command-documentation-excellence-gate.md)).

## MANDATORY KB Consultation

Before validation:
1. Search: `grep "command\|excellence\|validation" kb/README.md`
2. Must read:
   - command-documentation-excellence-gate
   - command-helper-script-extraction-pattern ([extraction pattern](../../../kb/command-helper-script-extraction-pattern.md))
   - validation-feedback-loop-effectiveness
   - command-output-documentation-pattern
3. Apply command quality standards
   - Scripts should be extracted for reusability: one focused script per shell block ([details](../../../kb/command-helper-script-extraction-pattern.md))
   - Use domain-specific naming for extracted scripts ([details](../../../kb/script-domain-naming-convention.md))
   - This command exemplifies the orchestrator pattern: command file shows all steps directly ([details](../../../kb/command-orchestrator-pattern.md))

## CHECKPOINT WORKFLOW ENFORCEMENT

**CRITICAL**: This command MUST use checkpoint tracking for progress tracking ONLY. All verification is done via step-verifier sub-agent, NOT via checkpoint scripts ([details](../../../kb/checkpoint-tracking-verification-separation.md)).

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

After completing each step (before calling `checkpoint-update.sh`), you MUST:

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
     - Mark checkpoint step as complete (for progress tracking only - verification already done by step-verifier)
  
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
Prompt: "Verify Step N: [Step Title] completion for check-commands command.

Todo Item: [Description of what the step accomplished]
Command File: .opencode/command/ar/check-commands.md
Step: Step N: [Step Title]

Accomplishment Report:
[Report what was accomplished with evidence: files created/modified, commands executed, outputs produced, etc. The step-verifier will independently verify these claims by reading files, checking git status, etc.]"
```

**CRITICAL**: 
- Report accomplishments with evidence, NOT instructions
- The step-verifier independently verifies by reading command files, checking files, git status/diff, etc.
- If step-verifier reports "⚠️ STOP EXECUTION", you MUST fix issues before proceeding

### In-Progress Workflow Detection

If a `/check-commands` workflow is already in progress:

```bash
./scripts/checkpoint-status.sh check-commands --verbose
# Resume: ./scripts/checkpoint-update.sh check-commands STEP=N
# Or reset: ./scripts/checkpoint-cleanup.sh check-commands && ./scripts/checkpoint-init.sh check-commands "Scan Commands" "Validate Structure" "Calculate Scores" "Identify Issues" "Generate Report"
```

### First-Time Initialization Check

```bash
./scripts/checkpoint-init.sh check-commands "Scan Commands" "Validate Structure" "Calculate Scores" "Identify Issues" "Generate Report"
```

## PRECONDITION: Checkpoint Tracking Must Be Initialized

```bash
./scripts/checkpoint-require.sh check-commands
```

## MANDATORY: Initialize All Todo Items

**CRITICAL**: Before executing ANY steps, add ALL step and verification todo items to the session todo list using `todo_write`:

**Step and Verification Todo Items:**
- Add todo item: "Step 1: Scan Commands" - Status: pending
- Add todo item: "Verify Step 1: Scan Commands" - Status: pending
- Add todo item: "Step 2: Validate Structure" - Status: pending
- Add todo item: "Verify Step 2: Validate Structure" - Status: pending
- Add todo item: "Step 3: Calculate Scores" - Status: pending
- Add todo item: "Verify Step 3: Calculate Scores" - Status: pending
- Add todo item: "Step 4: Identify Issues" - Status: pending
- Add todo item: "Verify Step 4: Identify Issues" - Status: pending
- Add todo item: "Step 5: Generate Report" - Status: pending
- Add todo item: "Verify Step 5: Generate Report" - Status: pending
- Add todo item: "Verify Complete Workflow: check-commands" - Status: pending

**Important**: All todo items are initialized as `pending` and will be updated to `in_progress` when their respective step/verification begins, then to `completed` after verification passes.

# Check Commands
## Checkpoint Tracking

This command validates the structure and quality of all Claude Code commands. The process has 5 checkpoints to ensure systematic validation and improvement using iterative feedback loops ([details](../../../kb/validation-feedback-loop-effectiveness.md)).

### Initialize Tracking
```bash
# Start the command validation process
./scripts/checkpoint-init.sh check-commands "Scan Commands" "Validate Structure" "Calculate Scores" "Identify Issues" "Generate Report"
```

**Expected output:**
```
📍 Starting: check-commands (5 steps)
📁 Tracking: /tmp/check-commands-progress.txt
→ Run: ./scripts/checkpoint-update.sh check-commands STEP=1
```

### Check Progress
```bash
./scripts/checkpoint-status.sh check-commands
```

**Expected output (example at 40% completion):**
```
📈 check-commands: 2/5 steps (40%)
   [████████░░░░░░░░░░░░] 40%
→ Next: ./scripts/checkpoint-update.sh check-commands STEP=3
```

## Minimum Requirements

**MANDATORY for command quality:**
- [ ] All commands must be scanned (including ar/ subdirectory)
- [ ] Structure validation must check all required sections
- [ ] Average score must be 90% or higher
- [ ] Poor-scoring commands must be identified for fixing

**EXCELLENCE**: Commands should follow the new-learnings.md gold standard structure

### What it does

This command performs comprehensive validation of all Claude Code commands:

### Stage 1: Structure Analysis
Validates each command has:
- First-line description
- Single h1 title
- Checkpoint/step tracking system
- Clear checkpoint markers ([CHECKPOINT START], [CHECKPOINT END])
- Bash commands to mark progress
- Critical thinking prompts and deep questions
- Expected outputs for all commands
- Minimum requirements section
- Troubleshooting section

### Stage 2: Score Calculation
Assigns scores based on:
- Checkpoint presence and quality (40%)
- Expected outputs documentation (20%)
- Troubleshooting guidance (20%)
- Requirements clarity (20%)

### Stage 1: Scanning (Step 1)



#### Step 1: Scan Commands

```bash
./scripts/scan-commands.sh
```

### Stage 2: Validation (Steps 2-3)



#### Step 2: Validate Structure

```bash
./scripts/validate-command-structure.sh
```

#### Step 3: Calculate Scores

```bash
./scripts/calculate-command-scores.sh
```

### Stage 3: Analysis (Steps 4-5)



#### Step 4: Identify Issues

```bash
./scripts/identify-command-issues.sh
```

#### Step 5: Generate Report

```bash
./scripts/generate-command-report.sh
```

```bash
./scripts/checkpoint-complete.sh check-commands
```

**Expected completion output:**
```
========================================
   CHECKPOINT COMPLETION SUMMARY
========================================

📈 check-commands: X/Y steps (Z%)
   [████████████████████] 100%

✅ Checkpoint workflow complete
```
rm -f /tmp/check-commands-*.txt
```

### Usage

```bash
make check-commands
```

**Or with checkpoint tracking:**
```bash
# Initialize and run through checkpoints
./scripts/checkpoint-init.sh check-commands '...'
# Follow checkpoint steps above
```

## Why it's important:

Even when commands work correctly, they may lack:
- Clear expected outputs for users to verify success
- Troubleshooting guidance when things go wrong
- Checkpoint tracking for complex multi-step processes
- Minimum requirements documentation
- Critical thinking prompts to guide decision-making

## Recommended workflow:

1. Run `make check-commands` regularly
2. Review commands scoring below 90%
3. Use `--fix` flag to get improvement suggestions
4. Update commands following the new-learnings.md pattern

## Command validation output:

The checker provides detailed scoring:
- Individual command scores with emoji indicators
- Average score across all commands
- Distribution breakdown by quality tier
- Specific issues identified for each command

To see detailed issues: `python3 scripts/check_commands.py --verbose`
To generate fix suggestions: `python3 scripts/check_commands.py --fix`

This check ensures all Claude Code commands provide excellent user experience with comprehensive documentation, clear tracking, and helpful troubleshooting guidance.

## Troubleshooting

### If check-commands fails:
```bash
# View detailed validation output
python3 scripts/check_commands.py --verbose

# Check specific command structure
grep -A5 "command-name.md" /tmp/check-commands-output.txt

# Generate improvement suggestions
python3 scripts/check_commands.py --fix > improvements.md
```

### Common issues:
1. **Missing checkpoints**: Add checkpoint tracking section
2. **No expected outputs**: Document what users should see
3. **Missing troubleshooting**: Add common problems and solutions
4. **Unclear requirements**: Add minimum requirements section

### Quick fixes:
```bash
# For low-scoring commands, use template from high scorer:
cp .opencode/command/ar/new-learnings.md template.md
# Edit template.md and adapt for your command

# Validate improvements:
make check-commands
```

## Related Documentation

### Command Structure Patterns
- [Command Documentation Excellence Gate](../../../kb/command-documentation-excellence-gate.md)
- [Command Thoroughness Requirements Pattern](../../../kb/command-thoroughness-requirements-pattern.md)
- [Multi-Step Checkpoint Tracking Pattern](../../../kb/multi-step-checkpoint-tracking-pattern.md)

### Quality Standards