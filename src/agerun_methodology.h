#ifndef AGERUN_METHODOLOGY_H
#define AGERUN_METHODOLOGY_H

#include <stdbool.h>
#include "agerun_method.h"

/**
 * Get a method definition by name and version
 * @param name Method name
 * @param version Method version (0 for latest)
 * @return Pointer to method definition, or NULL if not found
 */
method_t* ar_method_get(const char *name, version_t version);

// Interface functions for agerun_method.c
int ar_methodology_find_method_idx(const char *name);
method_t* ar_methodology_get_method_storage(int method_idx, int version_idx);
int* ar_methodology_get_method_counts(void);
int* ar_methodology_get_method_name_count(void);

#endif /* AGERUN_METHODOLOGY_H */
