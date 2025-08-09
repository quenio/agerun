#include "ar_executable_fixture.h"
#include "ar_heap.h"
#include "ar_assert.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

/**
 * @file ar_executable_fixture.c
 * @brief Implementation of executable fixture module for AgeRun executable testing
 */

/* Executable fixture structure */
struct ar_executable_fixture_s {
    char temp_build_dir[256];     /* Temporary build directory path */
    bool initialized;              /* Whether build directory has been initialized */
};

/* Initialize temporary build directory */
static void _init_temp_build_dir(ar_executable_fixture_t *mut_fixture) {
    if (!mut_fixture->initialized) {
        pid_t pid = getpid();
        snprintf(mut_fixture->temp_build_dir, sizeof(mut_fixture->temp_build_dir), 
                 "/tmp/agerun_test_%d_build", (int)pid);
        
        char mkdir_cmd[512];
        snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p %s/obj", mut_fixture->temp_build_dir);
        
        int result = system(mkdir_cmd);
        if (result != 0) {
            fprintf(stderr, "WARNING: Failed to create temp build directory: %s\n", 
                    mut_fixture->temp_build_dir);
        } else {
            printf("Created temporary build directory: %s\n", mut_fixture->temp_build_dir);
        }
        
        mut_fixture->initialized = true;
    }
}

/* Clean up temporary build directory */
static void _cleanup_temp_build_dir(ar_executable_fixture_t *mut_fixture) {
    if (mut_fixture->initialized && mut_fixture->temp_build_dir[0] != '\0') {
        char cleanup_cmd[512];
        snprintf(cleanup_cmd, sizeof(cleanup_cmd), "rm -rf %s 2>&1", mut_fixture->temp_build_dir);
        
        int result = system(cleanup_cmd);
        if (result != 0) {
            printf("WARNING: Failed to remove temporary build directory: %s\n", 
                   mut_fixture->temp_build_dir);
        } else {
            printf("Cleaned up temporary build directory: %s\n", mut_fixture->temp_build_dir);
        }
        
        mut_fixture->temp_build_dir[0] = '\0';  // Clear the path
        mut_fixture->initialized = false;
    }
}

ar_executable_fixture_t* ar_executable_fixture__create(void) {
    ar_executable_fixture_t *own_fixture = AR__HEAP__MALLOC(sizeof(ar_executable_fixture_t), 
                                                            "Executable fixture");
    if (!own_fixture) {
        return NULL;
    }
    
    // Initialize fields
    own_fixture->temp_build_dir[0] = '\0';
    own_fixture->initialized = false;
    
    // Initialize the temporary build directory
    _init_temp_build_dir(own_fixture);
    
    return own_fixture; // Ownership transferred to caller
}

void ar_executable_fixture__destroy(ar_executable_fixture_t *own_fixture) {
    if (!own_fixture) {
        return;
    }
    
    // Clean up the temporary build directory
    _cleanup_temp_build_dir(own_fixture);
    
    // Free the fixture
    AR__HEAP__FREE(own_fixture);
}

char* ar_executable_fixture__create_methods_dir(ar_executable_fixture_t *mut_fixture) {
    if (!mut_fixture) {
        return NULL;
    }
    
    pid_t pid = getpid();
    
    // Allocate memory for the path
    char *own_methods_dir = AR__HEAP__MALLOC(256, "Methods directory path");
    if (!own_methods_dir) {
        return NULL;
    }
    
    snprintf(own_methods_dir, 256, "/tmp/agerun_test_%d_methods", (int)pid);
    
    char setup_cmd[1024];
    snprintf(setup_cmd, sizeof(setup_cmd),
        "rm -rf %s 2>/dev/null && "
        "mkdir -p %s && "
        "cp ../../methods/* %s/",
        own_methods_dir, own_methods_dir, own_methods_dir);
    
    int result = system(setup_cmd);
    if (result != 0) {
        AR__HEAP__FREE(own_methods_dir);
        AR_ASSERT(false, "Failed to copy methods directory");
        return NULL;
    }
    
    printf("Copied methods to: %s\n", own_methods_dir);
    return own_methods_dir; // Ownership transferred to caller
}

FILE* ar_executable_fixture__build_and_run(const ar_executable_fixture_t *ref_fixture, 
                                           const char *ref_methods_dir) {
    if (!ref_fixture || !ref_methods_dir) {
        return NULL;
    }
    
    char build_cmd[1024];
    snprintf(build_cmd, sizeof(build_cmd), 
        "cd ../.. && "
        "AGERUN_METHODS_DIR=%s RUN_EXEC_DIR=%s make run-exec 2>&1", 
        ref_methods_dir, ref_fixture->temp_build_dir);
    
    return popen(build_cmd, "r");
}

void ar_executable_fixture__destroy_methods_dir(ar_executable_fixture_t *mut_fixture,
                                                char *own_methods_dir) {
    if (!mut_fixture || !own_methods_dir) {
        return;
    }
    
    char cleanup_cmd[512];
    snprintf(cleanup_cmd, sizeof(cleanup_cmd), "rm -rf %s 2>&1", own_methods_dir);
    
    int result = system(cleanup_cmd);
    if (result != 0) {
        printf("WARNING: Failed to remove temporary directory: %s\n", own_methods_dir);
    } else {
        printf("Cleaned up temporary directory: %s\n", own_methods_dir);
    }
    
    // Free the allocated path string
    AR__HEAP__FREE(own_methods_dir);
}