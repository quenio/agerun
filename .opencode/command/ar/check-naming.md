Run naming convention validation to check for typedef and function naming issues.

## MANDATORY KB Consultation

Before checking:
1. Search: `grep "naming\|convention\|pattern" kb/README.md`
2. Review naming standards in AGENTS.md Section 5
3. Check for KB articles about:
   - Naming convention patterns
   - Function naming standards
   - Type naming rules
4. Apply all naming standards

# Check Naming Conventions
## Checkpoint Tracking

This command uses checkpoint tracking to ensure systematic execution and verification.

### Initialize Tracking
```bash
# Start the check naming process
make checkpoint-init CMD=check_naming STEPS='"Prepare" "Execute" "Verify"'
```

**Expected output:**
```
📍 Starting: check_naming (3 steps)
📁 Tracking: /tmp/check_naming_progress.txt
→ Run: make checkpoint-update CMD=check_naming STEP=1
```

### Check Progress
```bash
make checkpoint-status CMD=check_naming
```

**Expected output (example at 33% completion):**
```
📈 command: X/Y steps (Z%)
   [████░░░░░░░░░░░░░░░░] Z%
→ Next: make checkpoint-update CMD=command STEP=N
```

## Minimum Requirements

**MANDATORY for successful completion:**
- [ ] Command executes without errors
- [ ] Expected output is produced
- [ ] No unexpected warnings or issues




**MANDATORY**: Fix all naming violations before committing. Consistent naming prevents confusion.

**CRITICAL**: The naming conventions are:
- Typedefs: `ar_<module>_t`
- Functions: `ar_<module>__<function>`
- Static functions: `_<function>`

**Common violations to watch for**:
- Missing double underscore in function names
- Static functions without underscore prefix
- Non-static functions with underscore prefix
- Typedefs not ending in `_t`

For example: `ar_data_create()` should be `ar_data__create_integer()`

#### [EXECUTION GATE]
```bash
# Verify ready to execute
make checkpoint-gate CMD=check_naming GATE="Ready" REQUIRED="1"
```

**Expected gate output:**
```
✅ GATE 'Ready' - PASSED
   Verified: Steps 1
```

## Command

#### [CHECKPOINT START - EXECUTION]

```bash
make check-naming

# Mark execution complete
make checkpoint-update CMD=check_naming STEP=2
```


#### [CHECKPOINT END - EXECUTION]
## Expected Output

### Success State
```
Checking naming conventions...
  Checking typedef naming (ar_*_t pattern)...
  Checking function naming (ar_*__ pattern)...
  Checking static function naming (_* pattern)...
  Checking test function naming (test_*__ pattern)...
  Checking heap macro naming (AR__HEAP__* pattern)...

All naming conventions are correct!
```

### Failure States

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


#### [CHECKPOINT COMPLETE]
```bash
# Show final summary
make checkpoint-status CMD=check_naming
```

**Expected completion output:**
```
========================================
   CHECKPOINT STATUS: check_naming
========================================

Progress: 3/3 steps (100%)

[████████████████████] 100%

✅ ALL CHECKPOINTS COMPLETE!

Summary:
  Prepare: ✓ Complete
  Execute: ✓ Complete  
  Verify: ✓ Complete

The check naming completed successfully!
```

```bash
# Clean up tracking
make checkpoint-cleanup CMD=check_naming
```

## Key Points

- **Enforces strict naming conventions** across entire codebase
- **Exit code 1** on any violation - blocks commits
- **Patterns checked**:
  - Typedefs: `ar_<module>_t`
  - Functions: `ar_<module>__<function>`
  - Static functions: `_<function>`
  - Test functions: `test_<module>__<test>`
  - Heap macros: `AR__HEAP__<OPERATION>`