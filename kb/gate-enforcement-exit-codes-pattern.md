# Gate Enforcement with Exit Codes Pattern

## Learning
Use exit codes and gate checks to enforce prerequisite completion in multi-step processes. Gates act as quality checkpoints that prevent progression until specific conditions are met, using exit code 1 for blocked gates and 0 for passed gates.

## Importance
Without enforcement gates, users can accidentally skip critical steps, leading to incomplete implementations, broken builds, or invalid states. Exit codes enable automation and chaining of commands, making gates work seamlessly with shell operators like `&&`.

## Example
```bash
#!/bin/bash
# checkpoint-gate.sh - Enforce completion of prerequisite steps

COMMAND_NAME=$1
GATE_NAME=$2
REQUIRED_STEPS=$3
TRACKING_FILE="/tmp/${COMMAND_NAME}-progress.txt"

echo "========================================"
echo "   GATE: $GATE_NAME"
echo "========================================"
echo ""

# Parse required steps
IFS=',' read -ra STEPS <<< "$REQUIRED_STEPS"
INCOMPLETE_STEPS=()

# Check each required step
for step in "${STEPS[@]}"; do
    if grep -q "STEP_${step}=pending" "$TRACKING_FILE" 2>/dev/null; then
        STEP_DESC=$(grep "STEP_${step}=" "$TRACKING_FILE" | sed 's/.*# //')
        INCOMPLETE_STEPS+=("$step")
        echo "  ⏳ Step $step: $STEP_DESC"
    fi
done

# Enforce gate based on results
if [ ${#INCOMPLETE_STEPS[@]} -eq 0 ]; then
    echo "✅ GATE PASSED: All required steps are complete!"
    echo ""
    echo "You may proceed to the next section."
    exit 0  # Success - gate passed
else
    echo ""
    echo "❌ GATE BLOCKED: Cannot proceed!"
    echo ""
    echo "⛔ STOP: Complete the missing steps before continuing."
    exit 1  # Failure - gate blocked
fi
```

## Generalization
1. **Use exit 0 for success, exit 1 for failure** - Standard Unix convention
2. **Chain with && operator** - `gate && next_step` ensures enforcement
3. **Provide clear blocking messages** - Tell user exactly what's missing
4. **Group related steps into gates** - "Article Creation", "Integration", "Final"
5. **Make gates mandatory** - Document as "MANDATORY GATE" in commands

## Implementation
```bash
# Using gates in command flow
./scripts/checkpoint-gate.sh build "Prerequisites" "1,2,3"
if [ $? -eq 0 ]; then
    echo "Gate passed, continuing..."
    ./scripts/checkpoint-update.sh build 4
else
    echo "Cannot continue until prerequisites are met"
    exit 1
fi

# Or use && for automatic flow control
./scripts/checkpoint-gate.sh build "Phase1" "1,2" && \
./scripts/checkpoint-update.sh build 3 && \
./scripts/checkpoint-gate.sh build "Phase2" "3,4" && \
./scripts/checkpoint-update.sh build 5
```

## Related Patterns
- [Checkpoint Conditional Flow Pattern](checkpoint-conditional-flow-pattern.md) - Using gates for conditional workflow branching
- [Checkpoint Workflow Enforcement Pattern](checkpoint-workflow-enforcement-pattern.md) - Prevention-focused enforcement complementing gate detection
- [Build System Exit Code Verification](build-system-exit-code-verification.md)
- [Multi-Step Checkpoint Tracking Pattern](multi-step-checkpoint-tracking-pattern.md)
- [Exit Code Propagation Popen](exit-code-propagation-popen.md)
- [Concise Script Output Principle](concise-script-output-principle.md)
- [Unmissable Documentation Pattern](unmissable-documentation-pattern.md)
- [KB Target Compliance Enforcement](kb-target-compliance-enforcement.md) - Enforce quantitative targets with gates