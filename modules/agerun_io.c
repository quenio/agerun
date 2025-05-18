#include "agerun_io.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdarg.h>

// Define backup file extension
#define BACKUP_EXTENSION ".bak"
#define TEMP_EXTENSION ".tmp"

/**
 * Prints an error message to stderr
 * @param format Printf-style format string
 */
void ar_io_error(const char *format, ...) {
    // Prepare a buffer for our formatted message with prefix and newline
    char buffer[2048];
    int prefix_len = snprintf(buffer, sizeof(buffer), "Error: ");

    if (prefix_len < 0 || prefix_len >= (int)sizeof(buffer)) {
        return; // Buffer too small for prefix
    }

    // Format the message into buffer after prefix
    va_list args;
    va_start(args, format);

    // Disable format-nonliteral warning for this function call
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wformat-nonliteral"
    int msg_len = vsnprintf(buffer + prefix_len, sizeof(buffer) - (size_t)prefix_len, format, args);
    #pragma GCC diagnostic pop
    va_end(args);

    if (msg_len < 0 || prefix_len + msg_len >= (int)sizeof(buffer) - 2) {
        // Message truncated, ensure null termination and add newline
        buffer[sizeof(buffer) - 2] = '\n';
        buffer[sizeof(buffer) - 1] = '\0';
    } else {
        // Add newline and null terminator
        buffer[prefix_len + msg_len] = '\n';
        buffer[prefix_len + msg_len + 1] = '\0';
    }

    // Write the complete message at once
    fputs(buffer, stderr);
}

/**
 * Prints a warning message to stderr
 * @param format Printf-style format string
 */
void ar_io_warning(const char *format, ...) {
    // Prepare a buffer for our formatted message with prefix and newline
    char buffer[2048];
    int prefix_len = snprintf(buffer, sizeof(buffer), "Warning: ");

    if (prefix_len < 0 || prefix_len >= (int)sizeof(buffer)) {
        return; // Buffer too small for prefix
    }

    // Format the message into buffer after prefix
    va_list args;
    va_start(args, format);

    // Disable format-nonliteral warning for this function call
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wformat-nonliteral"
    int msg_len = vsnprintf(buffer + prefix_len, sizeof(buffer) - (size_t)prefix_len, format, args);
    #pragma GCC diagnostic pop
    va_end(args);

    if (msg_len < 0 || prefix_len + msg_len >= (int)sizeof(buffer) - 2) {
        // Message truncated, ensure null termination and add newline
        buffer[sizeof(buffer) - 2] = '\n';
        buffer[sizeof(buffer) - 1] = '\0';
    } else {
        // Add newline and null terminator
        buffer[prefix_len + msg_len] = '\n';
        buffer[prefix_len + msg_len + 1] = '\0';
    }

    // Write the complete message at once
    fputs(buffer, stderr);
}

/**
 * Prints an informational message to stdout
 * @param format Printf-style format string
 */
void ar_io_info(const char *format, ...) {
    // Prepare a buffer for our formatted message with newline
    char buffer[2048];

    // No prefix for info messages, just format directly
    va_list args;
    va_start(args, format);

    // Disable format-nonliteral warning for this function call
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wformat-nonliteral"
    int msg_len = vsnprintf(buffer, sizeof(buffer) - 1, format, args); // Reserve space for newline
    #pragma GCC diagnostic pop
    va_end(args);

    if (msg_len < 0 || msg_len >= (int)sizeof(buffer) - 2) {
        // Message truncated, ensure null termination and add newline
        buffer[sizeof(buffer) - 2] = '\n';
        buffer[sizeof(buffer) - 1] = '\0';
    } else {
        // Add newline and null terminator
        buffer[msg_len] = '\n';
        buffer[msg_len + 1] = '\0';
    }

    // Write the complete message at once
    fputs(buffer, stdout);
}

/**
 * Safely prints to the specified stream with error checking
 * @param stream Stream to print to
 * @param format Printf-style format string
 */
void ar_io_fprintf(FILE *stream, const char *format, ...) {
    if (!stream || !format) {
        fputs("Error: Invalid parameters for ar_io_fprintf\n", stderr);
        return;
    }

    // Prepare a buffer for the formatted message
    char buffer[4096];
    va_list args;
    va_start(args, format);

    // Format the message into the buffer
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wformat-nonliteral"
    int msg_len = vsnprintf(buffer, sizeof(buffer), format, args);
    #pragma GCC diagnostic pop
    va_end(args);

    if (msg_len < 0 || msg_len >= (int)sizeof(buffer)) {
        fputs("Error: Format string too long or formatting error\n", stderr);
        return;
    }

    // Write to stream and handle errors
    if (fputs(buffer, stream) == EOF) {
        fputs("Error: Failed to write to output stream\n", stderr);
    }
}

/**
 * Helper function for secure file reading with bounds checking
 *
 * @param fp Pointer to an open file
 * @param buffer Buffer to store the read line
 * @param buffer_size Size of the buffer
 * @param filename Name of the file being read (for error messages)
 * @return true if a line was read successfully, false otherwise
 */
bool ar_io_read_line(FILE *fp, char *buffer, int buffer_size, const char *filename) {
    // Validate input parameters
    if (!fp || !buffer || buffer_size <= 0 || !filename) {
        ar_io_error("Invalid parameters for safe_read_line");
        return false;
    }

    // Initialize buffer with empty string for safety
    buffer[0] = '\0';

    // First check for EOF
    if (feof(fp)) {
        ar_io_error("Unexpected end of file in %s", filename);
        return false;
    }

    // Check for errors and clear them if present
    clearerr(fp);

    // Read character by character with bounds checking
    int i = 0;
    int c = 0; // Initialize c to avoid garbage value warning

    // Ensure we leave room for null terminator and optional newline
    const int max_chars = buffer_size - 2;

    // Read with strict bounds checking
    while (i < max_chars) {
        c = fgetc(fp);

        // Check for EOF or newline
        if (c == EOF || c == '\n') {
            break;
        }

        // Check for control characters (except tab, which is allowed)
        if (iscntrl(c) && c != '\t') {
            // Skip control characters for security, but don't fail
            continue;
        }

        // Store valid character
        buffer[i++] = (char)c;
    }

    // Ensure null termination
    buffer[i] = '\0';

    // Handle newline if we found one (preserve it)
    if (c == '\n') {
        // Only add the newline if we have room
        if (i < max_chars) {
            buffer[i] = '\n';
            buffer[i+1] = '\0';
        }
    }

    // Check for file errors
    if (ferror(fp)) {
        ar_io_error("Failed to read file %s", filename);
        clearerr(fp);
        return false;
    }

    // Check if we hit EOF with no content
    if (i == 0 && c == EOF) {
        ar_io_error("Unexpected end of file in %s", filename);
        return false;
    }

    // Check if line was truncated due to buffer size
    if (i >= max_chars && c != '\n' && c != EOF) {
        // Continue reading until end of line or EOF to maintain file position
        // but don't store these characters
        while ((c = fgetc(fp)) != EOF && c != '\n') {
            // Just consume the characters
        }
        ar_io_warning("Line truncated in %s (buffer size: %d)", filename, buffer_size);
        // We still return true as we did read data successfully
    }

    return true;
}

/**
 * Safely opens a file with proper error checking and reporting
 *
 * @param filename Path to the file to open
 * @param mode Mode to open the file in ("r", "w", etc.)
 * @param file_ptr Pointer to store the opened file handle
 * @return FILE_SUCCESS if successful, appropriate error code otherwise
 */
file_result_t ar_io_open_file(const char *filename, const char *mode, FILE **file_ptr) {
    // Validate parameters
    if (!filename || !mode || !file_ptr) {
        ar_io_error("Invalid parameters for safe_open_file");
        return FILE_ERROR_UNKNOWN;
    }

    // Try to open the file
    *file_ptr = fopen(filename, mode);
    if (!(*file_ptr)) {
        // Determine specific error type
        if (errno == EACCES || errno == EPERM) {
            ar_io_error("Permission denied opening %s: %s", filename, strerror(errno));
            return FILE_ERROR_PERMISSIONS;
        } else if (errno == ENOENT && mode[0] == 'r') {
            ar_io_error("File not found: %s: %s", filename, strerror(errno));
            return FILE_ERROR_NOT_FOUND;
        } else if (errno == EEXIST && mode[0] == 'w' && mode[1] == 'x') {
            ar_io_error("File already exists: %s", filename);
            return FILE_ERROR_ALREADY_EXISTS;
        } else {
            ar_io_error("Failed to open %s: %s", filename, strerror(errno));
            return FILE_ERROR_OPEN;
        }
    }

    return FILE_SUCCESS;
}

/**
 * Safely closes a file with error checking
 *
 * @param fp Pointer to the file to close
 * @param filename Name of the file (for error reporting)
 * @return FILE_SUCCESS if successful, appropriate error code otherwise
 */
file_result_t ar_io_close_file(FILE *fp, const char *filename) {
    if (!fp) {
        return FILE_SUCCESS; // Already closed, not an error
    }

    // Flush any buffered data
    if (fflush(fp) != 0) {
        ar_io_error("Failed to flush data to %s: %s", filename, strerror(errno));
        // Continue with close even if flush failed
    }

    // Close the file
    if (fclose(fp) != 0) {
        ar_io_error("Failed to close %s: %s", filename, strerror(errno));
        return FILE_ERROR_UNKNOWN;
    }

    return FILE_SUCCESS;
}

/**
 * Creates a backup of a file before modifying it
 *
 * @param filename Path to the file to backup
 * @return FILE_SUCCESS if backup was created successfully, appropriate error code otherwise
 */
file_result_t ar_io_create_backup(const char *filename) {
    if (!filename) {
        ar_io_error("Invalid parameters for safe_create_backup");
        return FILE_ERROR_UNKNOWN;
    }

    // Check if source file exists
    struct stat st;
    if (stat(filename, &st) != 0) {
        if (errno == ENOENT) {
            // Source file doesn't exist, nothing to backup
            return FILE_SUCCESS;
        }
        ar_io_error("Failed to stat %s: %s", filename, strerror(errno));
        return FILE_ERROR_UNKNOWN;
    }

    // Create backup filename
    size_t backup_name_len = strlen(filename) + strlen(BACKUP_EXTENSION) + 1;
    char *backup_filename = (char*)malloc(backup_name_len);
    if (!backup_filename) {
        ar_io_error("Memory allocation failed for backup filename");
        return FILE_ERROR_UNKNOWN;
    }

    snprintf(backup_filename, backup_name_len, "%s%s", filename, BACKUP_EXTENSION);

    // Open source file
    FILE *source;
    file_result_t result = ar_io_open_file(filename, "rb", &source);
    if (result != FILE_SUCCESS) {
        free(backup_filename);
        return result;
    }

    // Open backup file
    FILE *backup;
    result = ar_io_open_file(backup_filename, "wb", &backup);
    if (result != FILE_SUCCESS) {
        ar_io_close_file(source, filename);
        free(backup_filename);
        return result;
    }

    // Copy data
    const size_t buffer_size = 8192;
    char *buffer = (char*)malloc(buffer_size);
    if (!buffer) {
        ar_io_error("Memory allocation failed for backup buffer");
        ar_io_close_file(source, filename);
        ar_io_close_file(backup, backup_filename);
        free(backup_filename);
        return FILE_ERROR_UNKNOWN;
    }

    size_t bytes_read;
    bool success = true;

    // Read and write in chunks until EOF or error
    while (!feof(source) && !ferror(source)) {
        bytes_read = fread(buffer, 1, buffer_size, source);

        // Break if no bytes read and we reached EOF
        if (bytes_read == 0) {
            break;
        }

        if (fwrite(buffer, 1, bytes_read, backup) != bytes_read) {
            ar_io_error("Failed to write to backup file %s", backup_filename);
            success = false;
            break;
        }
    }

    // Check for read error
    if (ferror(source)) {
        ar_io_error("Failed to read from source file %s", filename);
        success = false;
    }

    // Cleanup
    free(buffer);
    ar_io_close_file(source, filename);
    ar_io_close_file(backup, backup_filename);
    free(backup_filename);

    return success ? FILE_SUCCESS : FILE_ERROR_UNKNOWN;
}

/**
 * Restores a backup file if the main operation failed
 *
 * @param filename Path to the file to restore
 * @return FILE_SUCCESS if backup was restored successfully, appropriate error code otherwise
 */
file_result_t ar_io_restore_backup(const char *filename) {
    if (!filename) {
        ar_io_error("Invalid parameters for safe_restore_backup");
        return FILE_ERROR_UNKNOWN;
    }

    // Create backup filename
    size_t backup_name_len = strlen(filename) + strlen(BACKUP_EXTENSION) + 1;
    char *backup_filename = (char*)malloc(backup_name_len);
    if (!backup_filename) {
        ar_io_error("Memory allocation failed for backup filename");
        return FILE_ERROR_UNKNOWN;
    }

    snprintf(backup_filename, backup_name_len, "%s%s", filename, BACKUP_EXTENSION);

    // Check if backup file exists
    struct stat st;
    if (stat(backup_filename, &st) != 0) {
        if (errno == ENOENT) {
            ar_io_error("Backup file %s does not exist", backup_filename);
            free(backup_filename);
            return FILE_ERROR_NOT_FOUND;
        }
        ar_io_error("Failed to stat backup file %s: %s", backup_filename, strerror(errno));
        free(backup_filename);
        return FILE_ERROR_UNKNOWN;
    }

    // Remove target file if it exists
    if (remove(filename) != 0 && errno != ENOENT) {
        ar_io_error("Failed to remove target file %s: %s", filename, strerror(errno));
        free(backup_filename);
        return FILE_ERROR_UNKNOWN;
    }

    // Rename backup to original
    if (rename(backup_filename, filename) != 0) {
        ar_io_error("Failed to restore backup %s to %s: %s", backup_filename, filename, strerror(errno));
        free(backup_filename);
        return FILE_ERROR_UNKNOWN;
    }

    free(backup_filename);
    return FILE_SUCCESS;
}

/**
 * Applies secure permissions to a file (owner read/write only)
 *
 * @param filename Path to the file to secure
 * @return FILE_SUCCESS if permissions were set successfully, appropriate error code otherwise
 */
file_result_t ar_io_set_secure_permissions(const char *filename) {
    if (!filename) {
        ar_io_error("Invalid parameters for safe_set_secure_permissions");
        return FILE_ERROR_UNKNOWN;
    }

#ifdef _WIN32
    // Windows implementation
    if (_chmod(filename, _S_IREAD | _S_IWRITE) != 0) {
        ar_io_error("Failed to set secure permissions on %s: %s", filename, strerror(errno));
        return FILE_ERROR_PERMISSIONS;
    }
#else
    // Unix/Linux/macOS implementation
    if (chmod(filename, S_IRUSR | S_IWUSR) != 0) {
        ar_io_error("Failed to set secure permissions on %s: %s", filename, strerror(errno));
        return FILE_ERROR_PERMISSIONS;
    }
#endif

    return FILE_SUCCESS;
}

/**
 * Function prototype for the callback function passed to ar_io_write_file
 */
typedef bool (*write_callback_t)(FILE *fp, void *context);

/**
 * Safely writes a file using a temporary file and atomic rename
 *
 * @param filename Path to the target file
 * @param write_func Function that writes the content to the temp file
 * @param context Context passed to the write function
 * @return FILE_SUCCESS if successful, appropriate error code otherwise
 */
file_result_t ar_io_write_file(const char *filename,
                               bool (*write_func)(FILE *fp, void *context),
                               void *context) {
    if (!filename || !write_func) {
        ar_io_error("Invalid parameters for safe_write_file");
        return FILE_ERROR_UNKNOWN;
    }

    // Create temporary filename
    size_t temp_name_len = strlen(filename) + strlen(TEMP_EXTENSION) + 1;
    char *temp_filename = (char*)malloc(temp_name_len);
    if (!temp_filename) {
        ar_io_error("Memory allocation failed for temporary filename");
        return FILE_ERROR_UNKNOWN;
    }

    snprintf(temp_filename, temp_name_len, "%s%s", filename, TEMP_EXTENSION);

    // Create backup of original file if it exists
    file_result_t result = ar_io_create_backup(filename);
    if (result != FILE_SUCCESS) {
        free(temp_filename);
        return result;
    }

    // Open temporary file
    FILE *temp_file;
    result = ar_io_open_file(temp_filename, "w", &temp_file);
    if (result != FILE_SUCCESS) {
        free(temp_filename);
        return result;
    }

    // Apply secure permissions to the temporary file
    result = ar_io_set_secure_permissions(temp_filename);
    if (result != FILE_SUCCESS) {
        ar_io_close_file(temp_file, temp_filename);
        remove(temp_filename);
        free(temp_filename);
        return result;
    }

    // Call the write function to generate the file content
    bool write_success = write_func(temp_file, context);
    if (!write_success) {
        ar_io_error("Failed to write content to temporary file %s", temp_filename);
        ar_io_close_file(temp_file, temp_filename);
        remove(temp_filename);
        free(temp_filename);
        return FILE_ERROR_WRITE;
    }

    // Close the temporary file
    result = ar_io_close_file(temp_file, temp_filename);
    if (result != FILE_SUCCESS) {
        remove(temp_filename);
        free(temp_filename);
        return result;
    }

    // Rename temporary file to target file
    if (rename(temp_filename, filename) != 0) {
        ar_io_error("Failed to rename temporary file %s to %s: %s",
                 temp_filename, filename, strerror(errno));
        remove(temp_filename);
        free(temp_filename);

        // Try to restore from backup
        ar_io_warning("Attempting to restore from backup...");
        if (ar_io_restore_backup(filename) != FILE_SUCCESS) {
            ar_io_error("Failed to restore from backup. Data may be lost.");
        } else {
            ar_io_error("Successfully restored from backup.");
        }

        return FILE_ERROR_UNKNOWN;
    }

    free(temp_filename);
    return FILE_SUCCESS;
}

/**
 * Gets a detailed error message for a file_result_t code
 *
 * @param result The result code to get a message for
 * @return A human-readable error message
 */
const char* ar_io_error_message(file_result_t result) {
    switch (result) {
        case FILE_SUCCESS:
            return "Operation completed successfully";
        case FILE_ERROR_OPEN:
            return "Failed to open file";
        case FILE_ERROR_READ:
            return "Failed to read from file";
        case FILE_ERROR_WRITE:
            return "Failed to write to file";
        case FILE_ERROR_PERMISSIONS:
            return "Insufficient permissions";
        case FILE_ERROR_NOT_FOUND:
            return "File not found";
        case FILE_ERROR_CORRUPT:
            return "File is corrupt or malformed";
        case FILE_ERROR_ALREADY_EXISTS:
            return "File already exists";
        case FILE_ERROR_UNKNOWN:
        default:
            return "Unknown error";
    }
}

/**
 * Safely copies a string with proper bounds checking and null termination
 * @param dest Destination buffer
 * @param src Source string to copy
 * @param dest_size Size of the destination buffer (including space for null terminator)
 * @return true if the copy was successful, false if truncation occurred
 */
bool ar_io_string_copy(char *dest, const char *src, size_t dest_size) {
    if (!dest || !src || dest_size == 0) {
        if (dest && dest_size > 0) {
            dest[0] = '\0'; // Ensure null termination if buffer exists
        }
        return false;
    }

    // Initialize destination to empty string for safety
    dest[0] = '\0';
    
    // Calculate source length
    size_t src_len = strlen(src);
    
    // Check if destination buffer is large enough
    if (src_len >= dest_size) {
        // Truncation required
        // Copy as many characters as possible and add null terminator
        memcpy(dest, src, dest_size - 1);
        dest[dest_size - 1] = '\0';
        return false; // Indicate truncation occurred
    }
    
    // Copy the full string with null terminator
    memcpy(dest, src, src_len);
    dest[src_len] = '\0';
    
    return true;
}

/**
 * Securely formats a string with proper bounds checking and null termination
 * This is a safer replacement for sprintf
 * @param dest Destination buffer
 * @param dest_size Size of the destination buffer (including space for null terminator)
 * @param format Printf-style format string
 * @return true if formatting was successful, false if truncation occurred
 */
bool ar_io_string_format(char *dest, size_t dest_size, const char *format, ...) {
    if (!dest || !format || dest_size == 0) {
        if (dest && dest_size > 0) {
            dest[0] = '\0'; // Ensure null termination if buffer exists
        }
        return false;
    }

    // Initialize destination to empty string for safety
    dest[0] = '\0';
    
    va_list args;
    va_start(args, format);
    
    // Format the string with vsnprintf which handles null termination and bounds checking
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wformat-nonliteral"
    int result = vsnprintf(dest, dest_size, format, args);
    #pragma GCC diagnostic pop
    
    va_end(args);
    
    // Check for errors or truncation
    if (result < 0) {
        // Formatting error
        dest[0] = '\0';
        return false;
    }
    
    if ((size_t)result >= dest_size) {
        // Truncation occurred
        // vsnprintf already ensures null termination
        return false;
    }
    
    // Success
    return true;
}

