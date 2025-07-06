#include "ar_io.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <stdint.h>

// Forward declarations
static void test_io__error_message_success(void);
static void test_io__error_message_all_codes(void);
static void test_io__string_copy_normal(void);
static void test_io__string_copy_truncation(void);
static void test_io__string_copy_null_parameters(void);
static void test_io__string_copy_empty_string(void);
static void test_io__string_format_normal(void);
static void test_io__string_format_truncation(void);
static void test_io__string_format_null_parameters(void);
static void test_io__open_file_write_mode(void);
static void test_io__open_file_read_mode(void);
static void test_io__open_file_non_existent(void);
static void test_io__open_file_null_parameters(void);
static void test_io__close_file_normal(void);
static void test_io__close_file_null_handle(void);
static void test_io__read_line_normal(void);
static void test_io__read_line_empty_line(void);
static void test_io__read_line_null_parameters(void);
static void test_io__create_backup_normal(void);
static void test_io__create_backup_non_existent(void);
static void test_io__restore_backup_normal(void);
static void test_io__restore_backup_non_existent(void);
static void test_io__set_secure_permissions_normal(void);
static void test_io__set_secure_permissions_non_existent(void);
static void test_io__write_file_normal(void);
static void test_io__write_file_null_parameters(void);

// Test helper function for write_file
static bool write_test_content(FILE *fp, void *context) {
    const char *content = (const char *)context;
    fprintf(fp, "%s", content);
    return true;
}

static void test_io__error_message_success(void) {
    printf("Testing ar_io__error_message() with FILE_SUCCESS...\n");
    
    // Given a success code
    ar_file_result_t code = FILE_SUCCESS;
    
    // When getting the error message
    const char *message = ar_io__error_message(code);
    
    // Then it should return the correct message
    assert(strcmp(message, "Operation completed successfully") == 0);
    
    printf("ar_io__error_message() FILE_SUCCESS test passed!\n");
}

static void test_io__error_message_all_codes(void) {
    printf("Testing ar_io__error_message() with all error codes...\n");
    
    // Given various error codes and their expected messages
    struct {
        ar_file_result_t code;
        const char *expected;
    } test_cases[] = {
        {FILE_ERROR_OPEN, "Failed to open file"},
        {FILE_ERROR_READ, "Failed to read from file"},
        {FILE_ERROR_WRITE, "Failed to write to file"},
        {FILE_ERROR_PERMISSIONS, "Insufficient permissions"},
        {FILE_ERROR_NOT_FOUND, "File not found"},
        {FILE_ERROR_CORRUPT, "File is corrupt or malformed"},
        {FILE_ERROR_ALREADY_EXISTS, "File already exists"},
        {FILE_ERROR_UNKNOWN, "Unknown error"}
    };
    
    // When getting error messages for each code
    for (size_t i = 0; i < sizeof(test_cases) / sizeof(test_cases[0]); i++) {
        const char *message = ar_io__error_message(test_cases[i].code);
        
        // Then each should match the expected message
        assert(strcmp(message, test_cases[i].expected) == 0);
    }
    
    printf("ar_io__error_message() all codes test passed!\n");
}

static void test_io__string_copy_normal(void) {
    printf("Testing ar_io__string_copy() with normal string...\n");
    
    // Given a destination buffer and source string
    char dest[20];
    const char *src = "Hello World";
    
    // When copying the string
    bool result = ar_io__string_copy(dest, src, sizeof(dest));
    
    // Then it should succeed and copy correctly
    assert(result == true);
    assert(strcmp(dest, src) == 0);
    
    printf("ar_io__string_copy() normal test passed!\n");
}

static void test_io__string_copy_truncation(void) {
    printf("Testing ar_io__string_copy() with truncation...\n");
    
    // Given a small buffer and long string
    char dest[5];
    const char *src = "This is too long";
    
    // When copying the string
    bool result = ar_io__string_copy(dest, src, sizeof(dest));
    
    // Then it should return false and truncate properly
    assert(result == false);
    assert(strlen(dest) == sizeof(dest) - 1);
    assert(dest[sizeof(dest) - 1] == '\0');
    
    printf("ar_io__string_copy() truncation test passed!\n");
}

static void test_io__string_copy_null_parameters(void) {
    printf("Testing ar_io__string_copy() with NULL parameters...\n");
    
    char dest[10];
    
    // Given NULL destination
    // When copying
    bool result = ar_io__string_copy(NULL, "test", 10);
    // Then it should return false
    assert(result == false);
    
    // Given NULL source
    // When copying
    result = ar_io__string_copy(dest, NULL, sizeof(dest));
    // Then it should return false
    assert(result == false);
    
    // Given zero size
    // When copying
    result = ar_io__string_copy(dest, "test", 0);
    // Then it should return false
    assert(result == false);
    
    printf("ar_io__string_copy() NULL parameters test passed!\n");
}

static void test_io__string_copy_empty_string(void) {
    printf("Testing ar_io__string_copy() with empty string...\n");
    
    // Given a destination buffer and empty source
    char dest[10] = "original";
    const char *src = "";
    
    // When copying the empty string
    bool result = ar_io__string_copy(dest, src, sizeof(dest));
    
    // Then it should succeed and result in empty string
    assert(result == true);
    assert(strcmp(dest, "") == 0);
    
    printf("ar_io__string_copy() empty string test passed!\n");
}

static void test_io__string_format_normal(void) {
    printf("Testing ar_io__string_format() with normal format...\n");
    
    // Given a destination buffer and format string
    char dest[50];
    
    // When formatting the string
    bool result = ar_io__string_format(dest, sizeof(dest), "Hello %s %d", "World", 42);
    
    // Then it should succeed and format correctly
    assert(result == true);
    assert(strcmp(dest, "Hello World 42") == 0);
    
    printf("ar_io__string_format() normal test passed!\n");
}

static void test_io__string_format_truncation(void) {
    printf("Testing ar_io__string_format() with truncation...\n");
    
    // Given a small buffer
    char dest[5];
    
    // When formatting a long string
    bool result = ar_io__string_format(dest, sizeof(dest), "Hello World");
    
    // Then it should return false
    assert(result == false);
    
    printf("ar_io__string_format() truncation test passed!\n");
}

static void test_io__string_format_null_parameters(void) {
    printf("Testing ar_io__string_format() with NULL parameters...\n");
    
    char dest[10];
    
    // Given NULL destination
    // When formatting
    bool result = ar_io__string_format(NULL, 10, "test");
    // Then it should return false
    assert(result == false);
    
    // Given NULL format
    // When formatting
    result = ar_io__string_format(dest, sizeof(dest), NULL);
    // Then it should return false
    assert(result == false);
    
    // Given zero size
    // When formatting
    result = ar_io__string_format(dest, 0, "test");
    // Then it should return false
    assert(result == false);
    
    printf("ar_io__string_format() NULL parameters test passed!\n");
}

static void test_io__open_file_write_mode(void) {
    printf("Testing ar_io__open_file() in write mode...\n");
    
    // Given a temporary filename
    char temp_filename[] = "/tmp/ar_io_test_XXXXXX";
    int fd = mkstemp(temp_filename);
    assert(fd != -1);
    close(fd);
    
    // When opening the file for writing
    FILE *fp = NULL;
    ar_file_result_t result = ar_io__open_file(temp_filename, "w", &fp);
    
    // Then it should succeed
    assert(result == FILE_SUCCESS);
    assert(fp != NULL);
    
    // Clean up
    ar_io__close_file(fp, temp_filename);
    unlink(temp_filename);
    
    printf("ar_io__open_file() write mode test passed!\n");
}

static void test_io__open_file_read_mode(void) {
    printf("Testing ar_io__open_file() in read mode...\n");
    
    // Given an existing file
    char temp_filename[] = "/tmp/ar_io_test_XXXXXX";
    int fd = mkstemp(temp_filename);
    assert(fd != -1);
    close(fd);
    
    // When opening the file for reading
    FILE *fp = NULL;
    ar_file_result_t result = ar_io__open_file(temp_filename, "r", &fp);
    
    // Then it should succeed
    assert(result == FILE_SUCCESS);
    assert(fp != NULL);
    
    // Clean up
    ar_io__close_file(fp, temp_filename);
    unlink(temp_filename);
    
    printf("ar_io__open_file() read mode test passed!\n");
}

static void test_io__open_file_non_existent(void) {
    printf("Testing ar_io__open_file() with non-existent file...\n");
    
    // Given a non-existent filename
    const char *filename = "/tmp/non_existent_file_12345.txt";
    
    // When trying to open it for reading
    FILE *fp = NULL;
    ar_file_result_t result = ar_io__open_file(filename, "r", &fp);
    
    // Then it should fail with FILE_ERROR_NOT_FOUND
    assert(result == FILE_ERROR_NOT_FOUND);
    assert(fp == NULL);
    
    printf("ar_io__open_file() non-existent file test passed!\n");
}

static void test_io__open_file_null_parameters(void) {
    printf("Testing ar_io__open_file() with NULL parameters...\n");
    
    FILE *fp = NULL;
    
    // Given NULL filename
    // When opening
    ar_file_result_t result = ar_io__open_file(NULL, "r", &fp);
    // Then it should fail
    assert(result == FILE_ERROR_UNKNOWN);
    
    // Given NULL mode
    // When opening
    result = ar_io__open_file("test.txt", NULL, &fp);
    // Then it should fail
    assert(result == FILE_ERROR_UNKNOWN);
    
    // Given NULL file pointer
    // When opening
    result = ar_io__open_file("test.txt", "r", NULL);
    // Then it should fail
    assert(result == FILE_ERROR_UNKNOWN);
    
    printf("ar_io__open_file() NULL parameters test passed!\n");
}

static void test_io__close_file_normal(void) {
    printf("Testing ar_io__close_file() with valid file...\n");
    
    // Given an open file
    char temp_filename[] = "/tmp/ar_io_test_XXXXXX";
    int fd = mkstemp(temp_filename);
    assert(fd != -1);
    close(fd);
    
    FILE *fp = NULL;
    ar_file_result_t result = ar_io__open_file(temp_filename, "w", &fp);
    assert(result == FILE_SUCCESS);
    assert(fp != NULL);
    
    // When closing the file
    result = ar_io__close_file(fp, temp_filename);
    
    // Then it should succeed
    assert(result == FILE_SUCCESS);
    
    // Clean up
    unlink(temp_filename);
    
    printf("ar_io__close_file() normal test passed!\n");
}

static void test_io__close_file_null_handle(void) {
    printf("Testing ar_io__close_file() with NULL handle...\n");
    
    // Given a NULL file handle
    FILE *fp = NULL;
    
    // When closing it
    ar_file_result_t result = ar_io__close_file(fp, "dummy.txt");
    
    // Then it should succeed (no-op)
    assert(result == FILE_SUCCESS);
    
    printf("ar_io__close_file() NULL handle test passed!\n");
}

static void test_io__read_line_normal(void) {
    printf("Testing ar_io__read_line() with normal lines...\n");
    
    // Given a file with multiple lines
    char temp_filename[] = "/tmp/ar_io_test_XXXXXX";
    int fd = mkstemp(temp_filename);
    assert(fd != -1);
    const char *content = "Line 1\nLine 2\nLine 3\n";
    write(fd, content, strlen(content));
    close(fd);
    
    FILE *fp = NULL;
    ar_file_result_t result = ar_io__open_file(temp_filename, "r", &fp);
    assert(result == FILE_SUCCESS);
    
    char buffer[100];
    
    // When reading the first line
    bool read_result = ar_io__read_line(fp, buffer, sizeof(buffer), temp_filename);
    // Then it should succeed and read correctly
    assert(read_result == true);
    assert(strcmp(buffer, "Line 1\n") == 0);
    
    // When reading the second line
    read_result = ar_io__read_line(fp, buffer, sizeof(buffer), temp_filename);
    // Then it should succeed and read correctly
    assert(read_result == true);
    assert(strcmp(buffer, "Line 2\n") == 0);
    
    // Clean up
    ar_io__close_file(fp, temp_filename);
    unlink(temp_filename);
    
    printf("ar_io__read_line() normal test passed!\n");
}

static void test_io__read_line_empty_line(void) {
    printf("Testing ar_io__read_line() with empty line...\n");
    
    // Given a file with an empty line
    char temp_filename[] = "/tmp/ar_io_test_XXXXXX";
    int fd = mkstemp(temp_filename);
    assert(fd != -1);
    const char *content = "Line 1\n\nLine 3\n";
    write(fd, content, strlen(content));
    close(fd);
    
    FILE *fp = NULL;
    ar_file_result_t result = ar_io__open_file(temp_filename, "r", &fp);
    assert(result == FILE_SUCCESS);
    
    char buffer[100];
    
    // Skip first line
    ar_io__read_line(fp, buffer, sizeof(buffer), temp_filename);
    
    // When reading the empty line
    bool read_result = ar_io__read_line(fp, buffer, sizeof(buffer), temp_filename);
    // Then it should succeed and read empty line
    assert(read_result == true);
    assert(strcmp(buffer, "\n") == 0);
    
    // Clean up
    ar_io__close_file(fp, temp_filename);
    unlink(temp_filename);
    
    printf("ar_io__read_line() empty line test passed!\n");
}

static void test_io__read_line_null_parameters(void) {
    printf("Testing ar_io__read_line() with NULL parameters...\n");
    
    char buffer[100];
    FILE *fp = tmpfile();
    assert(fp != NULL);
    
    // Given NULL file pointer
    // When reading
    bool result = ar_io__read_line(NULL, buffer, sizeof(buffer), "test.txt");
    // Then it should fail
    assert(result == false);
    
    // Given NULL buffer
    // When reading
    result = ar_io__read_line(fp, NULL, sizeof(buffer), "test.txt");
    // Then it should fail
    assert(result == false);
    
    // Given zero buffer size
    // When reading
    result = ar_io__read_line(fp, buffer, 0, "test.txt");
    // Then it should fail
    assert(result == false);
    
    // Given NULL filename
    // When reading
    result = ar_io__read_line(fp, buffer, sizeof(buffer), NULL);
    // Then it should fail
    assert(result == false);
    
    fclose(fp);
    
    printf("ar_io__read_line() NULL parameters test passed!\n");
}

static void test_io__create_backup_normal(void) {
    printf("Testing ar_io__create_backup() with existing file...\n");
    
    // Given an existing file with content
    char temp_filename[] = "/tmp/ar_io_test_XXXXXX";
    int fd = mkstemp(temp_filename);
    assert(fd != -1);
    const char *content = "Original content\n";
    write(fd, content, strlen(content));
    close(fd);
    
    // When creating a backup
    ar_file_result_t result = ar_io__create_backup(temp_filename);
    
    // Then it should succeed and backup file should exist
    assert(result == FILE_SUCCESS);
    
    char backup_filename[256];
    snprintf(backup_filename, sizeof(backup_filename), "%s.bak", temp_filename);
    assert(access(backup_filename, F_OK) == 0);
    
    // Clean up
    unlink(temp_filename);
    unlink(backup_filename);
    
    printf("ar_io__create_backup() normal test passed!\n");
}

static void test_io__create_backup_non_existent(void) {
    printf("Testing ar_io__create_backup() with non-existent file...\n");
    
    // Given a non-existent file
    const char *filename = "/tmp/non_existent_file_12345.txt";
    
    // When creating a backup
    ar_file_result_t result = ar_io__create_backup(filename);
    
    // Then it should succeed (no-op)
    assert(result == FILE_SUCCESS);
    
    printf("ar_io__create_backup() non-existent test passed!\n");
}

static void test_io__restore_backup_normal(void) {
    printf("Testing ar_io__restore_backup() with valid backup...\n");
    
    // Given a file and its backup
    char temp_filename[] = "/tmp/ar_io_test_XXXXXX";
    int fd = mkstemp(temp_filename);
    assert(fd != -1);
    const char *original_content = "Original content\n";
    write(fd, original_content, strlen(original_content));
    close(fd);
    
    // Create backup
    ar_file_result_t result = ar_io__create_backup(temp_filename);
    assert(result == FILE_SUCCESS);
    
    // Modify original file
    FILE *fp = fopen(temp_filename, "w");
    assert(fp != NULL);
    fprintf(fp, "Modified content\n");
    fclose(fp);
    
    // When restoring from backup
    result = ar_io__restore_backup(temp_filename);
    
    // Then it should succeed and content should be restored
    assert(result == FILE_SUCCESS);
    
    fp = fopen(temp_filename, "r");
    assert(fp != NULL);
    char buffer[100];
    assert(fgets(buffer, sizeof(buffer), fp) != NULL);
    assert(strcmp(buffer, original_content) == 0);
    fclose(fp);
    
    // Clean up
    unlink(temp_filename);
    
    printf("ar_io__restore_backup() normal test passed!\n");
}

static void test_io__restore_backup_non_existent(void) {
    printf("Testing ar_io__restore_backup() with non-existent backup...\n");
    
    // Given a file without backup
    const char *filename = "/tmp/non_existent_file_12345.txt";
    
    // When trying to restore
    ar_file_result_t result = ar_io__restore_backup(filename);
    
    // Then it should fail with FILE_ERROR_NOT_FOUND
    assert(result == FILE_ERROR_NOT_FOUND);
    
    printf("ar_io__restore_backup() non-existent test passed!\n");
}

static void test_io__set_secure_permissions_normal(void) {
    printf("Testing ar_io__set_secure_permissions() with valid file...\n");
    
    // Given an existing file
    char temp_filename[] = "/tmp/ar_io_test_XXXXXX";
    int fd = mkstemp(temp_filename);
    assert(fd != -1);
    close(fd);
    
    // When setting secure permissions
    ar_file_result_t result = ar_io__set_secure_permissions(temp_filename);
    
    // Then it should succeed
    assert(result == FILE_SUCCESS);
    
    // Clean up
    unlink(temp_filename);
    
    printf("ar_io__set_secure_permissions() normal test passed!\n");
}

static void test_io__set_secure_permissions_non_existent(void) {
    printf("Testing ar_io__set_secure_permissions() with non-existent file...\n");
    
    // Given a non-existent file
    const char *filename = "/tmp/non_existent_file_12345.txt";
    
    // When setting permissions
    ar_file_result_t result = ar_io__set_secure_permissions(filename);
    
    // Then it should fail
    assert(result == FILE_ERROR_PERMISSIONS);
    
    printf("ar_io__set_secure_permissions() non-existent test passed!\n");
}

static void test_io__write_file_normal(void) {
    printf("Testing ar_io__write_file() with normal content...\n");
    
    // Given a filename and content to write
    char temp_filename[] = "/tmp/ar_io_test_XXXXXX";
    int fd = mkstemp(temp_filename);
    assert(fd != -1);
    close(fd);
    
    const char *content = "Test content for write_file\n";
    
    // When writing the file
    ar_file_result_t result = ar_io__write_file(temp_filename, write_test_content, (void *)(uintptr_t)content);
    
    // Then it should succeed and content should be written
    assert(result == FILE_SUCCESS);
    
    FILE *fp = fopen(temp_filename, "r");
    assert(fp != NULL);
    char buffer[100];
    assert(fgets(buffer, sizeof(buffer), fp) != NULL);
    assert(strcmp(buffer, content) == 0);
    fclose(fp);
    
    // Clean up
    unlink(temp_filename);
    
    printf("ar_io__write_file() normal test passed!\n");
}

static void test_io__write_file_null_parameters(void) {
    printf("Testing ar_io__write_file() with NULL parameters...\n");
    
    const char *content = "test";
    
    // Given NULL filename
    // When writing
    ar_file_result_t result = ar_io__write_file(NULL, write_test_content, (void *)(uintptr_t)content);
    // Then it should fail
    assert(result == FILE_ERROR_UNKNOWN);
    
    // Given NULL write function
    // When writing
    result = ar_io__write_file("test.txt", NULL, (void *)(uintptr_t)content);
    // Then it should fail
    assert(result == FILE_ERROR_UNKNOWN);
    
    printf("ar_io__write_file() NULL parameters test passed!\n");
}

int main(void) {
    printf("Starting IO Module Tests...\n\n");
    
    // Error message tests
    test_io__error_message_success();
    test_io__error_message_all_codes();
    
    // String copy tests
    test_io__string_copy_normal();
    test_io__string_copy_truncation();
    test_io__string_copy_null_parameters();
    test_io__string_copy_empty_string();
    
    // String format tests
    test_io__string_format_normal();
    test_io__string_format_truncation();
    test_io__string_format_null_parameters();
    
    // File open/close tests
    test_io__open_file_write_mode();
    test_io__open_file_read_mode();
    test_io__open_file_non_existent();
    test_io__open_file_null_parameters();
    test_io__close_file_normal();
    test_io__close_file_null_handle();
    
    // Read line tests
    test_io__read_line_normal();
    test_io__read_line_empty_line();
    test_io__read_line_null_parameters();
    
    // Backup/restore tests
    test_io__create_backup_normal();
    test_io__create_backup_non_existent();
    test_io__restore_backup_normal();
    test_io__restore_backup_non_existent();
    
    // Permissions tests
    test_io__set_secure_permissions_normal();
    test_io__set_secure_permissions_non_existent();
    
    // Write file tests
    test_io__write_file_normal();
    test_io__write_file_null_parameters();
    
    printf("\nAll 26 tests passed!\n");
    
    return 0;
}