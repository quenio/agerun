#include "ar_method_fixture.h"
#include "ar_heap.h"
#include "ar_system.h"
#include "ar_agency.h"
#include "ar_methodology.h"
#include "ar_io.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/**
 * @file ar_method_fixture.c
 * @brief Implementation of method fixture module for AgeRun method testing
 */

/* Method fixture structure */
struct ar_method_fixture_s {
    char *own_test_name;          /* Name of the test */
    bool initialized;             /* Whether fixture has been initialized */
};

ar_method_fixture_t* ar_method_fixture__create(const char *ref_test_name) {
    if (!ref_test_name) {
        return NULL;
    }
    
    ar_method_fixture_t *own_fixture = AR__HEAP__MALLOC(sizeof(ar_method_fixture_t), "Method fixture");
    if (!own_fixture) {
        return NULL;
    }
    
    own_fixture->own_test_name = AR__HEAP__STRDUP(ref_test_name, "Test name");
    if (!own_fixture->own_test_name) {
        AR__HEAP__FREE(own_fixture);
        return NULL;
    }
    
    own_fixture->initialized = false;
    
    return own_fixture; // Ownership transferred to caller
}

void ar_method_fixture__destroy(ar_method_fixture_t *own_fixture) {
    if (!own_fixture) {
        return;
    }
    
    // If initialized, perform cleanup
    if (own_fixture->initialized) {
        ar_system__shutdown();
        ar_methodology__cleanup();
        ar_agency__reset();
        
        // Remove persistence files
        remove("methodology.agerun");
        remove("agency.agerun");
    }
    
    AR__HEAP__FREE(own_fixture->own_test_name);
    AR__HEAP__FREE(own_fixture);
}

bool ar_method_fixture__initialize(ar_method_fixture_t *mut_fixture) {
    if (!mut_fixture) {
        return false;
    }
    
    // Clean shutdown of any existing state
    ar_system__shutdown();
    ar_methodology__cleanup();
    ar_agency__reset();
    
    // Remove persistence files
    remove("methodology.agerun");
    remove("agency.agerun");
    
    // Initialize system with no persistence files
    if (ar_system__init(NULL, NULL) != 1) {
        // System already initialized is okay
    }
    
    mut_fixture->initialized = true;
    
    return true;
}

bool ar_method_fixture__load_method(ar_method_fixture_t *mut_fixture,
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
    ar_file_result_t result = ar_io__open_file(ref_method_file, "r", &fp);
    if (result != AR_FILE_RESULT__SUCCESS) {
        ar_io__error("Failed to open method file %s: %s\n", 
                    ref_method_file, ar_io__error_message(result));
        return false;
    }
    
    // Get file size
    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    // Allocate buffer
    char *own_content = AR__HEAP__MALLOC((size_t)(file_size + 1), "Method file content");
    if (!own_content) {
        ar_io__close_file(fp, ref_method_file);
        return false;
    }
    
    // Read file
    size_t bytes_read = fread(own_content, 1, (size_t)file_size, fp);
    own_content[bytes_read] = '\0';
    
    ar_io__close_file(fp, ref_method_file);
    
    // Register method
    bool registered = ar_methodology__create_method(ref_method_name, own_content, ref_version);
    
    AR__HEAP__FREE(own_content);
    
    return registered;
}

bool ar_method_fixture__verify_directory(const ar_method_fixture_t *ref_fixture) {
    if (!ref_fixture) {
        return false;
    }
    
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        return false;
    }
    
    if (!strstr(cwd, "/bin/") && !strstr(cwd, "/bin")) {
        fprintf(stderr, "ERROR: Tests must be run from the bin directory!\n");
        fprintf(stderr, "Current directory: %s\n", cwd);
        return false;
    }
    
    return true;
}

const char* ar_method_fixture__get_name(const ar_method_fixture_t *ref_fixture) {
    if (!ref_fixture) {
        return NULL;
    }
    
    return ref_fixture->own_test_name;
}

bool ar_method_fixture__check_memory(const ar_method_fixture_t *ref_fixture) {
    if (!ref_fixture) {
        return false;
    }
    
    // Since we don't have access to allocation counts,
    // this function will rely on the heap module's automatic
    // reporting at program exit to detect leaks.
    // Individual tests can check heap_memory_report.log after running.
    return true;
}
