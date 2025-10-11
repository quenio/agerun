#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "ar_proxy.h"
#include "ar_heap.h"

// Test function declarations
static void test_proxy__create_and_destroy(void);

int main(void) {
    // Directory check
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        if (!strstr(cwd, "/bin/") && !strstr(cwd, "/bin")) {
            fprintf(stderr, "ERROR: Tests must be run from the bin directory!\n");
            fprintf(stderr, "Current directory: %s\n", cwd);
            fprintf(stderr, "Please run: cd /Users/quenio/Repos/agerun/bin && ./ar_proxy_tests\n");
            return 1;
        }
    }

    printf("Running proxy module tests...\n");

    // Run tests
    test_proxy__create_and_destroy();

    printf("All proxy tests passed!\n");
    return 0;
}

static void test_proxy__create_and_destroy(void) {
    printf("  test_proxy__create_and_destroy...\n");

    // Given no prerequisites

    // When creating a proxy
    ar_proxy_t *own_proxy = ar_proxy__create();

    // Then the proxy should be created successfully
    if (!own_proxy) {
        fprintf(stderr, "    FAIL: Proxy creation returned NULL\n");
        return;
    }

    // Clean up
    ar_proxy__destroy(own_proxy);

    printf("    PASS\n");
}
