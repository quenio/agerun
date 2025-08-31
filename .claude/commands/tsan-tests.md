Run thread sanitizer on all tests for detecting data races.

## Command
```bash
make tsan-tests 2>&1
```

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

## Key Points

- **ThreadSanitizer (TSAN)** detects data races and deadlocks
- **Uses clang compiler** instead of gcc for better TSAN support
- **dlsym tests excluded** automatically to prevent conflicts
- **~10x slower** than normal tests but catches concurrency bugs
- **Exit code 66** indicates thread safety issues detected