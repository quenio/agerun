/**
 * @file ar_local_completion.h
 * @brief Local completion runtime adapter for complete() instructions
 */

#ifndef AGERUN_LOCAL_COMPLETION_H
#define AGERUN_LOCAL_COMPLETION_H

#include <stdbool.h>
#include <stdint.h>
#include "ar_list.h"

typedef struct ar_data_s ar_data_t;
typedef struct ar_log_s ar_log_t;

typedef struct ar_local_completion_s ar_local_completion_t;

ar_local_completion_t* ar_local_completion__create(ar_log_t *ref_log);
void ar_local_completion__destroy(ar_local_completion_t *own_runtime);
const char* ar_local_completion__get_model_path(const ar_local_completion_t *ref_runtime);
bool ar_local_completion__is_ready(const ar_local_completion_t *ref_runtime);
ar_data_t* ar_local_completion__complete(
    ar_local_completion_t *mut_runtime,
    const char *ref_template,
    const ar_list_t *ref_placeholders,
    int64_t timeout_ms
);

#endif
