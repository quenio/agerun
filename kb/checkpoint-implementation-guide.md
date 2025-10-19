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
   ./scripts/checkpoint-update.sh review-changes 1
   \`\`\`

2. **Code Smells Detection**: Scans for issues
   \`\`\`bash
   ./scripts/checkpoint-update.sh review-changes 2
   \`\`\`

**[QUALITY GATE 1: Code Quality Complete]**
\`\`\`bash
# MANDATORY: Must pass before proceeding
./scripts/checkpoint-gate.sh review-changes "Code Quality" "1,2,3,4,5,6"
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
All checkpoint functionality is implemented through standalone bash scripts that can be called directly:

```bash
# Initialize with all steps (creates /tmp/COMMAND-progress.txt)
./scripts/checkpoint-init.sh command-name "Step 1" "Step 2" ...

# Update after each step (marks steps complete, shows progress)
./scripts/checkpoint-update.sh command-name N

# Status display (shows progress bar and next action)
./scripts/checkpoint-status.sh command-name

# Precondition check (verifies checkpoint is initialized)
./scripts/checkpoint-require.sh command-name

# Gates between stages (verifies required steps before proceeding)
./scripts/checkpoint-gate.sh command-name "Stage Name" "1,2,3"

# Cleanup when done (removes tracking file)
./scripts/checkpoint-cleanup.sh command-name
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

## Direct Script Usage for Simple Integration

The checkpoint scripts are designed to be called directly from commands with minimal boilerplate:

```bash
# Initialize checkpoint with all steps (idempotent)
./scripts/checkpoint-init.sh command-name "Step 1" "Step 2" "Step 3"

# Verify checkpoint is initialized before proceeding
./scripts/checkpoint-require.sh command-name || exit 1

# Update after completing each step
./scripts/checkpoint-update.sh command-name 1

# Display current progress status
./scripts/checkpoint-status.sh command-name

# Verify all required steps are complete before proceeding
./scripts/checkpoint-gate.sh command-name "Gate Name" "1,2,3" || exit 1

# Cleanup tracking when complete
./scripts/checkpoint-complete.sh command-name
```

**Key features**:
- `checkpoint-init.sh` is idempotent - can be called multiple times safely
- Direct script calls eliminate indirection through wrappers or Makefile targets
- Each script handles its own error checking and user feedback
- Consistent argument order across all scripts for predictable behavior

## Implementation Checklist

When adding checkpoints to a command:
- [ ] Count total sections/steps in command
- [ ] Group into logical stages (3-6 steps each)
- [ ] **Extract embedded bash logic** to helper scripts (see [Command Helper Script Extraction Pattern](command-helper-script-extraction-pattern.md))
- [ ] Call checkpoint-init.sh at the beginning to initialize tracking
- [ ] Add checkpoint-update.sh calls after each step
- [ ] Add checkpoint-gate.sh calls at stage boundaries
- [ ] Add checkpoint-require.sh to verify initialization
- [ ] Add checkpoint-complete.sh for cleanup
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
- [Command File as Orchestrator Pattern](command-orchestrator-pattern.md) - Show orchestration logic inline in command file, not wrapper scripts
- [Checkpoint Step Consolidation Pattern](checkpoint-step-consolidation-pattern.md) - Consolidate per-item verification steps into interactive loops
- [Command Helper Script Extraction Pattern](command-helper-script-extraction-pattern.md) - Extract embedded bash to standalone scripts (one per block)
- [Multi-Step Checkpoint Tracking Pattern](multi-step-checkpoint-tracking-pattern.md)
- [Gate Enforcement Exit Codes Pattern](gate-enforcement-exit-codes-pattern.md)
- [Command Output Documentation Pattern](command-output-documentation-pattern.md)
- [Command Thoroughness Requirements Pattern](command-thoroughness-requirements-pattern.md)
- [Progress Visualization ASCII Pattern](progress-visualization-ascii-pattern.md)