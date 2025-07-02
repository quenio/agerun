#include "ar_log.h"
#include "ar_heap.h"
#include "ar_io.h"
#include "ar_list.h"
#include "ar_event.h"
#include <stdlib.h>
#include <stdio.h>

#define LOG_FILE_NAME "agerun.log"
#define BUFFER_LIMIT 10

struct ar_log_s {
    list_t *own_events;     /* Owned list of events */
    FILE *file;             /* Log file handle */
};

// Forward declarations
static void _flush_events(ar_log_t *mut_log);

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
    
    // Flush any buffered events before destroying
    if (own_log->own_events && ar_list__count(own_log->own_events) > 0) {
        _flush_events(own_log);
    }
    
    // Close the log file
    if (own_log->file) {
        ar_io__close_file(own_log->file, LOG_FILE_NAME);
    }
    
    // Destroy the events list and all events in it
    if (own_log->own_events) {
        // Clean up all events in the list (should be empty after flush)
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

// Helper function to format and write a single event
static void _write_event(FILE *file, ar_event_t *ref_event) {
    if (!file || !ref_event) {
        return;
    }
    
    // Format: [TIMESTAMP] TYPE: MESSAGE (at position X)
    fprintf(file, "[%s] ", ar_event__get_timestamp(ref_event));
    
    // Write event type
    ar_event_type_t type = ar_event__get_type(ref_event);
    const char *type_str = (type == AR_EVENT_ERROR) ? "ERROR" :
                          (type == AR_EVENT_WARNING) ? "WARNING" : "INFO";
    fprintf(file, "%s: ", type_str);
    
    // Write message
    fprintf(file, "%s", ar_event__get_message(ref_event));
    
    // Write position if available
    if (ar_event__has_position(ref_event)) {
        fprintf(file, " (at position %d)", ar_event__get_position(ref_event));
    }
    
    fprintf(file, "\n");
}

// Helper function to flush events to disk
static void _flush_events(ar_log_t *mut_log) {
    if (!mut_log || !mut_log->own_events || !mut_log->file) {
        return;
    }
    
    // Write all events to file
    void **items = ar_list__items(mut_log->own_events);
    if (!items) {
        return;
    }
    
    size_t count = ar_list__count(mut_log->own_events);
    for (size_t i = 0; i < count; i++) {
        ar_event_t *ref_event = (ar_event_t*)items[i];
        if (ref_event) {
            _write_event(mut_log->file, ref_event);
        }
    }
    
    // Flush to ensure data is written
    fflush(mut_log->file);
    
    // Clean up the items array
    AR__HEAP__FREE(items);
    
    // Clear the buffer by removing all events
    while (!ar_list__empty(mut_log->own_events)) {
        ar_event_t *own_event = ar_list__remove_first(mut_log->own_events);
        if (own_event) {
            ar_event__destroy(own_event);
        }
    }
}

// Helper function to add an event to the buffer
static void _buffer_event(ar_log_t *mut_log, ar_event_t *own_event) {
    if (!mut_log || !own_event) {
        return;
    }
    
    // Check if buffer is full
    if (ar_list__count(mut_log->own_events) >= BUFFER_LIMIT) {
        // Flush current buffer
        _flush_events(mut_log);
    }
    
    // Add event to the buffer
    ar_list__add_last(mut_log->own_events, own_event);
    
    // Note: Event ownership is now with the list
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

// Helper function to find the last event of a specific type
static ar_event_t* _get_last_event_by_type(ar_log_t *ref_log, ar_event_type_t type) {
    if (!ref_log || !ref_log->own_events) {
        return NULL;
    }
    
    // Get all events from the list
    void **items = ar_list__items(ref_log->own_events);
    if (!items) {
        return NULL;
    }
    
    // Iterate through the list backwards to find the last event of the specified type
    size_t count = ar_list__count(ref_log->own_events);
    ar_event_t *result = NULL;
    
    for (size_t i = count; i > 0; i--) {
        ar_event_t *ref_event = (ar_event_t*)items[i - 1];
        if (ref_event && ar_event__get_type(ref_event) == type) {
            result = ref_event;
            break;
        }
    }
    
    // Clean up the items array (but not the events themselves)
    AR__HEAP__FREE(items);
    
    return result;
}

ar_event_t* ar_log__get_last_error(ar_log_t *ref_log) {
    return _get_last_event_by_type(ref_log, AR_EVENT_ERROR);
}

ar_event_t* ar_log__get_last_warning(ar_log_t *ref_log) {
    return _get_last_event_by_type(ref_log, AR_EVENT_WARNING);
}

ar_event_t* ar_log__get_last_info(ar_log_t *ref_log) {
    return _get_last_event_by_type(ref_log, AR_EVENT_INFO);
}

const char* ar_log__get_last_error_message(ar_log_t *ref_log) {
    ar_event_t *error_event = ar_log__get_last_error(ref_log);
    if (!error_event) {
        return NULL;
    }
    return ar_event__get_message(error_event);
}

int ar_log__get_last_error_position(ar_log_t *ref_log) {
    ar_event_t *error_event = ar_log__get_last_error(ref_log);
    if (!error_event) {
        return 0;
    }
    if (!ar_event__has_position(error_event)) {
        return 0;
    }
    return ar_event__get_position(error_event);
}