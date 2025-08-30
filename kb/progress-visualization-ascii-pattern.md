# Progress Visualization with ASCII Pattern

## Learning
Long-running or multi-step processes should provide visual progress indicators using ASCII characters to create progress bars, percentages, and status displays. This improves user experience by providing immediate feedback about execution state.

## Importance
Without visual progress indicators, users cannot tell if a process is running, stuck, or how much longer it will take. This leads to premature termination, repeated executions, and user frustration. ASCII visualizations work universally across terminals.

## Example
```bash
#!/bin/bash
# Display progress with ASCII bar

COMPLETED=$1
TOTAL=$2
PERCENTAGE=$((COMPLETED * 100 / TOTAL))

# Calculate bar segments (20 character width)
FILLED=$((COMPLETED * 20 / TOTAL))

# Build progress bar
echo -n "["
for i in $(seq 1 20); do
    if [ $i -le $FILLED ]; then
        echo -n "█"  # Filled segment
    else
        echo -n "░"  # Empty segment
    fi
done
echo "] $PERCENTAGE%"

# Examples of output:
# [████████████████████] 100%
# [██████████░░░░░░░░░░] 50%
# [████░░░░░░░░░░░░░░░░] 20%
# [░░░░░░░░░░░░░░░░░░░░] 0%

# Enhanced with step details:
echo ""
echo "Step Details:"
echo "  ✓ Step 1: Initialize"
echo "  ✓ Step 2: Process"
echo "  ⏳ Step 3: Validate"
echo "  ⏸ Step 4: Deploy"
echo ""
echo "Next Action:"
echo "  → Step 3: Validate"
```

## Generalization
1. **Use consistent width** - 20 characters is readable and fits most terminals
2. **Include percentage** - Numerical progress complements visual bar
3. **Show current state** - ✓ complete, ⏳ in progress, ⏸ pending
4. **Display next action** - Tell user what comes next
5. **Update in place** - Use `\r` for dynamic updates in same position

## Implementation
```bash
# Function for reusable progress display
show_progress() {
    local current=$1
    local total=$2
    local width=${3:-20}
    
    local percent=$((current * 100 / total))
    local filled=$((current * width / total))
    
    # Clear line and return to start
    printf "\r"
    
    # Draw progress bar
    printf "["
    printf "%${filled}s" | tr ' ' '█'
    printf "%$((width - filled))s" | tr ' ' '░'
    printf "] %3d%%" $percent
    
    # Add newline when complete
    [ $current -eq $total ] && echo ""
}

# Usage in loop
for i in $(seq 1 100); do
    show_progress $i 100
    sleep 0.1
done
```

## Related Patterns
- [Multi-Step Checkpoint Tracking Pattern](multi-step-checkpoint-tracking-pattern.md)
- [Build Time Reporting](build-time-reporting.md)