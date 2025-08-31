Run thread sanitizer on the executable for detecting data races.


# Thread Sanitizer Executable
## Command
```bash
make tsan-exec 2>&1
```

## Expected Output

### Success State
```
Building executable with ThreadSanitizer (clang)...
Running agerun with thread sanitizer...
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

ThreadSanitizer: reported 0 warnings
```

### Failure States

**Data Race in Agent Communication:**
```
Building executable with ThreadSanitizer (clang)...
Running agerun with thread sanitizer...
... (startup messages)

==================
WARNING: ThreadSanitizer: data race (pid=12345)
  Write of size 8 at 0x7b0400000800 by thread T1:
    #0 ar_agent__send ar_agent.c:234
    #1 process_message ar_executable.c:156

  Previous read of size 8 at 0x7b0400000800 by thread T2:
    #0 ar_agent__receive ar_agent.c:198
    #1 agent_loop ar_executable.c:187

  Location is heap block of size 128 at 0x7b0400000800 allocated by main thread:
    #0 malloc
    #1 ar_agent__create ar_agent.c:45
    #2 main ar_executable.c:234

SUMMARY: ThreadSanitizer: data race ar_agent.c:234
==================
ThreadSanitizer: reported 1 warnings
make: *** [tsan-exec] Error 66
```

**Race Condition in Shutdown:**
```
==================
WARNING: ThreadSanitizer: data race (pid=12345)
  Write of size 1 at 0x7b0400000900 by main thread:
    #0 ar_system__shutdown ar_system.c:345
    #1 main ar_executable.c:289

  Previous read of size 1 at 0x7b0400000900 by thread T1:
    #0 ar_system__is_running ar_system.c:123
    #1 message_loop ar_executable.c:167

SUMMARY: ThreadSanitizer: data race ar_system.c:345
==================
```

## Key Points

- **Currently single-threaded** so no races expected
- **Future multi-threading** will require synchronization
- **Uses clang compiler** for better TSAN support
- **Exit code 66** indicates thread safety issues
- **Always use `2>&1`** to capture sanitizer output