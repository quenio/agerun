#include "../include/agerun_value.h"
#include <stdlib.h>

/**
 * Free resources associated with a value
 * @param value Pointer to the value to free
 */
void ar_free_value(value_t *value) {
    if (!value) return;
    
    if (value->type == VALUE_STRING && value->data.string_value) {
        free(value->data.string_value);
        value->data.string_value = NULL;
    }
}
