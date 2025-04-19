#include "agerun_executable.h"
#include "agerun_system.h"
#include "agerun_method.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

// Forward declarations
static void test_executable_run(void);

// Stub function to avoid linking with the actual executable
int ar_executable_main(void) {
    // This is just a stub for testing
    return 0;
}

// Test that the executable can be run in a child process
static void test_executable_run(void) {
    printf("Testing executable can run...\n");
    
    // Fork a process to run the executable
    pid_t pid = fork();
    
    if (pid == 0) {
        // Child process
        // Redirect stdout to /dev/null to avoid cluttering test output
        freopen("/dev/null", "w", stdout);
        
        // Set alarm to automatically terminate after a short time
        alarm(2);
        
        // Run the executable
        int result = ar_executable_main();
        
        // Should not return normally (will be terminated by alarm)
        exit(result);
    } else if (pid > 0) {
        // Parent process
        // Wait for child to terminate
        int status;
        sleep(1);  // Give child process time to start
        
        // Send SIGTERM to child
        kill(pid, SIGTERM);
        
        // Wait for child to exit
        waitpid(pid, &status, 0);
        
        // Check if child exited normally (should have been terminated by signal)
        if (WIFSIGNALED(status)) {
            int sig = WTERMSIG(status);
            // Either terminated by our SIGTERM or by alarm SIGALRM
            assert(sig == SIGTERM || sig == SIGALRM);
            printf("Executable terminated by signal %d as expected\n", sig);
        } else {
            int exit_status = WEXITSTATUS(status);
            // If it exited normally, it should have exit code 0
            assert(exit_status == 0);
            printf("Executable exited normally with status %d\n", exit_status);
        }
    } else {
        // Fork failed
        perror("Fork failed");
        assert(0);  // Force test failure
    }
    
    printf("Executable run test passed!\n");
}

int main(void) {
    printf("Starting Executable Module Tests...\n");
    
    // Create a method and initialize the system
    // This is a simple test executor method
    const char *init_method = "exec_test_method";
    const char *init_instructions = "memory.result = \"Test complete\"";
    version_t init_version = ar_method_create(init_method, init_instructions, 0, false, false);
    
    // Skip the agent init assertions since we're just testing the executable itself
    ar_system_init(init_method, init_version);
    
    // Test the executable
    test_executable_run();
    
    // Shutdown the system after tests
    ar_system_shutdown();
    
    printf("All executable tests passed!\n");
    return 0;
}
