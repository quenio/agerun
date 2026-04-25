#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include "ar_local_completion.h"
#include "ar_log.h"
#include "ar_list.h"
#include "ar_data.h"
#include "ar_heap.h"

#if defined(__has_feature)
#if __has_feature(address_sanitizer) || __has_feature(thread_sanitizer)
#define AR_LOCAL_COMPLETION_SANITIZER_BUILD 1
#endif
#endif
#if defined(__SANITIZE_ADDRESS__) || defined(__SANITIZE_THREAD__)
#define AR_LOCAL_COMPLETION_SANITIZER_BUILD 1
#endif
#ifndef AR_LOCAL_COMPLETION_SANITIZER_BUILD
#define AR_LOCAL_COMPLETION_SANITIZER_BUILD 0
#endif

static void _setup_fake_runner_with_output(
    char *mut_runner_path,
    size_t runner_size,
    char *mut_model_path,
    size_t model_size,
    const char *ref_output_script
) {
    assert(mut_runner_path != NULL);
    assert(mut_model_path != NULL);
    assert(ref_output_script != NULL);
    snprintf(mut_runner_path, runner_size, "./fake-llama-cli-%ld.sh", (long)getpid());
    snprintf(mut_model_path, model_size, "./fake-model-%ld.gguf", (long)getpid());

    FILE *own_model = fopen(mut_model_path, "w");
    assert(own_model != NULL);
    fputs("fake-model", own_model);
    fclose(own_model);

    FILE *own_runner = fopen(mut_runner_path, "w");
    assert(own_runner != NULL);
    fputs("#!/bin/sh\n", own_runner);
    fputs(ref_output_script, own_runner);
    fclose(own_runner);
    assert(chmod(mut_runner_path, 0700) == 0);

    assert(setenv("AGERUN_COMPLETE_RUNNER", mut_runner_path, 1) == 0);
    assert(setenv("AGERUN_COMPLETE_MODEL", mut_model_path, 1) == 0);
}

static void _setup_fake_runner(char *mut_runner_path, size_t runner_size, char *mut_model_path, size_t model_size) {
    _setup_fake_runner_with_output(
        mut_runner_path,
        runner_size,
        mut_model_path,
        model_size,
        "printf 'country=Brazil\\ncity=Brasilia\\ncontinent=South America\\nbracey={invalid}\\nspacey= value \\nempty=\\n'\n"
    );
}

static void _cleanup_fake_runner(const char *ref_runner_path, const char *ref_model_path) {
    unsetenv("AGERUN_COMPLETE_RUNNER");
    unsetenv("AGERUN_COMPLETE_MODEL");
    if (ref_runner_path != NULL && ref_runner_path[0] != '\0') {
        remove(ref_runner_path);
    }
    if (ref_model_path != NULL && ref_model_path[0] != '\0') {
        remove(ref_model_path);
    }
}

static int64_t _elapsed_ms_since(const struct timespec *ref_start) {
    struct timespec own_now = {0};
    assert(clock_gettime(CLOCK_MONOTONIC, &own_now) == 0);
    int64_t seconds_ms = (int64_t)(own_now.tv_sec - ref_start->tv_sec) * 1000;
    int64_t nanos_ms = (int64_t)(own_now.tv_nsec - ref_start->tv_nsec) / 1000000;
    return seconds_ms + nanos_ms;
}

static bool _string_has_no_outer_whitespace(const char *ref_text) {
    size_t length = strlen(ref_text);
    if (length == 0U) {
        return false;
    }
    return ref_text[0] != ' ' && ref_text[0] != '\t' &&
           ref_text[length - 1U] != ' ' && ref_text[length - 1U] != '\t';
}

static ar_list_t* _create_placeholder_list(const char *ref_first, const char *ref_second) {
    ar_list_t *own_placeholders = ar_list__create();
    assert(own_placeholders != NULL);
    if (ref_first != NULL) {
        char *own_first = AR__HEAP__STRDUP(ref_first, "local_completion_test_placeholder_first");
        assert(own_first != NULL);
        assert(ar_list__add_last(own_placeholders, own_first) == true);
    }
    if (ref_second != NULL) {
        char *own_second = AR__HEAP__STRDUP(ref_second, "local_completion_test_placeholder_second");
        assert(own_second != NULL);
        assert(ar_list__add_last(own_placeholders, own_second) == true);
    }
    return own_placeholders;
}

static void _destroy_placeholder_list(ar_list_t *own_placeholders) {
    assert(own_placeholders != NULL);
    while (ar_list__count(own_placeholders) > 0U) {
        char *own_name = ar_list__remove_first(own_placeholders);
        if (own_name != NULL) {
            AR__HEAP__FREE(own_name);
        }
    }
    ar_list__destroy(own_placeholders);
}

typedef struct ar_complete_perf_fixture_s {
    const char *ref_template;
    const char *ref_first_placeholder;
    const char *ref_second_placeholder;
} ar_complete_perf_fixture_t;

static const char *g_vocab_only_model_path = "../../llama-cpp/models/ggml-vocab-phi-3.gguf";

static const ar_complete_perf_fixture_t g_complete_perf_fixtures[] = {
    {"The largest country in South America is {country}.", "country", NULL},
    {"The capital of Brazil is {city}.", "city", NULL},
    {"The capital of Argentina is {city}.", "city", NULL},
    {"The capital of Chile is {city}.", "city", NULL},
    {"The capital of Peru is {city}.", "city", NULL},
    {"The capital of Colombia is {city}.", "city", NULL},
    {"The capital of Uruguay is {city}.", "city", NULL},
    {"The capital of Paraguay is {city}.", "city", NULL},
    {"The capital of Japan is {city}.", "city", NULL},
    {"The capital of Canada is {city}.", "city", NULL},
    {"The capital of Australia is {city}.", "city", NULL},
    {"The official language of Brazil is {language}.", "language", NULL},
    {"The official language of Argentina is {language}.", "language", NULL},
    {"The Amazon rainforest is in {continent}.", "continent", NULL},
    {"The Nile river is in {continent}.", "continent", NULL},
    {"France is in {continent}.", "continent", NULL},
    {"Egypt is in {continent}.", "continent", NULL},
    {"Brasilia is the capital of {country}.", "country", NULL},
    {"Brasilia is the capital of {country} in {continent}.", "country", "continent"},
    {"The capital of Brazil is {city}. {city} remains the capital.", "city", NULL},
};

static size_t _count_placeholder_markers(const char *ref_template) {
    size_t count = 0U;
    const char *ref_cursor = ref_template;
    while ((ref_cursor = strchr(ref_cursor, '{')) != NULL) {
        count += 1U;
        ref_cursor += 1;
    }
    return count;
}

static void _assert_short_template_fixture(const ar_complete_perf_fixture_t *ref_fixture) {
    assert(ref_fixture != NULL);
    assert(ref_fixture->ref_template != NULL);
    assert(strlen(ref_fixture->ref_template) <= 120U);
    assert(_count_placeholder_markers(ref_fixture->ref_template) <= 2U);
}

static void _assert_generated_value_is_clean(const char *ref_text) {
    assert(ref_text != NULL);
    assert(strlen(ref_text) > 0U);
    assert(_string_has_no_outer_whitespace(ref_text) == true);
    assert(strchr(ref_text, '{') == NULL);
    assert(strchr(ref_text, '}') == NULL);
}

static void test_local_completion__create_destroy(void) {
    ar_log_t *own_log = ar_log__create();
    assert(own_log != NULL);
    ar_local_completion_t *own_runtime = ar_local_completion__create(own_log);
    assert(own_runtime != NULL);
    assert(ar_local_completion__is_ready(own_runtime) == false);
    ar_local_completion__destroy(own_runtime);
    ar_log__destroy(own_log);
}

static void test_local_completion__environment_override_and_lazy_initialization(void) {
    char own_runner_path[128] = {0};
    char own_model_path[128] = {0};
    _setup_fake_runner(own_runner_path, sizeof(own_runner_path), own_model_path, sizeof(own_model_path));

    ar_log_t *own_log = ar_log__create();
    assert(own_log != NULL);
    ar_local_completion_t *own_runtime = ar_local_completion__create(own_log);
    assert(own_runtime != NULL);
    assert(ar_local_completion__get_model_path(own_runtime) != NULL);
    assert(strcmp(ar_local_completion__get_model_path(own_runtime), own_model_path) == 0);
    assert(ar_local_completion__is_ready(own_runtime) == false);

    ar_list_t *own_placeholders = _create_placeholder_list("country", NULL);
    ar_data_t *own_values = ar_local_completion__complete(
        own_runtime,
        "The largest country in South America is {country}.",
        own_placeholders,
        15000
    );

    assert(own_values != NULL);
    assert(ar_local_completion__is_ready(own_runtime) == true);
    assert(strcmp(ar_data__get_map_string(own_values, "country"), "Brazil") == 0);

    ar_data__destroy(own_values);
    _destroy_placeholder_list(own_placeholders);
    ar_local_completion__destroy(own_runtime);
    ar_log__destroy(own_log);
    _cleanup_fake_runner(own_runner_path, own_model_path);
}

static void test_local_completion__default_path_handling(void) {
    unsetenv("AGERUN_COMPLETE_MODEL");
    unsetenv("AGERUN_COMPLETE_RUNNER");
    ar_log_t *own_log = ar_log__create();
    assert(own_log != NULL);
    ar_local_completion_t *own_runtime = ar_local_completion__create(own_log);
    assert(own_runtime != NULL);
    assert(ar_local_completion__get_model_path(own_runtime) != NULL);
    assert(strstr(ar_local_completion__get_model_path(own_runtime), "phi-3-mini-q4.gguf") != NULL);
    assert(ar_local_completion__is_ready(own_runtime) == false);
    ar_local_completion__destroy(own_runtime);
    ar_log__destroy(own_log);
}

static void test_local_completion__direct_backend_missing_model_file_failure(void) {
    unsetenv("AGERUN_COMPLETE_RUNNER");
    assert(setenv("AGERUN_COMPLETE_MODEL", "./missing-direct-model.gguf", 1) == 0);

    ar_log_t *own_log = ar_log__create();
    assert(own_log != NULL);
    ar_local_completion_t *own_runtime = ar_local_completion__create(own_log);
    assert(own_runtime != NULL);

    ar_list_t *own_placeholders = _create_placeholder_list("country", NULL);
    ar_data_t *own_values = ar_local_completion__complete(
        own_runtime,
        "The largest country in South America is {country}.",
        own_placeholders,
        15000
    );

    assert(own_values == NULL);
    assert(ar_local_completion__is_ready(own_runtime) == false);
    assert(ar_log__get_last_error_message(own_log) != NULL);
    assert(strstr(ar_log__get_last_error_message(own_log), "model file was not found") != NULL);

    _destroy_placeholder_list(own_placeholders);
    ar_local_completion__destroy(own_runtime);
    ar_log__destroy(own_log);
    unsetenv("AGERUN_COMPLETE_MODEL");
}

static void test_local_completion__direct_backend_vocab_only_model_failure(void) {
    unsetenv("AGERUN_COMPLETE_RUNNER");
    assert(setenv("AGERUN_COMPLETE_MODEL", g_vocab_only_model_path, 1) == 0);

    ar_log_t *own_log = ar_log__create();
    assert(own_log != NULL);
    ar_local_completion_t *own_runtime = ar_local_completion__create(own_log);
    assert(own_runtime != NULL);

    ar_list_t *own_placeholders = _create_placeholder_list("country", NULL);
    ar_data_t *own_values = ar_local_completion__complete(
        own_runtime,
        "The largest country in South America is {country}.",
        own_placeholders,
        15000
    );

    assert(own_values == NULL);
    assert(ar_local_completion__is_ready(own_runtime) == false);
    assert(ar_log__get_last_error_message(own_log) != NULL);
    assert(strstr(ar_log__get_last_error_message(own_log), "could not load the configured GGUF model with direct libllama") != NULL);

    _destroy_placeholder_list(own_placeholders);
    ar_local_completion__destroy(own_runtime);
    ar_log__destroy(own_log);
    unsetenv("AGERUN_COMPLETE_MODEL");
}

static void test_local_completion__success_payload_normalization_and_reuse(void) {
    char own_runner_path[128] = {0};
    char own_model_path[128] = {0};
    _setup_fake_runner(own_runner_path, sizeof(own_runner_path), own_model_path, sizeof(own_model_path));

    ar_log_t *own_log = ar_log__create();
    assert(own_log != NULL);
    ar_local_completion_t *own_runtime = ar_local_completion__create(own_log);
    assert(own_runtime != NULL);

    ar_list_t *own_placeholders = _create_placeholder_list("country", "city");
    ar_data_t *own_values = ar_local_completion__complete(
        own_runtime,
        "The largest country in South America is {country}. The capital is {city}.",
        own_placeholders,
        15000
    );

    assert(own_values != NULL);
    assert(strcmp(ar_data__get_map_string(own_values, "country"), "Brazil") == 0);
    assert(strcmp(ar_data__get_map_string(own_values, "city"), "Brasilia") == 0);
    assert(ar_local_completion__is_ready(own_runtime) == true);
    ar_data__destroy(own_values);
    _destroy_placeholder_list(own_placeholders);

    own_placeholders = _create_placeholder_list("continent", NULL);
    own_values = ar_local_completion__complete(
        own_runtime,
        "{continent}",
        own_placeholders,
        15000
    );

    assert(own_values != NULL);
    assert(strcmp(ar_data__get_map_string(own_values, "continent"), "South America") == 0);
    assert(ar_local_completion__is_ready(own_runtime) == true);

    ar_data__destroy(own_values);
    _destroy_placeholder_list(own_placeholders);
    ar_local_completion__destroy(own_runtime);
    ar_log__destroy(own_log);
    _cleanup_fake_runner(own_runner_path, own_model_path);
}

static void test_local_completion__real_phi3_model_smoke(void) {
    const char *ref_model_path = "../../models/phi-3-mini-q4.gguf";
    const int64_t cold_start_limit_ms = 30000;

    assert(access(ref_model_path, F_OK) == 0);
    unsetenv("AGERUN_COMPLETE_RUNNER");
    assert(setenv("AGERUN_COMPLETE_MODEL", ref_model_path, 1) == 0);

    ar_log_t *own_log = ar_log__create();
    assert(own_log != NULL);
    ar_local_completion_t *own_runtime = ar_local_completion__create(own_log);
    assert(own_runtime != NULL);

    ar_list_t *own_placeholders = _create_placeholder_list("country", NULL);
    struct timespec own_start = {0};
    assert(clock_gettime(CLOCK_MONOTONIC, &own_start) == 0);
    ar_data_t *own_values = ar_local_completion__complete(
        own_runtime,
        "The largest country in South America is {country}.",
        own_placeholders,
        cold_start_limit_ms
    );
    int64_t elapsed_ms = _elapsed_ms_since(&own_start);

    assert(elapsed_ms <= cold_start_limit_ms);
    assert(own_values != NULL);
    assert(ar_local_completion__is_ready(own_runtime) == true);
    assert(ar_data__get_map_data(own_values, "country") != NULL);
    assert(ar_data__get_map_string(own_values, "country") != NULL);
    _assert_generated_value_is_clean(ar_data__get_map_string(own_values, "country"));

    printf("Real phi-3 smoke value: country=%s (elapsed=%" PRId64 " ms)\n",
           ar_data__get_map_string(own_values, "country"),
           elapsed_ms);

    ar_data__destroy(own_values);
    _destroy_placeholder_list(own_placeholders);
    ar_local_completion__destroy(own_runtime);
    ar_log__destroy(own_log);
    unsetenv("AGERUN_COMPLETE_MODEL");
    unsetenv("AGERUN_COMPLETE_RUNNER");
}

static void test_local_completion__real_phi3_fixture_set_warm_run_support(void) {
    const char *ref_model_path = "../../models/phi-3-mini-q4.gguf";
    const int64_t warm_run_limit_ms = 15000;
    size_t success_count = 0U;
    size_t under_limit_count = 0U;
    int64_t max_elapsed_ms = 0;
    int64_t total_elapsed_ms = 0;

    assert(access(ref_model_path, F_OK) == 0);
    unsetenv("AGERUN_COMPLETE_RUNNER");
    assert(setenv("AGERUN_COMPLETE_MODEL", ref_model_path, 1) == 0);

    ar_log_t *own_log = ar_log__create();
    assert(own_log != NULL);
    ar_local_completion_t *own_runtime = ar_local_completion__create(own_log);
    assert(own_runtime != NULL);

    {
        ar_list_t *own_warmup_placeholders = _create_placeholder_list(
            g_complete_perf_fixtures[0].ref_first_placeholder,
            g_complete_perf_fixtures[0].ref_second_placeholder
        );
        ar_data_t *own_warmup_values = ar_local_completion__complete(
            own_runtime,
            g_complete_perf_fixtures[0].ref_template,
            own_warmup_placeholders,
            30000
        );
        assert(own_warmup_values != NULL);
        ar_data__destroy(own_warmup_values);
        _destroy_placeholder_list(own_warmup_placeholders);
    }

    for (size_t index = 0U; index < sizeof(g_complete_perf_fixtures) / sizeof(g_complete_perf_fixtures[0]); index += 1U) {
        const ar_complete_perf_fixture_t *ref_fixture = &g_complete_perf_fixtures[index];
        _assert_short_template_fixture(ref_fixture);

        ar_list_t *own_placeholders = _create_placeholder_list(
            ref_fixture->ref_first_placeholder,
            ref_fixture->ref_second_placeholder
        );
        struct timespec own_start = {0};
        assert(clock_gettime(CLOCK_MONOTONIC, &own_start) == 0);
        ar_data_t *own_values = ar_local_completion__complete(
            own_runtime,
            ref_fixture->ref_template,
            own_placeholders,
            warm_run_limit_ms
        );
        int64_t elapsed_ms = _elapsed_ms_since(&own_start);

        if (elapsed_ms > max_elapsed_ms) {
            max_elapsed_ms = elapsed_ms;
        }
        total_elapsed_ms += elapsed_ms;

        if (own_values != NULL) {
            _assert_generated_value_is_clean(
                ar_data__get_map_string(own_values, ref_fixture->ref_first_placeholder)
            );
            if (ref_fixture->ref_second_placeholder != NULL) {
                _assert_generated_value_is_clean(
                    ar_data__get_map_string(own_values, ref_fixture->ref_second_placeholder)
                );
            }
            success_count += 1U;
            ar_data__destroy(own_values);
        }
        if (elapsed_ms <= warm_run_limit_ms) {
            under_limit_count += 1U;
        }

        printf("Warm runtime fixture %zu elapsed=%" PRId64 " ms success=%s template=%s\n",
               index + 1U,
               elapsed_ms,
               own_values != NULL ? "yes" : "no",
               ref_fixture->ref_template);

        _destroy_placeholder_list(own_placeholders);
    }

    printf("Warm runtime summary: fixtures=%zu success=%zu under_%" PRId64 "ms=%zu avg=%" PRId64 " ms max=%" PRId64 " ms\n",
           sizeof(g_complete_perf_fixtures) / sizeof(g_complete_perf_fixtures[0]),
           success_count,
           warm_run_limit_ms,
           under_limit_count,
           total_elapsed_ms / (int64_t)(sizeof(g_complete_perf_fixtures) / sizeof(g_complete_perf_fixtures[0])),
           max_elapsed_ms);

    assert(success_count >= 18U);
    assert(under_limit_count >= 18U);

    ar_local_completion__destroy(own_runtime);
    ar_log__destroy(own_log);
    unsetenv("AGERUN_COMPLETE_MODEL");
    unsetenv("AGERUN_COMPLETE_RUNNER");
}

static void _run_subtest_subprocess(const char *ref_subtest_name) {
    char own_command[256] = {0};
    assert(ref_subtest_name != NULL);
    int written = snprintf(own_command,
                           sizeof(own_command),
                           "AGERUN_LOCAL_COMPLETION_SUBTEST=%s ./ar_local_completion_tests",
                           ref_subtest_name);
    assert(written > 0);
    assert((size_t)written < sizeof(own_command));

    int own_status = system(own_command);
    assert(own_status != -1);
    assert(WIFEXITED(own_status));
    assert(WEXITSTATUS(own_status) == 0);
}

static void test_local_completion__invalid_before_generation_rejects_without_runtime_initialization(void) {
    char own_runner_path[128] = {0};
    char own_model_path[128] = {0};
    _setup_fake_runner(own_runner_path, sizeof(own_runner_path), own_model_path, sizeof(own_model_path));

    ar_log_t *own_log = ar_log__create();
    assert(own_log != NULL);
    ar_local_completion_t *own_runtime = ar_local_completion__create(own_log);
    assert(own_runtime != NULL);

    ar_list_t *own_placeholders = _create_placeholder_list("country", NULL);
    ar_data_t *own_values = ar_local_completion__complete(
        own_runtime,
        "",
        own_placeholders,
        15000
    );

    assert(own_values == NULL);
    assert(ar_local_completion__is_ready(own_runtime) == false);
    assert(ar_log__get_last_error_message(own_log) != NULL);
    assert(strstr(ar_log__get_last_error_message(own_log), "failure_category=invalid_request") != NULL);
    assert(strstr(ar_log__get_last_error_message(own_log), "recovery_hint=") != NULL);

    _destroy_placeholder_list(own_placeholders);
    ar_local_completion__destroy(own_runtime);
    ar_log__destroy(own_log);
    _cleanup_fake_runner(own_runner_path, own_model_path);
}

static void test_local_completion__partial_generation_missing_placeholder_failure_is_actionable(void) {
    char own_runner_path[128] = {0};
    char own_model_path[128] = {0};
    _setup_fake_runner_with_output(
        own_runner_path,
        sizeof(own_runner_path),
        own_model_path,
        sizeof(own_model_path),
        "printf 'country=Brazil\\n'\n"
    );

    ar_log_t *own_log = ar_log__create();
    assert(own_log != NULL);
    ar_local_completion_t *own_runtime = ar_local_completion__create(own_log);
    assert(own_runtime != NULL);

    ar_list_t *own_placeholders = _create_placeholder_list("country", "city");
    ar_data_t *own_values = ar_local_completion__complete(
        own_runtime,
        "The largest country in South America is {country}. The capital is {city}.",
        own_placeholders,
        15000
    );

    assert(own_values == NULL);
    assert(ar_local_completion__is_ready(own_runtime) == true);
    assert(ar_log__get_last_error_message(own_log) != NULL);
    assert(strstr(ar_log__get_last_error_message(own_log), "failure_category=incomplete_placeholder") != NULL);
    assert(strstr(ar_log__get_last_error_message(own_log), "city") != NULL);
    assert(strstr(ar_log__get_last_error_message(own_log), "recovery_hint=") != NULL);

    _destroy_placeholder_list(own_placeholders);
    ar_local_completion__destroy(own_runtime);
    ar_log__destroy(own_log);
    _cleanup_fake_runner(own_runner_path, own_model_path);
}

static void test_local_completion__timeout_and_unavailable_runtime_failures(void) {
    char own_runner_path[128] = {0};
    char own_model_path[128] = {0};
    _setup_fake_runner(own_runner_path, sizeof(own_runner_path), own_model_path, sizeof(own_model_path));

    ar_log_t *own_log = ar_log__create();
    assert(own_log != NULL);
    ar_local_completion_t *own_runtime = ar_local_completion__create(own_log);
    assert(own_runtime != NULL);

    ar_list_t *own_placeholders = _create_placeholder_list("country", NULL);
    ar_data_t *own_values = ar_local_completion__complete(
        own_runtime,
        "The largest country in South America is {country}.",
        own_placeholders,
        0
    );

    assert(own_values == NULL);
    assert(ar_local_completion__is_ready(own_runtime) == false);
    assert(ar_log__get_last_error_message(own_log) != NULL);
    assert(strstr(ar_log__get_last_error_message(own_log), "timeout") != NULL);
    assert(strstr(ar_log__get_last_error_message(own_log), "failure_category=invalid_request") != NULL);
    assert(strstr(ar_log__get_last_error_message(own_log), "recovery_hint=") != NULL);

    ar_local_completion__destroy(own_runtime);
    assert(setenv("AGERUN_COMPLETE_RUNNER", "./definitely-missing-llama-cli", 1) == 0);
    own_runtime = ar_local_completion__create(own_log);
    assert(own_runtime != NULL);

    own_values = ar_local_completion__complete(
        own_runtime,
        "The largest country in South America is {country}.",
        own_placeholders,
        15000
    );
    assert(own_values == NULL);
    assert(ar_log__get_last_error_message(own_log) != NULL);
    assert(strstr(ar_log__get_last_error_message(own_log), "AGERUN_COMPLETE_RUNNER") != NULL);
    assert(strstr(ar_log__get_last_error_message(own_log), "failure_category=runtime_unavailable") != NULL);
    assert(strstr(ar_log__get_last_error_message(own_log), "recovery_hint=") != NULL);

    _destroy_placeholder_list(own_placeholders);
    ar_local_completion__destroy(own_runtime);
    ar_log__destroy(own_log);
    _cleanup_fake_runner(own_runner_path, own_model_path);
}

int main(void) {
    const char *ref_subtest_name = getenv("AGERUN_LOCAL_COMPLETION_SUBTEST");

    printf("Running local completion tests...\n");
    if (ref_subtest_name != NULL) {
        if (strcmp(ref_subtest_name, "real_phi3_model_smoke") == 0) {
            test_local_completion__real_phi3_model_smoke();
        } else if (strcmp(ref_subtest_name, "real_phi3_fixture_set_warm_run_support") == 0) {
            test_local_completion__real_phi3_fixture_set_warm_run_support();
        } else if (strcmp(ref_subtest_name, "direct_backend_missing_model_file_failure") == 0) {
            test_local_completion__direct_backend_missing_model_file_failure();
        } else if (strcmp(ref_subtest_name, "direct_backend_vocab_only_model_failure") == 0) {
            test_local_completion__direct_backend_vocab_only_model_failure();
        } else if (strcmp(ref_subtest_name, "invalid_before_generation_rejects_without_runtime_initialization") == 0) {
            test_local_completion__invalid_before_generation_rejects_without_runtime_initialization();
        } else if (strcmp(ref_subtest_name, "partial_generation_missing_placeholder_failure_is_actionable") == 0) {
            test_local_completion__partial_generation_missing_placeholder_failure_is_actionable();
        } else if (strcmp(ref_subtest_name, "timeout_and_unavailable_runtime_failures") == 0) {
            test_local_completion__timeout_and_unavailable_runtime_failures();
        } else {
            assert(false && "Unknown AGERUN_LOCAL_COMPLETION_SUBTEST value");
        }
        printf("Local completion subtest passed: %s\n", ref_subtest_name);
        return 0;
    }

    test_local_completion__create_destroy();
    test_local_completion__environment_override_and_lazy_initialization();
    test_local_completion__default_path_handling();
    test_local_completion__success_payload_normalization_and_reuse();
    if (getenv("AGERUN_LOCAL_COMPLETION_RUN_REAL_SMOKE") != NULL) {
        test_local_completion__real_phi3_model_smoke();
    } else {
        printf("Skipping real phi-3 smoke test in aggregate run; "
               "make complete-model-smoke covers the real backend.\n");
    }
    _run_subtest_subprocess("direct_backend_missing_model_file_failure");
    if (getenv("AGERUN_LOCAL_COMPLETION_RUN_VOCAB_ONLY_FAILURE") == NULL) {
        printf("Skipping vocab-only direct-backend failure subtest in aggregate run; "
               "set AGERUN_LOCAL_COMPLETION_RUN_VOCAB_ONLY_FAILURE to cover this fixture.\n");
    } else if (AR_LOCAL_COMPLETION_SANITIZER_BUILD) {
        printf("Skipping vocab-only direct-backend failure subtest in sanitizer aggregate run.\n");
    } else if (access(g_vocab_only_model_path, F_OK) != 0) {
        printf("Skipping vocab-only direct-backend failure subtest; vocab fixture is unavailable.\n");
    } else {
        _run_subtest_subprocess("direct_backend_vocab_only_model_failure");
    }
    _run_subtest_subprocess("invalid_before_generation_rejects_without_runtime_initialization");
    _run_subtest_subprocess("partial_generation_missing_placeholder_failure_is_actionable");
    _run_subtest_subprocess("timeout_and_unavailable_runtime_failures");
    printf("All local completion tests passed!\n");
    return 0;
}
