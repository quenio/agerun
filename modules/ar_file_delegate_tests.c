#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "ar_file_delegate.h"
#include "ar_heap.h"
#include "ar_assert.h"
#include "ar_log.h"
#include "ar_data.h"
#include "ar_io.h"

// Test function declarations
static void test_file_delegate__create_returns_non_null(void);
static void test_file_delegate__create_handles_null_log(void);
static void test_file_delegate__create_handles_null_path(void);
static void test_file_delegate__get_type(void);
static void test_file_delegate__destroy_cleans_up(void);
static void test_file_delegate__destroy_handles_null(void);
static void test_file_delegate__handle_read_message(void);
static void test_file_delegate__handle_write_message(void);
static void test_file_delegate__rejects_directory_traversal(void);
static void test_file_delegate__rejects_large_files(void);

static ar_file_delegate_t* _create_delegate_with_limit(ar_log_t *ref_log, size_t max_file_size) {
    char mut_cwd[1024];
    if (getcwd(mut_cwd, sizeof(mut_cwd)) == NULL) {
        return NULL;
    }

    return ar_file_delegate__create(ref_log, mut_cwd, max_file_size);
}

static bool _write_test_file(const char *ref_path, const char *ref_content) {
    FILE *mut_file = NULL;
    ar_file_result_t ref_result = ar_io__open_file(ref_path, "w", &mut_file);
    if (ref_result != AR_FILE_RESULT__SUCCESS) {
        return false;
    }

    size_t ref_content_len = strlen(ref_content);
    size_t ref_bytes_written = fwrite(ref_content, 1, ref_content_len, mut_file);
    ar_io__close_file(mut_file, ref_path);

    return ref_bytes_written == ref_content_len;
}

static char* _read_test_file(const char *ref_path, char *mut_buffer, size_t buffer_size) {
    FILE *mut_file = NULL;
    ar_file_result_t ref_open_result = ar_io__open_file(ref_path, "r", &mut_file);
    if (ref_open_result != AR_FILE_RESULT__SUCCESS) {
        return NULL;
    }

    char *ref_line = fgets(mut_buffer, (int)buffer_size, mut_file);
    ar_io__close_file(mut_file, ref_path);

    return ref_line;
}

int main(void) {
    // Directory check
    char mut_cwd[1024];
    if (getcwd(mut_cwd, sizeof(mut_cwd)) != NULL) {
        if (!strstr(mut_cwd, "/bin/") && !strstr(mut_cwd, "/bin")) {
            fprintf(stderr, "ERROR: Tests must be run from the bin directory!\n");
            fprintf(stderr, "Current directory: %s\n", mut_cwd);
            fprintf(stderr, "Please run: cd /Users/quenio/Repos/agerun/bin && ./ar_file_delegate_tests\n");
            return 1;
        }
    }

    printf("Running file_delegate module tests...\n");

    // Run tests
    test_file_delegate__create_returns_non_null();
    test_file_delegate__create_handles_null_log();
    test_file_delegate__create_handles_null_path();
    test_file_delegate__get_type();
    test_file_delegate__destroy_cleans_up();
    test_file_delegate__destroy_handles_null();
    test_file_delegate__handle_read_message();
    test_file_delegate__handle_write_message();
    test_file_delegate__rejects_directory_traversal();
    test_file_delegate__rejects_large_files();

    printf("All file_delegate tests passed!\n");
    return 0;
}

static void test_file_delegate__create_returns_non_null(void) {
    printf("  test_file_delegate__create_returns_non_null...\n");

    // Given a log instance for the delegate
    ar_log_t *own_log = ar_log__create();

    // When creating a FileDelegate
    ar_file_delegate_t *own_delegate = ar_file_delegate__create(own_log, "/tmp/allowed", 0);

    // Then it should return non-NULL
    AR_ASSERT(own_delegate != NULL, "FileDelegate should be created");

    // Cleanup
    ar_file_delegate__destroy(own_delegate);
    ar_log__destroy(own_log);

    printf("    PASS\n");
}

static void test_file_delegate__create_handles_null_log(void) {
    printf("  test_file_delegate__create_handles_null_log...\n");

    // Given a NULL log parameter (log module can handle NULL)
    ar_log_t *ref_log = NULL;

    // When creating a FileDelegate with NULL log
    ar_file_delegate_t *own_delegate = ar_file_delegate__create(ref_log, "/tmp/allowed", 0);

    // Then it should succeed (log module handles NULL gracefully)
    AR_ASSERT(own_delegate != NULL, "Should accept NULL log");

    // Cleanup
    ar_file_delegate__destroy(own_delegate);

    printf("    PASS\n");
}

static void test_file_delegate__create_handles_null_path(void) {
    printf("  test_file_delegate__create_handles_null_path...\n");

    // Given a log instance and a NULL path parameter
    ar_log_t *own_log = ar_log__create();

    // When creating a FileDelegate with NULL path
    ar_file_delegate_t *own_delegate = ar_file_delegate__create(own_log, NULL, 0);

    // Then it should return NULL
    AR_ASSERT(own_delegate == NULL, "Should reject NULL path");

    // Cleanup
    ar_log__destroy(own_log);

    printf("    PASS\n");
}

static void test_file_delegate__get_type(void) {
    printf("  test_file_delegate__get_type...\n");

    // Given a FileDelegate instance
    ar_log_t *own_log = ar_log__create();
    ar_file_delegate_t *own_delegate = ar_file_delegate__create(own_log, "/tmp/allowed", 0);

    // When getting the delegate type
    const char *ref_type = ar_file_delegate__get_type(own_delegate);

    // Then it should return "file"
    AR_ASSERT(strcmp(ref_type, "file") == 0, "Type should be 'file'");

    // Cleanup
    ar_file_delegate__destroy(own_delegate);
    ar_log__destroy(own_log);

    printf("    PASS\n");
}

static void test_file_delegate__destroy_cleans_up(void) {
    printf("  test_file_delegate__destroy_cleans_up...\n");

    // Given a FileDelegate instance
    ar_log_t *own_log = ar_log__create();
    ar_file_delegate_t *own_delegate = ar_file_delegate__create(own_log, "/tmp/allowed", 0);
    AR_ASSERT(own_delegate != NULL, "Setup: delegate created");

    // When destroying the delegate
    ar_file_delegate__destroy(own_delegate);

    // Then no memory should leak (verified by memory report)
    // No explicit assertion needed - memory leak check validates this

    // Cleanup
    ar_log__destroy(own_log);

    printf("    PASS\n");
}

static void test_file_delegate__destroy_handles_null(void) {
    printf("  test_file_delegate__destroy_handles_null...\n");

    // Given a NULL delegate
    ar_file_delegate_t *own_delegate = NULL;

    // When destroying NULL delegate
    ar_file_delegate__destroy(own_delegate);

    // Then it should not crash (no assertion needed - just shouldn't crash)
    // Success = no segfault

    printf("    PASS\n");
}

static void test_file_delegate__handle_read_message(void) {
    printf("  test_file_delegate__handle_read_message...\n");

    // Given a FileDelegate and a readable file
    ar_log_t *own_log = ar_log__create();
    ar_file_delegate_t *own_delegate = _create_delegate_with_limit(own_log, 1024);
    AR_ASSERT(own_delegate != NULL, "Setup: delegate created");

    const char *ref_path = "file_delegate_read.txt";
    AR_ASSERT(_write_test_file(ref_path, "hello"), "Setup: wrote test file");

    ar_data_t *own_message = ar_data__create_map();
    AR_ASSERT(own_message != NULL, "Setup: message created");
    AR_ASSERT(ar_data__set_map_string(own_message, "action", "read"), "Setup: action set");
    AR_ASSERT(ar_data__set_map_string(own_message, "path", ref_path), "Setup: path set");

    // When handling the message
    ar_data_t *own_response = ar_file_delegate__handle_message(own_delegate, own_message, 1);

    // Then it should return success with content
    AR_ASSERT(own_response != NULL, "Response should be returned");
    const char *ref_status = ar_data__get_map_string(own_response, "status");
    const char *ref_content = ar_data__get_map_string(own_response, "content");
    AR_ASSERT(ref_status != NULL, "Status should be set");
    AR_ASSERT(strcmp(ref_status, "success") == 0, "Status should be success");
    AR_ASSERT(ref_content != NULL, "Content should be set");
    AR_ASSERT(strcmp(ref_content, "hello") == 0, "Content should match file");

    // Cleanup
    ar_data__destroy(own_response);
    ar_data__destroy(own_message);
    remove(ref_path);
    ar_file_delegate__destroy(own_delegate);
    ar_log__destroy(own_log);

    printf("    PASS\n");
}

static void test_file_delegate__handle_write_message(void) {
    printf("  test_file_delegate__handle_write_message...\n");

    // Given a FileDelegate
    ar_log_t *own_log = ar_log__create();
    ar_file_delegate_t *own_delegate = _create_delegate_with_limit(own_log, 1024);
    AR_ASSERT(own_delegate != NULL, "Setup: delegate created");

    const char *ref_path = "file_delegate_write.txt";

    ar_data_t *own_message = ar_data__create_map();
    AR_ASSERT(own_message != NULL, "Setup: message created");
    AR_ASSERT(ar_data__set_map_string(own_message, "action", "write"), "Setup: action set");
    AR_ASSERT(ar_data__set_map_string(own_message, "path", ref_path), "Setup: path set");
    AR_ASSERT(ar_data__set_map_string(own_message, "content", "payload"), "Setup: content set");

    // When handling the message
    ar_data_t *own_response = ar_file_delegate__handle_message(own_delegate, own_message, 1);

    // Then it should return success and write the file
    AR_ASSERT(own_response != NULL, "Response should be returned");
    const char *ref_status = ar_data__get_map_string(own_response, "status");
    AR_ASSERT(ref_status != NULL, "Status should be set");
    AR_ASSERT(strcmp(ref_status, "success") == 0, "Status should be success");

    char mut_file_contents[32] = {0};
    AR_ASSERT(_read_test_file(ref_path, mut_file_contents, sizeof(mut_file_contents)) != NULL,
              "File should be written");
    AR_ASSERT(strcmp(mut_file_contents, "payload") == 0, "File should contain payload");

    // Cleanup
    ar_data__destroy(own_response);
    ar_data__destroy(own_message);
    remove(ref_path);
    ar_file_delegate__destroy(own_delegate);
    ar_log__destroy(own_log);

    printf("    PASS\n");
}

static void test_file_delegate__rejects_directory_traversal(void) {
    printf("  test_file_delegate__rejects_directory_traversal...\n");

    // Given a FileDelegate
    ar_log_t *own_log = ar_log__create();
    ar_file_delegate_t *own_delegate = _create_delegate_with_limit(own_log, 1024);
    AR_ASSERT(own_delegate != NULL, "Setup: delegate created");

    ar_data_t *own_message = ar_data__create_map();
    AR_ASSERT(own_message != NULL, "Setup: message created");
    AR_ASSERT(ar_data__set_map_string(own_message, "action", "read"), "Setup: action set");
    AR_ASSERT(ar_data__set_map_string(own_message, "path", "../secret.txt"), "Setup: path set");

    // When handling the message
    ar_data_t *own_response = ar_file_delegate__handle_message(own_delegate, own_message, 1);

    // Then it should return an error
    AR_ASSERT(own_response != NULL, "Response should be returned");
    const char *ref_status = ar_data__get_map_string(own_response, "status");
    const char *ref_message = ar_data__get_map_string(own_response, "message");
    AR_ASSERT(ref_status != NULL, "Status should be set");
    AR_ASSERT(strcmp(ref_status, "error") == 0, "Status should be error");
    AR_ASSERT(ref_message != NULL, "Message should be set");
    AR_ASSERT(strcmp(ref_message, "Invalid path") == 0, "Message should mention invalid path");

    // Cleanup
    ar_data__destroy(own_response);
    ar_data__destroy(own_message);
    ar_file_delegate__destroy(own_delegate);
    ar_log__destroy(own_log);

    printf("    PASS\n");
}

static void test_file_delegate__rejects_large_files(void) {
    printf("  test_file_delegate__rejects_large_files...\n");

    // Given a FileDelegate with a small max file size
    ar_log_t *own_log = ar_log__create();
    ar_file_delegate_t *own_delegate = _create_delegate_with_limit(own_log, 4);
    AR_ASSERT(own_delegate != NULL, "Setup: delegate created");

    const char *ref_path = "file_delegate_large.txt";
    AR_ASSERT(_write_test_file(ref_path, "12345"), "Setup: wrote test file");

    ar_data_t *own_message = ar_data__create_map();
    AR_ASSERT(own_message != NULL, "Setup: message created");
    AR_ASSERT(ar_data__set_map_string(own_message, "action", "read"), "Setup: action set");
    AR_ASSERT(ar_data__set_map_string(own_message, "path", ref_path), "Setup: path set");

    // When handling the message
    ar_data_t *own_response = ar_file_delegate__handle_message(own_delegate, own_message, 1);

    // Then it should return a size error
    AR_ASSERT(own_response != NULL, "Response should be returned");
    const char *ref_status = ar_data__get_map_string(own_response, "status");
    const char *ref_message = ar_data__get_map_string(own_response, "message");
    AR_ASSERT(ref_status != NULL, "Status should be set");
    AR_ASSERT(strcmp(ref_status, "error") == 0, "Status should be error");
    AR_ASSERT(ref_message != NULL, "Message should be set");
    AR_ASSERT(strcmp(ref_message, "File too large") == 0, "Message should mention file size");

    // Cleanup
    ar_data__destroy(own_response);
    ar_data__destroy(own_message);
    remove(ref_path);
    ar_file_delegate__destroy(own_delegate);
    ar_log__destroy(own_log);

    printf("    PASS\n");
}
