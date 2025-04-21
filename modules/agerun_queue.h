/*
 * This header is now deprecated in favor of using the list module for queue operations.
 * Including this header simply defines compatibility macros to map the old queue interface
 * to the new list interface for backward compatibility.
 * New code should directly use the list module's API.
 */

#ifndef AGERUN_QUEUE_H
#define AGERUN_QUEUE_H

#include "agerun_list.h"

// Compatibility type definition
typedef list_t queue_t;

// Compatibility function macros for backward compatibility
#define ar_queue_create() ar_list_create()
#define ar_queue_destroy(q) ar_list_destroy(q)
#define ar_queue_push(q, r) ar_list_add_last(q, (void*)(r))
#define ar_queue_pop(q) ar_list_remove_first(q)
#define ar_queue_is_empty(q) ar_list_empty(q)

#endif /* AGERUN_QUEUE_H */
