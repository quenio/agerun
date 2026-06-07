#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <unistd.h>
#include "ar_assert.h"
#include "ar_local_completion.h"
#include "ar_log.h"

static void _create_directory_if_missing(const char *ref_path) {
    if (mkdir(ref_path, 0777) != 0) {
        AR_ASSERT(errno == EEXIST, "Directory should be created or already exist");
    }
}

static void _create_fake_shared_model(const char *ref_model_path) {
    FILE *own_model = fopen(ref_model_path, "w");
    AR_ASSERT(own_model != NULL, "Fake shared model should be created");
    fputs("fake-model", own_model);
    fclose(own_model);
}

static void test_local_completion__default_model_path_uses_shared_home_from_exec_cwd(void) {
    char own_original_cwd[PATH_MAX];
    char own_run_tests_cwd[PATH_MAX];
    char own_run_exec_cwd[PATH_MAX];
    char own_original_home[PATH_MAX] = {0};
    char own_fake_home[PATH_MAX];
    char own_fake_state_dir[PATH_MAX];
    char own_fake_bin_dir[PATH_MAX];
    char own_fake_exec_dir[PATH_MAX];
    char own_fake_model_dir[PATH_MAX];
    char own_expected_model_path[PATH_MAX];
    const char *ref_original_home = getenv("HOME");
    bool had_original_home = ref_original_home != NULL;

    AR_ASSERT(getcwd(own_original_cwd, sizeof(own_original_cwd)) != NULL,
              "Should determine original working directory");
    AR_ASSERT(strstr(own_original_cwd, "/bin/") != NULL,
              "Test should run from a bin directory");
    AR_ASSERT(getcwd(own_run_tests_cwd, sizeof(own_run_tests_cwd)) != NULL,
              "Should determine run-tests working directory");
    if (had_original_home) {
        int written = snprintf(own_original_home, sizeof(own_original_home), "%s", ref_original_home);
        AR_ASSERT(written >= 0 && (size_t)written < sizeof(own_original_home),
                  "Original HOME should fit in buffer");
    }

    int written = snprintf(own_fake_home, sizeof(own_fake_home),
                           "%s/fake-home-%ld", own_run_tests_cwd, (long)getpid());
    AR_ASSERT(written >= 0 && (size_t)written < sizeof(own_fake_home),
              "Fake HOME path should fit in buffer");
    written = snprintf(own_fake_state_dir, sizeof(own_fake_state_dir),
                       "%s/.agerun", own_fake_home);
    AR_ASSERT(written >= 0 && (size_t)written < sizeof(own_fake_state_dir),
              "Fake .agerun path should fit in buffer");
    written = snprintf(own_fake_bin_dir, sizeof(own_fake_bin_dir),
                       "%s/bin", own_fake_home);
    AR_ASSERT(written >= 0 && (size_t)written < sizeof(own_fake_bin_dir),
              "Fake bin path should fit in buffer");
    written = snprintf(own_fake_exec_dir, sizeof(own_fake_exec_dir),
                       "%s/run-exec", own_fake_bin_dir);
    AR_ASSERT(written >= 0 && (size_t)written < sizeof(own_fake_exec_dir),
              "Fake run-exec path should fit in buffer");
    written = snprintf(own_fake_model_dir, sizeof(own_fake_model_dir),
                       "%s/models", own_fake_state_dir);
    AR_ASSERT(written >= 0 && (size_t)written < sizeof(own_fake_model_dir),
              "Fake shared model directory should fit in buffer");
    written = snprintf(own_expected_model_path, sizeof(own_expected_model_path),
                       "%s/phi-3-mini-q4.gguf", own_fake_model_dir);
    AR_ASSERT(written >= 0 && (size_t)written < sizeof(own_expected_model_path),
              "Expected shared model path should fit in buffer");

    _create_directory_if_missing(own_fake_home);
    _create_directory_if_missing(own_fake_state_dir);
    _create_directory_if_missing(own_fake_bin_dir);
    _create_directory_if_missing(own_fake_exec_dir);
    _create_directory_if_missing(own_fake_model_dir);
    _create_fake_shared_model(own_expected_model_path);
    AR_ASSERT(chdir(own_fake_exec_dir) == 0,
              "Should switch into run-exec directory");
    AR_ASSERT(getcwd(own_run_exec_cwd, sizeof(own_run_exec_cwd)) != NULL,
              "Should determine run-exec working directory");
    AR_ASSERT(strstr(own_run_exec_cwd, "/bin/run-exec") != NULL,
              "Test should switch into bin/run-exec");

    unsetenv("AGERUN_COMPLETE_MODEL");
    unsetenv("AGERUN_COMPLETE_RUNNER");
    AR_ASSERT(setenv("HOME", own_fake_home, 1) == 0,
              "Fake HOME should be configured");

    ar_log_t *own_log = ar_log__create();
    AR_ASSERT(own_log != NULL, "Log should be created");
    ar_local_completion_t *own_runtime = ar_local_completion__create(own_log);
    AR_ASSERT(own_runtime != NULL, "Runtime should be created");
    AR_ASSERT(ar_local_completion__get_model_path(own_runtime) != NULL,
              "Default model path should resolve");
    AR_ASSERT(strcmp(ar_local_completion__get_model_path(own_runtime), own_expected_model_path) == 0,
              "Default model path should resolve under HOME/.agerun/models");
    AR_ASSERT(access(ar_local_completion__get_model_path(own_runtime), F_OK) == 0,
              "Resolved model path should exist");

    ar_local_completion__destroy(own_runtime);
    ar_log__destroy(own_log);

    AR_ASSERT(chdir(own_original_cwd) == 0,
              "Should restore original working directory");
    if (had_original_home) {
        AR_ASSERT(setenv("HOME", own_original_home, 1) == 0,
                  "Original HOME should be restored");
    } else {
        unsetenv("HOME");
    }
    remove(own_expected_model_path);
    rmdir(own_fake_model_dir);
    rmdir(own_fake_state_dir);
    rmdir(own_fake_exec_dir);
    rmdir(own_fake_bin_dir);
    rmdir(own_fake_home);
}

int main(void) {
    printf("Running local completion path tests...\n");
    test_local_completion__default_model_path_uses_shared_home_from_exec_cwd();
    printf("All local completion path tests passed!\n");
    return 0;
}
