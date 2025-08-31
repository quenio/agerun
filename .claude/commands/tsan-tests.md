Run thread sanitizer on all tests for detecting data races.


# Thread Sanitizer Tests
## Checkpoint Tracking

This command uses checkpoint tracking to ensure systematic execution and verification.

### Initialize Tracking
```bash
# Start the tsan tests process
make checkpoint-init CMD=tsan_tests STEPS='"Prepare" "Execute" "Verify"'
```

**Expected output:**
```
========================================
   CHECKPOINT TRACKING INITIALIZED
========================================

Command: tsan_tests
Tracking file: /tmp/tsan_tests_progress.txt
Total steps: 3

Steps to complete:
  1. Prepare
  2. Execute
  3. Verify

Goal: Complete tsan tests successfully
```

### Check Progress
```bash
make checkpoint-status CMD=tsan_tests
```

**Expected output (example at 33% completion):**
```
========================================
   CHECKPOINT STATUS: tsan_tests
========================================

Progress: 1/3 steps (33%)

[██████░░░░░░░░░░░░] 33%

Current Status: Preparing...

Next Action:
  → Step 2: Execute
```

## Minimum Requirements

**MANDATORY for successful completion:**
- [ ] Command executes without errors
- [ ] Expected output is produced
- [ ] No unexpected warnings or issues




**MANDATORY**: Design tests with thread safety in mind, even if currently single-threaded.

**CRITICAL**: TSAN in tests helps validate thread-safe design patterns:
- Proper mutex usage
- Atomic operations
- Lock-free data structures

**Test design questions**:
- Could this test expose race conditions?
- Are we testing thread-safe interfaces?
- Do fixtures need synchronization?

For example, concurrent agent tests will need careful synchronization.

#### [EXECUTION GATE]
```bash
# Verify ready to execute
make checkpoint-gate CMD=tsan_tests GATE="Ready" REQUIRED="1"
```

**Expected gate output:**
```
========================================
   GATE: Ready
========================================

✅ GATE PASSED: Ready to execute!

Prerequisites verified:
  ✓ Environment prepared
  ✓ Dependencies available
  
Proceed with execution.
```

## Command

#### [CHECKPOINT START - EXECUTION]

```bash
make tsan-tests 2>&1

# Mark execution complete
make checkpoint-update CMD=tsan_tests STEP=2
```


#### [CHECKPOINT END - EXECUTION]
## Expected Output

### Success State
```
Building with ThreadSanitizer (clang)...
Running tests with thread sanitizer...
  ✓ ar_agent_tests (TSAN)
  ✓ ar_agency_tests (TSAN)
  ✓ ar_data_tests (TSAN)
  ... (65 more tests)
All 68 tests passed with ThreadSanitizer!
No data races detected.
```

### Failure States

**Data Race Detection:**
```
Building with ThreadSanitizer (clang)...
Running tests with thread sanitizer...
  ✗ ar_agent_tests (TSAN)

==================
WARNING: ThreadSanitizer: data race (pid=12345)
  Write of size 8 at 0x7b0400000800 by thread T1:
    #0 ar_agent__set_id ar_agent.c:123
    #1 test_concurrent_access ar_agent_tests.c:456

  Previous read of size 8 at 0x7b0400000800 by thread T2:
    #0 ar_agent__get_id ar_agent.c:87
    #1 test_concurrent_access ar_agent_tests.c:467

  Thread T1 (tid=12346, running) created by main thread at:
    #0 pthread_create
    #1 test_concurrent_access ar_agent_tests.c:445

  Thread T2 (tid=12347, running) created by main thread at:
    #0 pthread_create
    #1 test_concurrent_access ar_agent_tests.c:450

SUMMARY: ThreadSanitizer: data race ar_agent.c:123
==================
make: *** [tsan-tests] Error 66
```

**Mutex Deadlock:**
```
==================
WARNING: ThreadSanitizer: lock-order-inversion (potential deadlock)
  Cycle in lock order graph: M1 => M2 => M1

  Mutex M2 acquired here while holding mutex M1:
    #0 pthread_mutex_lock
    #1 ar_agent__update ar_agent.c:234

  Mutex M1 previously acquired by the same thread here:
    #0 pthread_mutex_lock
    #1 ar_agent__send ar_agent.c:198

SUMMARY: ThreadSanitizer: lock-order-inversion (potential deadlock)
```


#### [CHECKPOINT COMPLETE]
```bash
# Show final summary
make checkpoint-status CMD=tsan_tests
```

**Expected completion output:**
```
========================================
   CHECKPOINT STATUS: tsan_tests
========================================

Progress: 3/3 steps (100%)

[████████████████████] 100%

✅ ALL CHECKPOINTS COMPLETE!

Summary:
  Preparation: ✓ Complete
  Execution: ✓ Complete  
  Verification: ✓ Complete

The tsan tests completed successfully!
```

```bash
# Clean up tracking
make checkpoint-cleanup CMD=tsan_tests
```

## Key Points

- **ThreadSanitizer (TSAN)** detects data races and deadlocks
- **Uses clang compiler** instead of gcc for better TSAN support
- **dlsym tests excluded** automatically to prevent conflicts
- **~10x slower** than normal tests but catches concurrency bugs
- **Exit code 66** indicates thread safety issues detected