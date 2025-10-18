Run naming convention validation to check for typedef and function naming issues.

## ⚠️ CRITICAL: Let the script manage checkpoints

**DO NOT manually initialize checkpoints before running this command.** The script handles all checkpoint initialization, execution, and cleanup automatically. Just run the script and let it complete.

## Quick Start

```bash
./scripts/run-check-naming.sh
```

That's it! The script will handle everything automatically. Do not run any `make checkpoint-*` commands manually unless the script fails.

## MANDATORY KB Consultation

Before checking:
1. Search: `grep "naming\|convention\|pattern" kb/README.md`
2. Review naming standards in AGENTS.md Section 5
3. Check for KB articles about:
   - Naming convention patterns
   - Function naming standards
   - Type naming rules
4. Apply all naming standards

## CHECKPOINT WORKFLOW ENFORCEMENT

**CRITICAL**: This command MUST use checkpoint tracking for ALL execution. This command demonstrates the [Checkpoint Conditional Flow Pattern](../../../kb/checkpoint-conditional-flow-pattern.md) where naming violations trigger fix workflows. See [Checkpoint Sequential Execution Discipline](../../../kb/checkpoint-sequential-execution-discipline.md) for important requirements about sequential ordering and work verification.

## Checkpoint Tracking

This command uses checkpoint tracking via wrapper scripts to ensure systematic execution of all naming convention validation steps.

### Checkpoint Wrapper Scripts

The `run-check-naming.sh` script uses the following standardized wrapper scripts:

- **`./scripts/init-checkpoint.sh`**: Initializes or resumes checkpoint tracking
- **`./scripts/require-checkpoint.sh`**: Verifies checkpoint is ready before proceeding
- **`./scripts/gate-checkpoint.sh`**: Validates gate conditions at workflow boundaries
- **`./scripts/complete-checkpoint.sh`**: Shows completion summary and cleanup

These wrappers provide centralized checkpoint management across all commands.

## Workflow Execution

Run the complete checkpoint-based workflow:

#### [CHECKPOINT START]

```bash
./scripts/run-check-naming.sh
```

This script handles all stages of the naming convention validation process:

### What the Script Does

1. **Check Naming**: Runs `make check-naming` to validate all naming conventions
2. **Analyze Violations**: Identifies and categorizes any naming violations found
3. **Document Findings**: Reports results and next steps
4. **Checkpoint Completion**: Marks the workflow as complete

### Expected Output

#### Success State
```
Checking naming conventions...
  Checking typedef naming (ar_*_t pattern)...
  Checking function naming (ar_*__ pattern)...
  Checking static function naming (_* pattern)...
  Checking test function naming (test_*__ pattern)...
  Checking heap macro naming (AR__HEAP__* pattern)...

✅ All naming conventions are correct!
```

#### Failure States

**Typedef Naming Violation:**
```
Checking naming conventions...
  Checking typedef naming (ar_*_t pattern)...
    ERROR: Invalid typedef name: AgentData (should be ar_agent_t)
    ERROR: Invalid typedef name: MethodologyData (should be ar_methodology_t)
  Found 2 typedef naming violations

make: *** [check-naming] Error 1
```

**Function Naming Violation:**
```
Checking naming conventions...
  Checking function naming (ar_*__ pattern)...
    ERROR: Invalid function name: ar_data_create (should be ar_data__create_integer)
    ERROR: Invalid function name: ar_string_trim (should be ar_string__trim)
  Found 2 function naming violations

make: *** [check-naming] Error 1
```

**Static Function Violation:**
```
Checking naming conventions...
  Checking static function naming (_* pattern)...
    ERROR: Static function without underscore: validate_input (should be _validate_input)
    ERROR: Non-static with underscore: _ar_data__internal (should not start with _)
  Found 2 static function naming violations

make: *** [check-naming] Error 1
```

#### [CHECKPOINT END]

## Troubleshooting: Manual Checkpoint Control

Only use these commands if the script fails and you need to manually intervene:

```bash
# Check current progress (if workflow interrupted)
make checkpoint-status CMD=check-naming VERBOSE=--verbose

# Resume from a specific step (only if you know it's stuck)
make checkpoint-update CMD=check-naming STEP=N

# ONLY use this if you need to reset everything and start over
rm -f /tmp/check-naming-progress.txt
./scripts/run-check-naming.sh
```

## Minimum Requirements

**MANDATORY for successful completion:**
- [ ] Command executes without errors
- [ ] All naming conventions validated
- [ ] Violations identified and documented
- [ ] No unexpected warnings or issues

**MANDATORY**: Fix all naming violations before committing. Consistent naming prevents confusion.

**CRITICAL**: The naming conventions are:
- Typedefs: `ar_<module>_t`
- Functions: `ar_<module>__<function>`
- Static functions: `_<function>`

**Common violations to watch for** ([details](../../../kb/function-naming-state-change-convention.md)):
- Missing double underscore in function names
- Static functions without underscore prefix
- Non-static functions with underscore prefix
- Typedefs not ending in `_t`
- Using `get_` for state-changing operations (should be `take_` for ownership transfer)

For example: `ar_data_create()` should be `ar_data__create_integer()`

## Key Points

- **Enforces strict naming conventions** across entire codebase
- **Exit code 1** on any violation - blocks commits
- **Patterns checked**:
  - Typedefs: `ar_<module>_t`
  - Functions: `ar_<module>__<function>`
  - Static functions: `_<function>`
  - Test functions: `test_<module>__<test>`
  - Heap macros: `AR__HEAP__<OPERATION>`