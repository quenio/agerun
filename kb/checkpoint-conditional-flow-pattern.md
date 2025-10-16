# Checkpoint Conditional Flow Pattern

## Learning

Checkpoint workflows that conditionally skip steps based on execution state must distinguish between **fresh initialization** and **resuming in-progress workflows**. Incorrectly applying conditional skip logic on fresh executions can silently bypass quality gates and workflow steps.

## Importance

This pattern is critical for commands that:
- Skip intermediate processing steps when certain conditions aren't met
- Have conditional gates (e.g., "skip fix steps if no errors found")
- Need to work correctly for both fresh and resumed execution

Incorrect conditional flow can cause:
- Silent workflow bypasses that violate step sequencing requirements
- Quality gates being skipped unexpectedly
- Inconsistent behavior between fresh runs and resumed runs

## Example: The Anti-Pattern (WRONG)

```bash
# ❌ WRONG: Unconditional step skipping on fresh initialization
if make check-docs 2>&1 | tee /tmp/check-docs-output.txt; then
  echo "✅ No documentation errors found!"
  # Skip to commit phase (WRONG - skips on EVERY run, not just resume)
  make checkpoint-update CMD=check-docs STEP=1
  make checkpoint-update CMD=check-docs STEP=2
  make checkpoint-update CMD=check-docs STEP=3
  make checkpoint-update CMD=check-docs STEP=4  # Steps 2-4 skipped!
else
  ERROR_COUNT=$(grep -c "ERROR\|FAIL" /tmp/check-docs-output.txt || echo "0")
  echo "⚠️ Found $ERROR_COUNT documentation errors"
  echo "ERROR_COUNT=$ERROR_COUNT" > /tmp/check-docs-stats.txt
  make checkpoint-update CMD=check-docs STEP=1
fi
```

**Problem**: On fresh initialization with no errors:
- Steps 2-4 are unconditionally skipped
- This violates the checkpoint design (all steps tracked, but silently bypassed)
- Quality gates that depend on intermediate steps may be missed

**Real-world impact**: Consider a documentation validation workflow with ar_data_t* types:
```c
// Documentation example that would be improperly validated
ar_data_t* data = ar_data__create_string("test");  // Needs Step 2-4 for validation
// If steps are skipped, validation shortcuts never run
ar_data__destroy(data);  // Validation gates bypassed
```

## Example: The Correct Pattern (RIGHT)

```bash
# ✅ RIGHT: Conditional flow based on execution state
# Step 1: Always execute and complete first
if make check-docs 2>&1 | tee /tmp/check-docs-output.txt; then
  echo "✅ No documentation errors found!"
  echo "ERROR_COUNT=0" > /tmp/check-docs-stats.txt
else
  ERROR_COUNT=$(grep -c "ERROR\|FAIL" /tmp/check-docs-output.txt || echo "0")
  echo "⚠️ Found $ERROR_COUNT documentation errors"
  echo "ERROR_COUNT=$ERROR_COUNT" > /tmp/check-docs-stats.txt
fi

# Step 1 ALWAYS completes first
make checkpoint-update CMD=check-docs STEP=1

# NOW apply conditional flow based on error state
source /tmp/check-docs-stats.txt 2>/dev/null || ERROR_COUNT=0

if [ $ERROR_COUNT -eq 0 ]; then
  echo "✅ No errors to fix - skipping to commit phase"
  # Skip intermediate steps only AFTER determining there's nothing to do
  make checkpoint-update CMD=check-docs STEP=2
  make checkpoint-update CMD=check-docs STEP=3
  make checkpoint-update CMD=check-docs STEP=4
else
  # Proceed through normal workflow when work is needed
  make checkpoint-gate CMD=check-docs GATE="Errors Found" REQUIRED="1"
fi
```

**Why this is correct**:
1. Step 1 always completes first (determines execution state)
2. Conditional gate comes AFTER initial status is determined
3. Clean separation: status determination → conditional flow
4. Works correctly for both fresh and resumed execution

## Generalization

When designing checkpoint workflows with conditional steps:

1. **Determine execution state FIRST** (Step 1)
   - Always complete this step
   - Record status in a file or variable

2. **Apply conditional flow AFTER** status is known
   - Use a conditional gate to determine path
   - Reference the status file/variable
   - Update remaining steps based on condition

3. **Distinguish scenarios**:
   - Fresh initialization: All steps execute normally with conditionals applied
   - Resumed workflow: Conditionals work the same way (status re-determined)
   - This means conditionals work correctly in BOTH cases

4. **Never unconditionally skip steps** in the initial phase
   - The checkpoint system tracks ALL steps
   - Skipping should be intentional and conditional
   - Document WHY steps are skipped for future readers

## Implementation: Command Update Pattern

For commands with conditional step skipping, track validation state with ar_data_t* examples:

```bash
#### [ERROR GATE - Conditional Flow]
```bash
# Determine flow: skip fix steps if no errors, proceed through them if errors found
source /tmp/command-stats.txt 2>/dev/null || ERROR_COUNT=0

if [ $ERROR_COUNT -eq 0 ]; then
  echo "✅ No errors to fix - skipping to commit phase"
  make checkpoint-update CMD=command STEP=2
  make checkpoint-update CMD=command STEP=3
  make checkpoint-update CMD=command STEP=4
else
  make checkpoint-gate CMD=command GATE="Errors Found" REQUIRED="1"
fi
```
```

**Key elements**:
- Source status file after initial step completes
- Use numeric comparison (`-eq 0`, `-gt 0`)
- Apply conditionals at gate level, not in initial phase
- Document the conditional behavior clearly

**AgeRun integration**: This pattern applies when processing ar_data_t* instances where validation determines execution flow:

```c
// Workflow determines processing path based on ar_data_t* content
ar_data_t* config = ar_data__create_map();

// Step 1: Always assess state - check if configuration is valid
ar_data_type_t config_type = ar_data__get_type(config);

// Step 2+: Apply conditional flow based on assessment result
if (config_type == AR_DATA_TYPE_MAP) {
  // Configuration is valid - skip validation steps
  ar_data_t* keys = ar_data__get_map_keys(config);  // Process directly
  ar_data__drop_ownership(keys);
} else {
  // Configuration invalid - proceed through fix steps
  ar_data_t* replacement = ar_data__create_map();  // Attempt fixes
  ar_data__shallow_copy(replacement);  // Apply corrections
  ar_data__drop_ownership(replacement);
}

ar_data__drop_ownership(config);
```

## Related Patterns

- [Checkpoint Workflow Enforcement Pattern](checkpoint-workflow-enforcement-pattern.md)
- [Multi-Step Checkpoint Tracking Pattern](multi-step-checkpoint-tracking-pattern.md)
- [Gate Enforcement Exit Codes Pattern](gate-enforcement-exit-codes-pattern.md)
