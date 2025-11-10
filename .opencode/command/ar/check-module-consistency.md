Check for consistency improvements needed across related modules.

## MANDATORY KB Consultation

Before checking consistency:
1. Search: `grep "consistency\|module\|systematic" kb/README.md`
2. Must read:
   - module-consistency-verification
   - systematic-consistency-verification
3. Apply systematic verification approach

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
Prompt: "Verify Step N: [Step Title] completion for check-module-consistency command.

Todo Item: [Description of what the step accomplished]
Command File: .opencode/command/ar/check-module-consistency.md
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
- Add todo item: "Step 1: Describe Improvement" - Status: pending
- Add todo item: "Verify Step 1: Describe Improvement" - Status: pending
- Add todo item: "Step 2: Identify Pattern" - Status: pending
- Add todo item: "Verify Step 2: Identify Pattern" - Status: pending
- Add todo item: "Step 3: Find Sister Modules" - Status: pending
- Add todo item: "Verify Step 3: Find Sister Modules" - Status: pending
- Add todo item: "Step 4: Find Similar Purpose" - Status: pending
- Add todo item: "Verify Step 4: Find Similar Purpose" - Status: pending
- Add todo item: "Step 5: Find Same Subsystem" - Status: pending
- Add todo item: "Verify Step 5: Find Same Subsystem" - Status: pending
- Add todo item: "Step 6: Check Module 1" - Status: pending
- Add todo item: "Verify Step 6: Check Module 1" - Status: pending
- Add todo item: "Step 7: Check Module 2" - Status: pending
- Add todo item: "Verify Step 7: Check Module 2" - Status: pending
- Add todo item: "Step 8: Check Module 3" - Status: pending
- Add todo item: "Verify Step 8: Check Module 3" - Status: pending
- Add todo item: "Step 9: Check Module 4" - Status: pending
- Add todo item: "Verify Step 9: Check Module 4" - Status: pending
- Add todo item: "Step 10: Check Module 5" - Status: pending
- Add todo item: "Verify Step 10: Check Module 5" - Status: pending
- Add todo item: "Step 11: Analyze Findings" - Status: pending
- Add todo item: "Verify Step 11: Analyze Findings" - Status: pending
- Add todo item: "Step 12: List Modules Needing Update" - Status: pending
- Add todo item: "Verify Step 12: List Modules Needing Update" - Status: pending
- Add todo item: "Step 13: Estimate Effort" - Status: pending
- Add todo item: "Verify Step 13: Estimate Effort" - Status: pending
- Add todo item: "Step 14: Create Priority Order" - Status: pending
- Add todo item: "Verify Step 14: Create Priority Order" - Status: pending
- Add todo item: "Step 15: Document Plan" - Status: pending
- Add todo item: "Verify Step 15: Document Plan" - Status: pending
- Add todo item: "Verify Complete Workflow: check-module-consistency" - Status: pending

**Important**: All todo items are initialized as `pending` and will be updated to `in_progress` when their respective step/verification begins, then to `completed` after verification passes.

### In-Progress Workflow Detection

If a `/check-module-consistency` workflow is already in progress:

### First-Time Initialization Check

## PRECONDITION: Checkpoint Tracking Must Be Initialized

# Check Module Consistency
## Checkpoint Tracking

This command uses progress tracking to ensure systematic consistency checking across related modules. The process has 15 checkpoints across 5 phases.

### Initialize Tracking
```bash
# Start the consistency checking process
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
**Expected output (example at 53% completion):**
```
📈 command: X/Y steps (Z%)
   [████░░░░░░░░░░░░░░░░] Z%
→ Next: ```

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

```

#### Step 2: Identify Pattern

```bash
# Identify the specific pattern to check for
echo "Pattern to verify across modules:"
echo "- Check type: [error logging/API/state/docs]"
echo "- Specific requirement: [description]"
echo "- Expected behavior: [description]"

```

#### [UNDERSTANDING GATE]
```bash
# MANDATORY: Clear understanding before searching
```

**Expected gate output:**
```
✅ GATE 'Understanding' - PASSED
   Verified: Steps 1,2
```

### Stage 2: Discovery (Steps 3-5)

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
```

**Expected gate output:**
```
✅ GATE 'Discovery' - PASSED
   Verified: Steps 3,4,5
```

### Stage 3: Analysis (Steps 6-10)

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

```

**Checkpoint 7-10: Check Modules 2-5**

```bash
# Repeat for modules 2-5
# After each check:
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

#### Step 4: Create Improvement Plan

#### Step 11: Analyze Findings

```bash
echo "Analyzing consistency findings..."
source /tmp/check-consistency-tracking.txt

echo "Summary of findings:"
echo "- Total modules checked: $MODULES_CHECKED"
echo "- Modules needing update: $MODULES_NEEDING_UPDATE"
echo "- Consistency rate: $((100 * (MODULES_CHECKED - MODULES_NEEDING_UPDATE) / MODULES_CHECKED))%"

```

#### Step 12: List Modules Needing Update

For modules that need the same improvement:

1. **List all modules needing update**
   ```bash
   echo "Modules requiring consistency updates:"
   echo "1. ar_[module1] - Missing error logging"
   echo "2. ar_[module2] - Has global state"
   echo "3. ar_[module3] - Lacks NULL validation"
   
      ```

#### Step 13: Estimate Effort

2. **Estimate TDD cycles required** for each
   ```bash
   echo "Effort estimation (TDD cycles):"
   echo "- ar_[module1]: 2 cycles (add error logging)"
   echo "- ar_[module2]: 3 cycles (remove global state)"
   echo "- ar_[module3]: 1 cycle (add validation)"
   echo "Total estimated effort: 6 TDD cycles"
   
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
   
      ```

#### [PLANNING GATE]
```bash
# MANDATORY: Ensure comprehensive plan
```

**Expected gate output:**
```
✅ GATE 'Planning' - PASSED
   Verified: Steps 11,12,13,14
```

### Stage 5: Documentation (Step 15)

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

```

**Expected completion output:**
```
========================================
   CHECKPOINT COMPLETION SUMMARY
========================================

📈 check-module-consistency: X/Y steps (Z%)
   [████████████████████] 100%

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