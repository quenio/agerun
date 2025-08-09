# Executable Fixture Module

The executable fixture module (`ar_executable_fixture`) provides test infrastructure for the AgeRun executable, managing build directories and methods directories with proper isolation to prevent conflicts in parallel test execution.

## Purpose

The module was created to solve a critical problem: when running tests in parallel with different compilers (gcc for normal tests, clang for Thread Sanitizer), the shared build directories caused linker errors ("invalid control bits in libagerun.a"). The fixture provides isolated build environments for each test run.

## Key Features

### Build Directory Isolation
Each test run gets its own temporary build directory (`/tmp/agerun_test_<pid>_build`) that is:
- Created when the fixture is initialized
- Used for all executable builds during that test run  
- Automatically cleaned up when the fixture is destroyed
- Isolated from other test runs to prevent compiler conflicts

### Methods Directory Management
The fixture manages temporary copies of the methods directory:
- Creates isolated copies at `/tmp/agerun_test_<pid>_methods`
- Allows tests to modify method files without affecting other tests
- Automatically cleans up after each test
- Returns dynamically allocated paths with clear ownership

### Process Execution
The fixture builds and runs the executable:
- Uses make with appropriate environment variables
- Captures output via popen for test verification
- Handles build failures gracefully
- Returns FILE* for reading executable output

## API Functions

### Creation and Destruction

```c
ar_executable_fixture_t* ar_executable_fixture__create(void);
```
Creates a new fixture and initializes the temporary build directory.

```c
void ar_executable_fixture__destroy(ar_executable_fixture_t *own_fixture);
```
Destroys the fixture and cleans up all temporary directories.

### Methods Directory Management

```c
char* ar_executable_fixture__create_methods_dir(ar_executable_fixture_t *mut_fixture);
```
Creates a temporary methods directory and copies all method files into it. Returns an owned string that must be destroyed with `ar_executable_fixture__destroy_methods_dir()`.

```c
void ar_executable_fixture__destroy_methods_dir(ar_executable_fixture_t *mut_fixture, 
                                                char *own_methods_dir);
```
Destroys the temporary methods directory and frees the path string.

### Executable Execution

```c
FILE* ar_executable_fixture__build_and_run(const ar_executable_fixture_t *ref_fixture,
                                           const char *ref_methods_dir);
```
Builds and runs the executable with the specified methods directory. Returns a FILE* from popen() that must be closed with pclose().

## Usage Example

```c
// Create fixture for test run
ar_executable_fixture_t *own_fixture = ar_executable_fixture__create();

// Create isolated methods directory
char *own_methods_dir = ar_executable_fixture__create_methods_dir(own_fixture);

// Build and run executable
FILE *pipe = ar_executable_fixture__build_and_run(own_fixture, own_methods_dir);
if (pipe != NULL) {
    // Read output from executable
    char buffer[512];
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        // Process output...
    }
    pclose(pipe);
}

// Clean up methods directory
ar_executable_fixture__destroy_methods_dir(own_fixture, own_methods_dir);

// Destroy fixture (also cleans up build directory)
ar_executable_fixture__destroy(own_fixture);
```

## Design Principles

### Opaque Type
The fixture structure is opaque, following Parnas principles. Clients cannot access internal fields directly.

### Clear Ownership
All ownership is clearly documented:
- Functions returning pointers transfer ownership
- The `own_` prefix indicates owned values
- The `mut_` prefix indicates mutable references
- The `ref_` prefix indicates read-only references

### Resource Management
The module ensures proper cleanup:
- Temporary directories are removed even if tests fail
- Memory is tracked with AR__HEAP__ macros
- NULL parameters are handled gracefully

## Testing

The module has comprehensive tests in `ar_executable_fixture_tests.c` that verify:
- Fixture creation and destruction
- Build directory creation and cleanup
- Methods directory copying and cleanup
- Multiple methods directories handling
- Build and run functionality
- NULL parameter handling

All tests pass with zero memory leaks.

## Dependencies

- `ar_heap`: For memory tracking
- `ar_assert`: For test assertions
- System libraries: stdio, stdlib, unistd, sys/types

## Implementation Notes

The module uses process IDs to create unique temporary directories, ensuring complete isolation between parallel test runs. This prevents the "invalid control bits" error that occurred when Thread Sanitizer (clang) and regular tests (gcc) shared build artifacts.

The fixture does not cache builds - each test gets a fresh build to ensure test independence. While this adds some overhead, it guarantees that tests are completely isolated and reproducible.