#include "ar_executable_fixture.h"
#include "ar_heap.h"
#include "ar_assert.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

/**
 * @file ar_executable_fixture.c
 * @brief Implementation of executable fixture module for AgeRun executable testing
 */

/* Executable fixture structure */
struct ar_executable_fixture_s {
    char temp_build_dir[256];     /* Temporary build directory path */
    char complete_runner_path[512]; /* Deterministic complete() runner for executable tests */
    bool initialized;              /* Whether build directory has been initialized */
};

/* Create a deterministic complete() runner for executable tests. */
static void _create_complete_runner(ar_executable_fixture_t *mut_fixture) {
    snprintf(mut_fixture->complete_runner_path, sizeof(mut_fixture->complete_runner_path),
             "%s/deterministic-complete-runner.sh", mut_fixture->temp_build_dir);

    FILE *own_runner = fopen(mut_fixture->complete_runner_path, "w");
    AR_ASSERT(own_runner != NULL, "Should be able to create deterministic complete runner");

    fputs("#!/bin/sh\n", own_runner);
    fputs("prompt=\"\"\n", own_runner);
    fputs("while [ \"$#\" -gt 0 ]; do\n", own_runner);
    fputs("    case \"$1\" in\n", own_runner);
    fputs("        -p)\n", own_runner);
    fputs("            shift\n", own_runner);
    fputs("            prompt=\"$1\"\n", own_runner);
    fputs("            ;;\n", own_runner);
    fputs("    esac\n", own_runner);
    fputs("    shift || break\n", own_runner);
    fputs("done\n", own_runner);
    fputs("case \"$prompt\" in\n", own_runner);
    fputs("    *advance*)\n", own_runner);
    fputs("        printf 'outcome=advance\\nreason=approved\\n'\n", own_runner);
    fputs("        ;;\n", own_runner);
    fputs("    *)\n", own_runner);
    fputs("        printf 'outcome=ready\\nreason=ok\\n'\n", own_runner);
    fputs("        ;;\n", own_runner);
    fputs("esac\n", own_runner);
    fputs("printf 'country=Brazil\\ncity=Brasilia\\ncontinent=South America\\n'\n", own_runner);
    fclose(own_runner);

    int result = chmod(mut_fixture->complete_runner_path, 0700);
    AR_ASSERT(result == 0, "Deterministic complete runner should be executable");
}

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

        _create_complete_runner(mut_fixture);
        
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
    own_fixture->complete_runner_path[0] = '\0';
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
    
    char setup_cmd[2048];
    snprintf(setup_cmd, sizeof(setup_cmd),
        "rm -rf %s 2>/dev/null && "
        "mkdir -p %s && "
        "cp ../../methods/* %s/ && "
        "mkdir -p %s/workflows && "
        "cp ../../workflows/* %s/workflows/",
        own_methods_dir, own_methods_dir, own_methods_dir,
        mut_fixture->temp_build_dir, mut_fixture->temp_build_dir);
    
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
    return ar_executable_fixture__build_and_run_with_options(
        ref_fixture,
        ref_methods_dir,
        NULL,
        false);
}

FILE* ar_executable_fixture__build_and_run_with_boot_method(
    const ar_executable_fixture_t *ref_fixture,
    const char *ref_methods_dir,
    const char *ref_boot_method) {
    return ar_executable_fixture__build_and_run_with_options(
        ref_fixture,
        ref_methods_dir,
        ref_boot_method,
        false);
}

FILE* ar_executable_fixture__build_and_run_with_options(
    const ar_executable_fixture_t *ref_fixture,
    const char *ref_methods_dir,
    const char *ref_boot_method,
    bool no_persistence) {
    if (!ref_fixture || !ref_methods_dir) {
        return NULL;
    }

    char mut_complete_env[640] = "";
    const char *ref_complete_runner = getenv("AGERUN_COMPLETE_RUNNER");
    if ((ref_complete_runner == NULL || ref_complete_runner[0] == '\0') &&
        ref_fixture->complete_runner_path[0] != '\0') {
        snprintf(mut_complete_env, sizeof(mut_complete_env),
                 "AGERUN_COMPLETE_RUNNER=%s ",
                 ref_fixture->complete_runner_path);
    }

    char mut_build_cmd[1536];
    if (ref_boot_method && ref_boot_method[0] != '\0' && no_persistence) {
        snprintf(mut_build_cmd, sizeof(mut_build_cmd),
            "cd ../.. && "
            "%sAGERUN_METHODS_DIR=%s RUN_EXEC_DIR=%s make run-exec "
            "BOOT_METHOD=%s NO_PERSISTENCE=1 2>&1",
            mut_complete_env,
            ref_methods_dir,
            ref_fixture->temp_build_dir,
            ref_boot_method);
    } else if (ref_boot_method && ref_boot_method[0] != '\0') {
        snprintf(mut_build_cmd, sizeof(mut_build_cmd),
            "cd ../.. && "
            "%sAGERUN_METHODS_DIR=%s RUN_EXEC_DIR=%s make run-exec BOOT_METHOD=%s 2>&1",
            mut_complete_env,
            ref_methods_dir,
            ref_fixture->temp_build_dir,
            ref_boot_method);
    } else if (no_persistence) {
        snprintf(mut_build_cmd, sizeof(mut_build_cmd),
            "cd ../.. && "
            "%sAGERUN_METHODS_DIR=%s RUN_EXEC_DIR=%s make run-exec NO_PERSISTENCE=1 2>&1",
            mut_complete_env,
            ref_methods_dir,
            ref_fixture->temp_build_dir);
    } else {
        snprintf(mut_build_cmd, sizeof(mut_build_cmd),
            "cd ../.. && "
            "%sAGERUN_METHODS_DIR=%s RUN_EXEC_DIR=%s make run-exec 2>&1",
            mut_complete_env,
            ref_methods_dir,
            ref_fixture->temp_build_dir);
    }

    return popen(mut_build_cmd, "r");
}

FILE* ar_executable_fixture__build_and_run_with_extra_args(
    const ar_executable_fixture_t *ref_fixture,
    const char *ref_methods_dir,
    const char *ref_extra_args) {
    if (!ref_fixture || !ref_methods_dir || !ref_extra_args || ref_extra_args[0] == '\0') {
        return NULL;
    }

    char mut_complete_env[640] = "";
    const char *ref_complete_runner = getenv("AGERUN_COMPLETE_RUNNER");
    if ((ref_complete_runner == NULL || ref_complete_runner[0] == '\0') &&
        ref_fixture->complete_runner_path[0] != '\0') {
        snprintf(mut_complete_env, sizeof(mut_complete_env),
                 "AGERUN_COMPLETE_RUNNER=%s ",
                 ref_fixture->complete_runner_path);
    }

    char mut_build_cmd[1536];
    snprintf(mut_build_cmd, sizeof(mut_build_cmd),
        "cd ../.. && "
        "%sAGERUN_METHODS_DIR=%s RUN_EXEC_DIR=%s make run-exec AGERUN_ARGS='%s' 2>&1",
        mut_complete_env,
        ref_methods_dir,
        ref_fixture->temp_build_dir,
        ref_extra_args);

    return popen(mut_build_cmd, "r");
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

const char* ar_executable_fixture__get_build_dir(const ar_executable_fixture_t *ref_fixture) {
    if (!ref_fixture) {
        return NULL;
    }
    return ref_fixture->temp_build_dir;
}

void ar_executable_fixture__clean_persisted_files(const ar_executable_fixture_t *ref_fixture) {
    if (!ref_fixture || !ref_fixture->initialized) {
        return;
    }

    // Remove methodology file
    char methodology_path[512];
    snprintf(methodology_path, sizeof(methodology_path), "%s/agerun.methodology", ref_fixture->temp_build_dir);
    remove(methodology_path);

    // Remove agency file
    char agency_path[512];
    snprintf(agency_path, sizeof(agency_path), "%s/agerun.agency", ref_fixture->temp_build_dir);
    remove(agency_path);

    // Remove log file
    char log_path[512];
    snprintf(log_path, sizeof(log_path), "%s/agerun.log", ref_fixture->temp_build_dir);
    remove(log_path);
}
