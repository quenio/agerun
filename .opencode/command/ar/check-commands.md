Validate that all Claude Code commands follow comprehensive structure standards.

**Excellence Gate**: Commands must achieve 90%+ average score for quality documentation. The checker validates checkpoint tracking, expected outputs, troubleshooting sections, and minimum requirements ([details](../../../kb/command-documentation-excellence-gate.md)).

## MANDATORY KB Consultation

Before validation:
1. Search: `grep "command\|excellence\|validation" kb/README.md`
2. Must read:
   - command-documentation-excellence-gate
   - validation-feedback-loop-effectiveness
   - command-output-documentation-pattern
3. Apply command quality standards

## CHECKPOINT WORKFLOW ENFORCEMENT

**CRITICAL**: This command MUST use checkpoint tracking for ALL execution.

### In-Progress Workflow Detection

If a `/check-commands` workflow is already in progress:

```bash
make checkpoint-status CMD=check-commands VERBOSE=--verbose
# Resume: make checkpoint-update CMD=check-commands STEP=N
# Or reset: make checkpoint-cleanup CMD=check-commands && make checkpoint-init CMD=check-commands STEPS='"Scan Commands" "Validate Structure" "Calculate Scores" "Identify Issues" "Generate Report"'
```

### First-Time Initialization Check

```bash
if [ ! -f /tmp/check_commands_progress.txt ]; then
  echo "⚠️  Initializing checkpoint tracking..."
  make checkpoint-init CMD=check-commands STEPS='"Scan Commands" "Validate Structure" "Calculate Scores" "Identify Issues" "Generate Report"'
else
  make checkpoint-status CMD=check-commands
fi
```

## PRECONDITION: Checkpoint Tracking Must Be Initialized

```bash
if [ ! -f /tmp/check_commands_progress.txt ]; then
  echo "❌ ERROR: Checkpoint tracking not initialized!"
  exit 1
fi
```

# Check Commands
## Checkpoint Tracking

This command validates the structure and quality of all Claude Code commands. The process has 5 checkpoints to ensure systematic validation and improvement using iterative feedback loops ([details](../../../kb/validation-feedback-loop-effectiveness.md)).

### Initialize Tracking
```bash
# Start the command validation process
make checkpoint-init CMD=check-commands STEPS='"Scan Commands" "Validate Structure" "Calculate Scores" "Identify Issues" "Generate Report"'
```

**Expected output:**
```
📍 Starting: check-commands (5 steps)
📁 Tracking: /tmp/check-commands_progress.txt
→ Run: make checkpoint-update CMD=check-commands STEP=1
```

### Check Progress
```bash
make checkpoint-status CMD=check-commands
```

**Expected output (example at 40% completion):**
```
📈 check-commands: 2/5 steps (40%)
   [████████░░░░░░░░░░░░] 40%
→ Next: make checkpoint-update CMD=check-commands STEP=3
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
# Scan for all command files
echo "Scanning for command files..."
COMMAND_COUNT=$(find .claude/commands -name "*.md" -type f | wc -l)
echo "Found $COMMAND_COUNT command files"

if [ $COMMAND_COUNT -eq 0 ]; then
  echo "❌ No command files found!"
  exit 1
fi

echo "✅ Successfully found $COMMAND_COUNT commands"
make checkpoint-update CMD=check-commands STEP=1
```

### Stage 2: Validation (Steps 2-3)

#### [CHECKPOINT START - STAGE 2]

#### [CHECKPOINT END]

#### Step 2: Validate Structure

```bash
# Run structure validation
echo "Validating command structures..."

if python3 scripts/check_commands.py --verbose 2>&1 | tee /tmp/check-commands-output.txt; then
  echo "✅ Structure validation completed"
else
  echo "⚠️ Structure validation found issues"
fi

make checkpoint-update CMD=check-commands STEP=2
```

#### Step 3: Calculate Scores

```bash
# Extract and analyze scores
echo "Calculating command quality scores..."

# Extract average score from output
AVG_SCORE=$(grep "Average Score:" /tmp/check-commands-output.txt | awk '{print $3}' | tr -d '%')

if [ -z "$AVG_SCORE" ]; then
  echo "❌ Could not calculate average score"
  exit 1
fi

echo "Average Score: $AVG_SCORE%"

# Check if meets excellence threshold
if (( $(echo "$AVG_SCORE >= 90" | bc -l) )); then
  echo "✅ Excellent! Commands meet quality threshold"
  STATUS="EXCELLENT"
else
  echo "⚠️ Commands need improvement (target: 90%+)"
  STATUS="NEEDS_WORK"
fi

echo "AVG_SCORE=$AVG_SCORE" > /tmp/check-commands-stats.txt
echo "STATUS=$STATUS" >> /tmp/check-commands-stats.txt

make checkpoint-update CMD=check-commands STEP=3
```

### Stage 3: Analysis (Steps 4-5)

#### [CHECKPOINT START - STAGE 3]

#### [CHECKPOINT END]

#### Step 4: Identify Issues

```bash
# Identify commands needing improvement
source /tmp/check-commands-stats.txt

echo "Identifying commands that need work..."

# Count distribution
EXCELLENT=$(grep -c "🌟 Excellent" /tmp/check-commands-output.txt || echo "0")
GOOD=$(grep -c "✅ Good" /tmp/check-commands-output.txt || echo "0")
NEEDS_WORK=$(grep -c "⚠️ Needs Work" /tmp/check-commands-output.txt || echo "0")
POOR=$(grep -c "❌ Poor" /tmp/check-commands-output.txt || echo "0")

echo "Distribution:"
echo "  🌟 Excellent (90-100%): $EXCELLENT commands"
echo "  ✅ Good (70-89%): $GOOD commands"
echo "  ⚠️ Needs Work (50-69%): $NEEDS_WORK commands"
echo "  ❌ Poor (0-49%): $POOR commands"

if [ "$NEEDS_WORK" -gt 0 ] || [ "$POOR" -gt 0 ]; then
  echo ""
  echo "Commands needing improvement:"
  grep -E "⚠️|❌" /tmp/check-commands-output.txt | head -10
fi

make checkpoint-update CMD=check-commands STEP=4
```

#### Step 5: Generate Report

```bash
# Generate final report
source /tmp/check-commands-stats.txt

echo ""
echo "========================================="
echo "   COMMAND QUALITY REPORT"
echo "========================================="
echo ""
echo "Overall Score: $AVG_SCORE%"
echo "Status: $STATUS"
echo ""

if [ "$STATUS" = "EXCELLENT" ]; then
  echo "✅ All commands meet quality standards!"
  echo "Documentation is comprehensive and well-structured."
else
  echo "⚠️ Some commands need improvement"
  echo "Run with --fix flag to generate improvement suggestions:"
  echo "  python3 scripts/check_commands.py --fix"
fi

make checkpoint-update CMD=check-commands STEP=5
```

#### [CHECKPOINT COMPLETE]
```bash
# Show final summary
make checkpoint-status CMD=check-commands
```

**Expected completion output:**
```
========================================
   CHECKPOINT STATUS: check-commands
========================================

Progress: 5/5 steps (100%)

[████████████████████████████████] 100%

✅ ALL CHECKPOINTS COMPLETE!

Final Status:
  Commands scanned: 26
  Average score: 94.6%
  Quality: EXCELLENT

All commands meet documentation standards!
```

```bash
# Clean up tracking
make checkpoint-cleanup CMD=check-commands
rm -f /tmp/check-commands-*.txt
```

### Usage

```bash
make check-commands
```

**Or with checkpoint tracking:**
```bash
# Initialize and run through checkpoints
make checkpoint-init CMD=check-commands STEPS='...'
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
cp .claude/commands/ar/new-learnings.md template.md
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