# Static Analysis Error Handling Requirements

## Learning
Static analyzers like scan-build require checking return values of system calls that can fail. Functions like dup(), pipe(), and dup2() return -1 on error, and using these values without checking can lead to undefined behavior.

## Importance
Unchecked error conditions can cause crashes, security vulnerabilities, or silent failures. Static analysis tools catch these issues at build time, preventing runtime problems. Even test code needs proper error handling to avoid false positives in analysis.

## Example
```c
// WRONG: Unchecked return value
static void test_with_bad_error_handling(void) {
    int original_stdout = dup(1);  // Could return -1
    int pipefd[2];
    pipe(pipefd);  // Could fail
    dup2(pipefd[1], 1);  // Using potentially invalid fd
    // Static analyzer warning: The 1st argument to 'dup2' is -1
}

// CORRECT: Proper error checking
static void test_with_proper_error_handling(void) {
    int original_stdout = dup(1);
    if (original_stdout == -1) {
        printf("FAIL: Could not duplicate stdout\n");
        assert(0);
    }
    
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        close(original_stdout);
        printf("FAIL: Could not create pipe\n");
        assert(0);
    }
    
    if (dup2(pipefd[1], 1) == -1) {
        close(original_stdout);
        close(pipefd[0]);
        close(pipefd[1]);
        printf("FAIL: Could not redirect stdout\n");
        assert(0);
    }
    // No static analyzer warnings
}
```

## Generalization
System call error handling patterns:
1. **Check return values**: -1 typically indicates error
2. **Clean up resources**: Close any opened descriptors on failure
3. **Report clearly**: Indicate which operation failed
4. **Fail appropriately**: Use assert() in tests, return codes in production
5. **Handle cascading**: Each failure may require different cleanup

## Implementation
```c
// Error handling macro for tests
#define CHECK_SYSCALL(call, cleanup) \
    do { \
        if ((call) == -1) { \
            cleanup; \
            printf("FAIL: %s failed at %s:%d\n", #call, __FILE__, __LINE__); \
            assert(0); \
        } \
    } while(0)

// Usage example
void test_with_macro(void) {
    int fd = -1;
    int pipefd[2] = {-1, -1};
    
    CHECK_SYSCALL(fd = dup(1), );
    CHECK_SYSCALL(pipe(pipefd), close(fd));
    CHECK_SYSCALL(dup2(pipefd[1], 1), 
                  do { close(fd); close(pipefd[0]); close(pipefd[1]); } while(0));
    
    // Continue with valid descriptors...
}

// Production error handling
bool safe_redirect_stdout(int *original_fd, int *pipe_read_fd) {
    *original_fd = dup(1);
    if (*original_fd == -1) {
        ar_io__warning("Failed to duplicate stdout: %s\n", strerror(errno));
        return false;
    }
    
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        ar_io__warning("Failed to create pipe: %s\n", strerror(errno));
        close(*original_fd);
        return false;
    }
    
    if (dup2(pipefd[1], 1) == -1) {
        ar_io__warning("Failed to redirect stdout: %s\n", strerror(errno));
        close(*original_fd);
        close(pipefd[0]);
        close(pipefd[1]);
        return false;
    }
    
    close(pipefd[1]);
    *pipe_read_fd = pipefd[0];
    return true;
}
```

## Related Patterns
- [Build System Exit Code Verification](build-system-exit-code-verification.md)
- [Zig Static Analysis Tools](zig-static-analysis-tools.md)
- [Development Debug Tools](development-debug-tools.md)