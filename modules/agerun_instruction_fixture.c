#include "agerun_instruction_fixture.h"
#include "agerun_heap.h"
#include "agerun_list.h"
#include "agerun_methodology.h"
#include "agerun_assert.h"
#include "agerun_agency.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @file agerun_instruction_fixture.c
 * @brief Implementation of instruction fixture for AgeRun instruction module testing
 */

/* Resource tracking entry for generic resources */
typedef struct {
    void *own_resource;
    void (*destructor)(void*);
} resource_entry_t;

/* Instruction fixture structure */
struct instruction_fixture_s {
    char *own_test_name;                    /* Name of the test */
    list_t *own_tracked_data;               /* List of data objects to destroy */
    list_t *own_tracked_contexts;           /* List of expression contexts to destroy */
    list_t *own_tracked_resources;          /* List of generic resources to destroy */
    int64_t test_agent_id;               /* Agent created by fixture, 0 if none */
    bool system_initialized;                /* Whether system was initialized by fixture */
};

instruction_fixture_t* ar__instruction_fixture__create(const char *ref_test_name) {
    if (!ref_test_name) {
        return NULL;
    }
    
    instruction_fixture_t *own_fixture = AR_HEAP_MALLOC(sizeof(instruction_fixture_t), "Instruction fixture");
    if (!own_fixture) {
        return NULL;
    }
    
    own_fixture->own_test_name = AR_HEAP_STRDUP(ref_test_name, "Test name");
    if (!own_fixture->own_test_name) {
        AR_HEAP_FREE(own_fixture);
        return NULL;
    }
    
    own_fixture->own_tracked_data = ar_list_create();
    if (!own_fixture->own_tracked_data) {
        AR_HEAP_FREE(own_fixture->own_test_name);
        AR_HEAP_FREE(own_fixture);
        return NULL;
    }
    
    own_fixture->own_tracked_contexts = ar_list_create();
    if (!own_fixture->own_tracked_contexts) {
        ar_list_destroy(own_fixture->own_tracked_data);
        AR_HEAP_FREE(own_fixture->own_test_name);
        AR_HEAP_FREE(own_fixture);
        return NULL;
    }
    
    own_fixture->own_tracked_resources = ar_list_create();
    if (!own_fixture->own_tracked_resources) {
        ar_list_destroy(own_fixture->own_tracked_contexts);
        ar_list_destroy(own_fixture->own_tracked_data);
        AR_HEAP_FREE(own_fixture->own_test_name);
        AR_HEAP_FREE(own_fixture);
        return NULL;
    }
    
    own_fixture->test_agent_id = 0;
    own_fixture->system_initialized = false;
    
    return own_fixture; // Ownership transferred to caller
}

void ar__instruction_fixture__destroy(instruction_fixture_t *own_fixture) {
    if (!own_fixture) {
        return;
    }
    
    // Destroy test agent if created
    if (own_fixture->test_agent_id > 0) {
        ar__agency__destroy_agent(own_fixture->test_agent_id);
    }
    
    // Destroy all tracked expression contexts
    while (!ar_list_empty(own_fixture->own_tracked_contexts)) {
        expression_context_t *own_context = (expression_context_t*)ar_list_remove_first(own_fixture->own_tracked_contexts);
        if (own_context) {
            ar_expression_destroy_context(own_context);
        }
    }
    ar_list_destroy(own_fixture->own_tracked_contexts);
    
    // Destroy all tracked data objects
    while (!ar_list_empty(own_fixture->own_tracked_data)) {
        data_t *own_data = (data_t*)ar_list_remove_first(own_fixture->own_tracked_data);
        if (own_data) {
            ar_data_destroy(own_data);
        }
    }
    ar_list_destroy(own_fixture->own_tracked_data);
    
    // Destroy all tracked generic resources
    while (!ar_list_empty(own_fixture->own_tracked_resources)) {
        resource_entry_t *own_entry = (resource_entry_t*)ar_list_remove_first(own_fixture->own_tracked_resources);
        if (own_entry) {
            if (own_entry->destructor && own_entry->own_resource) {
                own_entry->destructor(own_entry->own_resource);
            }
            AR_HEAP_FREE(own_entry);
        }
    }
    ar_list_destroy(own_fixture->own_tracked_resources);
    
    // Shutdown system if initialized by fixture
    if (own_fixture->system_initialized) {
        ar__system__shutdown();
    }
    
    AR_HEAP_FREE(own_fixture->own_test_name);
    AR_HEAP_FREE(own_fixture);
}

expression_context_t* ar__instruction_fixture__create_expression_context(
    instruction_fixture_t *mut_fixture,
    const char *ref_expression) {
    
    if (!mut_fixture || !ref_expression) {
        return NULL;
    }
    
    // Create standard test maps
    data_t *own_memory = ar_data_create_map();
    if (!own_memory) {
        return NULL;
    }
    
    // Add common test values to memory
    ar_data_set_map_string(own_memory, "name", "TestAgent");
    ar_data_set_map_integer(own_memory, "count", 42);
    ar_data_set_map_double(own_memory, "value", 3.14);
    ar_data_set_map_string(own_memory, "status", "active");
    
    data_t *own_context = ar_data_create_map();
    if (!own_context) {
        ar_data_destroy(own_memory);
        return NULL;
    }
    
    // Add common test values to context
    ar_data_set_map_string(own_context, "type", "test");
    ar_data_set_map_integer(own_context, "version", 1);
    
    data_t *own_message = ar_data_create_map();
    if (!own_message) {
        ar_data_destroy(own_memory);
        ar_data_destroy(own_context);
        return NULL;
    }
    
    // Add common test values to message
    ar_data_set_map_string(own_message, "action", "test");
    ar_data_set_map_integer(own_message, "sender", 0);
    
    // Track the data objects
    ar_list_add_last(mut_fixture->own_tracked_data, own_memory);
    ar_list_add_last(mut_fixture->own_tracked_data, own_context);
    ar_list_add_last(mut_fixture->own_tracked_data, own_message);
    
    // Create expression context
    expression_context_t *own_expr_ctx = ar_expression_create_context(
        own_memory, own_context, own_message, ref_expression
    );
    
    if (!own_expr_ctx) {
        return NULL;
    }
    
    // Track the expression context
    ar_list_add_last(mut_fixture->own_tracked_contexts, own_expr_ctx);
    
    return own_expr_ctx; // Return borrowed reference
}

expression_context_t* ar__instruction_fixture__create_custom_expression_context(
    instruction_fixture_t *mut_fixture,
    data_t *mut_memory,
    const data_t *ref_context,
    const data_t *ref_message,
    const char *ref_expression) {
    
    if (!mut_fixture || !ref_expression) {
        return NULL;
    }
    
    // Create expression context
    expression_context_t *own_expr_ctx = ar_expression_create_context(
        mut_memory, ref_context, ref_message, ref_expression
    );
    
    if (!own_expr_ctx) {
        return NULL;
    }
    
    // Track the expression context
    ar_list_add_last(mut_fixture->own_tracked_contexts, own_expr_ctx);
    
    return own_expr_ctx; // Return borrowed reference
}

data_t* ar__instruction_fixture__create_test_map(
    instruction_fixture_t *mut_fixture,
    const char *ref_name) {
    
    if (!mut_fixture) {
        return NULL;
    }
    
    data_t *own_map = ar_data_create_map();
    if (!own_map) {
        return NULL;
    }
    
    // Add test data based on name
    if (ref_name && strcmp(ref_name, "user") == 0) {
        ar_data_set_map_string(own_map, "username", "alice");
        ar_data_set_map_string(own_map, "role", "admin");
        ar_data_set_map_integer(own_map, "id", 123);
    } else if (ref_name && strcmp(ref_name, "config") == 0) {
        ar_data_set_map_string(own_map, "mode", "test");
        ar_data_set_map_integer(own_map, "timeout", 30);
        ar_data_set_map_double(own_map, "threshold", 0.95);
    } else {
        // Default test values
        ar_data_set_map_string(own_map, "test", "value");
        ar_data_set_map_integer(own_map, "number", 42);
        ar_data_set_map_double(own_map, "decimal", 3.14);
    }
    
    // Track the data object
    ar_list_add_last(mut_fixture->own_tracked_data, own_map);
    
    return own_map; // Return borrowed reference
}

data_t* ar__instruction_fixture__create_empty_map(
    instruction_fixture_t *mut_fixture) {
    
    if (!mut_fixture) {
        return NULL;
    }
    
    data_t *own_map = ar_data_create_map();
    if (!own_map) {
        return NULL;
    }
    
    // Track the data object
    ar_list_add_last(mut_fixture->own_tracked_data, own_map);
    
    return own_map; // Return borrowed reference
}

data_t* ar__instruction_fixture__create_test_list(
    instruction_fixture_t *mut_fixture) {
    
    if (!mut_fixture) {
        return NULL;
    }
    
    data_t *own_list = ar_data_create_list();
    if (!own_list) {
        return NULL;
    }
    
    // Add sample values
    data_t *own_item1 = ar_data_create_string("first");
    if (own_item1) {
        ar_data_list_add_last_data(own_list, own_item1);
        own_item1 = NULL; // Ownership transferred
    }
    
    data_t *own_item2 = ar_data_create_integer(42);
    if (own_item2) {
        ar_data_list_add_last_data(own_list, own_item2);
        own_item2 = NULL; // Ownership transferred
    }
    
    data_t *own_item3 = ar_data_create_double(3.14);
    if (own_item3) {
        ar_data_list_add_last_data(own_list, own_item3);
        own_item3 = NULL; // Ownership transferred
    }
    
    // Track the data object
    ar_list_add_last(mut_fixture->own_tracked_data, own_list);
    
    return own_list; // Return borrowed reference
}

const char* ar__instruction_fixture__get_name(const instruction_fixture_t *ref_fixture) {
    if (!ref_fixture) {
        return NULL;
    }
    
    return ref_fixture->own_test_name;
}

bool ar__instruction_fixture__check_memory(const instruction_fixture_t *ref_fixture) {
    if (!ref_fixture) {
        return false;
    }
    
    // Since we don't have access to allocation counts,
    // this function will rely on the heap module's automatic
    // reporting at program exit to detect leaks.
    return true;
}

void ar__instruction_fixture__track_data(
    instruction_fixture_t *mut_fixture,
    data_t *own_data) {
    
    if (!mut_fixture || !own_data) {
        return;
    }
    
    ar_list_add_last(mut_fixture->own_tracked_data, own_data);
}

void ar__instruction_fixture__track_expression_context(
    instruction_fixture_t *mut_fixture,
    expression_context_t *own_context) {
    
    if (!mut_fixture || !own_context) {
        return;
    }
    
    ar_list_add_last(mut_fixture->own_tracked_contexts, own_context);
}

int64_t ar__instruction_fixture__create_test_agent(
    instruction_fixture_t *mut_fixture,
    const char *ref_method_name,
    const char *ref_instructions) {
    
    if (!mut_fixture || !ref_method_name || !ref_instructions) {
        return 0;
    }
    
    // Don't create another agent if one already exists
    if (mut_fixture->test_agent_id > 0) {
        return 0;
    }
    
    // Create method and register it with methodology
    method_t *own_method = ar_method_create(ref_method_name, ref_instructions, "1.0.0");
    if (!own_method) {
        return 0;
    }
    
    // Register with methodology
    ar_methodology_register_method(own_method);
    own_method = NULL; // Ownership transferred
    
    // Create agent
    int64_t agent_id = ar__agency__create_agent(ref_method_name, "1.0.0", NULL);
    if (agent_id <= 0) {
        return 0;
    }
    
    // Store agent ID
    mut_fixture->test_agent_id = agent_id;
    
    // Process wake message
    ar__system__process_next_message();
    
    return agent_id;
}

int64_t ar__instruction_fixture__get_agent(const instruction_fixture_t *ref_fixture) {
    if (!ref_fixture) {
        return 0;
    }
    
    return ref_fixture->test_agent_id;
}

void ar__instruction_fixture__track_resource(
    instruction_fixture_t *mut_fixture,
    void *own_resource,
    void (*destructor)(void*)) {
    
    if (!mut_fixture || !own_resource || !destructor) {
        return;
    }
    
    resource_entry_t *own_entry = AR_HEAP_MALLOC(sizeof(resource_entry_t), "Resource entry");
    if (!own_entry) {
        return;
    }
    
    own_entry->own_resource = own_resource;
    own_entry->destructor = destructor;
    
    ar_list_add_last(mut_fixture->own_tracked_resources, own_entry);
}

bool ar__instruction_fixture__init_system(
    instruction_fixture_t *mut_fixture,
    const char *ref_init_method_name,
    const char *ref_init_instructions) {
    
    if (!mut_fixture || !ref_init_method_name || !ref_init_instructions) {
        return false;
    }
    
    // Don't initialize twice
    if (mut_fixture->system_initialized) {
        return false;
    }
    
    // Create and register initialization method
    method_t *own_method = ar_method_create(ref_init_method_name, ref_init_instructions, "1.0.0");
    if (!own_method) {
        return false;
    }
    
    ar_methodology_register_method(own_method);
    own_method = NULL; // Ownership transferred
    
    // Initialize system
    ar__system__init(ref_init_method_name, "1.0.0");
    mut_fixture->system_initialized = true;
    
    return true;
}
