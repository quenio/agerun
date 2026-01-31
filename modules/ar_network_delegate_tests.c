#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "ar_network_delegate.h"
#include "ar_log.h"
#include "ar_data.h"
#include "ar_assert.h"

// Test function declarations
static void test_network_delegate__create_and_destroy(void);
static void test_network_delegate__handle_get_message(void);
static void test_network_delegate__rejects_non_whitelisted_url(void);
static void test_network_delegate__respects_size_limit(void);
static void test_network_delegate__handle_post_message(void);

static ar_data_t* _create_message(const char *ref_action, const char *ref_url) {
    ar_data_t *own_message = ar_data__create_map();
    if (!own_message) {
        return NULL;
    }

    if (!ar_data__set_map_string(own_message, "action", ref_action)) {
        ar_data__destroy(own_message);
        return NULL;
    }
    if (!ar_data__set_map_string(own_message, "url", ref_url)) {
        ar_data__destroy(own_message);
        return NULL;
    }

    return own_message;
}

int main(void) {
    // Directory check
    char mut_cwd[1024];
    if (getcwd(mut_cwd, sizeof(mut_cwd)) != NULL) {
        if (!strstr(mut_cwd, "/bin/") && !strstr(mut_cwd, "/bin")) {
            fprintf(stderr, "ERROR: Tests must be run from the bin directory!\n");
            fprintf(stderr, "Current directory: %s\n", mut_cwd);
            fprintf(stderr, "Please run: cd /Users/quenio/Repos/agerun/bin && ./ar_network_delegate_tests\n");
            return 1;
        }
    }

    printf("Running network_delegate module tests...\n");

    test_network_delegate__create_and_destroy();
    test_network_delegate__handle_get_message();
    test_network_delegate__rejects_non_whitelisted_url();
    test_network_delegate__respects_size_limit();
    test_network_delegate__handle_post_message();

    printf("All network_delegate tests passed!\n");
    return 0;
}

static void test_network_delegate__create_and_destroy(void) {
    printf("  test_network_delegate__create_and_destroy...\n");

    const char *ref_whitelist[] = {"https://example.com"};
    ar_log_t *own_log = ar_log__create();

    ar_network_delegate_t *own_delegate = ar_network_delegate__create(
        own_log,
        ref_whitelist,
        1,
        0,
        0);

    AR_ASSERT(own_delegate != NULL, "Delegate should be created");

    ar_network_delegate__destroy(own_delegate);
    ar_log__destroy(own_log);

    printf("    PASS\n");
}

static void test_network_delegate__handle_get_message(void) {
    printf("  test_network_delegate__handle_get_message...\n");

    const char *ref_whitelist[] = {"https://example.com"};
    ar_log_t *own_log = ar_log__create();

    ar_network_delegate_t *own_delegate = ar_network_delegate__create(
        own_log,
        ref_whitelist,
        1,
        16,
        5);
    AR_ASSERT(own_delegate != NULL, "Setup: delegate created");

    ar_data_t *own_message = _create_message("GET", "https://example.com/data");
    AR_ASSERT(own_message != NULL, "Setup: message created");

    ar_data_t *own_response = ar_network_delegate__handle_message(own_delegate, own_message, 1);
    AR_ASSERT(own_response != NULL, "Response should be returned");

    const char *ref_status = ar_data__get_map_string(own_response, "status");
    const char *ref_content = ar_data__get_map_string(own_response, "content");
    int ref_stubbed = ar_data__get_map_integer(own_response, "stubbed");
    AR_ASSERT(ref_status != NULL, "Status should be set");
    AR_ASSERT(strcmp(ref_status, "success") == 0, "Status should be success");
    AR_ASSERT(ref_content != NULL, "Content should be set");
    AR_ASSERT(strcmp(ref_content, "stub") == 0, "Content should be stub");
    AR_ASSERT(ref_stubbed == 1, "Stubbed flag should be set");

    ar_data__destroy(own_response);
    ar_data__destroy(own_message);
    ar_network_delegate__destroy(own_delegate);
    ar_log__destroy(own_log);

    printf("    PASS\n");
}

static void test_network_delegate__rejects_non_whitelisted_url(void) {
    printf("  test_network_delegate__rejects_non_whitelisted_url...\n");

    const char *ref_whitelist[] = {"https://allowed.com"};
    ar_log_t *own_log = ar_log__create();

    ar_network_delegate_t *own_delegate = ar_network_delegate__create(
        own_log,
        ref_whitelist,
        1,
        16,
        5);
    AR_ASSERT(own_delegate != NULL, "Setup: delegate created");

    ar_data_t *own_message = _create_message("GET", "https://blocked.com");
    AR_ASSERT(own_message != NULL, "Setup: message created");

    ar_data_t *own_response = ar_network_delegate__handle_message(own_delegate, own_message, 1);
    AR_ASSERT(own_response != NULL, "Response should be returned");

    const char *ref_status = ar_data__get_map_string(own_response, "status");
    const char *ref_message = ar_data__get_map_string(own_response, "message");
    AR_ASSERT(ref_status != NULL, "Status should be set");
    AR_ASSERT(strcmp(ref_status, "error") == 0, "Status should be error");
    AR_ASSERT(ref_message != NULL, "Message should be set");
    AR_ASSERT(strcmp(ref_message, "URL not whitelisted") == 0, "Message should mention whitelist");

    ar_data__destroy(own_response);
    ar_data__destroy(own_message);
    ar_network_delegate__destroy(own_delegate);
    ar_log__destroy(own_log);

    printf("    PASS\n");
}

static void test_network_delegate__respects_size_limit(void) {
    printf("  test_network_delegate__respects_size_limit...\n");

    const char *ref_whitelist[] = {"https://example.com"};
    ar_log_t *own_log = ar_log__create();

    ar_network_delegate_t *own_delegate = ar_network_delegate__create(
        own_log,
        ref_whitelist,
        1,
        2,
        5);
    AR_ASSERT(own_delegate != NULL, "Setup: delegate created");

    ar_data_t *own_message = _create_message("GET", "https://example.com/data");
    AR_ASSERT(own_message != NULL, "Setup: message created");

    ar_data_t *own_response = ar_network_delegate__handle_message(own_delegate, own_message, 1);
    AR_ASSERT(own_response != NULL, "Response should be returned");

    const char *ref_status = ar_data__get_map_string(own_response, "status");
    const char *ref_message = ar_data__get_map_string(own_response, "message");
    AR_ASSERT(ref_status != NULL, "Status should be set");
    AR_ASSERT(strcmp(ref_status, "error") == 0, "Status should be error");
    AR_ASSERT(ref_message != NULL, "Message should be set");
    AR_ASSERT(strcmp(ref_message, "Response too large") == 0, "Message should mention size");

    ar_data__destroy(own_response);
    ar_data__destroy(own_message);
    ar_network_delegate__destroy(own_delegate);
    ar_log__destroy(own_log);

    printf("    PASS\n");
}

static void test_network_delegate__handle_post_message(void) {
    printf("  test_network_delegate__handle_post_message...\n");

    const char *ref_whitelist[] = {"https://example.com"};
    ar_log_t *own_log = ar_log__create();

    ar_network_delegate_t *own_delegate = ar_network_delegate__create(
        own_log,
        ref_whitelist,
        1,
        16,
        5);
    AR_ASSERT(own_delegate != NULL, "Setup: delegate created");

    ar_data_t *own_message = _create_message("POST", "https://example.com/api");
    AR_ASSERT(own_message != NULL, "Setup: message created");
    AR_ASSERT(ar_data__set_map_string(own_message, "body", "payload"), "Setup: body set");

    ar_data_t *own_response = ar_network_delegate__handle_message(own_delegate, own_message, 1);
    AR_ASSERT(own_response != NULL, "Response should be returned");

    const char *ref_status = ar_data__get_map_string(own_response, "status");
    const char *ref_content = ar_data__get_map_string(own_response, "content");
    int ref_stubbed = ar_data__get_map_integer(own_response, "stubbed");
    AR_ASSERT(ref_status != NULL, "Status should be set");
    AR_ASSERT(strcmp(ref_status, "success") == 0, "Status should be success");
    AR_ASSERT(ref_content != NULL, "Content should be set");
    AR_ASSERT(strcmp(ref_content, "stub") == 0, "Content should be stub");
    AR_ASSERT(ref_stubbed == 1, "Stubbed flag should be set");

    ar_data__destroy(own_response);
    ar_data__destroy(own_message);
    ar_network_delegate__destroy(own_delegate);
    ar_log__destroy(own_log);

    printf("    PASS\n");
}
