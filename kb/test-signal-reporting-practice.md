# Test Signal Reporting Best Practice

## Learning
When testing process termination, always capture and report the signal number before asserting, as this diagnostic information is critical for debugging failures, especially in CI/CD environments.

## Importance
A generic "process terminated by signal" error provides no actionable information. Knowing whether it was SIGTERM (15), SIGKILL (9), SIGSEGV (11), or another signal immediately indicates the failure mode and guides debugging efforts.

## Example
```c
#include "ar_assert.h"
#include <sys/wait.h>
#include <signal.h>

static void test_executable_termination(void) {
    // Given we run a child process
    pid_t pid = fork();
    
    if (pid > 0) {  // Parent process
        // When we wait for the child to exit
        int status;
        waitpid(pid, &status, 0);
        
        // Then check termination with signal reporting
        if (WIFSIGNALED(status)) {
            int sig = WTERMSIG(status);
            // ALWAYS report the signal before asserting
            printf("Process terminated by signal %d (%s)\n", sig, strsignal(sig));
            
            bool expected_signal = (sig == SIGTERM || sig == SIGALRM);
            AR_ASSERT(expected_signal, "Process should terminate by SIGTERM or SIGALRM");
        } else if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            printf("Process exited with code %d\n", exit_code);
            AR_ASSERT(exit_code == 0, "Process should exit successfully");
        } else {
            AR_ASSERT(false, "Process should terminate normally");
        }
    }
}
```

## Generalization
Apply diagnostic reporting before all assertions:
- **Signal numbers**: Report which signal terminated the process
- **Exit codes**: Report the actual exit code received
- **Error codes**: Report errno or system error codes
- **Unexpected values**: Report what was received vs expected
- **State information**: Report relevant system state before failure

## Implementation
```c
// Pattern 1: Report termination details
if (WIFSIGNALED(status)) {
    int sig = WTERMSIG(status);
    printf("FAIL: Terminated by signal %d\n", sig);
    AR_ASSERT(false, "Should not terminate by signal");
}

// Pattern 2: Report comparison failures
int actual = ar_data__get_integer(result);
int expected = 42;
if (actual != expected) {
    printf("FAIL: Expected %d but got %d\n", expected, actual);
}
AR_ASSERT(actual == expected, "Result should match expected value");

// Pattern 3: Report file operations
struct stat st;
int ret = stat(filename, &st);
if (ret != 0) {
    printf("FAIL: stat(%s) failed with errno %d: %s\n", 
           filename, errno, strerror(errno));
}
AR_ASSERT(ret == 0, "File should exist");

// Pattern 4: Capture context before assertions
bool valid = validate_state(state);
if (!valid) {
    printf("FAIL: Invalid state at line %d\n", __LINE__);
    dump_state(state);  // Print detailed state information
}
AR_ASSERT(valid, "State should be valid");
```

## Related Patterns
- [Process Termination Analysis](process-termination-analysis.md)
- [Evidence-based Debugging](evidence-based-debugging.md)
- [AR_ASSERT for Descriptive Failures](ar-assert-descriptive-failures.md)
- [Test Diagnostic Output Preservation](test-diagnostic-output-preservation.md)