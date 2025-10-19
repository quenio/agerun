Validate that all naming conventions follow the strict AgeRun patterns: typedefs, functions, static functions, test functions, and heap macros. This command demonstrates best practices for script extraction and orchestration: each step has its own focused script, the command file is the orchestrator showing all logic, and scripts use domain-specific naming ([extraction](../../../kb/command-helper-script-extraction-pattern.md), [orchestration](../../../kb/command-orchestrator-pattern.md), [naming](../../../kb/script-domain-naming-convention.md)).

## ⚠️ CRITICAL: Follow all steps sequentially

**DO NOT skip steps or manually initialize checkpoints.** Execute each step in order. The checkpoint system enforces sequential execution and prevents jumping ahead.

## MANDATORY KB Consultation

Before checking:
1. Search: `grep "naming\|convention\|pattern" kb/README.md`
2. Review naming standards in AGENTS.md Section 5
3. Check for KB articles about:
   - Naming convention patterns ([details](../../../kb/function-naming-state-change-convention.md))
   - Function naming standards
   - Type naming rules
4. Apply all naming standards

## CHECKPOINT WORKFLOW ENFORCEMENT

**CRITICAL**: This command MUST use checkpoint tracking for ALL execution. This command demonstrates the [Checkpoint Conditional Flow Pattern](../../../kb/checkpoint-conditional-flow-pattern.md) where naming violations trigger analysis workflows. See [Checkpoint Sequential Execution Discipline](../../../kb/checkpoint-sequential-execution-discipline.md) for important requirements about sequential ordering and work verification.

## Checkpoint Tracking

This command uses checkpoint tracking to ensure systematic execution of all naming convention validation steps.

### Initialize Tracking

```bash
# Start the naming check process with 3 steps
./scripts/checkpoint-init.sh check-naming "Check Naming" "Analyze Violations" "Complete"
./scripts/checkpoint-require.sh check-naming
```

**Expected output:**
```
✅ Checkpoint tracking initialized for check-naming
📍 Starting: check-naming (3 steps)
📁 Tracking: /tmp/check-naming-progress.txt
```

### Check Progress

Check current progress at any time:

```bash
./scripts/checkpoint-status.sh check-naming
```

**Expected output:**
```
📈 check-naming: 2/3 steps (67%)
   [██████████░░░░░░░░░] 67%
→ Next: ./scripts/checkpoint-update.sh check-naming 3
```

## Workflow Execution

#### [CHECKPOINT START - STAGE 1]

#### Step 1: Check Naming Conventions

#### [CHECKPOINT START - STEP 1]

```bash
./scripts/run-naming-check.sh
```

Runs naming convention validation and captures violation count for conditional workflow logic.

**Expected output**: Shows validation results and saves VIOLATION_COUNT to /tmp/check-naming-stats.txt

#### [CHECKPOINT END - STEP 1]
```bash
./scripts/checkpoint-update.sh check-naming 1
```

#### [CHECKPOINT END - STAGE 1]

#### Step 2: Conditional Flow (Violation Gate)

#### [CHECKPOINT START - STEP 2]

If no violations found, skip Step 3. If violations found, continue to analysis.

```bash
./scripts/check-naming-conditional-flow.sh
```

#### [CHECKPOINT END - STEP 2]

#### [CHECKPOINT START - STAGE 2]

#### Step 3: Analyze Violations

#### [CHECKPOINT START - STEP 3]

```bash
./scripts/analyze-naming-violations.sh
```

Analyzes and categorizes any naming violations found. Only runs if violations exist.

**Expected output**: Shows detailed analysis of violations by category and next steps.

#### [CHECKPOINT END - STEP 3]
```bash
./scripts/checkpoint-update.sh check-naming 3
```

#### [CHECKPOINT END - STAGE 2]

#### [CHECKPOINT COMPLETE]

```bash
./scripts/checkpoint-complete.sh check-naming
rm -f /tmp/check-naming-*.txt
```

**Expected output:**
```
✅ Naming convention check workflow complete!
```

## Troubleshooting: Manual Checkpoint Control

Only use these commands if the script fails and you need to manually intervene:

```bash
# Check current progress (if workflow interrupted)
./scripts/checkpoint-status.sh check-naming VERBOSE=--verbose

# Resume from a specific step (only if you know it's stuck)
./scripts/checkpoint-update.sh check-naming N

# ONLY use this if you need to reset everything and start over
rm -f /tmp/check-naming-progress.txt
./scripts/checkpoint-init.sh check-naming "Check Naming" "Analyze Violations" "Document Findings"
./scripts/checkpoint-require.sh check-naming
```

## Minimum Requirements

**MANDATORY for successful completion:**
- [ ] All naming conventions validated
- [ ] Violations identified and documented
- [ ] Command executes without unexpected errors
- [ ] Git status shows clean tree (all violations fixed)

**MANDATORY**: Fix all naming violations before committing. Consistent naming prevents confusion.

**CRITICAL**: The naming conventions are:
- Typedefs: `ar_<module>_t`
- Functions: `ar_<module>__<function>`
- Static functions: `_<function>`
- Test functions: `test_<module>__<test>`
- Heap macros: `AR__HEAP__<OPERATION>`

**Common violations to watch for** ([details](../../../kb/function-naming-state-change-convention.md)):
- Missing double underscore in function names
- Static functions without underscore prefix
- Non-static functions with underscore prefix
- Typedefs not ending in `_t`
- Using `get_` for state-changing operations (should be `take_` for ownership transfer)

Example: `ar_data_create()` should be `ar_data__create_integer()`

## Key Points

- **Enforces strict naming conventions** across entire codebase
- **Exit code 1** on any violation - blocks commits
- **Patterns checked**:
  - Typedefs: `ar_<module>_t`
  - Functions: `ar_<module>__<function>`
  - Static functions: `_<function>`
  - Test functions: `test_<module>__<test>`
  - Heap macros: `AR__HEAP__<OPERATION>`
