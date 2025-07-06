/**
 * I/O utilities for the AgeRun project
 */
#ifndef AGERUN_IO_H
#define AGERUN_IO_H

#include <stdio.h>
#include <stdbool.h>

/**
 * Prints an error message to stderr
 * @param format Printf-style format string
 */
void ar_io__error(const char *format, ...);

/**
 * Prints a warning message to stderr
 * @param format Printf-style format string
 */
void ar_io__warning(const char *format, ...);

/**
 * Prints an informational message to stdout
 * @param format Printf-style format string
 */
void ar_io__info(const char *format, ...);

/**
 * Safely prints to the specified stream with error checking
 * @param stream Stream to print to
 * @param format Printf-style format string
 */
void ar_io__fprintf(FILE *stream, const char *format, ...);

/* The SAFE_ macros have been removed in favor of directly calling the corresponding functions:
 * - ar_io__error instead of SAFE_ERROR
 * - ar_io__warning instead of SAFE_WARNING
 * - ar_io__fprintf instead of SAFE_FPRINTF
 */

/**
 * Enumeration for file operation results with detailed error types
 */
typedef enum file_result_e {
    FILE_SUCCESS,             // Operation completed successfully
    FILE_ERROR_OPEN,          // Failed to open file
    FILE_ERROR_READ,          // Failed to read from file
    FILE_ERROR_WRITE,         // Failed to write to file
    FILE_ERROR_PERMISSIONS,   // Insufficient permissions
    FILE_ERROR_NOT_FOUND,     // File not found
    FILE_ERROR_CORRUPT,       // File is corrupt or malformed
    FILE_ERROR_ALREADY_EXISTS, // File already exists (for creation operations)
    FILE_ERROR_UNKNOWN        // Unknown error
} ar_file_result_t;

/**
 * Helper function for secure file reading with bounds checking
 *
 * @param fp Pointer to an open file
 * @param buffer Buffer to store the read line
 * @param buffer_size Size of the buffer
 * @param filename Name of the file being read (for error messages)
 * @return true if a line was read successfully, false otherwise
 */
bool ar_io__read_line(FILE *fp, char *buffer, int buffer_size, const char *filename);

/**
 * Safely opens a file with proper error checking and reporting
 *
 * @param filename Path to the file to open
 * @param mode Mode to open the file in ("r", "w", etc.)
 * @param file_ptr Pointer to store the opened file handle
 * @return FILE_SUCCESS if successful, appropriate error code otherwise
 */
ar_file_result_t ar_io__open_file(const char *filename, const char *mode, FILE **file_ptr);

/**
 * Safely closes a file with error checking
 *
 * @param fp Pointer to the file to close
 * @param filename Name of the file (for error reporting)
 * @return FILE_SUCCESS if successful, appropriate error code otherwise
 */
ar_file_result_t ar_io__close_file(FILE *fp, const char *filename);

/**
 * Creates a backup of a file before modifying it
 *
 * @param filename Path to the file to backup
 * @return FILE_SUCCESS if backup was created successfully, appropriate error code otherwise
 */
ar_file_result_t ar_io__create_backup(const char *filename);

/**
 * Restores a backup file if the main operation failed
 *
 * @param filename Path to the file to restore
 * @return FILE_SUCCESS if backup was restored successfully, appropriate error code otherwise
 */
ar_file_result_t ar_io__restore_backup(const char *filename);

/**
 * Applies secure permissions to a file (owner read/write only)
 *
 * @param filename Path to the file to secure
 * @return FILE_SUCCESS if permissions were set successfully, appropriate error code otherwise
 */
ar_file_result_t ar_io__set_secure_permissions(const char *filename);

/**
 * Safely writes a file using a temporary file and atomic rename
 *
 * @param filename Path to the target file
 * @param write_func Function that writes the content to the temp file
 * @param context Context passed to the write function
 * @return FILE_SUCCESS if successful, appropriate error code otherwise
 */
ar_file_result_t ar_io__write_file(const char *filename,
                           bool (*write_func)(FILE *fp, void *context),
                           void *context);

/**
 * Gets a detailed error message for a ar_file_result_t code
 *
 * @param result The result code to get a message for
 * @return A human-readable error message
 */
const char* ar_io__error_message(ar_file_result_t result);

/**
 * Safely copies a string with proper bounds checking and null termination
 * @param dest Destination buffer
 * @param src Source string to copy
 * @param dest_size Size of the destination buffer (including space for null terminator)
 * @return true if the copy was successful, false if truncation occurred
 */
bool ar_io__string_copy(char *dest, const char *src, size_t dest_size);

/**
 * Securely formats a string with proper bounds checking and null termination
 * This is a safer replacement for sprintf
 * @param dest Destination buffer
 * @param dest_size Size of the destination buffer (including space for null terminator)
 * @param format Printf-style format string
 * @return true if formatting was successful, false if truncation occurred
 */
bool ar_io__string_format(char *dest, size_t dest_size, const char *format, ...);

/**
 * Reports a memory allocation failure with consistent error formatting
 * @param file Source file where the allocation failed
 * @param line Line number where the allocation failed
 * @param size Size of the allocation that failed
 * @param description Description of what was being allocated
 * @param context Optional context about the allocation (e.g., function name)
 */
void ar_io__report_allocation_failure(const char *file, int line, size_t size, 
                                    const char *description, const char *context);

/**
 * Attempts to recover from a memory allocation failure
 * This function implements a strategy to free non-critical memory or take
 * alternative actions when memory allocation fails in critical operations.
 * 
 * @param required_size Minimum size needed to continue operation
 * @param criticality Level of importance (0-100, with 100 being most critical)
 * @return true if recovery was successful (retry allocation recommended), false otherwise
 */
bool ar_io__attempt_memory_recovery(size_t required_size, int criticality);

/**
 * Checks a memory allocation result and reports failure if needed
 * 
 * @param ptr Pointer returned from memory allocation
 * @param size Size of the requested allocation
 * @param file Source file where the allocation occurred
 * @param line Line number where the allocation occurred
 * @param description Description of what was being allocated
 * @param context Optional context about the allocation (e.g., function name)
 * @return true if allocation succeeded, false otherwise
 */
bool ar_io__check_allocation(void *ptr, size_t size, const char *file, int line, 
                           const char *description, const char *context);

#endif /* AGERUN_IO_H */
