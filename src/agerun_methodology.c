#include "agerun_methodology.h"
#include "agerun_method.h"
#include "agerun_string.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/* Constants */
#define MAX_METHODS 256
#define MAX_VERSIONS_PER_METHOD 64
#define MAX_METHOD_NAME_LENGTH 64

/* Global State */
static method_t methods[MAX_METHODS][MAX_VERSIONS_PER_METHOD];
static int method_counts[MAX_METHODS];
static int method_name_count = 0;

/* Forward Declarations */
static int find_method_idx(const char *name);
static method_t* find_latest_method(const char *name);
static method_t* find_method(const char *name, version_t version);

/* Method Search Functions */
static int find_method_idx(const char *name) {
    for (int i = 0; i < method_name_count; i++) {
        if (strcmp(methods[i][0].name, name) == 0) {
            return i;
        }
    }
    
    return -1;
}

static method_t* find_latest_method(const char *name) {
    int method_idx = find_method_idx(name);
    if (method_idx < 0 || method_counts[method_idx] == 0) {
        return NULL;
    }
    
    // Find the most recent version
    version_t latest_version = 0;
    int latest_idx = -1;
    
    for (int i = 0; i < method_counts[method_idx]; i++) {
        if (methods[method_idx][i].version > latest_version) {
            latest_version = methods[method_idx][i].version;
            latest_idx = i;
        }
    }
    
    if (latest_idx >= 0) {
        return &methods[method_idx][latest_idx];
    }
    
    return NULL;
}

static method_t* find_method(const char *name, version_t version) {
    int method_idx = find_method_idx(name);
    if (method_idx < 0) {
        return NULL;
    }
    
    // Case 1: Exact version match
    for (int i = 0; i < method_counts[method_idx]; i++) {
        if (methods[method_idx][i].version == version) {
            return &methods[method_idx][i];
        }
    }
    
    // Case 2: Find compatible version
    version_t latest_compatible = 0;
    int latest_idx = -1;
    
    for (int i = 0; i < method_counts[method_idx]; i++) {
        if (methods[method_idx][i].backward_compatible && 
            methods[method_idx][i].version > version && 
            methods[method_idx][i].version > latest_compatible) {
            latest_compatible = methods[method_idx][i].version;
            latest_idx = i;
        }
    }
    
    if (latest_idx >= 0) {
        return &methods[method_idx][latest_idx];
    }
    
    return NULL; // No compatible version found
}

// Interface functions for agerun_method.c
int ar_methodology_find_method_idx(const char *name) {
    return find_method_idx(name);
}

method_t* ar_methodology_get_method_storage(int method_idx, int version_idx) {
    return &methods[method_idx][version_idx];
}

int* ar_methodology_get_method_counts(void) {
    return method_counts;
}

int* ar_methodology_get_method_name_count(void) {
    return &method_name_count;
}

// Main method access function
method_t* ar_methodology_get_method(const char *name, version_t version) {
    if (version == 0) {
        // Use latest version
        return find_latest_method(name);
    } else {
        // Use specific version
        return find_method(name, version);
    }
}
