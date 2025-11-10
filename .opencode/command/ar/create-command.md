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
   grep "command-helper-script-extraction-pattern\|session-todo-list-tracking-pattern\|command-documentation-excellence-gate" kb/README.md
   ```
   - [Command Helper Script Extraction Pattern](../../../kb/command-helper-script-extraction-pattern.md) ⭐ **CRITICAL** - Extract embedded bash to scripts
   - [Session Todo List Tracking Pattern](../../../kb/session-todo-list-tracking-pattern.md) - How to track steps across sessions
   - [Command Documentation Excellence Gate](../../../kb/command-documentation-excellence-gate.md) - Must achieve 90%+ score

2. **Command Quality Requirements**:
   - [Command Thoroughness Requirements Pattern](../../../kb/command-thoroughness-requirements-pattern.md) - Minimum requirements
   - [Command Output Documentation Pattern](../../../kb/command-output-documentation-pattern.md) - Document expected outputs
   - [Unmissable Documentation Pattern](../../../kb/unmissable-documentation-pattern.md) - Mandatory progress tracking

3. **Supporting Patterns**:
   - [Architectural Change Verification Pattern](../../../kb/architectural-change-verification-pattern.md) - Verify command files reflect architectural changes
   - [Lesson-Based Command Design Pattern](../../../kb/lesson-based-command-design-pattern.md) - Unified verification approach
   - [Sub-Agent Verification Pattern](../../../kb/sub-agent-verification-pattern.md) - Step verification via sub-agents
   - [Gate Enforcement Exit Codes Pattern](../../../kb/gate-enforcement-exit-codes-pattern.md) - Quality gates

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
Prompt: "Verify Step N: [Step Title] completion for create-command command.

Todo Item: [Description of what the step accomplished]
Command File: .opencode/command/ar/create-command.md
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
- Add todo item: "Step 1: Validate Args" - Status: pending
- Add todo item: "Verify Step 1: Validate Args" - Status: pending
- Add todo item: "Step 2: KB Consultation" - Status: pending
- Add todo item: "Verify Step 2: KB Consultation" - Status: pending
- Add todo item: "Step 3: Create Structure" - Status: pending
- Add todo item: "Verify Step 3: Create Structure" - Status: pending
- Add todo item: "Step 4: Add Session Todo Tracking" - Status: pending
- Add todo item: "Verify Step 4: Add Session Todo Tracking" - Status: pending
- Add todo item: "Step 5: Add Quality Gates" - Status: pending
- Add todo item: "Verify Step 5: Add Quality Gates" - Status: pending
- Add todo item: "Step 6: Add Documentation" - Status: pending
- Add todo item: "Verify Step 6: Add Documentation" - Status: pending
- Add todo item: "Step 7: Verify Excellence" - Status: pending
- Add todo item: "Verify Step 7: Verify Excellence" - Status: pending
- Add todo item: "Verify Complete Workflow: create-command" - Status: pending

**Important**: All todo items are initialized as `pending` and will be updated to `in_progress` when their respective step/verification begins, then to `completed` after verification passes.

# Create Command

This command creates a new command file with proper structure, session todo tracking, step-verifier verification, quality gates, and documentation following all excellence standards.

1. **Read quality requirements**:
   ```bash
   cat kb/command-thoroughness-requirements-pattern.md
   cat kb/command-output-documentation-pattern.md
   cat kb/session-todo-list-tracking-pattern.md
   ```

2. **Read supporting patterns**:
   ```bash
   cat kb/sub-agent-verification-pattern.md
   cat kb/gate-enforcement-exit-codes-pattern.md
   ```

**Key learnings to apply**:
- Extract complex bash logic to `scripts/` directory (10+ lines)
- Structure commands with 3-6 steps per stage
- Add quality gates between stages
- Document expected outputs for all operations
- Define minimum requirements per stage
- Must follow command structure best practices

#### [QUALITY GATE 1: Preparation Complete]
```bash
# MANDATORY: Must pass before proceeding
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

### Stage 2: Command Structure Creation (Steps 3-5)

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

# COMMAND_NAME

## MANDATORY: Session Todo List Tracking

Each step MUST be added to the session todo list before execution begins:
- Use \`todo_write\` to add each step as a todo item with status \`pending\` initially
- Update to \`in_progress\` when step starts
- Update to \`completed\` after step-verifier verification passes

## MANDATORY: Step Verification

All verification is performed by the step-verifier sub-agent.

### Initialize All Todo Items

**CRITICAL**: Before executing ANY steps, add ALL step and verification todo items to the session todo list using \`todo_write\`:

**Step and Verification Todo Items:**
- Add todo item: "Step 1: [Title]" - Status: pending
- Add todo item: "Verify Step 1: [Title]" - Status: pending
- Add todo item: "Step 2: [Title]" - Status: pending
- Add todo item: "Verify Step 2: [Title]" - Status: pending
- etc.

**Important**: All todo items are initialized as \`pending\` and will be updated to \`in_progress\` when their respective step/verification begins, then to \`completed\` after verification passes.

## Minimum Requirements

**MANDATORY for successful completion:**
- [ ] Command executes without errors
- [ ] Expected output is produced
- [ ] All quality checks pass

## Execution Workflow

### Stage 1: [Stage Name] (Steps 1-N)

#### Step 1: [First Step Name]

[Description of what this step does]

\`\`\`bash
# Implement step logic here
\`\`\`

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
echo "   Structure: Basic template with progress tracking"
echo ""

```

#### Step 4: Add Session Todo Tracking

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
echo "  3. Add session todo tracking initialization"
echo "  4. Add step-verifier verification after each step"
echo ""

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
echo '  echo '  ```'
echo ""
echo "Add gates to $COMMAND_FILE between stages"
echo ""

```

#### [QUALITY GATE 2: Structure Complete]
```bash
# MANDATORY: Must pass before proceeding
```

**Expected gate output:**
```
✅ GATE 'Structure' - PASSED
   Verified: Steps 3,4,5
```

**Minimum Requirements for Stage 2:**
- [ ] Command file created with proper structure
- [ ] Session todo tracking infrastructure added
- [ ] Step-verifier verification added
- [ ] Quality gates planned

### Stage 3: Documentation and Verification (Steps 6-7)

#### Step 6: Add Documentation Sections

**⚠️ MANDATORY: Complete documentation required**

```bash
echo "📝 Add documentation to: $COMMAND_NAME"
echo ""
echo "Required sections:"
echo "  ✓ KB Consultation (already added)"
echo "  ✓ Session Todo Tracking (already added)"
echo "  ✓ Step Verification (already added)"
echo "  ✓ Minimum Requirements (already added)"
echo "  □ Expected outputs for all operations"
echo "  □ Troubleshooting section"
echo "  □ Related commands"
echo "  □ Related KB articles"
echo ""
echo "Edit $COMMAND_FILE to complete documentation"
echo ""

```

#### Step 7: Verify Excellence Standards

**⚠️ CRITICAL: Manual review required**

```bash
# Review command structure manually
# Commands are validated through:
#   • Documentation checks (make check-docs)
#   • Manual review
#   • KB article cross-references
```

#### [QUALITY GATE 3: Excellence Achieved]
```bash
# MANDATORY: Must pass before completion
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

**Expected completion output:**
```
========================================
   WORKFLOW COMPLETION SUMMARY
========================================

📈 create-command: X/Y steps (Z%)
   [████████████████████] 100%

```
```

```bash
# Clean up tracking file
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
- [ ] Review command structure manually
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
- Complete progress tracking with gates

## Output Format

The command creates a structured file with:
- **Header**: Purpose and usage
- **KB Consultation**: Required reading
- **Session Todo Tracking**: Initialize todos, track progress
- **Step Verification**: Step-verifier sub-agent verification
- **Minimum Requirements**: Quality criteria
- **Execution Workflow**: Staged steps with gates
- **Documentation**: Related commands and KB articles
- **Troubleshooting**: Common issues and solutions

## Related Commands

- `/new-learnings` - Extract patterns to create new KB articles

## Related KB Articles

### Command Creation
- [Command Helper Script Extraction Pattern](../../../kb/command-helper-script-extraction-pattern.md) ⭐ Extract embedded bash to scripts
- [Session Todo List Tracking Pattern](../../../kb/session-todo-list-tracking-pattern.md) - Track steps across sessions
- [Command Documentation Excellence Gate](../../../kb/command-documentation-excellence-gate.md) - 90%+ requirement

### Quality Standards
- [Command Thoroughness Requirements Pattern](../../../kb/command-thoroughness-requirements-pattern.md) - Minimum requirements
- [Command Output Documentation Pattern](../../../kb/command-output-documentation-pattern.md) - Expected outputs
- [Sub-Agent Verification Pattern](../../../kb/sub-agent-verification-pattern.md) - Step verification via sub-agents

### Supporting Patterns
- [Gate Enforcement Exit Codes Pattern](../../../kb/gate-enforcement-exit-codes-pattern.md) - Quality gates
- [Lesson-Based Command Design Pattern](../../../kb/lesson-based-command-design-pattern.md) - Unified verification

$ARGUMENTS