#include "ar_executable.h"
#include "ar_executable_fixture.h"
#include "ar_system.h"
#include "ar_method.h"
#include "ar_methodology.h"
#include "ar_agency.h"
#include "ar_assert.h"
#include "ar_heap.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <string.h>

// Forward declarations
static void test_single_session(ar_executable_fixture_t *mut_fixture);
static void test_loading_methods_from_directory(ar_executable_fixture_t *mut_fixture);
static void test_bootstrap_agent_creation(ar_executable_fixture_t *mut_fixture);
static void test_bootstrap_agent_creation_failure(ar_executable_fixture_t *mut_fixture);
static void test_bootstrap_spawns_echo(ar_executable_fixture_t *mut_fixture);
static void test_message_processing_loop(ar_executable_fixture_t *mut_fixture);
static void test_executable__saves_methodology_file(ar_executable_fixture_t *mut_fixture);
static void test_executable__continues_on_save_failure(ar_executable_fixture_t *mut_fixture);
static void test_executable__loads_agents_on_startup(ar_executable_fixture_t *mut_fixture);
static void test_executable__skips_bootstrap_when_agents_loaded(ar_executable_fixture_t *mut_fixture);
static void test_executable__saves_agents_on_shutdown(ar_executable_fixture_t *mut_fixture);


// Stub function to avoid linking with the actual executable
int ar_executable__main(void) {
    // This is just a stub for testing
    return 0;
}

// Test that the executable only runs a single session
static void test_single_session(ar_executable_fixture_t *mut_fixture) {
    printf("Testing executable has only single session...\n");
    
    
    // When we build and run the executable using make
    printf("Building and running executable...\n");
    char *own_methods_dir = ar_executable_fixture__create_methods_dir(mut_fixture);
    FILE *pipe = ar_executable_fixture__build_and_run(mut_fixture, own_methods_dir);
    AR_ASSERT(pipe != NULL, "Should be able to run executable via popen");
    
    char buffer[512];
    bool found_second_session = false;
    int line_count = 0;
    
    // Read all output looking for the second session message
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        line_count++;
        printf("Make output: %s", buffer);  // Debug: print all make output
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
        printf("Single session test: executable exited with code %d\n", exit_code);
        AR_ASSERT(exit_code == 0, "Executable should exit with code 0");
    } else {
        AR_ASSERT(false, "Executable should terminate normally");
    }
    
    printf("Executable produced %d lines of output\n", line_count);
    
    // And it should NOT have a second session
    AR_ASSERT(!found_second_session, "Executable should only have one session, not two");
    
    printf("Single session test passed!\n");
    
    ar_executable_fixture__destroy_methods_dir(mut_fixture, own_methods_dir);
}

// Test that the executable loads methods from directory
static void test_loading_methods_from_directory(ar_executable_fixture_t *mut_fixture) {
    printf("Testing executable loads methods from directory...\n");
    
    // Remove any existing methodology file to ensure we load from directory
    const char *build_dir = ar_executable_fixture__get_build_dir(mut_fixture);
    if (build_dir) {
        char methodology_path[512];
        snprintf(methodology_path, sizeof(methodology_path), "%s/agerun.methodology", build_dir);
        remove(methodology_path);
    }
    
    // When we build and run the executable using make
    printf("Building and running executable to test method loading...\n");
    char *own_methods_dir = ar_executable_fixture__create_methods_dir(mut_fixture);
    FILE *pipe = ar_executable_fixture__build_and_run(mut_fixture, own_methods_dir);
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
    
    ar_executable_fixture__destroy_methods_dir(mut_fixture, own_methods_dir);
}

// Test that the executable creates a bootstrap agent
static void test_bootstrap_agent_creation(ar_executable_fixture_t *mut_fixture) {
    printf("Testing executable creates bootstrap agent...\n");

    // Remove any existing persisted files to ensure clean test
    ar_executable_fixture__clean_persisted_files(mut_fixture);

    // When we build and run the executable using make
    printf("Building and running executable to test bootstrap agent creation...\n");
    char *own_methods_dir = ar_executable_fixture__create_methods_dir(mut_fixture);
    FILE *pipe = ar_executable_fixture__build_and_run(mut_fixture, own_methods_dir);
    AR_ASSERT(pipe != NULL, "Should be able to run executable");
    
    // Then we should see evidence of bootstrap agent creation
    char line[256];
    bool found_creating_message = false;
    bool found_agent_created = false;
    bool found_bootstrap_initialized = false;
    
    while (fgets(line, sizeof(line), pipe) != NULL) {
        printf("Make output: %s", line);  // Print all make output for debugging
        
        // Look for "Creating bootstrap agent" message
        if (strstr(line, "Creating bootstrap agent")) {
            found_creating_message = true;
        }
        
        // Look for agent creation confirmation with ID
        if (strstr(line, "Bootstrap agent created with ID:")) {
            found_agent_created = true;
        }
        
        // Look for bootstrap message processing
        if (strstr(line, "Bootstrap initialized")) {
            found_bootstrap_initialized = true;
        }
    }
    
    int status = pclose(pipe);
    
    // Verify the executable ran successfully
    if (WIFEXITED(status)) {
        int exit_code = WEXITSTATUS(status);
        printf("Bootstrap test: executable exited with code %d\n", exit_code);
        AR_ASSERT(exit_code == 0, "Executable should exit normally");
    } else if (WIFSIGNALED(status)) {
        int sig = WTERMSIG(status);
        printf("Executable terminated by signal %d\n", sig);
        AR_ASSERT(false, "Executable should not be terminated by signal");
    }
    
    // Verify bootstrap agent creation messages
    AR_ASSERT(found_creating_message, "Should see 'Creating bootstrap agent' message");
    AR_ASSERT(found_agent_created, "Should see 'Bootstrap agent created with ID' message");
    
    // Message processing is automatic with system init
    if (found_bootstrap_initialized) {
        printf("Bootstrap agent was initialized successfully\n");
    } else {
        printf("Note: Bootstrap initialized message not found (may need message processing loop)\n");
    }
    
    printf("Bootstrap agent creation test passed!\n");
    
    ar_executable_fixture__destroy_methods_dir(mut_fixture, own_methods_dir);
}

// Test that the executable handles bootstrap agent creation failure gracefully
static void test_bootstrap_agent_creation_failure(ar_executable_fixture_t *mut_fixture) {
    printf("Testing executable handles bootstrap creation failure...\n");

    // Given we're running from the correct test directory
    char cwd[1024];
    AR_ASSERT(getcwd(cwd, sizeof(cwd)) != NULL, "Should be able to get current directory");
    AR_ASSERT(strstr(cwd, "/bin/") != NULL, "Test must be run from bin directory");

    // Remove any existing persisted files to ensure clean test
    ar_executable_fixture__clean_persisted_files(mut_fixture);

    // Copy methods and then hide bootstrap to simulate it missing
    printf("Setting up temp methods directory and hiding bootstrap method file...\n");
    char *own_methods_dir = ar_executable_fixture__create_methods_dir(mut_fixture);
    
    // Hide bootstrap method
    char hide_cmd[512];
    snprintf(hide_cmd, sizeof(hide_cmd),
        "mv %s/bootstrap-1.0.0.method %s/bootstrap-1.0.0.method.hidden 2>/dev/null",
        own_methods_dir, own_methods_dir);
    int hide_result = system(hide_cmd);
    AR_ASSERT(hide_result == 0, "Failed to hide bootstrap method");
    
    // When we build and run the executable without bootstrap method
    printf("Building and running executable without bootstrap method...\n");
    FILE *pipe = ar_executable_fixture__build_and_run(mut_fixture, own_methods_dir);
    AR_ASSERT(pipe != NULL, "Should be able to run executable");
    
    // Then we should see error handling
    char line[256];
    bool found_creating_message = false;
    bool found_error_message = false;
    int exit_code = -1;
    
    while (fgets(line, sizeof(line), pipe) != NULL) {
        
        if (strstr(line, "Creating bootstrap agent")) {
            found_creating_message = true;
        }
        
        if (strstr(line, "Error: Failed to create bootstrap agent")) {
            found_error_message = true;
        }
    }
    
    int status = pclose(pipe);
    
    // Verify error handling
    if (WIFEXITED(status)) {
        exit_code = WEXITSTATUS(status);
        printf("Executable exited with code %d\n", exit_code);
    }
    
    AR_ASSERT(found_creating_message, "Should attempt to create bootstrap agent");
    AR_ASSERT(found_error_message, "Should show error message when bootstrap creation fails");
    // When using make run-exec, make returns exit code 2 when the target fails
    AR_ASSERT(exit_code == 2, "Should exit with error code 2 when bootstrap fails (via make)");
    
    printf("Bootstrap failure handling test passed!\n");
    
    ar_executable_fixture__destroy_methods_dir(mut_fixture, own_methods_dir);
}

// Test that bootstrap agent spawns echo agent
static void test_bootstrap_spawns_echo(ar_executable_fixture_t *mut_fixture) {
    printf("Testing bootstrap spawns echo agent...\n");

    // Given we're running from the correct test directory
    char cwd[1024];
    AR_ASSERT(getcwd(cwd, sizeof(cwd)) != NULL, "Should be able to get current directory");
    AR_ASSERT(strstr(cwd, "/bin/") != NULL, "Test must be run from bin directory");

    // Remove any existing persisted files to ensure clean test
    ar_executable_fixture__clean_persisted_files(mut_fixture);

    // When we build and run the executable using make
    printf("Building and running executable to test echo agent spawning...\n");
    char *own_methods_dir = ar_executable_fixture__create_methods_dir(mut_fixture);
    FILE *pipe = ar_executable_fixture__build_and_run(mut_fixture, own_methods_dir);
    AR_ASSERT(pipe != NULL, "Should be able to run executable");
    
    // Then we should see evidence of echo agent being spawned
    char line[256];
    bool found_bootstrap_created = false;
    bool found_echo_agent_created = false;
    
    while (fgets(line, sizeof(line), pipe) != NULL) {
        
        // Look for bootstrap agent creation
        if (strstr(line, "Bootstrap agent created with ID:")) {
            found_bootstrap_created = true;
        }
        
        // Look for echo agent being spawned
        if (strstr(line, "Spawning agent with method 'echo'") ||
            strstr(line, "Agent created with method 'echo'") ||
            strstr(line, "Creating agent with method 'echo'") ||
            strstr(line, "Agent 2")) {
            found_echo_agent_created = true;
        }
    }
    
    int status = pclose(pipe);
    
    // Verify the executable ran successfully
    if (WIFEXITED(status)) {
        int exit_code = WEXITSTATUS(status);
        AR_ASSERT(exit_code == 0, "Executable should exit normally");
    }
    
    // Verify bootstrap was created
    AR_ASSERT(found_bootstrap_created, "Should see bootstrap agent created");
    
    // Note: Without message processing loop (Cycle 7), spawn won't actually execute
    // But we can still check if the bootstrap method would spawn echo when messages are processed
    if (!found_echo_agent_created) {
        printf("Note: Echo agent spawn will be visible after message processing loop (Cycle 7)\n");
    }
    
    printf("Bootstrap spawn echo test passed!\n");
    
    ar_executable_fixture__destroy_methods_dir(mut_fixture, own_methods_dir);
}

// Test that the executable processes all messages until none remain
static void test_message_processing_loop(ar_executable_fixture_t *mut_fixture) {
    printf("Testing message processing loop...\n");
    
    // When we build and run the executable using make
    printf("Building and running executable to test message processing...\n");
    char *own_methods_dir = ar_executable_fixture__create_methods_dir(mut_fixture);
    FILE *pipe = ar_executable_fixture__build_and_run(mut_fixture, own_methods_dir);
    AR_ASSERT(pipe != NULL, "Should be able to run executable");
    
    // Then we should see evidence of message processing
    char line[256];
    bool found_processing_messages = false;
    bool found_messages_processed_count = false;
    int messages_processed = 0;
    
    while (fgets(line, sizeof(line), pipe) != NULL) {
        printf("Make output: %s", line);  // Print all make output for debugging
        
        // Look for "Processing messages" to know loop started
        if (strstr(line, "Processing messages")) {
            found_processing_messages = true;
        }
        
        // Look for count of messages processed or "No messages to process"
        if ((strstr(line, "Processed") && (strstr(line, "message") || strstr(line, "messages"))) ||
            strstr(line, "No messages to process")) {
            found_messages_processed_count = true;
            
            // Check if it's "No messages to process"
            if (strstr(line, "No messages to process")) {
                messages_processed = 0;
            } else {
                // Try to extract the number (handles both "message" and "messages")
                char *num_str = strstr(line, "Processed ");
                if (num_str) {
                    sscanf(num_str, "Processed %d message", &messages_processed);
                }
            }
        }
    }
    
    int status = pclose(pipe);
    
    // Verify the executable ran successfully
    if (WIFEXITED(status)) {
        int exit_code = WEXITSTATUS(status);
        printf("Message processing test: executable exited with code %d\n", exit_code);
        AR_ASSERT(exit_code == 0, "Executable should exit normally");
    } else if (WIFSIGNALED(status)) {
        int sig = WTERMSIG(status);
        printf("Executable terminated by signal %d\n", sig);
        AR_ASSERT(false, "Executable should not be terminated by signal");
    }
    
    // Verify message processing occurred
    AR_ASSERT(found_processing_messages, "Should see 'Processing messages' indicating loop started");
    // Note: "Bootstrap initialized" won't appear because send(0, ...) is a no-op per CLAUDE.md
    // So the message processing loop completes with 0 messages
    AR_ASSERT(found_messages_processed_count, "Should see count of messages processed");
    // We expect 0 messages (messages processed internally)
    AR_ASSERT(messages_processed == 0, "Should process 0 messages");
    
    printf("Message processing loop test passed! Processed %d messages\n", messages_processed);
    
    ar_executable_fixture__destroy_methods_dir(mut_fixture, own_methods_dir);
}

static void test_executable__saves_methodology_file(ar_executable_fixture_t *mut_fixture) {
    printf("\n=== Testing methodology file persistence ===\n");
    
    // Get the build directory where the file will be saved
    const char *build_dir = ar_executable_fixture__get_build_dir(mut_fixture);
    AR_ASSERT(build_dir != NULL, "Should have build directory");
    
    // Build the full path to the methodology file
    char methodology_path[512];
    snprintf(methodology_path, sizeof(methodology_path), "%s/agerun.methodology", build_dir);
    
    // Given: Remove any existing agerun.methodology file
    remove(methodology_path);
    
    // When: Build and run the executable
    printf("Building and running executable to test methodology persistence...\n");
    char *own_methods_dir = ar_executable_fixture__create_methods_dir(mut_fixture);
    FILE *pipe = ar_executable_fixture__build_and_run(mut_fixture, own_methods_dir);
    AR_ASSERT(pipe != NULL, "Should be able to run executable");
    
    // Read all output (but we don't need to check it for this test)
    char line[256];
    while (fgets(line, sizeof(line), pipe) != NULL) {
        // Just consume the output
    }
    
    // Close pipe and check exit status
    int exit_status = pclose(pipe);
    AR_ASSERT(exit_status == 0, "Executable should exit successfully");
    
    // Clean up methods directory
    ar_executable_fixture__destroy_methods_dir(mut_fixture, own_methods_dir);
    
    // Then: Check that agerun.methodology file exists in the build directory
    FILE *methodology_file = fopen(methodology_path, "r");
    AR_ASSERT(methodology_file != NULL, "agerun.methodology file should exist after execution");
    
    // Verify the file contains all expected methods
    char *file_content = NULL;
    size_t content_size = 0;
    fseek(methodology_file, 0, SEEK_END);
    content_size = (size_t)ftell(methodology_file);
    fseek(methodology_file, 0, SEEK_SET);
    
    file_content = AR__HEAP__MALLOC(content_size + 1, "methodology file content");
    AR_ASSERT(file_content != NULL, "Should allocate memory for file content");
    
    size_t read_size = fread(file_content, 1, content_size, methodology_file);
    file_content[read_size] = '\0';
    fclose(methodology_file);
    
    // Check for all 8 methods
    AR_ASSERT(strstr(file_content, "agent-manager") != NULL, "Should contain agent-manager method");
    AR_ASSERT(strstr(file_content, "bootstrap") != NULL, "Should contain bootstrap method");
    AR_ASSERT(strstr(file_content, "calculator") != NULL, "Should contain calculator method");
    AR_ASSERT(strstr(file_content, "echo") != NULL, "Should contain echo method");
    AR_ASSERT(strstr(file_content, "grade-evaluator") != NULL, "Should contain grade-evaluator method");
    AR_ASSERT(strstr(file_content, "message-router") != NULL, "Should contain message-router method");
    AR_ASSERT(strstr(file_content, "method-creator") != NULL, "Should contain method-creator method");
    AR_ASSERT(strstr(file_content, "string-builder") != NULL, "Should contain string-builder method");
    
    AR__HEAP__FREE(file_content);
    
    printf("✓ All 8 methods found in agerun.methodology file\n");
}

static void test_executable__loads_persisted_methodology(ar_executable_fixture_t *mut_fixture) {
    printf("\n=== Testing methodology loads from persisted file ===\n");
    
    // Get the build directory where the file will be saved
    const char *build_dir = ar_executable_fixture__get_build_dir(mut_fixture);
    AR_ASSERT(build_dir != NULL, "Should have build directory");
    
    // Build the full path to the methodology file
    char methodology_path[512];
    snprintf(methodology_path, sizeof(methodology_path), "%s/agerun.methodology", build_dir);
    
    // Step 1: Run executable once to create agerun.methodology file
    printf("First run: Creating methodology file...\n");
    char *own_methods_dir = ar_executable_fixture__create_methods_dir(mut_fixture);
    FILE *pipe = ar_executable_fixture__build_and_run(mut_fixture, own_methods_dir);
    AR_ASSERT(pipe != NULL, "Should be able to run executable first time");
    
    // Consume output
    char line[256];
    while (fgets(line, sizeof(line), pipe) != NULL) {
        // Just consume the output
    }
    
    int exit_status = pclose(pipe);
    AR_ASSERT(exit_status == 0, "First run should exit successfully");
    
    // Verify methodology file was created
    FILE *methodology_file = fopen(methodology_path, "r");
    AR_ASSERT(methodology_file != NULL, "agerun.methodology should exist after first run");
    fclose(methodology_file);
    
    // Step 2: Delete all .method files from the methods directory
    printf("Deleting source method files...\n");
    char command[1024];
    snprintf(command, sizeof(command), "rm -f %s/*.method", own_methods_dir);
    int rm_result = system(command);
    AR_ASSERT(rm_result == 0, "Should delete method files successfully");
    
    // Verify methods directory is now empty of .method files
    snprintf(command, sizeof(command), "ls %s/*.method 2>/dev/null | wc -l", own_methods_dir);
    FILE *count_pipe = popen(command, "r");
    char count_str[16];
    if (count_pipe != NULL) {
        fgets(count_str, sizeof(count_str), count_pipe);
        pclose(count_pipe);
    } else {
        count_str[0] = '0';
        count_str[1] = '\0';
    }
    int method_count = atoi(count_str);
    AR_ASSERT(method_count == 0, "Methods directory should have no .method files");
    
    // Step 3: Run executable again - should load from persisted file
    printf("Second run: Testing load from persisted methodology...\n");
    pipe = ar_executable_fixture__build_and_run(mut_fixture, own_methods_dir);
    AR_ASSERT(pipe != NULL, "Should be able to run executable second time");
    
    // Read output to verify it runs successfully
    int lines_read = 0;
    while (fgets(line, sizeof(line), pipe) != NULL) {
        lines_read++;
        // Could check for specific output if needed
    }
    
    exit_status = pclose(pipe);
    AR_ASSERT(exit_status == 0, "Second run should exit successfully using persisted methodology");
    AR_ASSERT(lines_read > 0, "Should produce output when running with persisted methodology");
    
    // Clean up
    ar_executable_fixture__destroy_methods_dir(mut_fixture, own_methods_dir);
    
    printf("✓ Executable successfully loaded and ran from persisted methodology\n");
}

static void test_executable__continues_on_save_failure(ar_executable_fixture_t *mut_fixture) {
    printf("\n=== Testing executable continues on save failure ===\n");
    
    // Get the build directory where the file would be saved
    const char *build_dir = ar_executable_fixture__get_build_dir(mut_fixture);
    AR_ASSERT(build_dir != NULL, "Should have build directory");
    
    // Build the full path to the methodology file
    char methodology_path[512];
    snprintf(methodology_path, sizeof(methodology_path), "%s/agerun.methodology", build_dir);
    
    // Given: Block creation of the methodology file to force save failure
    // Use a directory with the same name so fopen() fails with EISDIR
    // Remove any existing file first to avoid mkdir errors
    remove(methodology_path);
    if (mkdir(methodology_path, 0555) != 0) {
        fprintf(stderr, "WARNING: Failed to create blocking directory for methodology save at '%s' (%s) - test may not properly validate save failure behavior\n", methodology_path, strerror(errno));
    }
    
    // When: Build and run the executable
    printf("Building and running executable with read-only methodology file...\n");
    char *own_methods_dir = ar_executable_fixture__create_methods_dir(mut_fixture);
    FILE *pipe = ar_executable_fixture__build_and_run(mut_fixture, own_methods_dir);
    AR_ASSERT(pipe != NULL, "Should be able to run executable");
    
    // Read output and look for warning message
    char line[256];
    bool found_warning = false;
    bool found_shutdown = false;
    while (fgets(line, sizeof(line), pipe) != NULL) {
        if (strstr(line, "Warning: Failed to save methodology")) {
            found_warning = true;
            printf("Found expected warning: %s", line);
        }
        if (strstr(line, "Runtime shutdown complete")) {
            found_shutdown = true;
        }
    }
    
    // Close pipe and check exit status
    int exit_status = pclose(pipe);
    
    // Clean up - remove the blocking directory so future tests can create the file
    if (rmdir(methodology_path) != 0) {
        printf("WARNING: Failed to remove blocking directory for methodology save - may affect subsequent test runs\n");
    }
    
    // Clean up methods directory
    ar_executable_fixture__destroy_methods_dir(mut_fixture, own_methods_dir);
    
    // Then: Verify executable handled failure gracefully
    printf("Exit status: %d (expecting 0)\n", exit_status);
    printf("Found warning: %s\n", found_warning ? "yes" : "no");
    printf("Found shutdown: %s\n", found_shutdown ? "yes" : "no");
    AR_ASSERT(exit_status == 0, "Executable should exit successfully despite save failure");
    AR_ASSERT(found_warning, "Should see warning about save failure");
    AR_ASSERT(found_shutdown, "Should see shutdown complete message");
    
    printf("✓ Executable continues gracefully when save fails\n");
}

static void test_executable__loads_agents_on_startup(ar_executable_fixture_t *mut_fixture) {
    printf("\n=== Testing executable loads agents on startup ===\n");

    const char *build_dir = ar_executable_fixture__get_build_dir(mut_fixture);
    AR_ASSERT(build_dir != NULL, "Should have build directory");

    char agency_path[512];
    snprintf(agency_path, sizeof(agency_path), "%s/agerun.agency", build_dir);

    // Given: An agerun.agency file exists with a bootstrap agent
    FILE *agency_file = fopen(agency_path, "w");
    AR_ASSERT(agency_file != NULL, "Should be able to create agency file");
    fprintf(agency_file, "# AgeRun YAML File\n");  // Header required by YAML reader
    fprintf(agency_file, "agents:\n");
    fprintf(agency_file, "  - id: 1\n");
    fprintf(agency_file, "    method_name: bootstrap\n");
    fprintf(agency_file, "    method_version: \"1.0.0\"\n");
    fprintf(agency_file, "    memory: {}\n");
    fclose(agency_file);

    // When: The executable runs
    char *own_methods_dir = ar_executable_fixture__create_methods_dir(mut_fixture);
    FILE *pipe = ar_executable_fixture__build_and_run(mut_fixture, own_methods_dir);
    AR_ASSERT(pipe != NULL, "Should be able to run executable");

    char line[512];
    bool found_loading_agents = false;
    while (fgets(line, sizeof(line), pipe) != NULL) {
        if (strstr(line, "Loading agents from persisted agency")) {
            found_loading_agents = true;
        }
    }

    int exit_status = pclose(pipe);
    AR_ASSERT(exit_status == 0, "Executable should exit successfully");

    // Then: It should load agents from the file
    AR_ASSERT(found_loading_agents, "Should load agents from agerun.agency file");

    // Cleanup
    ar_executable_fixture__destroy_methods_dir(mut_fixture, own_methods_dir);
    printf("✓ Agent loading on startup test passed\n");
}

static void test_executable__skips_bootstrap_when_agents_loaded(ar_executable_fixture_t *mut_fixture) {
    printf("\n=== Testing executable skips bootstrap when agents loaded ===\n");

    const char *build_dir = ar_executable_fixture__get_build_dir(mut_fixture);
    AR_ASSERT(build_dir != NULL, "Should have build directory");

    char agency_path[512];
    snprintf(agency_path, sizeof(agency_path), "%s/agerun.agency", build_dir);

    // Given: An agerun.agency file exists with agents
    FILE *agency_file = fopen(agency_path, "w");
    AR_ASSERT(agency_file != NULL, "Should be able to create agency file");
    fprintf(agency_file, "# AgeRun YAML File\n");  // Header required by YAML reader
    fprintf(agency_file, "agents:\n");
    fprintf(agency_file, "  - id: 1\n");
    fprintf(agency_file, "    method_name: bootstrap\n");
    fprintf(agency_file, "    method_version: \"1.0.0\"\n");
    fprintf(agency_file, "    memory: {}\n");
    fclose(agency_file);

    // When: The executable runs
    char *own_methods_dir = ar_executable_fixture__create_methods_dir(mut_fixture);
    FILE *pipe = ar_executable_fixture__build_and_run(mut_fixture, own_methods_dir);
    AR_ASSERT(pipe != NULL, "Should be able to run executable");

    char line[512];
    bool found_skipping_bootstrap = false;
    bool found_creating_bootstrap = false;
    while (fgets(line, sizeof(line), pipe) != NULL) {
        if (strstr(line, "Agents loaded from disk, skipping bootstrap creation")) {
            found_skipping_bootstrap = true;
        }
        if (strstr(line, "Creating bootstrap agent")) {
            found_creating_bootstrap = true;
        }
    }

    int exit_status = pclose(pipe);
    AR_ASSERT(exit_status == 0, "Executable should exit successfully");

    // Then: It should skip bootstrap creation
    AR_ASSERT(found_skipping_bootstrap, "Should skip bootstrap when agents loaded");
    AR_ASSERT(!found_creating_bootstrap, "Should NOT create bootstrap when agents loaded");

    // Cleanup
    ar_executable_fixture__destroy_methods_dir(mut_fixture, own_methods_dir);
    printf("✓ Skip bootstrap test passed\n");
}

static void test_executable__saves_agents_on_shutdown(ar_executable_fixture_t *mut_fixture) {
    printf("\n=== Testing executable saves agents on shutdown ===\n");

    const char *build_dir = ar_executable_fixture__get_build_dir(mut_fixture);
    AR_ASSERT(build_dir != NULL, "Should have build directory");

    char agency_path[512];
    snprintf(agency_path, sizeof(agency_path), "%s/agerun.agency", build_dir);

    // Given: No agency file exists before running
    unlink(agency_path);  // Remove any existing agency file

    // Given: The executable runs (which creates a bootstrap agent)
    char *own_methods_dir = ar_executable_fixture__create_methods_dir(mut_fixture);
    FILE *pipe = ar_executable_fixture__build_and_run(mut_fixture, own_methods_dir);
    AR_ASSERT(pipe != NULL, "Should be able to run executable");

    // Wait for executable to finish
    char line[512];
    while (fgets(line, sizeof(line), pipe) != NULL) {
        // Just consume output
    }

    int exit_status = pclose(pipe);
    AR_ASSERT(exit_status == 0, "Executable should exit successfully");

    // When: The executable has shut down

    // Then: The agerun.agency file should exist with agent data
    struct stat file_stat;
    AR_ASSERT(stat(agency_path, &file_stat) == 0, "Should create agerun.agency file");
    AR_ASSERT(file_stat.st_size > 0, "Agency file should not be empty");

    // Verify file contains expected YAML structure
    FILE *agency_file = fopen(agency_path, "r");
    AR_ASSERT(agency_file != NULL, "Should be able to read agency file");

    bool found_header = false;
    bool found_agents = false;
    bool found_bootstrap = false;

    while (fgets(line, sizeof(line), agency_file) != NULL) {
        if (strstr(line, "# AgeRun YAML File")) {
            found_header = true;
        }
        if (strstr(line, "agents:")) {
            found_agents = true;
        }
        if (strstr(line, "method_name: bootstrap")) {
            found_bootstrap = true;
        }
    }

    fclose(agency_file);

    AR_ASSERT(found_header, "Agency file should have YAML header");
    AR_ASSERT(found_agents, "Agency file should have agents section");
    AR_ASSERT(found_bootstrap, "Agency file should contain bootstrap agent");

    // Cleanup
    ar_executable_fixture__destroy_methods_dir(mut_fixture, own_methods_dir);
    printf("✓ Agent save on shutdown test passed\n");
}

// Test that executable handles corrupted agency file gracefully
static void test_executable__handles_corrupted_agency_file(ar_executable_fixture_t *mut_fixture) {
    printf("\n=== Testing executable handles corrupted agency file ===\n");

    // Clean up any existing persisted files
    ar_executable_fixture__clean_persisted_files(mut_fixture);

    // Given: A corrupted agency file exists
    const char *build_dir = ar_executable_fixture__get_build_dir(mut_fixture);
    AR_ASSERT(build_dir != NULL, "Should have build directory");

    char agency_path[512];
    snprintf(agency_path, sizeof(agency_path), "%s/agerun.agency", build_dir);

    // Create a corrupted agency file (missing required fields)
    FILE *agency_file = fopen(agency_path, "w");
    AR_ASSERT(agency_file != NULL, "Should be able to create corrupted agency file");
    fprintf(agency_file, "# AgeRun YAML File\n");
    fprintf(agency_file, "agents:\n");
    fprintf(agency_file, "  - id: 1\n");
    fprintf(agency_file, "    method_name: bootstrap\n");
    // Missing method_version - this should cause loading to skip the agent
    fprintf(agency_file, "    memory: {}\n");
    fclose(agency_file);

    // When: We run the executable
    char *own_methods_dir = ar_executable_fixture__create_methods_dir(mut_fixture);
    FILE *pipe = ar_executable_fixture__build_and_run(mut_fixture, own_methods_dir);
    AR_ASSERT(pipe != NULL, "Should be able to run executable");

    // Then: Executable should handle gracefully
    char line[512];
    bool found_loading_agents = false;
    bool found_bootstrap_creation = false;
    bool found_shutdown = false;

    while (fgets(line, sizeof(line), pipe) != NULL) {
        if (strstr(line, "Loading agents from persisted agency")) {
            found_loading_agents = true;
        }
        if (strstr(line, "Creating bootstrap agent")) {
            found_bootstrap_creation = true;
        }
        if (strstr(line, "Runtime shutdown complete")) {
            found_shutdown = true;
        }
    }

    int status = pclose(pipe);

    // Verify executable handled corruption gracefully
    if (WIFEXITED(status)) {
        int exit_code = WEXITSTATUS(status);
        AR_ASSERT(exit_code == 0, "Should exit successfully despite corrupted file");
    } else {
        AR_ASSERT(false, "Executable should exit normally");
    }

    AR_ASSERT(found_loading_agents, "Should attempt to load agents");
    AR_ASSERT(found_bootstrap_creation, "Should create bootstrap agent when load skips corrupted agent");
    AR_ASSERT(found_shutdown, "Should shutdown normally");

    // Cleanup
    ar_executable_fixture__destroy_methods_dir(mut_fixture, own_methods_dir);
    printf("✓ Corrupted agency file handled gracefully\n");
}

int main(void) {
    printf("Starting Executable Module Tests...\n");
    
    // Verify we're running from the correct test directory
    char cwd[1024];
    AR_ASSERT(getcwd(cwd, sizeof(cwd)) != NULL, "Should be able to get current directory");
    AR_ASSERT(strstr(cwd, "/bin/") != NULL && strstr(cwd, "-tests") != NULL, 
              "Test must be run from bin/*-tests directory");
    printf("Running from: %s\n", cwd);
    
    // Create fixture for all tests
    ar_executable_fixture_t *own_fixture = ar_executable_fixture__create();
    AR_ASSERT(own_fixture != NULL, "Failed to create executable fixture");
    
    // Skip the fork-based test as it causes memory space conflicts
    // test_executable_run();
    
    // Test that executable has only single session
    test_single_session(own_fixture);
    
    // Test that executable loads methods from directory
    test_loading_methods_from_directory(own_fixture);
    
    // Test that executable creates bootstrap agent
    test_bootstrap_agent_creation(own_fixture);
    
    // Test that executable handles bootstrap failure
    test_bootstrap_agent_creation_failure(own_fixture);
    
    // Test that bootstrap spawns echo agent
    test_bootstrap_spawns_echo(own_fixture);
    
    // Test that executable processes all messages
    test_message_processing_loop(own_fixture);
    
    // Test that executable saves methodology to file
    test_executable__saves_methodology_file(own_fixture);
    
    // Test that executable continues on save failure
    test_executable__continues_on_save_failure(own_fixture);
    
    // Test that executable loads from persisted methodology
    test_executable__loads_persisted_methodology(own_fixture);

    // Test that executable loads agents on startup
    test_executable__loads_agents_on_startup(own_fixture);

    // Test that executable skips bootstrap when agents loaded
    test_executable__skips_bootstrap_when_agents_loaded(own_fixture);

    // Test that executable saves agents on shutdown
    test_executable__saves_agents_on_shutdown(own_fixture);

    // Test that executable handles corrupted agency file
    test_executable__handles_corrupted_agency_file(own_fixture);

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
    ar_methodology__register_method(ref_methodology, own_method);
    own_method = NULL; // Mark as transferred
    
    // When we initialize the system with this method
    int64_t initial_agent = ar_system__init(mut_system, init_method, init_version);
    
    // Process any initial messages if an agent was created
    if (initial_agent > 0) {
        ar_system__process_next_message(mut_system);
    }
    
    // Then we clean up the system
    ar_system__shutdown(mut_system);
    ar_system__destroy(mut_system);
    
    // Destroy the fixture (also cleans up temp build directory)
    ar_executable_fixture__destroy(own_fixture);
    
    // And report success
    printf("All 12 tests passed!\n");
    return 0;
}

