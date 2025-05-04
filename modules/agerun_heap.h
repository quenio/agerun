#ifndef AGERUN_HEAP_H
#define AGERUN_HEAP_H

#include <assert.h>
#include <stddef.h>

/**
 * Heap utilities for the AgeRun system.
 * 
 * This module provides macros and utilities to help track memory allocations,
 * enforce memory ownership invariants, and detect memory leaks during development.
 */

/**
 * General assertion macro for checking conditions in debug builds.
 * In release builds, this becomes a no-op.
 */
#ifdef DEBUG
#define AR_ASSERT(cond, msg) assert((cond) && msg)
#else
#define AR_ASSERT(cond, msg) ((void)0)
#endif

/**
 * Helper macro for checking ownership invariants in debug builds.
 * In release builds, this becomes a no-op.
 * 
 * Use this to verify that pointers that should never be NULL (like owned
 * values after creation) actually have valid values.
 */
#ifdef DEBUG
#define AR_ASSERT_OWNERSHIP(ptr) assert((ptr) != NULL && "Ownership violation: NULL pointer")
#else
#define AR_ASSERT_OWNERSHIP(ptr) ((void)0)
#endif

/**
 * Helper macro for validating that a pointer is NULL after ownership transfer.
 * In release builds, this becomes a no-op.
 * 
 * Use this to verify that pointers have been properly set to NULL after
 * their ownership has been transferred to another function or container.
 */
#ifdef DEBUG
#define AR_ASSERT_TRANSFERRED(ptr) assert((ptr) == NULL && "Ownership violation: Pointer not NULL after transfer")
#else
#define AR_ASSERT_TRANSFERRED(ptr) ((void)0)
#endif

/**
 * Helper macro for checking that a pointer is not used after being freed.
 * In release builds, this becomes a no-op.
 * 
 * This is particularly useful for local variables that are destroyed and
 * should not be accessed afterward.
 */
#ifdef DEBUG
#define AR_ASSERT_NOT_USED_AFTER_FREE(ptr) assert((ptr) == NULL && "Usage after free: Pointer accessed after being freed")
#else
#define AR_ASSERT_NOT_USED_AFTER_FREE(ptr) ((void)0)
#endif

/* Memory tracking functions that are only available in debug builds */
#ifdef DEBUG

/**
 * Add a memory allocation record
 * @param ptr Pointer to the allocated memory
 * @param file Source file where the allocation occurred
 * @param line Line number where the allocation occurred
 * @param size Size of the allocation in bytes
 * @param description Description of the allocation
 */
void ar_heap_memory_add(void *ptr, const char *file, int line, size_t size, const char *description);

/**
 * Remove a memory allocation record
 * @param ptr Pointer to the memory being freed
 * @return 1 if found and removed, 0 if not found
 */
int ar_heap_memory_remove(void *ptr);

/**
 * Generate a memory leak report
 * Writes detailed information about active allocations to heap_memory_report.log
 */
void ar_heap_memory_report(void);

/**
 * Wrapper for malloc that tracks memory allocations
 * @param size Size to allocate
 * @param file Source file
 * @param line Line number
 * @param description Description of the allocation
 * @return Pointer to allocated memory
 */
void *ar_heap_malloc(size_t size, const char *file, int line, const char *description);

/**
 * Wrapper for calloc that tracks memory allocations
 * @param count Number of elements
 * @param size Size of each element
 * @param file Source file
 * @param line Line number
 * @param description Description of the allocation
 * @return Pointer to allocated memory
 */
void *ar_heap_calloc(size_t count, size_t size, const char *file, int line, const char *description);

/**
 * Wrapper for realloc that tracks memory allocations
 * @param ptr Original pointer
 * @param size New size
 * @param file Source file
 * @param line Line number
 * @param description Description of the allocation
 * @return Pointer to reallocated memory
 */
void *ar_heap_realloc(void *ptr, size_t size, const char *file, int line, const char *description);

/**
 * Wrapper for strdup that tracks memory allocations
 * @param str String to duplicate
 * @param file Source file
 * @param line Line number
 * @param description Description of the allocation
 * @return Pointer to allocated string
 */
char *ar_heap_strdup(const char *str, const char *file, int line, const char *description);

/**
 * Wrapper for free that tracks memory deallocations
 * @param ptr Pointer to free
 */
void ar_heap_free(void *ptr);

/* Convenience macros for memory tracking */
#define AR_MALLOC(size, desc) ar_heap_malloc((size), __FILE__, __LINE__, (desc))
#define AR_CALLOC(count, size, desc) ar_heap_calloc((count), (size), __FILE__, __LINE__, (desc))
#define AR_REALLOC(ptr, size, desc) ar_heap_realloc((ptr), (size), __FILE__, __LINE__, (desc))
#define AR_STRDUP(str, desc) ar_heap_strdup((str), __FILE__, __LINE__, (desc))
#define AR_FREE(ptr) ar_heap_free(ptr)

#else

/* In release builds, these macros map directly to standard functions */
#define AR_MALLOC(size, desc) malloc(size)
#define AR_CALLOC(count, size, desc) calloc(count, size)
#define AR_REALLOC(ptr, size, desc) realloc(ptr, size)
#define AR_STRDUP(str, desc) strdup(str)
#define AR_FREE(ptr) free(ptr)

#endif /* DEBUG */

#endif /* AGERUN_HEAP_H */
