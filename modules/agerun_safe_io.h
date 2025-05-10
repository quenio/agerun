/**
 * Safe I/O utilities for the AgeRun project
 */
#ifndef AGERUN_SAFE_IO_H
#define AGERUN_SAFE_IO_H

#include <stdio.h>
#include <stdbool.h>

/**
 * Prints an error message to stderr
 * @param format Printf-style format string
 */
void ar_safe_error(const char *format, ...);

/**
 * Prints a warning message to stderr
 * @param format Printf-style format string
 */
void ar_safe_warning(const char *format, ...);

/**
 * Safely prints to the specified stream with error checking
 * @param stream Stream to print to
 * @param format Printf-style format string
 */
void ar_safe_fprintf(FILE *stream, const char *format, ...);

/* Macros for backwards compatibility */
#define SAFE_ERROR ar_safe_error
#define SAFE_WARNING ar_safe_warning
#define SAFE_FPRINTF ar_safe_fprintf

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
} file_result_t;

/**
 * Helper function for secure file reading with bounds checking
 *
 * @param fp Pointer to an open file
 * @param buffer Buffer to store the read line
 * @param buffer_size Size of the buffer
 * @param filename Name of the file being read (for error messages)
 * @return true if a line was read successfully, false otherwise
 */
bool ar_safe_read_line(FILE *fp, char *buffer, int buffer_size, const char *filename);

/**
 * Safely opens a file with proper error checking and reporting
 *
 * @param filename Path to the file to open
 * @param mode Mode to open the file in ("r", "w", etc.)
 * @param file_ptr Pointer to store the opened file handle
 * @return FILE_SUCCESS if successful, appropriate error code otherwise
 */
file_result_t ar_safe_open_file(const char *filename, const char *mode, FILE **file_ptr);

/**
 * Safely closes a file with error checking
 *
 * @param fp Pointer to the file to close
 * @param filename Name of the file (for error reporting)
 * @return FILE_SUCCESS if successful, appropriate error code otherwise
 */
file_result_t ar_safe_close_file(FILE *fp, const char *filename);

/**
 * Creates a backup of a file before modifying it
 *
 * @param filename Path to the file to backup
 * @return FILE_SUCCESS if backup was created successfully, appropriate error code otherwise
 */
file_result_t ar_safe_create_backup(const char *filename);

/**
 * Restores a backup file if the main operation failed
 *
 * @param filename Path to the file to restore
 * @return FILE_SUCCESS if backup was restored successfully, appropriate error code otherwise
 */
file_result_t ar_safe_restore_backup(const char *filename);

/**
 * Applies secure permissions to a file (owner read/write only)
 *
 * @param filename Path to the file to secure
 * @return FILE_SUCCESS if permissions were set successfully, appropriate error code otherwise
 */
file_result_t ar_safe_set_secure_permissions(const char *filename);

/**
 * Safely writes a file using a temporary file and atomic rename
 *
 * @param filename Path to the target file
 * @param write_func Function that writes the content to the temp file
 * @param context Context passed to the write function
 * @return FILE_SUCCESS if successful, appropriate error code otherwise
 */
file_result_t ar_safe_write_file(const char *filename,
                               bool (*write_func)(FILE *fp, void *context),
                               void *context);

/**
 * Gets a detailed error message for a file_result_t code
 *
 * @param result The result code to get a message for
 * @return A human-readable error message
 */
const char* ar_safe_error_message(file_result_t result);

#endif /* AGERUN_SAFE_IO_H */
