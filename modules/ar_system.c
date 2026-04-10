/* Agerun Runtime System Implementation */
#include "ar_system.h"
#include "ar_method.h"
#include "ar_data.h"
#include "ar_agent.h"
#include "ar_agency.h"
#include "ar_list.h"
#include "ar_map.h"
#include "ar_heap.h"
#include "ar_interpreter.h"
#include "ar_log.h"
#include "ar_delegation.h"
#include "ar_delegate.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <inttypes.h>

/* Constants are now defined in their respective module header files:
 * - MAX_METHODS, MAX_VERSIONS_PER_METHOD, MAX_METHOD_NAME_LENGTH, MAX_INSTRUCTIONS_LENGTH in ar_methodology.h
 * - Lists used for message queues provide dynamic sizing
 * - MAX_AGENTS, MAX_METHOD_NAME_LENGTH in ar_agent.h
 */

/* Memory Map structure is now defined in ar_map.h */

/* Message Queue functionality is now provided by the list module */

/* Method Definition is now in ar_method.h/c */

/* Agent Definition is now in ar_agent.h */

/* System structure */
struct ar_system_s {
    bool is_initialized;
    ar_agency_t *own_agency;         // Always owned by the system
    ar_delegation_t *own_delegation; // Always owned by the system
    ar_interpreter_t *own_interpreter; // Always owned by the system
    ar_log_t *own_log;                // Always owned by the system
    ar_data_t *own_context;           // Shared context for all agents
    int64_t next_agent_hint;          // Search hint for faster message queue scanning
};

/* Implementation */

#define AR_SYSTEM__LOG_DELEGATE_ID (-102)

static bool _process_log_delegate_message(ar_system_t *mut_system, ar_data_t *own_message) {
    const char *ref_level;
    const char *ref_text;
    int64_t agent_id;
    const char *ref_level_string;
    size_t text_len;
    size_t buffer_len;
    char *own_log_message;
    int written;

    if (!mut_system || !own_message) {
        return false;
    }

    if (ar_data__get_type(own_message) != AR_DATA_TYPE__MAP) {
        ar_log__error(mut_system->own_log, "Invalid log delegate message");
        ar_data__destroy_if_owned(own_message, mut_system);
        return true;
    }

    ref_level = ar_data__get_map_string(own_message, "level");
    ref_text = ar_data__get_map_string(own_message, "message");
    agent_id = ar_data__get_map_integer(own_message, "agent_id");

    if (!ref_level || !ref_text) {
        ar_log__error(mut_system->own_log, "Invalid log delegate message");
        ar_data__destroy_if_owned(own_message, mut_system);
        return true;
    }

    ref_level_string = ref_level;
    text_len = strlen(ref_text);
    buffer_len = strlen(ref_level_string) + text_len + 64;
    own_log_message = AR__HEAP__MALLOC(buffer_len, "system log delegate message");
    if (!own_log_message) {
        ar_log__error(mut_system->own_log, "Failed to allocate log delegate message");
        ar_data__destroy_if_owned(own_message, mut_system);
        return true;
    }

    written = snprintf(
        own_log_message,
        buffer_len,
        "level=%s agent=%" PRId64 " message=%s",
        ref_level_string,
        agent_id,
        ref_text
    );
    if (written < 0) {
        AR__HEAP__FREE(own_log_message);
        ar_log__error(mut_system->own_log, "Failed to format log delegate message");
        ar_data__destroy_if_owned(own_message, mut_system);
        return true;
    }

    if (strcmp(ref_level_string, "error") == 0) {
        ar_log__error(mut_system->own_log, own_log_message);
    } else if (strcmp(ref_level_string, "warning") == 0) {
        ar_log__warning(mut_system->own_log, own_log_message);
    } else {
        ar_log__info(mut_system->own_log, own_log_message);
    }

    AR__HEAP__FREE(own_log_message);
    ar_data__destroy_if_owned(own_message, mut_system);
    return true;
}

/* Instance-based API implementation */
ar_system_t* ar_system__create(void) {
    
    ar_system_t *own_system = AR__HEAP__MALLOC(sizeof(ar_system_t), "system");
    if (!own_system) {
        return NULL;
    }
    
    // Initialize fields
    own_system->is_initialized = false;
    own_system->next_agent_hint = 0;
    
    // Create shared context for all agents
    own_system->own_context = ar_data__create_map();
    if (!own_system->own_context) {
        AR__HEAP__FREE(own_system);
        return NULL;
    }
    
    // Create owned resources
    own_system->own_log = ar_log__create();
    if (!own_system->own_log) {
        ar_data__destroy(own_system->own_context);
        AR__HEAP__FREE(own_system);
        return NULL;
    }

    // Create delegation
    own_system->own_delegation = ar_delegation__create(own_system->own_log);
    if (!own_system->own_delegation) {
        ar_log__destroy(own_system->own_log);
        ar_data__destroy(own_system->own_context);
        AR__HEAP__FREE(own_system);
        return NULL;
    }

    // Register built-in log delegate for fallback logging from methods
    ar_delegate_t *own_log_delegate = ar_delegate__create(own_system->own_log, "log");
    if (!own_log_delegate ||
        !ar_delegation__register_delegate(own_system->own_delegation, AR_SYSTEM__LOG_DELEGATE_ID, own_log_delegate)) {
        if (own_log_delegate) {
            ar_delegate__destroy(own_log_delegate);
        }
        ar_delegation__destroy(own_system->own_delegation);
        ar_log__destroy(own_system->own_log);
        ar_data__destroy(own_system->own_context);
        AR__HEAP__FREE(own_system);
        return NULL;
    }

    // Create agency with our log (it will create its own methodology)
    own_system->own_agency = ar_agency__create(own_system->own_log);
    if (!own_system->own_agency) {
        ar_delegation__destroy(own_system->own_delegation);
        ar_log__destroy(own_system->own_log);
        ar_data__destroy(own_system->own_context);
        AR__HEAP__FREE(own_system);
        return NULL;
    }

    own_system->own_interpreter = ar_interpreter__create(own_system->own_log, own_system->own_agency, own_system->own_delegation);
    if (!own_system->own_interpreter) {
        ar_agency__destroy(own_system->own_agency);
        ar_delegation__destroy(own_system->own_delegation);
        ar_log__destroy(own_system->own_log);
        ar_data__destroy(own_system->own_context);
        AR__HEAP__FREE(own_system);
        return NULL;
    }
    
    return own_system;
}

void ar_system__destroy(ar_system_t *own_system) {
    if (!own_system) {
        return;
    }

    // Destroy owned resources
    if (own_system->own_interpreter) {
        ar_interpreter__destroy(own_system->own_interpreter);
    }

    if (own_system->own_agency) {
        ar_agency__destroy(own_system->own_agency);
    }

    if (own_system->own_delegation) {
        ar_delegation__destroy(own_system->own_delegation);
    }

    if (own_system->own_log) {
        ar_log__destroy(own_system->own_log);
    }

    if (own_system->own_context) {
        ar_data__destroy(own_system->own_context);
    }

    AR__HEAP__FREE(own_system);
}

int64_t ar_system__init(ar_system_t *mut_system, const char *ref_method_name, const char *ref_version) {
    if (!mut_system) {
        return 0;
    }
    
    if (mut_system->is_initialized) {
        printf("System instance already initialized\n");
        return 0;
    }
    
    mut_system->is_initialized = true;
    
    // If we have an agency instance, initialize it
    // Auto-loading removed - executable now has full control over when to load files
    
    // Create initial agent if ref_method_name is provided
    if (ref_method_name != NULL) {
        int64_t initial_agent;
        
        // Use instance-based agency with shared context
        initial_agent = ar_agency__create_agent(mut_system->own_agency,
                                                             ref_method_name,
                                                             ref_version,
                                                             mut_system->own_context);
        return initial_agent;
    }
    
    return 0;
}

void ar_system__shutdown(ar_system_t *mut_system) {
    if (!mut_system || !mut_system->is_initialized) {
        return;
    }
    
    // Auto-saving removed - executable now has full control over when to save files
    
    // Reset the agency to clean up all agents
    ar_agency__reset(mut_system->own_agency);
    
    mut_system->is_initialized = false;
    mut_system->next_agent_hint = 0;
}

bool ar_system__process_next_message(ar_system_t *mut_system) {
    int64_t first_agent_id;
    int64_t start_agent_id;
    int64_t agent_id;
    ar_data_t *own_message = NULL;

    if (!mut_system || !mut_system->is_initialized) {
        return false;
    }

    first_agent_id = ar_agency__get_first_agent(mut_system->own_agency);
    if (first_agent_id == 0) {
        mut_system->next_agent_hint = 0;
        return false;
    }

    start_agent_id = mut_system->next_agent_hint;
    if (start_agent_id == 0 || !ar_agency__agent_exists(mut_system->own_agency, start_agent_id)) {
        start_agent_id = first_agent_id;
    }

    agent_id = start_agent_id;
    do {
        own_message = ar_agency__get_agent_message(mut_system->own_agency, agent_id);
        if (own_message != NULL) {
            break;
        }

        agent_id = ar_agency__get_next_agent(mut_system->own_agency, agent_id);
        if (agent_id == 0) {
            agent_id = first_agent_id;
        }
    } while (agent_id != start_agent_id);

    if (own_message == NULL) {
        ar_data_t *own_delegate_message = ar_delegation__take_delegate_message(
            mut_system->own_delegation,
            AR_SYSTEM__LOG_DELEGATE_ID
        );
        if (own_delegate_message == NULL) {
            return false;
        }

        ar_data__take_ownership(own_delegate_message, mut_system);
        return _process_log_delegate_message(mut_system, own_delegate_message);
    }

    // Take ownership of the message for the system
    ar_data__take_ownership(own_message, mut_system);

    ar_interpreter__execute_method(mut_system->own_interpreter, agent_id, own_message, mut_system);

    if (ar_agency__agent_has_messages(mut_system->own_agency, agent_id)) {
        mut_system->next_agent_hint = agent_id;
    } else {
        int64_t next_agent_id = ar_agency__get_next_agent(mut_system->own_agency, agent_id);
        if (next_agent_id == 0) {
            next_agent_id = first_agent_id;
        }
        mut_system->next_agent_hint = next_agent_id;
    }

    // Free the message as it's now been processed
    ar_data__destroy_if_owned(own_message, mut_system);
    own_message = NULL; // Mark as freed
    return true;
}

int ar_system__process_all_messages(ar_system_t *mut_system) {
    int count = 0;
    
    while (ar_system__process_next_message(mut_system)) {
        count++;
    }
    
    return count;
}

ar_agency_t* ar_system__get_agency(const ar_system_t *ref_system) {
    if (!ref_system) {
        return NULL;
    }
    
    return ref_system->own_agency;
}

ar_log_t* ar_system__get_log(const ar_system_t *ref_system) {
    if (!ref_system) {
        return NULL;
    }

    return ref_system->own_log;
}

ar_delegation_t* ar_system__get_delegation(const ar_system_t *ref_system) {
    if (!ref_system) {
        return NULL;
    }
    return ref_system->own_delegation;
}

ar_delegate_registry_t* ar_system__get_delegate_registry(const ar_system_t *ref_system) {
    if (!ref_system || !ref_system->own_delegation) {
        return NULL;
    }

    return ar_delegation__get_registry(ref_system->own_delegation);
}

bool ar_system__register_delegate(ar_system_t *mut_system, int64_t proxy_id, ar_delegate_t *own_proxy) {
    if (!mut_system || !mut_system->own_delegation) {
        return false;
    }

    return ar_delegation__register_delegate(mut_system->own_delegation, proxy_id, own_proxy);
}

