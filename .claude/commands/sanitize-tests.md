Run address sanitizer on all tests for detecting memory issues.

## Command
```bash
make sanitize-tests 2>&1
```

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

## Key Points

- **AddressSanitizer (ASAN)** detects memory leaks, use-after-free, buffer overflows
- **dlsym tests excluded** automatically to prevent conflicts
- **~2x slower** than normal tests but catches critical issues
- **Exit on first error** - fix immediately before continuing

**Important**: Tests using dlsym for function interception (named `*_dlsym_tests`) are automatically excluded from sanitizer builds to prevent conflicts. These tests still run in normal builds. ([details](../../kb/sanitizer-test-exclusion-pattern.md), [technique](../../kb/dlsym-test-interception-technique.md))

**Test Validation**: When testing error handling, use permission-based failure testing with chmod to create predictable failures ([details](../../kb/permission-based-failure-testing.md)). Always verify test effectiveness by temporarily breaking the implementation ([details](../../kb/test-effectiveness-verification.md)).