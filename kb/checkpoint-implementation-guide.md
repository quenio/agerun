# Checkpoint Implementation Guide

## Learning
When implementing checkpoint tracking in existing commands, structure the command into logical stages with numbered steps, add gates between stages, document expected outputs, and define minimum requirements for thoroughness.

## Importance
Converting monolithic commands to checkpoint-tracked processes ensures systematic execution, prevents skipping critical steps, enables progress resumption, and enforces quality standards through gates.

## Example
```markdown
## Review Process:

### Stage 1: Code Quality Review (Steps 1-6)

**[CHECKPOINT START - STAGE 1]**

1. **Diff Analysis**: Reviews git diff for all changes
   \`\`\`bash
   # After completing diff analysis
   make checkpoint-update CMD=review-changes STEP=1
   \`\`\`

2. **Code Smells Detection**: Scans for issues
   \`\`\`bash
   make checkpoint-update CMD=review-changes STEP=2
   \`\`\`

**[QUALITY GATE 1: Code Quality Complete]**
\`\`\`bash
# MANDATORY: Must pass before proceeding
make checkpoint-gate CMD=review-changes GATE="Code Quality" REQUIRED="1,2,3,4,5,6"
\`\`\`

**Expected gate output:**
\`\`\`
========================================
   GATE: Code Quality
========================================

✅ GATE PASSED: All code quality checks complete!
\`\`\`

**Minimum Requirements for Stage 1:**
- [ ] Check at least 3 types of code smells
- [ ] Verify ownership prefixes in all changed files
- [ ] Document all issues found with file:line references
```

## Generalization

### Step 1: Analyze Command Structure
1. **Count existing sections**: Identify logical groupings
2. **Define stages**: Group related sections (3-6 steps per stage)
3. **Identify critical points**: Where gates should enforce completion
4. **Map to checkpoints**: Each section becomes a numbered step

### Step 2: Add Infrastructure

**Checkpoint Script Architecture:**
All checkpoint functionality is implemented through bash scripts that are wrapped by Makefile targets:

```bash
# Initialize with all steps
# Uses: scripts/checkpoint-init.sh (creates /tmp/COMMAND-progress.txt)
make checkpoint-init CMD=command-name STEPS='"Step 1" "Step 2" ...'

# Update after each step
# Uses: scripts/checkpoint-update.sh (marks steps complete, shows progress)
make checkpoint-update CMD=command-name STEP=N

# Status display
# Uses: scripts/checkpoint-status.sh (shows progress bar and next action)
make checkpoint-status CMD=command-name

# Gates between stages
# Uses: scripts/checkpoint-gate.sh (verifies required steps before proceeding)
make checkpoint-gate CMD=command-name GATE="Stage Name" REQUIRED="1,2,3"

# Cleanup when done
# Uses: scripts/checkpoint-cleanup.sh (removes tracking file)
make checkpoint-cleanup CMD=command-name
```

**Script Implementation Details:**
- All scripts use cross-platform safe patterns for macOS/Linux compatibility
- Safe sed delimiters (`@` instead of `/`) to avoid escaping issues
- Proper OSTYPE detection with `[[ == ]]` pattern matching
- Comprehensive error handling with `set -e` and `set -o pipefail`
- (See [Cross-Platform Bash Script Patterns](../kb/cross-platform-bash-script-patterns.md) for details)

### Step 3: Document Expected Outputs
For each checkpoint operation, show:
- Success output format
- Failure conditions
- Progress visualization
- Gate enforcement messages

### Step 4: Define Requirements
Each stage needs:
- **Minimum thresholds**: "At least 3 checks"
- **Quality criteria**: "All tests must pass"
- **Verification methods**: "Run make check-docs"
- **Success indicators**: "READY TO COMMIT"

## Using Wrapper Scripts for Simpler Integration

The wrapper scripts (`scripts/init-checkpoint.sh`, `require-checkpoint.sh`, `gate-checkpoint.sh`, `complete-checkpoint.sh`) simplify checkpoint integration in commands by reducing boilerplate:

```bash
# Instead of 7-8 lines of initialization code:
❌ if [ ! -f "/tmp/command-progress.txt" ]; then
    make checkpoint-init CMD=command STEPS="..."
else
    echo "Checkpoint already initialized"
fi

# Use the wrapper script:
✅ ./scripts/init-checkpoint.sh command '"Step 1" "Step 2"'

# Instead of 5-line precondition check:
❌ PROGRESS_FILE="/tmp/command-progress.txt"
if [ ! -f "$PROGRESS_FILE" ]; then
  echo "ERROR: Not initialized"
  exit 1
fi

# Use the wrapper:
✅ ./scripts/require-checkpoint.sh command || exit 1

# Instead of 3-4 line gate verification:
❌ if ! make checkpoint-gate CMD=command GATE="Name" REQUIRED="1,2,3"; then
  echo "ERROR: Gate failed"
  exit 1
fi

# Use the wrapper:
✅ ./scripts/gate-checkpoint.sh command "Gate Name" "1,2,3" || exit 1

# Instead of 4-5 line completion pattern:
❌ make checkpoint-status CMD=command
make checkpoint-cleanup CMD=command

# Use the wrapper:
✅ ./scripts/complete-checkpoint.sh command
```

## Implementation Checklist

When adding checkpoints to a command:
- [ ] Count total sections/steps in command
- [ ] Group into logical stages (3-6 steps each)
- [ ] **Extract embedded bash logic** to helper scripts (see [Command Helper Script Extraction Pattern](command-helper-script-extraction-pattern.md))
- [ ] Use wrapper scripts for initialization, gates, precondition checks, and cleanup
- [ ] Add update calls after each step (these don't have wrappers, use directly)
- [ ] Document expected outputs for all operations
- [ ] Define minimum requirements per stage
- [ ] Add troubleshooting section
- [ ] Test the full workflow
- [ ] Update related KB articles

## Command Size Guidelines

Based on complexity:
- **Simple commands** (1-5 steps): Optional checkpoints
- **Medium commands** (6-12 steps): Recommended, 1-2 gates
- **Complex commands** (13+ steps): Required, 3-4 gates
- **Critical commands**: Always use gates regardless of size

## Best Practice: Extract Complex Logic

When implementing checkpoints, **extract multi-line embedded bash logic to helper scripts**:

```markdown
❌ **WRONG** - 30+ lines of embedded verification logic:
\`\`\`bash
for file in modules/*.c; do
  # ... 30 lines of complex parsing ...
done
make checkpoint-update CMD=review-changes STEP=2
\`\`\`

✅ **CORRECT** - Extract to helper script:
**Helper script available:**
\`\`\`bash
./scripts/detect-code-smells.sh [path]
# Returns: Exit 1 if code smells found, 0 if clean
\`\`\`

\`\`\`bash
make checkpoint-update CMD=review-changes STEP=2
\`\`\`
```

See [Command Helper Script Extraction Pattern](command-helper-script-extraction-pattern.md) for complete guidelines.

## Related Patterns
- [Checkpoint Step Consolidation Pattern](checkpoint-step-consolidation-pattern.md) - Consolidate per-item verification steps into interactive loops
- [Command Helper Script Extraction Pattern](command-helper-script-extraction-pattern.md) - Extract embedded bash to standalone scripts
- [Multi-Step Checkpoint Tracking Pattern](multi-step-checkpoint-tracking-pattern.md)
- [Gate Enforcement Exit Codes Pattern](gate-enforcement-exit-codes-pattern.md)
- [Command Output Documentation Pattern](command-output-documentation-pattern.md)
- [Command Thoroughness Requirements Pattern](command-thoroughness-requirements-pattern.md)
- [Progress Visualization ASCII Pattern](progress-visualization-ascii-pattern.md)