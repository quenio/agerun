# Multi-Step Checkpoint Tracking Pattern

## Learning
Complex multi-step processes benefit from formal checkpoint tracking systems that provide initialization, step updates, gate enforcement, progress visualization, and cleanup. This ensures thorough execution and prevents users from accidentally skipping critical steps.

## Importance
Without checkpoint tracking, multi-step commands often suffer from incomplete execution where users skip steps, forget prerequisites, or lose track of progress. This leads to partial implementations, quality issues, and repeated debugging sessions.

## Example
```bash
#!/bin/bash
# checkpoint_init.sh - Initialize tracking for any multi-step command
COMMAND_NAME=$1
shift
STEPS=("$@")

TRACKING_FILE="/tmp/${COMMAND_NAME}_progress.txt"

# Create tracking file with all steps
echo "# Progress tracking for: $COMMAND_NAME" > "$TRACKING_FILE"
echo "# Started: $(date)" >> "$TRACKING_FILE"
echo "# Total steps: ${#STEPS[@]}" >> "$TRACKING_FILE"
echo "" >> "$TRACKING_FILE"

for i in "${!STEPS[@]}"; do
    STEP_NUM=$((i + 1))
    echo "STEP_${STEP_NUM}=pending    # ${STEPS[$i]}" >> "$TRACKING_FILE"
done

# checkpoint_gate.sh - Enforce prerequisites
REQUIRED_STEPS=$(echo $3 | tr ',' ' ')
INCOMPLETE=()

for step in $REQUIRED_STEPS; do
    if grep -q "STEP_${step}=pending" "$TRACKING_FILE"; then
        INCOMPLETE+=($step)
    fi
done

if [ ${#INCOMPLETE[@]} -gt 0 ]; then
    echo "❌ GATE BLOCKED: Steps ${INCOMPLETE[*]} must be completed first"
    exit 1
fi

echo "✅ GATE PASSED: All required steps complete"
exit 0
```

## Generalization
1. **Initialize with all steps upfront** - Define complete workflow at start
2. **Track state persistently** - Use files in /tmp for cross-session tracking
3. **Enforce gates at critical points** - Block progression without prerequisites
4. **Provide visual progress** - Show completion percentage and next steps
5. **Clean up when complete** - Remove tracking files after successful completion

## Implementation
Complete checkpoint system components:
```bash
# 1. Initialize: checkpoint_init.sh
# 2. Update: checkpoint_update.sh (marks steps complete)
# 3. Status: checkpoint_status.sh (shows progress with ASCII bar)
# 4. Gate: checkpoint_gate.sh (enforces prerequisites)
# 5. Cleanup: checkpoint_cleanup.sh (removes tracking)

# Integration in commands:
make checkpoint-init CMD=my-command STEPS='"Step 1" "Step 2" "Step 3"'
make checkpoint-update CMD=my-command STEP=1
make checkpoint-gate CMD=my-command GATE="Stage 1" REQUIRED="1,2"
make checkpoint-status CMD=my-command
make checkpoint-cleanup CMD=my-command
```

## Validation Operation Benefits

### Enhanced Effectiveness for Documentation/Code Validation

Checkpoint tracking proves especially valuable for validation workflows:

```c
// Example: ar_data_t validation workflow tracking
ar_data_t* own_validation_state = ar_data__create_map();
ar_data__set_map_string(own_validation_state, "current_step", "1");
ar_data__set_map_string(own_validation_state, "total_errors", "4");

// Each checkpoint provides clear progress visibility
// Step 1: Initial Check (found 4 errors)
// Step 2: Preview Fixes (reviewed automatic fixes)
// Step 3: Apply Fixes (manual intervention needed)
// Step 4: Verify Resolution (all errors resolved)

ar_data__destroy(own_validation_state);
```

**Key Benefits for Validation Processes**:
- **Error Recovery**: When automated fixes fail, checkpoints show exactly where manual intervention is needed
- **Progress Confidence**: Complex validation processes often take multiple iterations - tracking shows actual progress
- **Audit Trail**: Provides clear record of what was attempted and what worked
- **Parallel Development**: Multiple validation types can be tracked simultaneously

### Example Validation Workflow Integration
```bash
# Documentation validation with checkpoint tracking
make checkpoint-init CMD=check-docs STEPS='"Initial Check" "Preview Fixes" "Apply Fixes" "Verify Resolution" "Commit and Push"'

# Each step provides clear feedback
make check-docs 2>&1 | tee /tmp/check-docs-output.txt
make checkpoint-update CMD=check-docs STEP=1

# Gate enforcement prevents skipping critical verification
make checkpoint-gate CMD=check-docs GATE="Resolution" REQUIRED="4"
```

## Related Patterns
- [Checkpoint-Based Workflow Pattern](checkpoint-based-workflow-pattern.md) - Comprehensive workflow structure
- [Gate Enforcement Exit Codes Pattern](gate-enforcement-exit-codes-pattern.md)
- [Progress Visualization ASCII Pattern](progress-visualization-ascii-pattern.md)
- [Command Thoroughness Requirements Pattern](command-thoroughness-requirements-pattern.md)
- [Validation Feedback Loop Effectiveness](validation-feedback-loop-effectiveness.md)
- [Documentation Error Type Classification](documentation-error-type-classification.md)
- [Context Preservation Across Sessions](context-preservation-across-sessions.md) - Resumable multi-step work