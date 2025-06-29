#include "ar_log.h"
#include "ar_heap.h"
#include "ar_io.h"
#include "ar_list.h"
#include "ar_event.h"
#include <stdlib.h>

#define LOG_FILE_NAME "agerun.log"
#define BUFFER_LIMIT 10

struct ar_log_s {
    list_t *own_events;     /* Owned list of events */
    FILE *file;             /* Log file handle */
};

ar_log_t* ar_log__create(void) {
    ar_log_t *own_log = AR__HEAP__MALLOC(sizeof(ar_log_t), "log");
    if (!own_log) {
        return NULL;
    }
    
    own_log->own_events = ar_list__create();
    if (!own_log->own_events) {
        AR__HEAP__FREE(own_log);
        return NULL;
    }
    
    // Open log file in append mode
    file_result_t result = ar_io__open_file(LOG_FILE_NAME, "a", &own_log->file);
    if (result != FILE_SUCCESS) {
        ar_list__destroy(own_log->own_events);
        AR__HEAP__FREE(own_log);
        return NULL;
    }
    
    return own_log;
}

void ar_log__destroy(ar_log_t *own_log) {
    if (!own_log) {
        return;
    }
    
    // TODO: In future cycles, flush any buffered events here
    
    // Close the log file
    if (own_log->file) {
        ar_io__close_file(own_log->file, LOG_FILE_NAME);
    }
    
    // Destroy the events list and all events in it
    if (own_log->own_events) {
        // Clean up all events in the list
        while (!ar_list__empty(own_log->own_events)) {
            ar_event_t *own_event = ar_list__remove_first(own_log->own_events);
            if (own_event) {
                ar_event__destroy(own_event);
            }
        }
        ar_list__destroy(own_log->own_events);
    }
    
    AR__HEAP__FREE(own_log);
}

// Helper function to add an event to the buffer
static void _buffer_event(ar_log_t *mut_log, ar_event_t *own_event) {
    if (!mut_log || !own_event) {
        return;
    }
    
    // Add event to the buffer
    ar_list__add_last(mut_log->own_events, own_event);
    
    // Note: Event ownership is now with the list
    // We'll handle buffer flushing in future cycles
}

// Helper function to add an event to the log
static void _add_event(ar_log_t *mut_log, ar_event_type_t type, const char *message) {
    if (!mut_log || !message) {
        return;
    }
    
    // Create an event of the specified type
    ar_event_t *own_event = ar_event__create_typed(type, message);
    if (!own_event) {
        return;
    }
    
    _buffer_event(mut_log, own_event);
}

// Helper function to add an event with position to the log
static void _add_event_at(ar_log_t *mut_log, ar_event_type_t type, const char *message, int position) {
    if (!mut_log || !message) {
        return;
    }
    
    // Create an event with position
    ar_event_t *own_event = ar_event__create_with_position(type, message, position);
    if (!own_event) {
        return;
    }
    
    _buffer_event(mut_log, own_event);
}

void ar_log__error(ar_log_t *mut_log, const char *message) {
    _add_event(mut_log, AR_EVENT_ERROR, message);
}

void ar_log__warning(ar_log_t *mut_log, const char *message) {
    _add_event(mut_log, AR_EVENT_WARNING, message);
}

void ar_log__info(ar_log_t *mut_log, const char *message) {
    _add_event(mut_log, AR_EVENT_INFO, message);
}

void ar_log__error_at(ar_log_t *mut_log, const char *message, int position) {
    _add_event_at(mut_log, AR_EVENT_ERROR, message, position);
}

void ar_log__warning_at(ar_log_t *mut_log, const char *message, int position) {
    _add_event_at(mut_log, AR_EVENT_WARNING, message, position);
}

void ar_log__info_at(ar_log_t *mut_log, const char *message, int position) {
    _add_event_at(mut_log, AR_EVENT_INFO, message, position);
}