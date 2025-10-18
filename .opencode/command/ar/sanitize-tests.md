Run address sanitizer on all tests for detecting memory issues.

## CHECKPOINT WORKFLOW ENFORCEMENT

**CRITICAL**: This command MUST use checkpoint tracking for ALL execution.

### In-Progress Workflow Detection

If a `/sanitize-tests` workflow is already in progress:

```bash
make checkpoint-status CMD=sanitize_tests VERBOSE=--verbose
# Resume: make checkpoint-update CMD=sanitize_tests STEP=N
# Or reset: make checkpoint-cleanup CMD=sanitize_tests && make checkpoint-init CMD=sanitize_tests STEPS='"Prepare" "Execute" "Verify"'
```

### First-Time Initialization Check

```bash
./scripts/init-checkpoint.sh sanitize-tests '"Prepare" "Execute" "Verify"'
```

## PRECONDITION: Checkpoint Tracking Must Be Initialized

```bash
./scripts/require-checkpoint.sh sanitize-tests
```

# Sanitize Tests
## Checkpoint Tracking

This command uses checkpoint tracking to ensure systematic execution and verification.

### Initialize Tracking
```bash
# Start the sanitize tests process
make checkpoint-init CMD=sanitize_tests STEPS='"Prepare" "Execute" "Verify"'
```

**Expected output:**
```
📍 Starting: sanitize_tests (3 steps)
📁 Tracking: /tmp/sanitize_tests_progress.txt
→ Run: make checkpoint-update CMD=sanitize_tests STEP=1
```

### Check Progress
```bash
make checkpoint-status CMD=sanitize_tests
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



#### [EXECUTION GATE]
```bash
# Verify ready to execute
make checkpoint-gate CMD=sanitize_tests GATE="Ready" REQUIRED="1"
```

**Expected gate output:**
```
✅ GATE 'Ready' - PASSED
   Verified: Steps 1
```

## Command

#### [CHECKPOINT START - EXECUTION]

```bash
make sanitize-tests 2>&1

# Mark execution complete
make checkpoint-update CMD=sanitize_tests STEP=2
```


#### [CHECKPOINT END - EXECUTION]
## Expected Output

### Success State
```
Building with AddressSanitizer...
Running tests with sanitizer...
  ✓ ar_agent_tests (ASAN)
  ✓ ar_agency_tests (ASAN)
  ✓ ar_data_tests (ASAN)
  ... (65 more tests)
All 68 tests passed with AddressSanitizer!
No memory errors detected.
```

### Failure States

**Memory Leak Detection:**
```
Building with AddressSanitizer...
Running tests with sanitizer...
  ✓ ar_agent_tests (ASAN)
  ✗ ar_data_tests (ASAN)

=================================================================
==12345==ERROR: LeakSanitizer: detected memory leaks

Direct leak of 24 byte(s) in 1 object(s) allocated from:
    #0 0x7f... in malloc
    #1 0x4... in ar_data__create_string ar_data.c:123
    #2 0x4... in test_data__string_creation ar_data_tests.c:45

SUMMARY: AddressSanitizer: 24 byte(s) leaked in 1 allocation(s).
make: *** [sanitize-tests] Error 1
```

**Use-After-Free:**
```
=================================================================
==12345==ERROR: AddressSanitizer: heap-use-after-free
READ of size 8 at 0x60400000eff0 thread T0
    #0 0x4... in ar_agent__get_id ar_agent.c:87
    #1 0x4... in test_agent__use_after_destroy ar_agent_tests.c:123

freed by thread T0 here:
    #0 0x7f... in free
    #1 0x4... in ar_agent__destroy ar_agent.c:65

SUMMARY: AddressSanitizer: heap-use-after-free ar_agent.c:87
```

**Buffer Overflow:**
```
=================================================================
==12345==ERROR: AddressSanitizer: heap-buffer-overflow
WRITE of size 1 at 0x60200000eff1 thread T0
    #0 0x4... in strcpy
    #1 0x4... in ar_string__copy ar_string.c:45

SUMMARY: AddressSanitizer: heap-buffer-overflow
```


#### [CHECKPOINT COMPLETE]
```bash
# Show final summary
make checkpoint-status CMD=sanitize_tests
```

**Expected completion output:**
```
========================================
   CHECKPOINT STATUS: sanitize_tests
========================================

Progress: 3/3 steps (100%)

[████████████████████] 100%

✅ ALL CHECKPOINTS COMPLETE!

Summary:
  Prepare: ✓ Complete
  Execute: ✓ Complete  
  Verify: ✓ Complete

The sanitize tests completed successfully!
```

```bash
# Clean up tracking
make checkpoint-cleanup CMD=sanitize_tests
```

## Key Points

- **AddressSanitizer (ASAN)** detects memory leaks, use-after-free, buffer overflows
- **dlsym tests excluded** automatically to prevent conflicts
- **~2x slower** than normal tests but catches critical issues
- **Exit on first error** - fix immediately before continuing

**Important**: Tests using dlsym for function interception (named `*_dlsym_tests`) are automatically excluded from sanitizer builds to prevent conflicts. These tests still run in normal builds. ([details](../../../kb/sanitizer-test-exclusion-pattern.md), [technique](../../../kb/dlsym-test-interception-technique.md))

**Test Validation**: When testing error handling, use permission-based failure testing with chmod to create predictable failures ([details](../../../kb/permission-based-failure-testing.md)). Always verify test effectiveness by temporarily breaking the implementation ([details](../../../kb/test-effectiveness-verification.md)).

**Mocking Guidelines**: When using dlsym for testing, mock at the appropriate level - function-level mocking is simpler and more maintainable than system call interception ([details](../../../kb/mock-at-right-level-pattern.md)). Complex test code with retry loops or state tracking indicates mocking at the wrong abstraction level ([details](../../../kb/test-complexity-as-code-smell.md)). Always check how other tests solve similar problems before creating new infrastructure ([details](../../../kb/check-existing-solutions-first.md)).

**Diagnostic Troubleshooting**: When sanitizer tools report configuration issues despite working setups, apply systematic diagnostic troubleshooting ([details](../../../kb/shell-configuration-diagnostic-troubleshooting.md)). Shell loading order affects tool behavior ([details](../../../kb/shell-loading-order-tool-detection.md)).