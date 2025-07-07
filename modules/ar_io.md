# IO Module

The IO module provides secure file I/O operations with comprehensive error handling and safety measures. It's designed to be robust against common file operation vulnerabilities, ensuring that file operations are performed safely and securely.

## Overview

The IO module (`ar_io`) implements secure file operations including:

- Error and warning reporting with consistent formatting
- Secure file reading with bounds checking and buffer overflow prevention
- Safe file opening and closing with proper error detection and handling
- File backup creation and restoration for recovery from failed operations
- Secure file permissions management
- Atomic file writing using temporary files and safe renaming

This module serves as a foundation for other modules that need to perform file operations, ensuring that those operations are performed safely and consistently.

## Key Features

### Message Reporting

- **Consistent Message Formatting**: All messages use consistent formatting with proper prefixes and newline handling
- **Error Categories**: Different error types (permissions, not found, corrupt data) for precise reporting
- **Three-Tier Reporting System**:
  - Error function for critical issues (to stderr)
  - Warning function for non-critical issues (to stderr)
  - Info function for normal operational status (to stdout)

### File Safety

- **Bounds Checking**: All read operations include strict bounds checking to prevent buffer overflows
- **Atomic Operations**: File writes use the atomic rename pattern to prevent corruption
- **Automatic Backups**: Key operations automatically create backups that can be restored in case of failure
- **Temporary Files**: Uses temporary files with secure permissions for intermediate operations
- **Error Recovery**: Built-in error recovery mechanisms including backup restoration

### Cross-Platform Support

- **Platform-Independent API**: The module provides a consistent API across different platforms
- **Platform-Specific Implementations**: Specific implementations for different operating systems (e.g., permissions handling)

## API Reference

### Error, Warning, and Info Reporting

```c
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
```

### File Result Types

```c
/**
 * Enumeration for file operation results with detailed error types
 */
typedef enum file_result_e {
    AR_FILE_RESULT__SUCCESS,             // Operation completed successfully
    AR_FILE_RESULT__ERROR_OPEN,          // Failed to open file
    AR_FILE_RESULT__ERROR_READ,          // Failed to read from file
    AR_FILE_RESULT__ERROR_WRITE,         // Failed to write to file
    AR_FILE_RESULT__ERROR_PERMISSIONS,   // Insufficient permissions
    AR_FILE_RESULT__ERROR_NOT_FOUND,     // File not found
    AR_FILE_RESULT__ERROR_CORRUPT,       // File is corrupt or malformed
    AR_FILE_RESULT__ERROR_ALREADY_EXISTS, // File already exists (for creation operations)
    AR_FILE_RESULT__ERROR_UNKNOWN        // Unknown error
} ar_file_result_t;

/**
 * Gets a detailed error message for a ar_file_result_t code
 * @param result The result code to get a message for
 * @return A human-readable error message
 */
const char* ar_io__error_message(ar_file_result_t result);
```

### File Operations

```c
/**
 * Helper function for secure file reading with bounds checking
 * @param fp Pointer to an open file
 * @param buffer Buffer to store the read line
 * @param buffer_size Size of the buffer
 * @param filename Name of the file being read (for error messages)
 * @return true if a line was read successfully, false otherwise
 */
bool ar_io__read_line(FILE *fp, char *buffer, int buffer_size, const char *filename);

/**
 * Safely opens a file with proper error checking and reporting
 * @param filename Path to the file to open
 * @param mode Mode to open the file in ("r", "w", etc.)
 * @param file_ptr Pointer to store the opened file handle
 * @return AR_FILE_RESULT__SUCCESS if successful, appropriate error code otherwise
 */
ar_file_result_t ar_io__open_file(const char *filename, const char *mode, FILE **file_ptr);

/**
 * Safely closes a file with error checking
 * @param fp Pointer to the file to close
 * @param filename Name of the file (for error reporting)
 * @return AR_FILE_RESULT__SUCCESS if successful, appropriate error code otherwise
 */
ar_file_result_t ar_io__close_file(FILE *fp, const char *filename);
```

### Backup and Security

```c
/**
 * Creates a backup of a file before modifying it
 * @param filename Path to the file to backup
 * @return AR_FILE_RESULT__SUCCESS if backup was created successfully, appropriate error code otherwise
 */
ar_file_result_t ar_io__create_backup(const char *filename);

/**
 * Restores a backup file if the main operation failed
 * @param filename Path to the file to restore
 * @return AR_FILE_RESULT__SUCCESS if backup was restored successfully, appropriate error code otherwise
 */
ar_file_result_t ar_io__restore_backup(const char *filename);

/**
 * Applies secure permissions to a file (owner read/write only)
 * @param filename Path to the file to secure
 * @return AR_FILE_RESULT__SUCCESS if permissions were set successfully, appropriate error code otherwise
 */
ar_file_result_t ar_io__set_secure_permissions(const char *filename);
```

### Secure Writing

```c
/**
 * Safely writes a file using a temporary file and atomic rename
 * @param filename Path to the target file
 * @param write_func Function that writes the content to the temp file
 * @param context Context passed to the write function
 * @return AR_FILE_RESULT__SUCCESS if successful, appropriate error code otherwise
 */
ar_file_result_t ar_io__write_file(const char *filename,
                           bool (*write_func)(FILE *fp, void *context),
                           void *context);
```

## Usage Examples

### Message Reporting

```c
// Informational message (to stdout)
ar_io__info("Processing file %s", filename);

// Warning message for non-critical issues (to stderr)
if (file_size > MAX_RECOMMENDED_SIZE) {
    ar_io__warning("File %s exceeds recommended size (%d bytes)", filename, file_size);
}

// Error message for critical issues (to stderr)
if (result != AR_FILE_RESULT__SUCCESS) {
    ar_io__error("Failed to process file %s: %s", filename, ar_io__error_message(result));
    return false;
}
```

### Secure File Reading

```c
// Open file safely
FILE *file;
ar_file_result_t result = ar_io__open_file("data.txt", "r", &file);
if (result != AR_FILE_RESULT__SUCCESS) {
    // Handle error using ar_io__error_message(result)
    return;
}

// Read file content safely
char buffer[1024];
while (ar_io__read_line(file, buffer, sizeof(buffer), "data.txt")) {
    // Process line safely, knowing bounds are checked
    // ...
}

// Close file safely
ar_io__close_file(file, "data.txt");
```

### Secure File Writing with Backups

```c
// Define a writer function
bool write_data(FILE *fp, void *context) {
    // Cast context to appropriate type
    const char *data = (const char *)context;
    
    // Write data safely
    ar_io__fprintf(fp, "%s\n", data);
    
    return true; // Return false if writing fails
}

// Write file with automatic backup and security
const char *data = "Important data to write securely";
ar_file_result_t result = ar_io__write_file("important.txt", write_data, (void *)data);

if (result != AR_FILE_RESULT__SUCCESS) {
    ar_io__error("Failed to write file: %s", ar_io__error_message(result));
    
    // Handle failure - the module already attempted backup restoration if needed
    return;
}
```

## Best Practices

1. **Always Check Return Values**: All functions return either a success/failure boolean or a detailed result code. Always check these values and handle errors appropriately.

2. **Use Buffer Size Constants**: When using `ar_io__read_line`, always use sizeof(buffer) as the buffer size parameter to avoid hard-coding buffer sizes.

3. **File Backup Strategy**: Use the backup functions before any potentially destructive operations. The module's write functions handle this automatically.

4. **Secure Permissions**: Always call `ar_io__set_secure_permissions` after creating new files with sensitive data. The write function does this automatically.

5. **Message Handling**: Use the provided reporting functions (`ar_io__error`, `ar_io__warning`, `ar_io__info`) for consistent messaging:
   - `ar_io__error`: For error messages that indicate problems (writes to stderr)
   - `ar_io__warning`: For warning messages that indicate potential issues (writes to stderr)
   - `ar_io__info`: For informational messages that track normal operation (writes to stdout)

6. **Atomic Operations**: Use `ar_io__write_file` for all file writes that need to be atomic and secure.

## Memory Safety

The IO module follows the AgeRun Memory Management Model (MMM) principles:

- All function parameters use clear ownership semantics
- IO functions manage their own internal memory allocations
- Temporary resources are properly freed, even in error conditions
- Memory operations use bounds checking to prevent overflows
- File paths are validated before use

## Error Handling

The module provides comprehensive error handling:

1. **Detailed Error Codes**: Each file operation returns a specific error code.
2. **Human-Readable Messages**: The `ar_io__error_message` function converts error codes to readable messages.
3. **Message Reporting System**:
   - `ar_io__error`: Reports critical errors to stderr
   - `ar_io__warning`: Reports non-critical warnings to stderr
   - `ar_io__info`: Reports informational status messages to stdout
4. **Recovery Mechanisms**: Automatic backup and restore functionality for error recovery.

## Platform Compatibility

The IO module is designed to work across different platforms with specific adaptations:

- **Windows**: Uses appropriate Windows-specific file permission functions (`_chmod`)
- **Unix/Linux/macOS**: Uses POSIX-compliant file operations (`chmod`)
- **Cross-Platform API**: Maintains a consistent API across all platforms

## Implementation Notes

This module is implemented in Zig (`ar_io.zig`) while maintaining full C ABI compatibility. The Zig implementation provides additional memory safety guarantees while preserving the exact same interface and behavior as the original C implementation.

## Dependencies

The IO module is a foundational module with minimal dependencies:

- Standard C library (`stdio.h`, `stdlib.h`, `stdbool.h`, etc.)
- System-specific headers for file operations (`sys/stat.h`, etc.)

It does not depend on other AgeRun modules, making it suitable as a basic building block for higher-level modules.