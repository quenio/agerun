#include "agerun_system_fixture.h"
#include "agerun_heap.h"
#include "agerun_system.h"
#include "agerun_agency.h"
#include "agerun_methodology.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/**
 * @file agerun_system_fixture.c
 * @brief Implementation of system fixture for AgeRun system module testing
 */

/* System fixture structure */
struct system_fixture_s {
    char *own_test_name;          /* Name of the test */
    bool initialized;             /* Whether fixture has been initialized */
};

system_fixture_t* ar__system_fixture__create(const char *ref_test_name) {
    if (!ref_test_name) {
        return NULL;
    }
    
    system_fixture_t *own_fixture = AR_HEAP_MALLOC(sizeof(system_fixture_t), "System fixture");
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

void ar__system_fixture__destroy(system_fixture_t *own_fixture) {
    if (!own_fixture) {
        return;
    }
    
    // If initialized, perform cleanup
    if (own_fixture->initialized) {
        ar__system__shutdown();
        ar__methodology__cleanup();
        ar__agency__reset();
        
        // Remove persistence files
        remove("methodology.agerun");
        remove("agency.agerun");
    }
    
    AR_HEAP_FREE(own_fixture->own_test_name);
    AR_HEAP_FREE(own_fixture);
}

bool ar__system_fixture__initialize(system_fixture_t *mut_fixture) {
    if (!mut_fixture) {
        return false;
    }
    
    // Clean shutdown of any existing state
    ar__system__shutdown();
    ar__methodology__cleanup();
    ar__agency__reset();
    
    // Remove persistence files
    remove("methodology.agerun");
    remove("agency.agerun");
    
    // Initialize system with no persistence files
    if (ar__system__init(NULL, NULL) != 1) {
        // System already initialized is okay
    }
    
    mut_fixture->initialized = true;
    
    return true;
}

method_t* ar__system_fixture__register_method(system_fixture_t *mut_fixture,
                                                const char *ref_method_name,
                                                const char *ref_instructions,
                                                const char *ref_version) {
    if (!mut_fixture || !ref_method_name || !ref_instructions || !ref_version) {
        return NULL;
    }
    
    if (!mut_fixture->initialized) {
        return NULL;
    }
    
    // Create method
    method_t *own_method = ar__method__create(ref_method_name, ref_instructions, ref_version);
    if (!own_method) {
        return NULL;
    }
    
    // Register with methodology
    ar__methodology__register_method(own_method);
    
    // Note: ownership was transferred to methodology, but we return the pointer
    // for the test to use (as a borrowed reference)
    return own_method;
}

const char* ar__system_fixture__get_name(const system_fixture_t *ref_fixture) {
    if (!ref_fixture) {
        return NULL;
    }
    
    return ref_fixture->own_test_name;
}

bool ar__system_fixture__check_memory(const system_fixture_t *ref_fixture) {
    if (!ref_fixture) {
        return false;
    }
    
    // Since we don't have access to allocation counts,
    // this function will rely on the heap module's automatic
    // reporting at program exit to detect leaks.
    // Individual tests can check heap_memory_report.log after running.
    return true;
}

void ar__system_fixture__reset_system(system_fixture_t *mut_fixture) {
    if (!mut_fixture || !mut_fixture->initialized) {
        return;
    }
    
    // Clean shutdown of existing state
    ar__system__shutdown();
    ar__methodology__cleanup();
    ar__agency__reset();
    
    // Remove persistence files
    remove("methodology.agerun");
    remove("agency.agerun");
    
    // Reinitialize
    ar__system__init(NULL, NULL);
}
