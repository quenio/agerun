# Command Output Documentation Pattern

## Learning
Commands that invoke scripts or tools should document expected outputs for each invocation, showing both successful execution and failure states. This helps users understand whether the command is working correctly and recognize error conditions.

## Importance
Without expected output documentation, users cannot distinguish between correct behavior and silent failures. This uncertainty leads to confusion, repeated questions, and potential misuse of commands. Clear output examples serve as inline verification criteria.

## Example
```markdown
### Command with Expected Output Documentation

\`\`\`bash
# Initialize tracking system
make checkpoint-init CMD=my-command STEPS='"Step 1" "Step 2"'
\`\`\`

**Expected output:**
\`\`\`
========================================
   CHECKPOINT TRACKING INITIALIZED
========================================

Command: my-command
Tracking file: /tmp/my-command_progress.txt
Total steps: 2

Steps to complete:
  1. Step 1
  2. Step 2
\`\`\`

**Expected output when failing:**
\`\`\`
Error: Tracking file already exists: /tmp/my-command_progress.txt
Run 'checkpoint-cleanup.sh my-command' to reset
[Exit code: 1]
\`\`\`
```

## Generalization
1. **Document both success and failure states** - Users need to recognize both conditions
2. **Include exit codes for failures** - Helps with debugging and automation
3. **Show actual output format** - Use exact formatting, not paraphrased descriptions
4. **Update when output changes** - Keep documentation synchronized with implementation
5. **Test examples regularly** - Verify documented outputs match actual behavior

## Implementation
When documenting command outputs:
```bash
# Run the command and capture output
command 2>&1 | tee output.txt

# Document the exact output in markdown
echo '**Expected output:**'
echo '```'
cat output.txt
echo '```'

# For failure cases, include exit code
echo "[Exit code: $?]"
```

## Related Patterns
- [AR Assert Descriptive Failures](ar-assert-descriptive-failures.md)
- [Documentation Validation Enhancement Patterns](documentation-validation-enhancement-patterns.md)
- [Concise Script Output Principle](concise-script-output-principle.md)
- [Documentation Implementation Sync](documentation-implementation-sync.md)
- [Unmissable Documentation Pattern](unmissable-documentation-pattern.md)