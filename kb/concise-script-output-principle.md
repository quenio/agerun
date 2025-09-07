# Concise Script Output Principle

## Learning
Limiting script output to 3 lines maximum dramatically improves readability and user experience. Verbose output obscures important information and makes terminal interaction cumbersome.

## Importance
Users need immediate, actionable information. Excessive output creates cognitive overload, slows debugging, and makes it harder to spot actual problems. Concise output respects the user's attention and terminal space.

## Example
```bash
# Before: Verbose output (20+ lines)
echo "========================================" 
echo "   CHECKPOINT STATUS: $COMMAND_NAME"
echo "========================================"
echo ""
echo "Progress tracking for: $COMMAND_NAME"
echo "Tracking file: $TRACKING_FILE"
# ... many more lines ...

# After: Concise output (3 lines)
echo "📈 $COMMAND_NAME: $COMPLETED/$TOTAL steps (${PERCENT}%)"
echo "   [$PROGRESS_BAR] ${PERCENT}%"
echo "→ Next: make checkpoint-update CMD=$COMMAND_NAME STEP=$NEXT_STEP"
```

## Generalization
Design scripts with:
1. Maximum 3 lines for standard output
2. Icons/symbols for visual parsing (📍, ✓, ❌)
3. Progress bars for visual feedback
4. Next action clearly indicated
5. Verbose mode (--verbose) for detailed output when needed

## Implementation
```bash
# Add compact mode to scripts
if [ "$1" = "--compact" ]; then
    # 3-line output
    echo "Status: $STATUS"
    echo "Progress: $PROGRESS"
    echo "Next: $ACTION"
else
    # Detailed output for debugging
    echo "Full verbose output..."
fi

# Use ar_log for detailed logging
ar_log__info(log, "Detailed information for debugging");
```

## Related Patterns
- [Command Output Documentation Pattern](command-output-documentation-pattern.md)
- [Debug Output Preservation Strategy](debug-output-preservation-strategy.md)
- [Gate Enforcement Exit Codes Pattern](gate-enforcement-exit-codes-pattern.md)