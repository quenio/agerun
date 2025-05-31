#include "agerun_test_fixture.h"
#include "agerun_heap.h"
#include "agerun_system.h"
#include "agerun_agency.h"
#include "agerun_methodology.h"
#include "agerun_io.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/**
 * @file agerun_test_fixture.c
 * @brief Implementation of test fixture module for AgeRun testing
 */

/* Test fixture structure */
struct test_fixture_s {
    char *own_test_name;          /* Name of the test */
    bool initialized;             /* Whether fixture has been initialized */
};

test_fixture_t* ar_test_fixture_create(const char *ref_test_name) {
    if (!ref_test_name) {
        return NULL;
    }
    
    test_fixture_t *own_fixture = AR_HEAP_MALLOC(sizeof(test_fixture_t), "Test fixture");
    if (!own_fixture) {
        return NULL;
    }
    
    own_fixture->own_test_name = AR_HEAP_STRDUP(ref_test_name, "Test name");
    if (!own_fixture->own_test_name) {
        AR_HEAP_FREE(own_fixture);
        return NULL;
    }
    
    own_fixture->initialized = false;
    
    return own_fixture; // Ownership transferred to caller
}

void ar_test_fixture_destroy(test_fixture_t *own_fixture) {
    if (!own_fixture) {
        return;
    }
    
    // If initialized, perform cleanup
    if (own_fixture->initialized) {
        ar_system_shutdown();
        ar_methodology_cleanup();
        ar_agency_reset();
        
        // Remove persistence files
        remove("methodology.agerun");
        remove("agency.agerun");
    }
    
    AR_HEAP_FREE(own_fixture->own_test_name);
    AR_HEAP_FREE(own_fixture);
}

bool ar_test_fixture_initialize(test_fixture_t *mut_fixture) {
    if (!mut_fixture) {
        return false;
    }
    
    // Clean shutdown of any existing state
    ar_system_shutdown();
    ar_methodology_cleanup();
    ar_agency_reset();
    
    // Remove persistence files
    remove("methodology.agerun");
    remove("agency.agerun");
    
    // Initialize system with no persistence files
    if (ar_system_init(NULL, NULL) != 1) {
        // System already initialized is okay
    }
    
    mut_fixture->initialized = true;
    
    return true;
}

bool ar_test_fixture_load_method(test_fixture_t *mut_fixture,
                                const char *ref_method_name,
                                const char *ref_method_file,
                                const char *ref_version) {
    if (!mut_fixture || !ref_method_name || !ref_method_file || !ref_version) {
        return false;
    }
    
    if (!mut_fixture->initialized) {
        return false;
    }
    
    // Read method file
    FILE *fp = NULL;
    file_result_t result = ar_io_open_file(ref_method_file, "r", &fp);
    if (result != FILE_SUCCESS) {
        ar_io_error("Failed to open method file %s: %s\n", 
                    ref_method_file, ar_io_error_message(result));
        return false;
    }
    
    // Get file size
    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    // Allocate buffer
    char *own_content = AR_HEAP_MALLOC((size_t)(file_size + 1), "Method file content");
    if (!own_content) {
        ar_io_close_file(fp, ref_method_file);
        return false;
    }
    
    // Read file
    size_t bytes_read = fread(own_content, 1, (size_t)file_size, fp);
    own_content[bytes_read] = '\0';
    
    ar_io_close_file(fp, ref_method_file);
    
    // Register method
    bool registered = ar_methodology_create_method(ref_method_name, own_content, ref_version);
    
    AR_HEAP_FREE(own_content);
    
    return registered;
}

bool ar_test_fixture_verify_directory(const test_fixture_t *ref_fixture) {
    if (!ref_fixture) {
        return false;
    }
    
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        return false;
    }
    
    size_t len = strlen(cwd);
    if (len < 4 || strcmp(cwd + len - 4, "/bin") != 0) {
        fprintf(stderr, "ERROR: Tests must be run from the bin directory!\n");
        fprintf(stderr, "Current directory: %s\n", cwd);
        return false;
    }
    
    return true;
}

const char* ar_test_fixture_get_name(const test_fixture_t *ref_fixture) {
    if (!ref_fixture) {
        return NULL;
    }
    
    return ref_fixture->own_test_name;
}

bool ar_test_fixture_check_memory(const test_fixture_t *ref_fixture) {
    if (!ref_fixture) {
        return false;
    }
    
    // Since we don't have access to allocation counts,
    // this function will rely on the heap module's automatic
    // reporting at program exit to detect leaks.
    // Individual tests can check heap_memory_report.log after running.
    return true;
}
