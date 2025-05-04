#include "agerun_heap.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef DEBUG

/**
 * Memory tracking record structure
 */
typedef struct memory_record_s {
    void *ptr;                  // Pointer to allocated memory
    const char *file;           // Source file where allocation occurred
    int line;                   // Line number where allocation occurred
    size_t size;                // Size of the allocation in bytes
    char *description;          // Description of the allocation (owned by the record)
    time_t timestamp;           // Timestamp of when the allocation occurred
    struct memory_record_s *next; // Next record in the linked list
} memory_record_t;

// Global variables for memory tracking
static memory_record_t *g_memory_records = NULL;
static size_t g_active_allocations = 0;
static size_t g_total_allocations = 0;
static size_t g_active_memory = 0;
static size_t g_total_memory = 0;
static int g_initialized = 0;

/**
 * Initialize the memory tracking system
 */
static void ar_heap_memory_init(void) {
    if (g_initialized) return;
    
    g_memory_records = NULL;
    g_active_allocations = 0;
    g_total_allocations = 0;
    g_active_memory = 0;
    g_total_memory = 0;
    
    // Register cleanup at program exit
    atexit(ar_heap_memory_report);
    
    g_initialized = 1;
}

/**
 * Add a memory allocation record
 * @param ptr Pointer to the allocated memory
 * @param file Source file where the allocation occurred
 * @param line Line number where the allocation occurred
 * @param size Size of the allocation in bytes
 * @param description Description of the allocation
 */
void ar_heap_memory_add(void *ptr, const char *file, int line, size_t size, const char *description) {
    if (!g_initialized) ar_heap_memory_init();
    
    if (!ptr) return; // Don't track NULL allocations
    
    // Create a new record
    memory_record_t *record = (memory_record_t *)malloc(sizeof(memory_record_t));
    if (!record) {
        fprintf(stderr, "ERROR: Failed to allocate memory for tracking record\n");
        return;
    }
    
    // Setup the record
    record->ptr = ptr;
    record->file = file;
    record->line = line;
    record->size = size;
    record->description = description ? strdup(description) : strdup("Unknown");
    record->timestamp = time(NULL);
    record->next = g_memory_records;
    
    // Add to the list
    g_memory_records = record;
    
    // Update statistics
    g_active_allocations++;
    g_total_allocations++;
    g_active_memory += size;
    g_total_memory += size;
}

/**
 * Remove a memory allocation record
 * @param ptr Pointer to the memory being freed
 * @return 1 if found and removed, 0 if not found
 */
int ar_heap_memory_remove(void *ptr) {
    if (!g_initialized || !ptr) return 0;
    
    memory_record_t *prev = NULL;
    memory_record_t *curr = g_memory_records;
    
    while (curr) {
        if (curr->ptr == ptr) {
            // Found the record, remove it
            if (prev) {
                prev->next = curr->next;
            } else {
                g_memory_records = curr->next;
            }
            
            // Update statistics
            g_active_allocations--;
            g_active_memory -= curr->size;
            
            // Free the record
            free(curr->description);
            free(curr);
            
            return 1;
        }
        
        prev = curr;
        curr = curr->next;
    }
    
    // Not found
    return 0;
}

/**
 * Generate a memory leak report
 */
void ar_heap_memory_report(void) {
    if (!g_initialized) return;
    
    // Create the report file in the bin directory
    FILE *report = fopen("heap_memory_report.log", "w");
    if (!report) {
        fprintf(stderr, "ERROR: Failed to create memory report file\n");
        return;
    }
    
    fprintf(report, "=====================================\n");
    fprintf(report, "  AgeRun Memory Tracking Report\n");
    fprintf(report, "=====================================\n\n");
    
    // Print statistics
    fprintf(report, "Total allocations: %zu\n", g_total_allocations);
    fprintf(report, "Active allocations: %zu\n", g_active_allocations);
    fprintf(report, "Total memory allocated: %zu bytes\n", g_total_memory);
    fprintf(report, "Active memory: %zu bytes\n\n", g_active_memory);
    
    // Print memory leaks if any
    if (g_active_allocations > 0) {
        fprintf(report, "=====================================\n");
        fprintf(report, "  MEMORY LEAKS DETECTED: %zu\n", g_active_allocations);
        fprintf(report, "=====================================\n\n");
        
        memory_record_t *curr = g_memory_records;
        int leak_count = 0;
        
        while (curr) {
            leak_count++;
            
            // Format time
            char time_str[26];
            struct tm *tm_info = localtime(&curr->timestamp);
            strftime(time_str, 26, "%Y-%m-%d %H:%M:%S", tm_info);
            
            fprintf(report, "Leak #%d:\n", leak_count);
            fprintf(report, "  Address: %p\n", curr->ptr);
            fprintf(report, "  Size: %zu bytes\n", curr->size);
            fprintf(report, "  Allocated at: %s:%d\n", curr->file, curr->line);
            fprintf(report, "  Description: %s\n", curr->description);
            fprintf(report, "  Allocated on: %s\n\n", time_str);
            
            curr = curr->next;
        }
    } else {
        fprintf(report, "No memory leaks detected.\n");
    }
    
    // Close the report
    fclose(report);
    
    // If there are leaks, also print to stderr
    if (g_active_allocations > 0) {
        fprintf(stderr, "WARNING: %zu memory leaks detected. See heap_memory_report.log for details.\n", g_active_allocations);
    }
    
    // Cleanup all records (to avoid memory leaks in our leak detector)
    memory_record_t *curr = g_memory_records;
    while (curr) {
        memory_record_t *next = curr->next;
        free(curr->description);
        free(curr);
        curr = next;
    }
    
    g_memory_records = NULL;
    g_initialized = 0;
}

/**
 * Wrapper for malloc that tracks memory allocations
 * @param size Size to allocate
 * @param file Source file
 * @param line Line number
 * @param description Description of the allocation
 * @return Pointer to allocated memory
 */
void *ar_heap_malloc(size_t size, const char *file, int line, const char *description) {
    void *ptr = malloc(size);
    if (ptr) {
        ar_heap_memory_add(ptr, file, line, size, description);
    }
    return ptr;
}

/**
 * Wrapper for calloc that tracks memory allocations
 * @param count Number of elements
 * @param size Size of each element
 * @param file Source file
 * @param line Line number
 * @param description Description of the allocation
 * @return Pointer to allocated memory
 */
void *ar_heap_calloc(size_t count, size_t size, const char *file, int line, const char *description) {
    void *ptr = calloc(count, size);
    if (ptr) {
        ar_heap_memory_add(ptr, file, line, count * size, description);
    }
    return ptr;
}

/**
 * Wrapper for realloc that tracks memory allocations
 * @param ptr Original pointer
 * @param size New size
 * @param file Source file
 * @param line Line number
 * @param description Description of the allocation
 * @return Pointer to reallocated memory
 */
void *ar_heap_realloc(void *ptr, size_t size, const char *file, int line, const char *description) {
    // Remove the old pointer tracking
    if (ptr) {
        ar_heap_memory_remove(ptr);
    }
    
    // Reallocate
    void *new_ptr = realloc(ptr, size);
    
    // Add the new pointer tracking
    if (new_ptr) {
        ar_heap_memory_add(new_ptr, file, line, size, description);
    }
    
    return new_ptr;
}

/**
 * Wrapper for strdup that tracks memory allocations
 * @param str String to duplicate
 * @param file Source file
 * @param line Line number
 * @param description Description of the allocation
 * @return Pointer to allocated string
 */
char *ar_heap_strdup(const char *str, const char *file, int line, const char *description) {
    char *ptr = strdup(str);
    if (ptr) {
        ar_heap_memory_add(ptr, file, line, strlen(str) + 1, description);
    }
    return ptr;
}

/**
 * Wrapper for free that tracks memory deallocations
 * @param ptr Pointer to free
 */
void ar_heap_free(void *ptr) {
    if (ptr) {
        ar_heap_memory_remove(ptr);
        free(ptr);
    }
}

#endif /* DEBUG */
