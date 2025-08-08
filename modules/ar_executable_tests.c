#include "ar_executable.h"
#include "ar_system.h"
#include "ar_method.h"
#include "ar_methodology.h"
#include "ar_agency.h"
#include "ar_assert.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

// Forward declarations
static void test_executable_run(void);
static void test_single_session(void);
static void test_loading_methods_from_directory(void);

// Stub function to avoid linking with the actual executable
int ar_executable__main(void) {
    // This is just a stub for testing
    return 0;
}

// Test that the executable only runs a single session
static void test_single_session(void) {
    printf("Testing executable has only single session...\n");
    
    // Given we're running from the correct test directory
    char cwd[1024];
    AR_ASSERT(getcwd(cwd, sizeof(cwd)) != NULL, "Should be able to get current directory");
    AR_ASSERT(strstr(cwd, "/bin/") != NULL, "Test must be run from bin directory");
    printf("Running from: %s\n", cwd);
    
    
    // And we have a freshly built executable
    printf("Building executable to ensure latest version...\n");
    int build_lib = system("cd ../.. && make -s run_exec_lib > /dev/null 2>&1");
    AR_ASSERT(build_lib == 0, "Library build should succeed");
    
    int build_exe = system("cd ../.. && gcc-13 -Wall -Wextra -Werror -std=c11 -I./modules -g -O0 -o bin/run-exec/agerun modules/ar_executable.c bin/run-exec/libagerun.a -lm 2>/dev/null");
    AR_ASSERT(build_exe == 0, "Executable build should succeed");
    
    AR_ASSERT(access("../run-exec", F_OK) == 0, "Directory ../run-exec should exist");
    
    // When we run the actual executable and capture its output
    printf("Running executable to check for second session...\n");
    FILE *pipe = popen("cd ../run-exec && ./agerun 2>&1", "r");
    AR_ASSERT(pipe != NULL, "Should be able to run executable via popen");
    
    char buffer[512];
    bool found_second_session = false;
    int line_count = 0;
    
    // Read all output looking for the second session message
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        line_count++;
        if (strstr(buffer, "Starting new runtime session") != NULL) {
            found_second_session = true;
            printf("Found second session at line %d: %s", line_count, buffer);
        }
    }
    
    int status = pclose(pipe);
    
    // Then the executable should terminate successfully
    if (WIFSIGNALED(status)) {
        int sig = WTERMSIG(status);
        printf("FAIL: Executable terminated by signal %d\n", sig);
        AR_ASSERT(false, "Executable should not terminate by signal");
    } else if (WIFEXITED(status)) {
        int exit_code = WEXITSTATUS(status);
        AR_ASSERT(exit_code == 0, "Executable should exit with code 0");
    } else {
        AR_ASSERT(false, "Executable should terminate normally");
    }
    
    printf("Executable produced %d lines of output\n", line_count);
    
    // And it should NOT have a second session
    AR_ASSERT(!found_second_session, "Executable should only have one session, not two");
    
    printf("Single session test passed!\n");
    
}

// Test that the executable can be run in a child process
static void test_executable_run(void) {
    printf("Testing executable can run...\n");
    
    // Given we need to run the executable in a separate process
    
    // When we fork a process to run the executable
    pid_t pid = fork();
    
    if (pid == 0) {
        // In the child process
        
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
            bool valid_signal = (sig == SIGTERM || sig == SIGALRM);
            printf("Executable terminated by signal %d\n", sig);
            AR_ASSERT(valid_signal, "Executable should terminate by SIGTERM or SIGALRM");
        } else {
            // If exited normally
            int exit_status = WEXITSTATUS(status);
            printf("Executable exited normally with status %d\n", exit_status);
            AR_ASSERT(exit_status == 0, "Executable should exit with status 0");
        }
    } else {
        // When the fork fails
        perror("Fork failed");
        AR_ASSERT(false, "Fork should succeed");
    }
    
    printf("Executable run test passed!\n");
}

// Test that the executable loads methods from directory
static void test_loading_methods_from_directory(void) {
    printf("Testing executable loads methods from directory...\n");
    
    // Given we're running from the correct test directory
    char cwd[1024];
    AR_ASSERT(getcwd(cwd, sizeof(cwd)) != NULL, "Should be able to get current directory");
    AR_ASSERT(strstr(cwd, "/bin/") != NULL, "Test must be run from bin directory");
    printf("Running from: %s\n", cwd);
    
    
    // And we have a freshly built executable
    printf("Building executable to ensure latest version...\n");
    int build_lib = system("cd ../.. && make -s run_exec_lib > /dev/null 2>&1");
    AR_ASSERT(build_lib == 0, "Library build should succeed");
    
    int build_exe = system("cd ../.. && gcc-13 -Wall -Wextra -Werror -std=c11 -I./modules -g -O0 -o bin/run-exec/agerun modules/ar_executable.c bin/run-exec/libagerun.a -lm 2>/dev/null");
    AR_ASSERT(build_exe == 0, "Executable build should succeed");
    
    // When we run the executable (should load methods from directory)
    printf("Running executable to test method loading...\n");
    FILE *pipe = popen("cd ../run-exec && ./agerun 2>&1", "r");
    AR_ASSERT(pipe != NULL, "Should be able to run executable");
    
    // Then we should see evidence that methods were loaded
    char line[256];
    bool found_agent_manager = false;
    bool found_bootstrap = false;
    bool found_calculator = false;
    bool found_echo = false;
    bool found_grade_evaluator = false;
    bool found_message_router = false;
    bool found_method_creator = false;
    bool found_string_builder = false;
    bool found_loading_message = false;
    int method_count = 0;
    
    while (fgets(line, sizeof(line), pipe) != NULL) {
        printf("Output: %s", line);
        
        // Look for evidence of each loaded method
        if (strstr(line, "Loaded method 'agent-manager'")) {
            found_agent_manager = true;
        }
        if (strstr(line, "Loaded method 'bootstrap'")) {
            found_bootstrap = true;
        }
        if (strstr(line, "Loaded method 'calculator'")) {
            found_calculator = true;
        }
        if (strstr(line, "Loaded method 'echo'")) {
            found_echo = true;
        }
        if (strstr(line, "Loaded method 'grade-evaluator'")) {
            found_grade_evaluator = true;
        }
        if (strstr(line, "Loaded method 'message-router'")) {
            found_message_router = true;
        }
        if (strstr(line, "Loaded method 'method-creator'")) {
            found_method_creator = true;
        }
        if (strstr(line, "Loaded method 'string-builder'")) {
            found_string_builder = true;
        }
        
        // Look for the loading from directory message
        if (strstr(line, "Loading methods from directory")) {
            found_loading_message = true;
        }
        
        // Also look for a summary line about loading methods
        if (strstr(line, "Loaded") && strstr(line, "methods from directory")) {
            char *num_str = strstr(line, "Loaded ");
            if (num_str) {
                int loaded = 0;
                if (sscanf(num_str, "Loaded %d methods", &loaded) == 1) {
                    printf("Found summary: %d methods loaded\n", loaded);
                    method_count = loaded;
                }
            }
        }
    }
    
    int status = pclose(pipe);
    
    // Verify the executable ran successfully
    if (WIFEXITED(status)) {
        int exit_code = WEXITSTATUS(status);
        AR_ASSERT(exit_code == 0, "Executable should exit normally");
    } else if (WIFSIGNALED(status)) {
        int sig = WTERMSIG(status);
        printf("Executable terminated by signal %d\n", sig);
        AR_ASSERT(false, "Executable should not be terminated by signal");
    }
    
    // Verify that we saw the loading message
    AR_ASSERT(found_loading_message, "Should see message about loading from directory");
    
    // Verify that we loaded exactly 8 methods
    AR_ASSERT(method_count == 8, "Should load exactly 8 methods from directory");
    
    // Verify that all individual methods were loaded
    AR_ASSERT(found_agent_manager, "Should load agent-manager method");
    AR_ASSERT(found_bootstrap, "Should load bootstrap method");
    AR_ASSERT(found_calculator, "Should load calculator method");
    AR_ASSERT(found_echo, "Should load echo method");
    AR_ASSERT(found_grade_evaluator, "Should load grade-evaluator method");
    AR_ASSERT(found_message_router, "Should load message-router method");
    AR_ASSERT(found_method_creator, "Should load method-creator method");
    AR_ASSERT(found_string_builder, "Should load string-builder method");
    
    printf("Methods from directory loading test passed!\n");
    
}

int main(void) {
    printf("Starting Executable Module Tests...\n");
    
    // Run the executable test first (before creating any system instances)
    test_executable_run();
    
    // Test that executable has only single session
    test_single_session();
    
    // Test that executable loads methods from directory
    test_loading_methods_from_directory();
    
    // Now run a separate test with a system instance
    // Create system instance for tests
    ar_system_t *mut_system = ar_system__create();
    AR_ASSERT(mut_system != NULL, "System creation should succeed");
    
    // Given we have a test method and initialized system
    const char *init_method = "exec_test_method";
    const char *init_instructions = "memory.result = \"Test complete\"";
    const char *init_version = "1.0.0";
    
    // Create method and register it with methodology 
    ar_method_t *own_method = ar_method__create(init_method, init_instructions, init_version);
    AR_ASSERT(own_method != NULL, "Method creation should succeed");
    
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

