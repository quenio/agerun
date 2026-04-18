#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "ar_assert.h"
#include "ar_local_completion.h"
#include "ar_log.h"

static void test_local_completion__default_model_path_resolves_from_run_exec_directory(void) {
    char own_original_cwd[PATH_MAX];
    char own_run_exec_cwd[PATH_MAX];

    AR_ASSERT(getcwd(own_original_cwd, sizeof(own_original_cwd)) != NULL,
              "Should determine original working directory");
    AR_ASSERT(strstr(own_original_cwd, "/bin/") != NULL,
              "Test should run from a bin directory");
    AR_ASSERT(access("../run-exec", F_OK) == 0,
              "run-exec directory should exist relative to run-tests");
    AR_ASSERT(chdir("../run-exec") == 0,
              "Should switch into run-exec directory");
    AR_ASSERT(getcwd(own_run_exec_cwd, sizeof(own_run_exec_cwd)) != NULL,
              "Should determine run-exec working directory");
    AR_ASSERT(strstr(own_run_exec_cwd, "/bin/run-exec") != NULL,
              "Test should switch into bin/run-exec");
    AR_ASSERT(access("../../models/phi-3-mini-q4.gguf", F_OK) == 0,
              "Fallback model path should exist from run-exec");

    unsetenv("AGERUN_COMPLETE_MODEL");
    unsetenv("AGERUN_COMPLETE_RUNNER");

    ar_log_t *own_log = ar_log__create();
    AR_ASSERT(own_log != NULL, "Log should be created");
    ar_local_completion_t *own_runtime = ar_local_completion__create(own_log);
    AR_ASSERT(own_runtime != NULL, "Runtime should be created");
    AR_ASSERT(ar_local_completion__get_model_path(own_runtime) != NULL,
              "Default model path should resolve");
    AR_ASSERT(strcmp(ar_local_completion__get_model_path(own_runtime), "../../models/phi-3-mini-q4.gguf") == 0,
              "Default model path should fall back to repo-relative model from run-exec");
    AR_ASSERT(access(ar_local_completion__get_model_path(own_runtime), F_OK) == 0,
              "Resolved model path should exist");

    ar_local_completion__destroy(own_runtime);
    ar_log__destroy(own_log);

    AR_ASSERT(chdir(own_original_cwd) == 0,
              "Should restore original working directory");
}

int main(void) {
    printf("Running local completion path tests...\n");
    test_local_completion__default_model_path_resolves_from_run_exec_directory();
    printf("All local completion path tests passed!\n");
    return 0;
}
