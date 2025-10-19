Create a new Claude Code command following all quality standards and best practices.

**Usage**: `/create-command <command-name> <purpose>`

**Arguments**:
- `<command-name>`: The name of the command (e.g., "analyze-deps", "check-config")
- `<purpose>`: Brief description of the command's function/purpose/outcome

**Example**: `/create-command analyze-deps "Analyze module dependencies and detect circular references"`

## MANDATORY KB Consultation

**CRITICAL**: Before creating any command, read these KB articles IN FULL to understand command structure and quality standards:

1. **Command Structure & Standards** (READ THESE FIRST):
   ```bash
   # Core command design patterns
   grep "command-helper-script-extraction-pattern\|checkpoint-implementation-guide\|command-documentation-excellence-gate" kb/README.md
   ```
   - [Command Helper Script Extraction Pattern](../../../kb/command-helper-script-extraction-pattern.md) ⭐ **CRITICAL** - Extract embedded bash to scripts
   - [Checkpoint Implementation Guide](../../../kb/checkpoint-implementation-guide.md) - How to structure with checkpoints
   - [Command Documentation Excellence Gate](../../../kb/command-documentation-excellence-gate.md) - Must achieve 90%+ score

2. **Command Quality Requirements**:
   - [Command Thoroughness Requirements Pattern](../../../kb/command-thoroughness-requirements-pattern.md) - Minimum requirements
   - [Command Output Documentation Pattern](../../../kb/command-output-documentation-pattern.md) - Document expected outputs
   - [Unmissable Documentation Pattern](../../../kb/unmissable-documentation-pattern.md) - Mandatory checkpoint tracking

3. **Supporting Patterns**:
   - [Lesson-Based Command Design Pattern](../../../kb/lesson-based-command-design-pattern.md) - Unified verification approach
   - [Multi-Step Checkpoint Tracking Pattern](../../../kb/multi-step-checkpoint-tracking-pattern.md) - Checkpoint mechanics
   - [Gate Enforcement Exit Codes Pattern](../../../kb/gate-enforcement-exit-codes-pattern.md) - Quality gates

## CHECKPOINT WORKFLOW ENFORCEMENT

**CRITICAL**: This command MUST use checkpoint tracking for ALL execution.

### In-Progress Workflow Detection

If a `/create-command` workflow is already in progress:

```bash
./scripts/checkpoint-status.sh create-command VERBOSE=--verbose
# Resume: ./scripts/checkpoint-update.sh create-command STEP=N
# Or reset: ./scripts/checkpoint-cleanup.sh create-command && ./scripts/checkpoint-init.sh create-command "Validate Args" "KB Consultation" "Create Structure" "Add Checkpoints" "Add Quality Gates" "Add Documentation" "Verify Excellence"
```

### First-Time Initialization Check

```bash
./scripts/checkpoint-init.sh create-command "Validate Args" "KB Consultation" "Create Structure" "Add Checkpoints" "Add Quality Gates" "Add Documentation" "Verify Excellence"
```

## PRECONDITION: Checkpoint Tracking Must Be Initialized

```bash
./scripts/checkpoint-require.sh create-command
```

# Create Command

## Checkpoint Tracking

This command creates a new command file with proper structure, checkpoint tracking, quality gates, and documentation following all excellence standards.

### Initialize Tracking
```bash
# Start the command creation process
./scripts/checkpoint-init.sh create-command "Validate Args" "KB Consultation" "Create Structure" "Add Checkpoints" "Add Quality Gates" "Add Documentation" "Verify Excellence"
```

**Expected output:**
```
📍 Starting: create-command (7 steps)
📁 Tracking: /tmp/create-command-progress.txt
→ Run: ./scripts/checkpoint-update.sh create-command STEP=1
```

### Check Progress
```bash
./scripts/checkpoint-status.sh create-command
```

**Expected output (example at 43% completion):**
```
📈 create-command: 3/7 steps (43%)
   [████████░░░░░░░░░░░░] 43%
→ Next: ./scripts/checkpoint-update.sh create-command STEP=4
```

## Minimum Requirements

**MANDATORY for successful completion:**
- [ ] Command name provided and valid
- [ ] Purpose description provided
- [ ] All relevant KB articles consulted
- [ ] Command file created with proper structure
- [ ] Checkpoint tracking implemented (minimum 3 steps)
- [ ] Quality gates added between stages
- [ ] Expected outputs documented
- [ ] Minimum requirements section included
- [ ] Troubleshooting section added
- [ ] Command passes `make check-commands` validation (90%+ score)

## Execution Workflow

### Stage 1: Validation and Consultation (Steps 1-2)

#### [CHECKPOINT START - STAGE 1]

#### Step 1: Validate Arguments

**⚠️ MANDATORY: Both arguments required**

```bash
# Parse arguments from $ARGUMENTS
COMMAND_NAME=$(echo "$ARGUMENTS" | awk '{print $1}')
COMMAND_PURPOSE=$(echo "$ARGUMENTS" | sed 's/^[^ ]* //')

# Validate using helper script
./scripts/validate-command-args.sh "$COMMAND_NAME" "$COMMAND_PURPOSE"

# Set COMMAND_FILE for subsequent steps
COMMAND_FILE=".opencode/command/ar/${COMMAND_NAME}.md"
echo "   Target file: $COMMAND_FILE"
echo ""

./scripts/checkpoint-update.sh create-command STEP=1
```

#### Step 2: KB Consultation

**⚠️ MANDATORY: Read all command design articles before proceeding**

This is NOT optional - read these articles IN FULL:

1. **Read command structure patterns**:
   ```bash
   cat kb/command-helper-script-extraction-pattern.md
   cat kb/checkpoint-implementation-guide.md
   cat kb/command-documentation-excellence-gate.md
   ```

2. **Read quality requirements**:
   ```bash
   cat kb/command-thoroughness-requirements-pattern.md
   cat kb/command-output-documentation-pattern.md
   cat kb/unmissable-documentation-pattern.md
   ```

3. **Read supporting patterns**:
   ```bash
   cat kb/multi-step-checkpoint-tracking-pattern.md
   cat kb/gate-enforcement-exit-codes-pattern.md
   ```

**Key learnings to apply**:
- Extract complex bash logic to `scripts/` directory (10+ lines)
- Structure commands with 3-6 steps per stage
- Add quality gates between stages
- Document expected outputs for all operations
- Define minimum requirements per stage
- Must achieve 90%+ score in `make check-commands`

```bash
./scripts/checkpoint-update.sh create-command STEP=2
```

#### [QUALITY GATE 1: Preparation Complete]
```bash
# MANDATORY: Must pass before proceeding
./scripts/checkpoint-gate.sh create-command "Preparation" "1,2"
```

**Expected gate output:**
```
✅ GATE 'Preparation' - PASSED
   Verified: Steps 1,2
```

**Minimum Requirements for Stage 1:**
- [ ] Command name and purpose validated
- [ ] All KB articles read and understood
- [ ] Ready to create command structure

#### [CHECKPOINT END]

### Stage 2: Command Structure Creation (Steps 3-5)

#### [CHECKPOINT START - STAGE 2]

#### Step 3: Create Command File Structure

```bash
# Create command file with basic structure
cat > "$COMMAND_FILE" << 'EOF'
$COMMAND_PURPOSE

**Usage**: \`/COMMAND_NAME [arguments]\`

## MANDATORY KB Consultation

Before executing:
1. Search relevant KB articles: \`grep "relevant-pattern" kb/README.md\`
2. Read articles that apply to this command's domain
3. Apply patterns and best practices

## CHECKPOINT WORKFLOW ENFORCEMENT

**CRITICAL**: This command MUST use checkpoint tracking for ALL execution.

### In-Progress Workflow Detection

If a \`/COMMAND_NAME\` workflow is already in progress:

\`\`\`bash
./scripts/checkpoint-status.sh COMMAND_NAME VERBOSE=--verbose
# Resume: ./scripts/checkpoint-update.sh COMMAND_NAME STEP=N
# Or reset: ./scripts/checkpoint-cleanup.sh COMMAND_NAME && ./scripts/checkpoint-init.sh COMMAND_NAME "Step 1" "Step 2" "Step 3"
\`\`\`

### First-Time Initialization Check

\`\`\`bash
if [ ! -f /tmp/COMMAND_NAME-progress.txt ]; then
  echo "⚠️  Initializing checkpoint tracking..."
  ./scripts/checkpoint-init.sh COMMAND_NAME "Step 1" "Step 2" "Step 3"
else
  ./scripts/checkpoint-status.sh COMMAND_NAME
fi
\`\`\`

## PRECONDITION: Checkpoint Tracking Must Be Initialized

\`\`\`bash
if [ ! -f /tmp/COMMAND_NAME-progress.txt ]; then
  echo "❌ ERROR: Checkpoint tracking not initialized!"
  exit 1
fi
\`\`\`

# COMMAND_NAME

## Checkpoint Tracking

This command [brief description of what it does].

### Initialize Tracking
\`\`\`bash
# Start the COMMAND_NAME process
./scripts/checkpoint-init.sh COMMAND_NAME "Step 1" "Step 2" "Step 3"
\`\`\`

**Expected output:**
\`\`\`
📍 Starting: COMMAND_NAME (3 steps)
📁 Tracking: /tmp/COMMAND_NAME-progress.txt
→ Run: ./scripts/checkpoint-update.sh COMMAND_NAME STEP=1
\`\`\`

### Check Progress
\`\`\`bash
./scripts/checkpoint-status.sh COMMAND_NAME
\`\`\`

**Expected output (example at 33% completion):**
\`\`\`
📈 COMMAND_NAME: 1/3 steps (33%)
   [████████░░░░░░░░░░░░] 33%
→ Next: ./scripts/checkpoint-update.sh COMMAND_NAME STEP=2
\`\`\`

## Minimum Requirements

**MANDATORY for successful completion:**
- [ ] Command executes without errors
- [ ] Expected output is produced
- [ ] All quality checks pass

## Execution Workflow

### Stage 1: [Stage Name] (Steps 1-N)

#### [CHECKPOINT START - STAGE 1]

#### Step 1: [First Step Name]

[Description of what this step does]

\`\`\`bash
# Implement step logic here
./scripts/checkpoint-update.sh COMMAND_NAME STEP=1
\`\`\`

#### [CHECKPOINT END]

## Related Commands

- [Related command 1]
- [Related command 2]

## Related KB Articles

- [Related KB article 1]
- [Related KB article 2]

\$ARGUMENTS
EOF

# Replace placeholders
sed -i '' "s/COMMAND_NAME/${COMMAND_NAME}/g" "$COMMAND_FILE"
sed -i '' "s/\$COMMAND_PURPOSE/${COMMAND_PURPOSE}/" "$COMMAND_FILE"

echo "✅ Created command file: $COMMAND_FILE"
echo "   Structure: Basic template with checkpoint tracking"
echo ""

./scripts/checkpoint-update.sh create-command STEP=3
```

#### Step 4: Add Checkpoint Steps

**⚠️ IMPORTANT: Define logical workflow steps**

```bash
# Determine number of steps based on command complexity
echo "📋 Define workflow steps for: $COMMAND_NAME"
echo ""
echo "How many logical steps should this command have?"
echo "Guidelines:"
echo "  • Simple commands: 3-5 steps"
echo "  • Medium commands: 6-12 steps"
echo "  • Complex commands: 13+ steps"
echo ""
echo "Current template has 3 placeholder steps."
echo "Edit $COMMAND_FILE to:"
echo "  1. Define actual workflow steps"
echo "  2. Group into logical stages (3-6 steps per stage)"
echo "  3. Add checkpoint-update calls after each step"
echo ""

./scripts/checkpoint-update.sh create-command STEP=4
```

#### Step 5: Add Quality Gates

**⚠️ CRITICAL: Gates enforce quality between stages**

```bash
echo "🚪 Add quality gates to: $COMMAND_NAME"
echo ""
echo "Gates should:"
echo "  • Separate logical stages"
echo "  • Verify completion before proceeding"
echo "  • Enforce minimum requirements"
echo ""
echo "Template pattern:"
echo '  ```bash'
echo '  ./scripts/checkpoint-gate.sh COMMAND_NAME "Stage Name" "1,2,3"
echo '  ```'
echo ""
echo "Add gates to $COMMAND_FILE between stages"
echo ""

./scripts/checkpoint-update.sh create-command STEP=5
```

#### [QUALITY GATE 2: Structure Complete]
```bash
# MANDATORY: Must pass before proceeding
./scripts/checkpoint-gate.sh create-command "Structure" "3,4,5"
```

**Expected gate output:**
```
✅ GATE 'Structure' - PASSED
   Verified: Steps 3,4,5
```

**Minimum Requirements for Stage 2:**
- [ ] Command file created with proper structure
- [ ] Checkpoint tracking infrastructure added
- [ ] Quality gates planned

#### [CHECKPOINT END]

### Stage 3: Documentation and Verification (Steps 6-7)

#### [CHECKPOINT START - STAGE 3]

#### Step 6: Add Documentation Sections

**⚠️ MANDATORY: Complete documentation required**

```bash
echo "📝 Add documentation to: $COMMAND_NAME"
echo ""
echo "Required sections:"
echo "  ✓ KB Consultation (already added)"
echo "  ✓ Checkpoint Tracking (already added)"
echo "  ✓ Minimum Requirements (already added)"
echo "  □ Expected outputs for all operations"
echo "  □ Troubleshooting section"
echo "  □ Related commands"
echo "  □ Related KB articles"
echo ""
echo "Edit $COMMAND_FILE to complete documentation"
echo ""

./scripts/checkpoint-update.sh create-command STEP=6
```

#### Step 7: Verify Excellence Standards

**⚠️ CRITICAL: Must achieve 90%+ score**

```bash
# Verify command quality using helper script
./scripts/verify-command-quality.sh "$COMMAND_NAME"

./scripts/checkpoint-update.sh create-command STEP=7
```

#### [QUALITY GATE 3: Excellence Achieved]
```bash
# MANDATORY: Must pass before completion
./scripts/checkpoint-gate.sh create-command "Excellence" "6,7"
```

**Expected gate output:**
```
✅ GATE 'Excellence' - PASSED
   Verified: Steps 6,7
```

**Minimum Requirements for Stage 3:**
- [ ] All documentation sections complete
- [ ] Expected outputs documented
- [ ] Troubleshooting section added
- [ ] Command achieves 90%+ validation score

#### [CHECKPOINT END]

#### [CHECKPOINT COMPLETE]
```bash
./scripts/checkpoint-complete.sh create-command
```

**Expected completion output:**
```
========================================
   CHECKPOINT COMPLETION SUMMARY
========================================

📈 create-command: X/Y steps (Z%)
   [████████████████████] 100%

✅ Checkpoint workflow complete
```
```

```bash
# Clean up tracking file
./scripts/checkpoint-cleanup.sh create-command
```

## Best Practice: Helper Scripts

**IMPORTANT**: If your command requires complex verification logic (10+ lines of bash):

1. **Extract to helper script** in `scripts/` directory:
   ```bash
   scripts/verify-COMMAND_NAME-quality.sh
   ```

2. **Reference in command file**:
   ```markdown
   **Helper script available:**
   \`\`\`bash
   ./scripts/verify-COMMAND_NAME-quality.sh [args]
   # Returns: Exit 1 if issues found, 0 if clean
   \`\`\`
   ```

3. **See pattern**: [Command Helper Script Extraction Pattern](../../../kb/command-helper-script-extraction-pattern.md)

## Post-Creation Checklist

After creating the command:

- [ ] Command file exists: `.opencode/command/ar/$COMMAND_NAME.md`
- [ ] Run validation: `make check-commands`
- [ ] Verify 90%+ score achieved
- [ ] Test command: `/ar:$COMMAND_NAME`
- [ ] Extract any complex bash to `scripts/`
- [ ] Update related commands if needed
- [ ] Add to README or command index if applicable
- [ ] Commit: `git add .opencode/command/ar/$COMMAND_NAME.md`

## Troubleshooting

### Issue: "Command name is required"
**Solution**: Provide both command name and purpose:
```bash
/create-command analyze-deps "Analyze module dependencies"
```

### Issue: "Invalid command name format"
**Solution**: Use lowercase with hyphens only:
- ✅ Valid: `analyze-deps`, `check-config`, `run-validation`
- ❌ Invalid: `analyzeDeps`, `check_config`, `AnalyzeStuff`  // EXAMPLE: Future type

### Issue: "Command already exists"
**Solution**: Either:
- Choose a different name
- Delete existing command: `rm .opencode/command/ar/existing-name.md`
- Edit existing command instead

### Issue: "Validation score below 90%"
**Solution**: Add missing sections:
- Expected output examples for all operations
- Troubleshooting section with common issues
- Minimum requirements with checkboxes
- Complete checkpoint tracking with gates

## Output Format

The command creates a structured file with:
- **Header**: Purpose and usage
- **KB Consultation**: Required reading
- **Checkpoint Infrastructure**: Initialization, status, cleanup
- **Minimum Requirements**: Quality criteria
- **Execution Workflow**: Staged steps with gates
- **Documentation**: Related commands and KB articles
- **Troubleshooting**: Common issues and solutions

## Related Commands

- `/check-commands` - Validate command structure and quality
- `/new-learnings` - Extract patterns to create new KB articles

## Related KB Articles

### Command Creation
- [Command Helper Script Extraction Pattern](../../../kb/command-helper-script-extraction-pattern.md) ⭐ Extract embedded bash to scripts
- [Checkpoint Implementation Guide](../../../kb/checkpoint-implementation-guide.md) - Structuring with checkpoints
- [Command Documentation Excellence Gate](../../../kb/command-documentation-excellence-gate.md) - 90%+ requirement

### Quality Standards
- [Command Thoroughness Requirements Pattern](../../../kb/command-thoroughness-requirements-pattern.md) - Minimum requirements
- [Command Output Documentation Pattern](../../../kb/command-output-documentation-pattern.md) - Expected outputs
- [Unmissable Documentation Pattern](../../../kb/unmissable-documentation-pattern.md) - Checkpoint tracking

### Supporting Patterns
- [Multi-Step Checkpoint Tracking Pattern](../../../kb/multi-step-checkpoint-tracking-pattern.md) - Checkpoint mechanics
- [Gate Enforcement Exit Codes Pattern](../../../kb/gate-enforcement-exit-codes-pattern.md) - Quality gates
- [Lesson-Based Command Design Pattern](../../../kb/lesson-based-command-design-pattern.md) - Unified verification

$ARGUMENTS