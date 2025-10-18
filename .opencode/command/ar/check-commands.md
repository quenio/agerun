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
   - Scripts can be extracted for better reusability ([details](../../../kb/command-helper-script-extraction-pattern.md))
   - Use domain-specific naming for extracted scripts ([details](../../../kb/script-domain-naming-convention.md))

## CHECKPOINT WORKFLOW ENFORCEMENT

**CRITICAL**: This command MUST use checkpoint tracking for ALL execution.

### In-Progress Workflow Detection

If a `/check-commands` workflow is already in progress:

```bash
./scripts/status-checkpoint.sh check-commands VERBOSE=--verbose
# Resume: ./scripts/update-checkpoint.sh check-commands STEP=N
# Or reset: ./scripts/cleanup-checkpoint.sh check-commands && ./scripts/init-checkpoint.sh check-commands STEPS='"Scan Commands" "Validate Structure" "Calculate Scores" "Identify Issues" "Generate Report"'
```

### First-Time Initialization Check

```bash
./scripts/init-checkpoint.sh check-commands '"Scan Commands" "Validate Structure" "Calculate Scores" "Identify Issues" "Generate Report"'
```

## PRECONDITION: Checkpoint Tracking Must Be Initialized

```bash
./scripts/require-checkpoint.sh check-commands
```

# Check Commands
## Checkpoint Tracking

This command validates the structure and quality of all Claude Code commands. The process has 5 checkpoints to ensure systematic validation and improvement using iterative feedback loops ([details](../../../kb/validation-feedback-loop-effectiveness.md)).

### Initialize Tracking
```bash
# Start the command validation process
./scripts/init-checkpoint.sh check-commands STEPS='"Scan Commands" "Validate Structure" "Calculate Scores" "Identify Issues" "Generate Report"'
```

**Expected output:**
```
📍 Starting: check-commands (5 steps)
📁 Tracking: /tmp/check-commands-progress.txt
→ Run: ./scripts/update-checkpoint.sh check-commands STEP=1
```

### Check Progress
```bash
./scripts/status-checkpoint.sh check-commands
```

**Expected output (example at 40% completion):**
```
📈 check-commands: 2/5 steps (40%)
   [████████░░░░░░░░░░░░] 40%
→ Next: ./scripts/update-checkpoint.sh check-commands STEP=3
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

#### [CHECKPOINT START - STAGE 1]

#### [CHECKPOINT END]

#### Step 1: Scan Commands

```bash
./scripts/scan-commands.sh
```

### Stage 2: Validation (Steps 2-3)

#### [CHECKPOINT START - STAGE 2]

#### [CHECKPOINT END]

#### Step 2: Validate Structure

```bash
./scripts/validate-command-structure.sh
```

#### Step 3: Calculate Scores

```bash
./scripts/calculate-command-scores.sh
```

### Stage 3: Analysis (Steps 4-5)

#### [CHECKPOINT START - STAGE 3]

#### [CHECKPOINT END]

#### Step 4: Identify Issues

```bash
./scripts/identify-command-issues.sh
```

#### Step 5: Generate Report

```bash
./scripts/generate-command-report.sh
```

#### [CHECKPOINT COMPLETE]
```bash
./scripts/complete-checkpoint.sh check-commands
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
./scripts/init-checkpoint.sh check-commands STEPS='...'
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