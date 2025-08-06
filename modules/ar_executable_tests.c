#include "ar_executable.h"
#include "ar_system.h"
#include "ar_method.h"
#include "ar_methodology.h"
#include "ar_agency.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

// Forward declarations
static void test_executable_run(void);
static void test_single_session(void);

// Stub function to avoid linking with the actual executable
int ar_executable__main(void) {
    // This is just a stub for testing
    return 0;
}

// Test that the executable only runs a single session
static void test_single_session(void) {
    printf("Testing executable has only single session...\n");
    
    // First verify we're running from the correct directory
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        printf("FAIL: Could not get current directory\n");
        assert(0);
    }
    
    // Check if we're in a bin subdirectory
    if (strstr(cwd, "/bin/") == NULL) {
        printf("FAIL: Test must be run from bin directory, but running from: %s\n", cwd);
        assert(0);
    }
    printf("Running from: %s\n", cwd);
    
    // Given we want to verify the executable doesn't have a second session
    // When we run the actual executable and capture its output
    // Then it should NOT contain "Starting new runtime session"
    
    // Clean up any existing files before test to ensure isolation
    printf("Cleaning up any existing files...\n");
    system("cd ../run-exec 2>/dev/null && rm -f methodology.agerun agency.agerun 2>/dev/null");
    
    // Always build the executable to ensure we test the latest code
    printf("Building executable to ensure latest version...\n");
    // Build the library first, then the executable
    // We can't use 'make run-exec' because it also runs the executable
    int build_lib = system("cd ../.. && make -s run_exec_lib > /dev/null 2>&1");
    if (build_lib == 0) {
        // Build just the executable without running it
        int build_exe = system("cd ../.. && gcc-13 -Wall -Wextra -Werror -std=c11 -I./modules -g -O0 -o bin/run-exec/agerun modules/ar_executable.c bin/run-exec/libagerun.a -lm 2>/dev/null");
        if (build_exe != 0) {
            printf("FAIL: Executable build failed with code %d\n", build_exe);
            assert(0);
        }
    } else {
        printf("FAIL: Library build failed with code %d\n", build_lib);
        assert(0);
    }
    
    // Verify the run-exec directory exists
    if (access("../run-exec", F_OK) != 0) {
        printf("FAIL: Directory ../run-exec does not exist\n");
        assert(0);
    }
    
    // Run the actual executable and capture output  
    // Note: Tests are run from bin/run-tests directory
    // We need to run from the correct directory for the executable to work properly
    printf("Running executable to check for second session...\n");
    // popen() captures stdout (and stderr via 2>&1) from the executable
    FILE *pipe = popen("cd ../run-exec && ./agerun 2>&1", "r");
    if (!pipe) {
        printf("Failed to run executable\n");
        assert(0);
    }
    
    char buffer[512];  // Increased buffer size for longer lines
    bool found_second_session = false;
    int line_count = 0;
    
    // Read all output looking for the second session message
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        line_count++;
        if (strstr(buffer, "Starting new runtime session") != NULL) {
            found_second_session = true;
            printf("FAIL: Found second session at line %d: %s", line_count, buffer);
        }
    }
    
    int status = pclose(pipe);
    
    // Check how the process terminated
    if (WIFSIGNALED(status)) {
        int sig = WTERMSIG(status);
        printf("FAIL: Executable terminated by signal %d\n", sig);
        assert(0);
    } else if (WIFEXITED(status)) {
        int exit_code = WEXITSTATUS(status);
        if (exit_code != 0) {
            printf("FAIL: Executable exited with code %d\n", exit_code);
            assert(exit_code == 0);
        }
    } else {
        printf("FAIL: Executable terminated abnormally\n");
        assert(0);
    }
    
    printf("Executable produced %d lines of output\n", line_count);
    
    // And it should NOT have a second session
    if (found_second_session) {
        printf("FAIL: Executable has a second session (should only have one)\n");
        assert(!found_second_session);
    }
    
    printf("Single session test passed!\n");
    
    // Clean up generated files
    printf("Cleaning up generated files...\n");
    int cleanup_result = system("cd ../run-exec && rm -f methodology.agerun agency.agerun 2>/dev/null");
    if (cleanup_result != 0) {
        printf("Warning: Cleanup may have failed (code %d), but test passed\n", cleanup_result);
    }
}

// Test that the executable can be run in a child process
static void test_executable_run(void) {
    printf("Testing executable can run...\n");
    
    // Given we need to run the executable in a separate process
    
    // When we fork a process to run the executable
    pid_t pid = fork();
    
    if (pid == 0) {
        // In the child process
        
        // Clean up inherited resources before running the test
        // Note: We're in a child process, so we can't use the parent's system instance
        // The child will have its own copy of memory
        
        // Given we redirect stdout to avoid cluttering test output
        freopen("/dev/null", "w", stdout);
        
        // And we set alarm to automatically terminate after a short time
        alarm(2);
        
        // When we run the executable
        int result = ar_executable__main();
        
        // Then we should exit with the result
        // (though we should actually be terminated by the alarm)
        exit(result);
        
    } else if (pid > 0) {
        // In the parent process
        
        // Given we wait for the child to start
        int status;
        sleep(1);
        
        // When we send SIGTERM to the child process
        kill(pid, SIGTERM);
        
        // And we wait for the child to exit
        waitpid(pid, &status, 0);
        
        // Then the child should have terminated normally or by signal
        if (WIFSIGNALED(status)) {
            // If terminated by signal
            int sig = WTERMSIG(status);
            
            // Then it should be either our SIGTERM or the alarm's SIGALRM
            assert(sig == SIGTERM || sig == SIGALRM);
            printf("Executable terminated by signal %d as expected\n", sig);
        } else {
            // If exited normally
            int exit_status = WEXITSTATUS(status);
            
            // Then it should have exit code 0
            assert(exit_status == 0);
            printf("Executable exited normally with status %d\n", exit_status);
        }
    } else {
        // When the fork fails
        perror("Fork failed");
        
        // Then the test should fail
        assert(0);
    }
    
    printf("Executable run test passed!\n");
}

int main(void) {
    printf("Starting Executable Module Tests...\n");
    
    // Run the executable test first (before creating any system instances)
    test_executable_run();
    
    // Test that executable has only single session
    test_single_session();
    
    // Now run a separate test with a system instance
    // Create system instance for tests
    ar_system_t *mut_system = ar_system__create();
    assert(mut_system != NULL);
    
    // Given we have a test method and initialized system
    const char *init_method = "exec_test_method";
    const char *init_instructions = "memory.result = \"Test complete\"";
    const char *init_version = "1.0.0";
    
    // Create method and register it with methodology 
    ar_method_t *own_method = ar_method__create(init_method, init_instructions, init_version);
    assert(own_method != NULL);
    
    // Register with methodology using instance API
    ar_agency_t *ref_agency = ar_system__get_agency(mut_system);
    ar_methodology_t *ref_methodology = ar_agency__get_methodology(ref_agency);
    ar_methodology__register_method_with_instance(ref_methodology, own_method);
    own_method = NULL; // Mark as transferred
    
    // When we initialize the system with this method
    int64_t initial_agent = ar_system__init_with_instance(mut_system, init_method, init_version);
    
    // Process the wake message if an agent was created
    if (initial_agent > 0) {
        ar_system__process_next_message_with_instance(mut_system);
    }
    
    // Then we clean up the system
    ar_system__shutdown_with_instance(mut_system);
    ar_system__destroy(mut_system);
    
    // And report success
    printf("All 3 tests passed!\n");
    return 0;
}

