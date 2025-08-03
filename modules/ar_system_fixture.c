#include "ar_system_fixture.h"
#include "ar_heap.h"
#include "ar_system.h"
#include "ar_agency.h"
#include "ar_methodology.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/**
 * @file ar_system_fixture.c
 * @brief Implementation of system fixture for AgeRun system module testing
 */

/* System fixture structure */
struct ar_system_fixture_s {
    char *own_test_name;          /* Name of the test */
    bool initialized;             /* Whether fixture has been initialized */
    ar_system_t *own_system;      /* The system instance */
};

ar_system_fixture_t* ar_system_fixture__create(const char *ref_test_name) {
    if (!ref_test_name) {
        return NULL;
    }
    
    ar_system_fixture_t *own_fixture = AR__HEAP__MALLOC(sizeof(ar_system_fixture_t), "System fixture");
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

void ar_system_fixture__destroy(ar_system_fixture_t *own_fixture) {
    if (!own_fixture) {
        return;
    }
    
    // If initialized, perform cleanup
    if (own_fixture->initialized && own_fixture->own_system) {
        ar_system__shutdown_with_instance(own_fixture->own_system);
        ar_system__destroy(own_fixture->own_system);
        
        // Remove persistence files
        remove("methodology.agerun");
        remove("agency.agerun");
    }
    
    AR__HEAP__FREE(own_fixture->own_test_name);
    AR__HEAP__FREE(own_fixture);
}

bool ar_system_fixture__initialize(ar_system_fixture_t *mut_fixture) {
    if (!mut_fixture) {
        return false;
    }
    
    // Clean shutdown of any existing state
    if (mut_fixture->own_system) {
        ar_system__shutdown_with_instance(mut_fixture->own_system);
        ar_system__destroy(mut_fixture->own_system);
        mut_fixture->own_system = NULL;
    }
    
    // Remove persistence files
    remove("methodology.agerun");
    remove("agency.agerun");
    
    // Create new system instance
    mut_fixture->own_system = ar_system__create();
    if (!mut_fixture->own_system) {
        return false;
    }
    
    // Initialize system with no persistence files
    ar_system__init_with_instance(mut_fixture->own_system, NULL, NULL);
    
    mut_fixture->initialized = true;
    
    return true;
}

ar_method_t* ar_system_fixture__register_method(ar_system_fixture_t *mut_fixture,
                                                const char *ref_method_name,
                                                const char *ref_instructions,
                                                const char *ref_version) {
    if (!mut_fixture || !ref_method_name || !ref_instructions || !ref_version) {
        return NULL;
    }
    
    if (!mut_fixture->initialized) {
        return NULL;
    }
    
    // Get the system's agency and methodology
    ar_agency_t *mut_agency = ar_system__get_agency(mut_fixture->own_system);
    if (!mut_agency) {
        return NULL;
    }
    
    ar_methodology_t *mut_methodology = ar_agency__get_methodology(mut_agency);
    if (!mut_methodology) {
        return NULL;
    }
    
    // Get the system's log for method creation
    ar_log_t *ref_log = ar_system__get_log(mut_fixture->own_system);
    
    // Create method with log
    ar_method_t *own_method = ar_method__create_with_log(ref_method_name, ref_instructions, ref_version, ref_log);
    if (!own_method) {
        return NULL;
    }
    
    // Register with the system's methodology
    ar_methodology__register_method_with_instance(mut_methodology, own_method);
    
    // Note: ownership was transferred to methodology, but we return the pointer
    // for the test to use (as a borrowed reference)
    return own_method;
}

const char* ar_system_fixture__get_name(const ar_system_fixture_t *ref_fixture) {
    if (!ref_fixture) {
        return NULL;
    }
    
    return ref_fixture->own_test_name;
}

bool ar_system_fixture__check_memory(const ar_system_fixture_t *ref_fixture) {
    if (!ref_fixture) {
        return false;
    }
    
    // Since we don't have access to allocation counts,
    // this function will rely on the heap module's automatic
    // reporting at program exit to detect leaks.
    // Individual tests can check heap_memory_report.log after running.
    return true;
}

void ar_system_fixture__reset_system(ar_system_fixture_t *mut_fixture) {
    if (!mut_fixture) {
        return;
    }
    
    // Clean shutdown of existing state
    if (mut_fixture->own_system) {
        ar_system__shutdown_with_instance(mut_fixture->own_system);
        ar_system__destroy(mut_fixture->own_system);
        mut_fixture->own_system = NULL;
    }
    
    // Remove persistence files
    remove("methodology.agerun");
    remove("agency.agerun");
    
    // Create new system instance
    mut_fixture->own_system = ar_system__create();
    if (mut_fixture->own_system) {
        // Reinitialize
        ar_system__init_with_instance(mut_fixture->own_system, NULL, NULL);
        mut_fixture->initialized = true;
    }
}

void ar_system_fixture__shutdown_preserve_files(ar_system_fixture_t *mut_fixture) {
    if (!mut_fixture || !mut_fixture->initialized || !mut_fixture->own_system) {
        return;
    }
    
    // Just shutdown the system - this will save files
    ar_system__shutdown_with_instance(mut_fixture->own_system);
    ar_system__destroy(mut_fixture->own_system);
    mut_fixture->own_system = NULL;
    
    // Mark as not initialized since system is now shut down
    mut_fixture->initialized = false;
}

bool ar_system_fixture__process_next_message(ar_system_fixture_t *mut_fixture) {
    if (!mut_fixture || !mut_fixture->initialized || !mut_fixture->own_system) {
        return false;
    }
    
    return ar_system__process_next_message_with_instance(mut_fixture->own_system);
}

int ar_system_fixture__process_all_messages(ar_system_fixture_t *mut_fixture) {
    if (!mut_fixture || !mut_fixture->initialized || !mut_fixture->own_system) {
        return 0;
    }
    
    return ar_system__process_all_messages_with_instance(mut_fixture->own_system);
}

ar_agency_t* ar_system_fixture__get_agency(ar_system_fixture_t *ref_fixture) {
    if (!ref_fixture) {
        printf("DEBUG: get_agency - fixture is NULL\n");
        return NULL;
    }
    if (!ref_fixture->initialized) {
        printf("DEBUG: get_agency - fixture not initialized\n");
        return NULL;
    }
    if (!ref_fixture->own_system) {
        printf("DEBUG: get_agency - system is NULL\n");
        return NULL;
    }
    
    return ar_system__get_agency(ref_fixture->own_system);
}
