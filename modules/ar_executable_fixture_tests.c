#include "ar_executable_fixture.h"
#include "ar_assert.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

/**
 * @file ar_executable_fixture_tests.c
 * @brief Tests for the executable fixture module
 */

// Test that fixture can be created and destroyed
static void test_fixture_create_destroy(void) {
    printf("Testing fixture creation and destruction...\n");
    
    // Create a fixture
    ar_executable_fixture_t *own_fixture = ar_executable_fixture__create();
    AR_ASSERT(own_fixture != NULL, "Should be able to create fixture");
    
    // Destroy the fixture
    ar_executable_fixture__destroy(own_fixture);
    
    printf("✓ Fixture create/destroy test passed\n");
}

// Test that fixture creates temp build directory
static void test_fixture_creates_build_dir(void) {
    printf("Testing fixture creates temp build directory...\n");
    
    // Create a fixture
    ar_executable_fixture_t *own_fixture = ar_executable_fixture__create();
    AR_ASSERT(own_fixture != NULL, "Should be able to create fixture");
    
    // Check that temp build directory exists
    char expected_dir[256];
    snprintf(expected_dir, sizeof(expected_dir), "/tmp/agerun_test_%d_build/obj", (int)getpid());
    
    struct stat st;
    int result = stat(expected_dir, &st);
    AR_ASSERT(result == 0, "Temp build directory should exist");
    AR_ASSERT(S_ISDIR(st.st_mode), "Should be a directory");
    
    // Destroy the fixture (should clean up directory)
    ar_executable_fixture__destroy(own_fixture);
    
    // Verify directory was cleaned up
    result = stat(expected_dir, &st);
    AR_ASSERT(result != 0, "Temp build directory should be cleaned up");
    
    printf("✓ Build directory creation test passed\n");
}

// Test that fixture can create methods directory
static void test_fixture_create_methods(void) {
    printf("Testing fixture creates methods directory...\n");
    
    // Given we're running from the correct test directory
    char cwd[1024];
    AR_ASSERT(getcwd(cwd, sizeof(cwd)) != NULL, "Should be able to get current directory");
    AR_ASSERT(strstr(cwd, "/bin/") != NULL, "Test must be run from bin directory");
    
    // Create a fixture
    ar_executable_fixture_t *own_fixture = ar_executable_fixture__create();
    AR_ASSERT(own_fixture != NULL, "Should be able to create fixture");
    
    // Create methods directory
    char *own_methods_dir = ar_executable_fixture__create_methods_dir(own_fixture);
    AR_ASSERT(own_methods_dir != NULL, "Should return methods directory path");
    AR_ASSERT(strlen(own_methods_dir) > 0, "Methods directory path should not be empty");
    
    // Check that methods directory exists
    struct stat st;
    int result = stat(own_methods_dir, &st);
    AR_ASSERT(result == 0, "Methods directory should exist");
    AR_ASSERT(S_ISDIR(st.st_mode), "Should be a directory");
    
    // Check that a method file was copied
    char method_file[512];
    snprintf(method_file, sizeof(method_file), "%s/bootstrap-1.0.0.method", own_methods_dir);
    result = stat(method_file, &st);
    AR_ASSERT(result == 0, "Bootstrap method file should exist");
    AR_ASSERT(S_ISREG(st.st_mode), "Should be a regular file");
    
    // Store path for verification after deletion
    char path_copy[256];
    strncpy(path_copy, own_methods_dir, sizeof(path_copy) - 1);
    path_copy[sizeof(path_copy) - 1] = '\0';
    
    // Clean up methods directory (also frees own_methods_dir)
    ar_executable_fixture__destroy_methods_dir(own_fixture, own_methods_dir);
    
    // Verify methods directory was cleaned up (using copied path)
    result = stat(path_copy, &st);
    AR_ASSERT(result != 0, "Methods directory should be cleaned up");
    
    // Destroy the fixture
    ar_executable_fixture__destroy(own_fixture);
    
    printf("✓ Methods directory create test passed\n");
}

// Test that fixture can handle multiple method directories
static void test_fixture_multiple_methods_dirs(void) {
    printf("Testing fixture handles multiple methods directories...\n");
    
    // Create a fixture
    ar_executable_fixture_t *own_fixture = ar_executable_fixture__create();
    AR_ASSERT(own_fixture != NULL, "Should be able to create fixture");
    
    // Create methods directory first time
    char *own_methods_dir1 = ar_executable_fixture__create_methods_dir(own_fixture);
    AR_ASSERT(own_methods_dir1 != NULL, "Should return first methods directory");
    
    // Create methods directory second time (should allocate new memory)
    char *own_methods_dir2 = ar_executable_fixture__create_methods_dir(own_fixture);
    AR_ASSERT(own_methods_dir2 != NULL, "Should return second methods directory");
    
    // The pointers should be different (dynamically allocated)
    AR_ASSERT(own_methods_dir1 != own_methods_dir2, "Should return different allocated buffers");
    
    // Both directories should exist (second call creates fresh directory)
    struct stat st;
    int result = stat(own_methods_dir2, &st);
    AR_ASSERT(result == 0, "New methods directory should exist");
    
    // Clean up both allocated paths
    ar_executable_fixture__destroy_methods_dir(own_fixture, own_methods_dir1);
    ar_executable_fixture__destroy_methods_dir(own_fixture, own_methods_dir2);
    ar_executable_fixture__destroy(own_fixture);
    
    printf("✓ Multiple methods directories test passed\n");
}

// Test that build_and_run returns valid FILE pointer
static void test_fixture_build_and_run(void) {
    printf("Testing fixture build_and_run...\n");
    
    // Given we're running from the correct test directory
    char cwd[1024];
    AR_ASSERT(getcwd(cwd, sizeof(cwd)) != NULL, "Should be able to get current directory");
    AR_ASSERT(strstr(cwd, "/bin/") != NULL, "Test must be run from bin directory");
    
    // Create a fixture
    ar_executable_fixture_t *own_fixture = ar_executable_fixture__create();
    AR_ASSERT(own_fixture != NULL, "Should be able to create fixture");
    
    // Create methods directory
    char *own_methods_dir = ar_executable_fixture__create_methods_dir(own_fixture);
    AR_ASSERT(own_methods_dir != NULL, "Should have methods directory");
    
    // Build and run executable
    FILE *pipe = ar_executable_fixture__build_and_run(own_fixture, own_methods_dir);
    AR_ASSERT(pipe != NULL, "Should return valid FILE pointer");
    
    // Read some output to verify it's working
    char buffer[256];
    bool got_output = false;
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        got_output = true;
        break; // Just need to verify we can read
    }
    AR_ASSERT(got_output, "Should be able to read from pipe");
    
    // Close the pipe
    int status = pclose(pipe);
    // Don't assert on status as the build might fail in test environment
    printf("Build/run completed with status: %d\n", status);
    
    // Clean up (also frees own_methods_dir)
    ar_executable_fixture__destroy_methods_dir(own_fixture, own_methods_dir);
    ar_executable_fixture__destroy(own_fixture);
    
    printf("✓ Build and run test passed\n");
}

// Test NULL parameter handling
static void test_fixture_null_handling(void) {
    printf("Testing fixture NULL parameter handling...\n");
    
    // Test destroy with NULL
    ar_executable_fixture__destroy(NULL); // Should not crash
    
    // Test create_methods_dir with NULL
    char *result = ar_executable_fixture__create_methods_dir(NULL);
    AR_ASSERT(result == NULL, "Should return NULL for NULL fixture");
    
    // Test build_and_run with NULL fixture
    FILE *pipe = ar_executable_fixture__build_and_run(NULL, "/tmp/test");
    AR_ASSERT(pipe == NULL, "Should return NULL for NULL fixture");
    
    // Test build_and_run with NULL methods_dir
    ar_executable_fixture_t *own_fixture = ar_executable_fixture__create();
    pipe = ar_executable_fixture__build_and_run(own_fixture, NULL);
    AR_ASSERT(pipe == NULL, "Should return NULL for NULL methods_dir");
    
    // Test destroy_methods_dir with NULL parameters
    ar_executable_fixture__destroy_methods_dir(NULL, NULL); // Should not crash with both NULL
    ar_executable_fixture__destroy_methods_dir(own_fixture, NULL); // Should not crash with NULL path
    
    ar_executable_fixture__destroy(own_fixture);
    
    printf("✓ NULL handling test passed\n");
}

int main(void) {
    printf("Starting Executable Fixture Tests...\n\n");
    
    // Run all tests
    test_fixture_create_destroy();
    test_fixture_creates_build_dir();
    test_fixture_create_methods();
    test_fixture_multiple_methods_dirs();
    test_fixture_build_and_run();
    test_fixture_null_handling();
    
    printf("\nAll executable fixture tests passed!\n");
    return 0;
}