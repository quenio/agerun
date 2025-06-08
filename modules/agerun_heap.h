#ifndef AGERUN_HEAP_H
#define AGERUN_HEAP_H

#include "agerun_assert.h"
#include <stddef.h>

/**
 * Heap memory management utilities for the AgeRun system.
 * 
 * This module provides:
 * - Memory allocation tracking and leak detection
 * - Wrappers for standard memory allocation functions (malloc, calloc, realloc, strdup, free)
 * - Detailed memory usage reporting
 * 
 * All tracking and validation logic is conditionally compiled using the DEBUG macro,
 * ensuring zero overhead in release builds.
 */

/* Memory tracking functions that are only available in debug builds */
#ifdef DEBUG

/**
 * Generate a comprehensive memory leak report
 * 
 * Creates a detailed report with information about:
 * - Total allocations made during program execution
 * - Currently active (unfreed) allocations
 * - Total memory allocated
 * - Currently active memory usage
 * - Detailed listing of each memory leak with location, size, and timestamp
 * 
 * The report is written to heap_memory_report.log in the current directory.
 * This function is automatically called at program exit, but can also be
 * manually called to generate reports at specific points during execution.
 */
void ar_heap_memory_report(void);

/**
 * Tracked memory allocation function (wrapper for malloc)
 * 
 * Allocates memory while recording allocation metadata for leak detection.
 * This function should not be called directly - use the AR_HEAP_MALLOC macro instead.
 * 
 * @param size Size in bytes to allocate
 * @param file Source file where allocation occurs (typically passed via __FILE__ macro)
 * @param line Line number where allocation occurs (typically passed via __LINE__ macro)
 * @param description Human-readable description of the memory's purpose
 * @return Pointer to allocated memory (owned by caller, must be freed with AR_HEAP_FREE)
 * @note Ownership: Returns an owned pointer that caller must free with AR_HEAP_FREE
 */
void *ar_heap_malloc(size_t size, const char *file, int line, const char *description);

/**
 * Tracked memory allocation with zero-initialization (wrapper for calloc)
 * 
 * Allocates and zero-initializes memory while recording allocation metadata.
 * This function should not be called directly - use the AR_HEAP_CALLOC macro instead.
 * 
 * @param count Number of elements to allocate
 * @param size Size of each element in bytes
 * @param file Source file where allocation occurs (typically passed via __FILE__ macro)
 * @param line Line number where allocation occurs (typically passed via __LINE__ macro)
 * @param description Human-readable description of the memory's purpose
 * @return Pointer to zero-initialized allocated memory (owned by caller, must be freed with AR_HEAP_FREE)
 * @note Ownership: Returns an owned pointer that caller must free with AR_HEAP_FREE
 */
void *ar_heap_calloc(size_t count, size_t size, const char *file, int line, const char *description);

/**
 * Tracked memory reallocation function (wrapper for realloc)
 * 
 * Resizes a previously allocated memory block while updating allocation metadata.
 * This function should not be called directly - use the AR_HEAP_REALLOC macro instead.
 * 
 * @param ptr Pointer to previously allocated memory to resize (must be from AR_HEAP_MALLOC/AR_HEAP_CALLOC/AR_HEAP_REALLOC)
 * @param size New size in bytes
 * @param file Source file where reallocation occurs (typically passed via __FILE__ macro)
 * @param line Line number where reallocation occurs (typically passed via __LINE__ macro)
 * @param description Human-readable description of the memory's purpose
 * @return Pointer to resized memory block (owned by caller, must be freed with AR_HEAP_FREE)
 * @note Ownership: Returns an owned pointer that caller must free with AR_HEAP_FREE
 *       The original ptr should not be used after this call, as it may have been freed
 */
void *ar_heap_realloc(void *ptr, size_t size, const char *file, int line, const char *description);

/**
 * Tracked string duplication function (wrapper for strdup)
 * 
 * Duplicates a string while recording allocation metadata for leak detection.
 * This function should not be called directly - use the AR_HEAP_STRDUP macro instead.
 * 
 * @param str String to duplicate (must be null-terminated)
 * @param file Source file where duplication occurs (typically passed via __FILE__ macro)
 * @param line Line number where duplication occurs (typically passed via __LINE__ macro)
 * @param description Human-readable description of the string's purpose
 * @return Pointer to newly allocated copy of the string (owned by caller, must be freed with AR_HEAP_FREE)
 * @note Ownership: Returns an owned pointer that caller must free with AR_HEAP_FREE
 */
char *ar_heap_strdup(const char *str, const char *file, int line, const char *description);

/**
 * Tracked memory deallocation function (wrapper for free)
 * 
 * Frees memory and removes its allocation record from the tracking system.
 * This function should not be called directly - use the AR_HEAP_FREE macro instead.
 * 
 * @param ptr Pointer to memory to free (must be from AR_HEAP_MALLOC/AR_HEAP_CALLOC/AR_HEAP_REALLOC/AR_HEAP_STRDUP)
 * @note After calling AR_HEAP_FREE, you MUST set the pointer to NULL to prevent use-after-free errors.
 *       AR_HEAP_FREE(ptr); ptr = NULL;
 */
void ar_heap_free(void *ptr);

/**
 * Memory allocation and tracking macros
 * 
 * These macros provide a consistent interface for memory allocation across
 * debug and release builds. In debug builds, these map to the tracking wrappers,
 * while in release builds they map directly to the standard C library functions.
 * 
 * Always use these macros instead of direct malloc/calloc/realloc/strdup/free calls.
 * The 'desc' parameter should be a short, descriptive string explaining the
 * purpose of the allocation, which helps in tracking down memory leaks.
 * 
 * Example usage:
 *   char *own_buffer = AR_HEAP_MALLOC(1024, "JSON parse buffer");
 *   int *own_array = AR_HEAP_CALLOC(10, sizeof(int), "Score history");
 *   own_buffer = AR_HEAP_REALLOC(own_buffer, 2048, "Expanded JSON buffer");
 *   char *own_copy = AR_HEAP_STRDUP(original, "Config path");
 *   AR_HEAP_FREE(own_buffer);
 *   own_buffer = NULL;  // IMPORTANT: Always set to NULL after freeing
 */
#define AR_HEAP_MALLOC(size, desc) ar_heap_malloc((size), __FILE__, __LINE__, (desc))
#define AR_HEAP_CALLOC(count, size, desc) ar_heap_calloc((count), (size), __FILE__, __LINE__, (desc))
#define AR_HEAP_REALLOC(ptr, size, desc) ar_heap_realloc((ptr), (size), __FILE__, __LINE__, (desc))
#define AR_HEAP_STRDUP(str, desc) ar_heap_strdup((str), __FILE__, __LINE__, (desc))
#define AR_HEAP_FREE(ptr) ar_heap_free(ptr)

#else

/**
 * Release build memory allocation macros
 * 
 * In release builds, the memory tracking is disabled for performance reasons.
 * These macros map directly to the standard C library functions without
 * any tracking overhead. The interface remains identical to the debug build,
 * ensuring consistent usage across all build configurations.
 * 
 * Note that the 'desc' parameter is still required for interface compatibility
 * but is completely ignored in release builds.
 */
#define AR_HEAP_MALLOC(size, desc) malloc(size)
#define AR_HEAP_CALLOC(count, size, desc) calloc(count, size)
#define AR_HEAP_REALLOC(ptr, size, desc) realloc(ptr, size)
#define AR_HEAP_STRDUP(str, desc) strdup(str)
#define AR_HEAP_FREE(ptr) free(ptr)

#endif /* DEBUG */

#endif /* AGERUN_HEAP_H */
