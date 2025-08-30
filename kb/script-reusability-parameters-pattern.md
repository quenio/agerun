# Script Reusability Through Parameters Pattern

## Learning
Scripts should be designed generically with parameters rather than hardcoded for specific use cases. This enables reuse across different commands, reduces code duplication, and creates a library of composable utilities.

## Importance
Command-specific scripts lead to proliferation of similar code with slight variations. This increases maintenance burden, introduces inconsistencies, and makes it harder to fix bugs or add features uniformly across all uses.

## Example
```bash
#!/bin/bash
# BAD: Hardcoded script for specific command
# new_learnings_checkpoint.sh
echo "Checking new-learnings progress..."
if grep -q "STEP_1=complete" /tmp/new_learnings_progress.txt; then
    echo "Step 1 of new-learnings is complete"
fi

#!/bin/bash
# GOOD: Generic parameterized script
# checkpoint_status.sh
COMMAND_NAME=${1:-"command"}
VERBOSE=${2:-""}
TRACKING_FILE="/tmp/${COMMAND_NAME}_progress.txt"

if [ ! -f "$TRACKING_FILE" ]; then
    echo "Error: No tracking for command: $COMMAND_NAME"
    exit 1
fi

echo "Checking $COMMAND_NAME progress..."
COMPLETED=$(grep -c "=complete" "$TRACKING_FILE")
TOTAL=$(grep -c "^STEP_" "$TRACKING_FILE")

echo "Progress: $COMPLETED/$TOTAL completed"

if [ "$VERBOSE" = "--verbose" ]; then
    grep "^STEP_" "$TRACKING_FILE" | while read line; do
        echo "  $line"
    done
fi
```

## Generalization
1. **Accept command/context as first parameter** - Makes script work for any command
2. **Use consistent parameter ordering** - Command, required params, optional flags
3. **Provide helpful defaults** - `${VAR:-default}` for optional parameters
4. **Validate parameters early** - Check required params before processing
5. **Design for composition** - Scripts that work together through pipes/exit codes

## Implementation
```bash
# Parameter handling template
#!/bin/bash
set -e  # Exit on error

# Required parameters
COMMAND=${1:?Error: Command name required}
ACTION=${2:?Error: Action required}

# Optional parameters with defaults
VERBOSE=${3:-false}
OUTPUT_DIR=${4:-/tmp}

# Validation
if [ ! -d "$OUTPUT_DIR" ]; then
    echo "Error: Output directory does not exist: $OUTPUT_DIR"
    exit 1
fi

# Generic processing
case "$ACTION" in
    init|update|status|cleanup)
        # Process based on action
        ;;
    *)
        echo "Error: Unknown action: $ACTION"
        echo "Usage: $0 <command> <init|update|status|cleanup> [verbose] [output_dir]"
        exit 1
        ;;
esac
```

## Related Patterns
- [Script Enhancement Over One-Off](script-enhancement-over-one-off.md)
- [Batch Update Script Pattern](batch-update-script-pattern.md)
- [Shell Script Command Substitution](shell-script-command-substitution.md)