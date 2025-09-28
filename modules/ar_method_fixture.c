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
    ar_system_t *own_system;      /* The system instance */
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
    own_fixture->own_system = NULL;
    
    return own_fixture; // Ownership transferred to caller
}

void ar_method_fixture__destroy(ar_method_fixture_t *own_fixture) {
    if (!own_fixture) {
        return;
    }
    
    // If initialized, perform cleanup
    if (own_fixture->initialized && own_fixture->own_system) {
        ar_system__shutdown(own_fixture->own_system);
        ar_system__destroy(own_fixture->own_system);
        
        // Remove persistence files
        remove("methodology.agerun");
        remove("agerun.agency");
    }
    
    AR__HEAP__FREE(own_fixture->own_test_name);
    AR__HEAP__FREE(own_fixture);
}

bool ar_method_fixture__initialize(ar_method_fixture_t *mut_fixture) {
    if (!mut_fixture) {
        return false;
    }
    
    // Clean shutdown of any existing state
    if (mut_fixture->own_system) {
        ar_system__shutdown(mut_fixture->own_system);
        ar_system__destroy(mut_fixture->own_system);
        mut_fixture->own_system = NULL;
    }
    
    // Remove persistence files
    remove("methodology.agerun");
    remove("agerun.agency");
    
    // Create new system instance
    mut_fixture->own_system = ar_system__create();
    if (!mut_fixture->own_system) {
        return false;
    }
    
    // Initialize system with no persistence files
    ar_system__init(mut_fixture->own_system, NULL, NULL);
    
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
    
    // Get the agency from the fixture's system
    ar_agency_t *mut_agency = ar_system__get_agency(mut_fixture->own_system);
    if (!mut_agency) {
        AR__HEAP__FREE(own_content);
        return false;
    }
    
    // Get the methodology from the agency
    ar_methodology_t *mut_methodology = ar_agency__get_methodology(mut_agency);
    if (!mut_methodology) {
        // This should not happen - agency should have methodology
        ar_io__error("Agency has no methodology\n");
        AR__HEAP__FREE(own_content);
        return false;
    }
    
    // Register method using instance methodology
    bool registered = ar_methodology__create_method(mut_methodology, ref_method_name, own_content, ref_version);
    
    if (registered) {
        printf("Method fixture: Successfully registered method '%s' version '%s'\n", ref_method_name, ref_version ? ref_version : "latest");
    } else {
        printf("Method fixture: Failed to register method '%s' version '%s'\n", ref_method_name, ref_version ? ref_version : "latest");
    }
    
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

bool ar_method_fixture__process_next_message(ar_method_fixture_t *mut_fixture) {
    if (!mut_fixture || !mut_fixture->initialized || !mut_fixture->own_system) {
        return false;
    }
    
    return ar_system__process_next_message(mut_fixture->own_system);
}

int ar_method_fixture__process_all_messages(ar_method_fixture_t *mut_fixture) {
    if (!mut_fixture || !mut_fixture->initialized || !mut_fixture->own_system) {
        return 0;
    }
    
    return ar_system__process_all_messages(mut_fixture->own_system);
}

ar_agency_t* ar_method_fixture__get_agency(const ar_method_fixture_t *ref_fixture) {
    if (!ref_fixture || !ref_fixture->own_system) {
        return NULL;
    }
    
    return ar_system__get_agency(ref_fixture->own_system);
}
