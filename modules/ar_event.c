#include "ar_event.h"
#include "ar_heap.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

struct ar_event_s {
    char *own_message;      /* Owned message string */
    ar_event_type_t type;   /* Event type */
    int position;           /* Position in source, -1 if not set */
    bool has_position;      /* True if position is set */
    char timestamp[20];     /* ISO 8601 timestamp: YYYY-MM-DDTHH:MM:SS\0 */
};

ar_event_t* ar_event__create(void) {
    ar_event_t *own_event = AR__HEAP__MALLOC(sizeof(ar_event_t), "event");
    if (!own_event) {
        return NULL;
    }
    
    own_event->own_message = NULL;
    own_event->type = AR_EVENT_TYPE__INFO;  // Default to INFO
    own_event->position = -1;
    own_event->has_position = false;
    
    // Set timestamp
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    strftime(own_event->timestamp, sizeof(own_event->timestamp), 
             "%Y-%m-%dT%H:%M:%S", tm_info);
    
    return own_event;
}

ar_event_t* ar_event__create_with_message(const char *message) {
    ar_event_t *own_event = ar_event__create();
    if (!own_event) {
        return NULL;
    }
    
    if (message) {
        own_event->own_message = AR__HEAP__STRDUP(message, "event message");
        if (!own_event->own_message) {
            ar_event__destroy(own_event);
            return NULL;
        }
    }
    
    return own_event;
}

void ar_event__destroy(ar_event_t *own_event) {
    if (!own_event) {
        return;
    }
    
    if (own_event->own_message) {
        AR__HEAP__FREE(own_event->own_message);
    }
    
    AR__HEAP__FREE(own_event);
}

const char* ar_event__get_message(const ar_event_t *ref_event) {
    if (!ref_event) {
        return NULL;
    }
    
    return ref_event->own_message;
}

ar_event_t* ar_event__create_typed(ar_event_type_t type, const char *message) {
    ar_event_t *own_event = ar_event__create_with_message(message);
    if (!own_event) {
        return NULL;
    }
    
    own_event->type = type;
    
    return own_event;
}

ar_event_type_t ar_event__get_type(const ar_event_t *ref_event) {
    if (!ref_event) {
        return AR_EVENT_TYPE__ERROR;  // Default for NULL
    }
    
    return ref_event->type;
}

ar_event_t* ar_event__create_with_position(ar_event_type_t type, const char *message, int position) {
    ar_event_t *own_event = ar_event__create_typed(type, message);
    if (!own_event) {
        return NULL;
    }
    
    own_event->position = position;
    own_event->has_position = true;
    
    return own_event;
}

int ar_event__get_position(const ar_event_t *ref_event) {
    if (!ref_event || !ref_event->has_position) {
        return -1;
    }
    
    return ref_event->position;
}

bool ar_event__has_position(const ar_event_t *ref_event) {
    if (!ref_event) {
        return false;
    }
    
    return ref_event->has_position;
}

const char* ar_event__get_timestamp(const ar_event_t *ref_event) {
    if (!ref_event) {
        return NULL;
    }
    
    return ref_event->timestamp;
}