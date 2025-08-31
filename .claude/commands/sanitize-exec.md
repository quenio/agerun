Run address sanitizer on the executable for detecting memory issues.

## Command
```bash
make sanitize-exec 2>&1
```

## Expected Output

### Success State
```
Building executable with AddressSanitizer...
Running agerun with sanitizer...
Loading methods from directory: methods/
  Loaded: bootstrap-1.0.0.method
  Loaded: echo-1.0.0.method
  ... (6 more methods)
Creating bootstrap agent (ID: 1)
Processing messages...
  Agent 1 spawned echo agent (ID: 2)
  Agent 2 processed message
No more messages to process
Saving methodology to agerun.methodology
Shutdown complete

=================================================================
==12345==ERROR: LeakSanitizer: detected memory leaks

... (expected methodology leaks - known issue)

SUMMARY: AddressSanitizer: 1024 byte(s) leaked in 8 allocation(s).
```

### Failure States

**Use-After-Free:**
```
Building executable with AddressSanitizer...
Running agerun with sanitizer...
... (startup messages)

=================================================================
==12345==ERROR: AddressSanitizer: heap-use-after-free
READ of size 8 at 0x60400000eff0 thread T0
    #0 0x4... in ar_agent__send ar_agent.c:234
    #1 0x4... in main ar_executable.c:156

freed by thread T0 here:
    #0 0x7f... in free
    #1 0x4... in ar_agent__destroy ar_agent.c:98

SUMMARY: AddressSanitizer: heap-use-after-free ar_agent.c:234
make: *** [sanitize-exec] Error 1
```

**Double Free:**
```
=================================================================
==12345==ERROR: AddressSanitizer: attempting double-free
    #0 0x7f... in free
    #1 0x4... in ar_data__destroy ar_data.c:456
    #2 0x4... in ar_agent__destroy ar_agent.c:102

previously allocated by thread T0 here:
    #0 0x7f... in malloc
    #1 0x4... in ar_data__create_string ar_data.c:123

SUMMARY: AddressSanitizer: double-free
```

## Key Points

- **Known issue**: Methodology leaks are expected (not yet fixed)
- **Critical errors**: Use-after-free and double-free stop execution
- **Always use `2>&1`** to capture sanitizer output
- **Slower execution** but catches runtime memory errors