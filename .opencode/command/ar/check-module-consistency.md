Check for consistency improvements needed across related modules.

## MANDATORY KB Consultation

Before checking consistency:
1. Search: `grep "consistency\|module\|systematic" kb/README.md`
2. Must read:
   - module-consistency-verification
   - systematic-consistency-verification
3. Apply systematic verification approach

## CHECKPOINT WORKFLOW ENFORCEMENT

**CRITICAL**: This command MUST use checkpoint tracking for ALL execution.

### In-Progress Workflow Detection

If a `/check-module-consistency` workflow is already in progress:

```bash
./scripts/checkpoint-status.sh check-module-consistency VERBOSE=--verbose
# Resume: ./scripts/checkpoint-update.sh check-module-consistency STEP=N
# Or reset: ./scripts/checkpoint-cleanup.sh check-module-consistency && ./scripts/checkpoint-init.sh check-module-consistency "Describe Improvement" "Identify Pattern" "Find Sister Modules" "Find Similar Purpose" "Find Same Subsystem" "Check Module 1" "Check Module 2" "Check Module 3" "Check Module 4" "Check Module 5" "Analyze Findings" "List Modules Needing Update" "Estimate Effort" "Create Priority Order" "Document Plan"
```

### First-Time Initialization Check

```bash
./scripts/checkpoint-init.sh check-module-consistency "Describe Improvement" "Identify Pattern" "Find Sister Modules" "Find Similar Purpose" "Find Same Subsystem" "Check Module 1" "Check Module 2" "Check Module 3" "Check Module 4" "Check Module 5" "Analyze Findings" "List Modules Needing Update" "Estimate Effort" "Create Priority Order" "Document Plan"
```

## PRECONDITION: Checkpoint Tracking Must Be Initialized

```bash
./scripts/checkpoint-require.sh check-module-consistency
```

# Check Module Consistency
## Checkpoint Tracking

This command uses checkpoint tracking to ensure systematic consistency checking across related modules. The process has 15 checkpoints across 5 phases.

### Initialize Tracking
```bash
# Start the consistency checking process
./scripts/checkpoint-init.sh check-module-consistency "Describe Improvement" "Identify Pattern" "Find Sister Modules" "Find Similar Purpose" "Find Same Subsystem" "Check Module 1" "Check Module 2" "Check Module 3" "Check Module 4" "Check Module 5" "Analyze Findings" "List Modules Needing Update" "Estimate Effort" "Create Priority Order" "Document Plan"
```

**Expected output:**
```
========================================
   CHECKPOINT TRACKING INITIALIZED
========================================

Command: check-module-consistency
Tracking file: /tmp/check-module-consistency-progress.txt
Total steps: 15

Steps to complete:
  1. Describe Improvement
  2. Identify Pattern
  3. Find Sister Modules
  4. Find Similar Purpose
  5. Find Same Subsystem
  6. Check Module 1
  7. Check Module 2
  8. Check Module 3
  9. Check Module 4
  10. Check Module 5
  11. Analyze Findings
  12. List Modules Needing Update
  13. Estimate Effort
  14. Create Priority Order
  15. Document Plan

Goal: Check consistency across related modules
Minimum: Check 3+ modules for same patterns
```

### Check Progress
```bash
./scripts/checkpoint-status.sh check-module-consistency
```

**Expected output (example at 53% completion):**
```
📈 command: X/Y steps (Z%)
   [████░░░░░░░░░░░░░░░░] Z%
→ Next: ./scripts/checkpoint-update.sh command STEP=N
```

## Minimum Requirements

**MANDATORY for successful completion:**
- [ ] Check at least 3 related modules
- [ ] Verify all pattern types (error logging, API, state, docs)
- [ ] Document findings for each module
- [ ] Create actionable improvement plan
- [ ] Estimate effort for each module needing update

### Overview

After making improvements to one module, use this command to systematically check if related or sister modules need the same improvements. This prevents technical debt from accumulating and maintains architectural coherence. ([details](../../../kb/module-consistency-verification.md))

### Stage 1: Understanding (Steps 1-2)

#### [CHECKPOINT START - STAGE 1]

#### [CHECKPOINT END]

#### Step 1: Identify the Improvement Made

#### Step 1: Describe Improvement

First, clearly describe what improvement was just made:
- What module was improved?
- What specific enhancement was added? (e.g., error logging, instance-based API, validation)
- What pattern was applied?

```bash
# Document the improvement
echo "Module improved: [module_name]"
echo "Enhancement: [description]"
echo "Pattern applied: [pattern_name]"

# Initialize tracking file
echo "MODULES_FOUND=0" > /tmp/check-consistency-tracking.txt
echo "MODULES_CHECKED=0" >> /tmp/check-consistency-tracking.txt
echo "MODULES_NEEDING_UPDATE=0" >> /tmp/check-consistency-tracking.txt

./scripts/checkpoint-update.sh check-module-consistency STEP=1
```

#### Step 2: Identify Pattern

```bash
# Identify the specific pattern to check for
echo "Pattern to verify across modules:"
echo "- Check type: [error logging/API/state/docs]"
echo "- Specific requirement: [description]"
echo "- Expected behavior: [description]"

./scripts/checkpoint-update.sh check-module-consistency STEP=2
```

#### [UNDERSTANDING GATE]
```bash
# MANDATORY: Clear understanding before searching
./scripts/checkpoint-gate.sh check-module-consistency "Understanding" "1,2"
```

**Expected gate output:**
```
✅ GATE 'Understanding' - PASSED
   Verified: Steps 1,2
```

### Stage 2: Discovery (Steps 3-5)

#### [CHECKPOINT START - STAGE 2]

#### [CHECKPOINT END]

#### Step 2: Find Related Modules

Identify modules that should be checked for consistency:

#### Steps 3-5: Discover Module Relationships

Run relationship discovery using helper script:

```bash
# Discover related modules (Steps 3-5 combined)
# Pass improved module name if doing consistency check on specific module
./scripts/discover-module-relationships.sh [module-name] | tee /tmp/check-consistency-tracking.txt

# If script exits 0, sufficient modules found - mark steps complete
if [ $? -eq 0 ]; then
  ./scripts/checkpoint-update.sh check-module-consistency STEP=3
  ./scripts/checkpoint-update.sh check-module-consistency STEP=4
  ./scripts/checkpoint-update.sh check-module-consistency STEP=5
else
  echo "❌ Insufficient modules found for consistency check"
  exit 1
fi
```

The script discovers:
1. **Sister Modules** - Paired patterns (reader/writer, parser/evaluator, ast/evaluator)
2. **Similar Purpose** - Modules with common patterns (logging, instance management, create/destroy)
3. **Same Subsystem** - Modules that depend on the improved module

#### [DISCOVERY GATE]
```bash
# MANDATORY: Ensure enough modules found
./scripts/checkpoint-gate.sh check-module-consistency "Discovery" "3,4,5"
```

**Expected gate output:**
```
✅ GATE 'Discovery' - PASSED
   Verified: Steps 3,4,5
```

### Stage 3: Analysis (Steps 6-10)

#### [CHECKPOINT START - STAGE 3]

#### [CHECKPOINT END]

#### Step 3: Check Each Related Module

**Module Tracking Function**:

Use the helper script to check module consistency:
```bash
./scripts/check-module-consistency.sh <module-path>
# Returns: 0 if consistent, 1 if issues found
# Example: ./scripts/check-module-consistency.sh modules/ar_data
```

#### Step 6: Check Module 1

```bash
MODULE1="modules/ar_[module1]"
if ./scripts/check-module-consistency.sh "$MODULE1"; then
  echo "Module 1: Consistent"
else
  source /tmp/check-consistency-tracking.txt
  MODULES_NEEDING_UPDATE=$((MODULES_NEEDING_UPDATE + 1))
  echo "MODULES_NEEDING_UPDATE=$MODULES_NEEDING_UPDATE" >> /tmp/check-consistency-tracking.txt
fi

MODULES_CHECKED=1
echo "MODULES_CHECKED=$MODULES_CHECKED" >> /tmp/check-consistency-tracking.txt

./scripts/checkpoint-update.sh check-module-consistency STEP=6
```

**Checkpoint 7-10: Check Modules 2-5**

```bash
# Repeat for modules 2-5
# After each check:
./scripts/checkpoint-update.sh check-module-consistency STEP=7  # Module 2
./scripts/checkpoint-update.sh check-module-consistency STEP=8  # Module 3
./scripts/checkpoint-update.sh check-module-consistency STEP=9  # Module 4
./scripts/checkpoint-update.sh check-module-consistency STEP=10 # Module 5
```

**Module Analysis Summary**:
```bash
source /tmp/check-consistency-tracking.txt
echo ""
echo "Module Analysis Summary:"
echo "  Modules Checked: $MODULES_CHECKED"
echo "  Needing Update: $MODULES_NEEDING_UPDATE"
echo "  Consistent: $((MODULES_CHECKED - MODULES_NEEDING_UPDATE))"
```

#### [ANALYSIS GATE]
```bash
# MANDATORY: Minimum 3 modules checked
source /tmp/check-consistency-tracking.txt
if [ $MODULES_CHECKED -lt 3 ]; then
  echo "❌ Only checked $MODULES_CHECKED modules (minimum: 3)"
  exit 1
fi

./scripts/checkpoint-gate.sh check-module-consistency "Analysis" "6,7,8,9,10"
```

**Expected gate output:**
```
✅ GATE 'Analysis' - PASSED
   Verified: Steps 6,7,8,9,10
```

For each related module identified, check if it needs the same improvement:

### Error Logging Check ([details](../../../kb/error-logging-instance-utilization.md))
```bash
# Check if module has ar_log but isn't using it
grep -l "ar_log_t.*ref_log" modules/MODULE.c
grep -c "ar_log__error" modules/MODULE.c
```

### Instance-Based API Check
```bash
# Check if module still uses global state
grep "^static.*g_\|^[^/]*g_" modules/MODULE.c | grep -v "g_default_instance"
```

### Stateless Verification ([details](../../../kb/stateless-module-verification.md))
```bash
# Check for hidden global state
grep -n "^static.*[^(]$\|^[^/]*g_" modules/MODULE.c | \
    grep -v "g_default_instance\|static const\|static.*("
```

### NULL Parameter Handling
```bash
# Check if NULL parameters are validated
grep -A 5 "if.*!.*\|\|.*!.*)" modules/MODULE.c
```

### Stage 4: Planning (Steps 11-14)

#### [CHECKPOINT START - STAGE 4]

#### [CHECKPOINT END]

#### Step 4: Create Improvement Plan

#### Step 11: Analyze Findings

```bash
echo "Analyzing consistency findings..."
source /tmp/check-consistency-tracking.txt

echo "Summary of findings:"
echo "- Total modules checked: $MODULES_CHECKED"
echo "- Modules needing update: $MODULES_NEEDING_UPDATE"
echo "- Consistency rate: $((100 * (MODULES_CHECKED - MODULES_NEEDING_UPDATE) / MODULES_CHECKED))%"

./scripts/checkpoint-update.sh check-module-consistency STEP=11
```

#### Step 12: List Modules Needing Update

For modules that need the same improvement:

1. **List all modules needing update**
   ```bash
   echo "Modules requiring consistency updates:"
   echo "1. ar_[module1] - Missing error logging"
   echo "2. ar_[module2] - Has global state"
   echo "3. ar_[module3] - Lacks NULL validation"
   
   ./scripts/checkpoint-update.sh check-module-consistency STEP=12
   ```

#### Step 13: Estimate Effort

2. **Estimate TDD cycles required** for each
   ```bash
   echo "Effort estimation (TDD cycles):"
   echo "- ar_[module1]: 2 cycles (add error logging)"
   echo "- ar_[module2]: 3 cycles (remove global state)"
   echo "- ar_[module3]: 1 cycle (add validation)"
   echo "Total estimated effort: 6 TDD cycles"
   
   ./scripts/checkpoint-update.sh check-module-consistency STEP=13
   ```

#### Step 14: Create Priority Order

3. **Prioritize by**:
   - User-facing impact
   - Frequency of use
   - Dependency order
   
   ```bash
   echo "Priority order:"
   echo "1. HIGH: ar_[module2] - Core module, high usage"
   echo "2. MEDIUM: ar_[module1] - Important for debugging"
   echo "3. LOW: ar_[module3] - Internal module"
   
   ./scripts/checkpoint-update.sh check-module-consistency STEP=14
   ```

#### [PLANNING GATE]
```bash
# MANDATORY: Ensure comprehensive plan
./scripts/checkpoint-gate.sh check-module-consistency "Planning" "11,12,13,14"
```

**Expected gate output:**
```
✅ GATE 'Planning' - PASSED
   Verified: Steps 11,12,13,14
```

### Stage 5: Documentation (Step 15)

#### [CHECKPOINT START - STAGE 5]

#### [CHECKPOINT END]

#### Step 15: Document Plan

```bash
# Create comprehensive improvement plan
cat > consistency-improvement-plan.md << 'EOF'
# Consistency Improvement Plan

## Pattern Applied
[Description of the pattern being propagated]

## Modules Requiring Update
1. ar_[module1] - [issues found]
2. ar_[module2] - [issues found]
3. ar_[module3] - [issues found]

## Implementation Order
1. HIGH Priority: [module] ([cycles] cycles)
2. MEDIUM Priority: [module] ([cycles] cycles)
3. LOW Priority: [module] ([cycles] cycles)

## Total Effort
[N] TDD cycles estimated

## Success Criteria
- All modules have consistent [pattern]
- Tests verify the improvement
- Documentation updated
EOF

echo "✓ Improvement plan documented"

./scripts/checkpoint-update.sh check-module-consistency STEP=15
```

#### [CHECKPOINT COMPLETE]
```bash
./scripts/checkpoint-complete.sh check-module-consistency
```

**Expected completion output:**
```
========================================
   CHECKPOINT COMPLETION SUMMARY
========================================

📈 check-module-consistency: X/Y steps (Z%)
   [████████████████████] 100%

✅ Checkpoint workflow complete
```
rm -f /tmp/check-consistency-tracking.txt
```

#### Step 5: Apply Improvements Systematically

Use TDD to apply the same pattern to each module:
- Create similar tests as the original improvement
- Apply the same implementation pattern
- Ensure consistency in:
  - Error messages
  - Parameter validation order
  - Return value conventions
  - Documentation updates

## Common Consistency Patterns to Check

1. **Error Logging**:
   - All modules with ar_log should use it for errors
   - Error messages should follow same format
   - NULL parameter checks should log when possible

2. **API Design**:
   - Sister modules should have parallel APIs
   - Naming conventions should match
   - Parameter order should be consistent

3. **State Management**:
   - All instance-based modules should be stateless
   - Global state should be eliminated consistently
   - Resource ownership should follow same patterns

4. **Documentation**:
   - Similar modules should have similar documentation structure
   - Error handling sections should be present
   - Test counts should be accurate

## Example Consistency Check

After adding error logging to ar_yaml_reader:
```bash
# 1. Check if ar_yaml_writer has same gaps
grep -l "ar_log_t" modules/ar_yaml_writer.c  # Has log instance?
grep -c "ar_log__error" modules/ar_yaml_writer.c  # Using it?

# 2. If not using it fully, plan improvements
echo "ar_yaml_writer needs error logging for:"
echo "- NULL data parameter"
echo "- NULL filename parameter"
echo "- File operation failures"

# 3. Apply same TDD approach
make ar_yaml_writer_tests  # Run before changes
# ... implement improvements ...
make ar_yaml_writer_tests  # Verify after
```

## Troubleshooting

### If not enough modules found:
```bash
# Broaden search patterns
ls modules/ar_*.c | head -20  # List more modules
grep -l "pattern" modules/*.c  # Search for specific patterns
```

### If module checks are unclear:
```bash
# Use the provided consistency check function
# Or manually verify each aspect:
grep "ar_log__error" module.c  # Check error logging
grep "^static.*g_" module.c    # Check global state
grep "if.*!" module.c          # Check validation
```

### To resume an interrupted session:
```bash
# Check progress
./scripts/checkpoint-status.sh check-module-consistency

# Load tracking data
source /tmp/check-consistency-tracking.txt
echo "Modules checked so far: $MODULES_CHECKED"
```

## Benefits of Systematic Consistency

- **Predictable behavior**: Users know what to expect
- **Easier maintenance**: Same patterns everywhere
- **Reduced bugs**: Consistency prevents edge cases
- **Better documentation**: Parallel structures are clearer
- **Faster development**: Patterns can be reused

## Related Documentation

### Checkpoint Patterns
- [Multi-Step Checkpoint Tracking Pattern](../../../kb/multi-step-checkpoint-tracking-pattern.md)
- [Gate Enforcement Exit Codes Pattern](../../../kb/gate-enforcement-exit-codes-pattern.md)
- [Command Thoroughness Requirements Pattern](../../../kb/command-thoroughness-requirements-pattern.md)

### Consistency Patterns
- [Module Consistency Verification](../../../kb/module-consistency-verification.md)
- [Symmetric Module Architecture Pattern](../../../kb/symmetric-module-architecture-pattern.md)
- [Error Logging Instance Utilization](../../../kb/error-logging-instance-utilization.md)
- [Stateless Module Verification](../../../kb/stateless-module-verification.md)
- [Documentation Index Consistency Pattern](../../../kb/documentation-index-consistency-pattern.md)
- [Script Debugging Through Isolation](../../../kb/script-debugging-through-isolation.md) - Use isolation testing when debugging consistency checker scripts
- [Cross-Platform Bash Script Patterns](../../../kb/cross-platform-bash-script-patterns.md) - Consistency scripts must work on macOS and Linux

Remember: When you improve one module, always ask "What other modules need this same improvement?" ([details](../../../kb/module-consistency-verification.md)). After integration, update modules/README.md index ([details](../../../kb/documentation-index-consistency-pattern.md))