#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include "ar_complete_instruction_evaluator.h"
#include "ar_local_completion.h"
#include "ar_instruction_ast.h"
#include "ar_expression_ast.h"
#include "ar_list.h"
#include "ar_evaluator_fixture.h"
#include "ar_frame.h"
#include "ar_data.h"
#include "ar_log.h"

static char g_runner_path[128] = {0};
static char g_model_path[128] = {0};

static void _setup_fake_runner(void) {
    snprintf(g_runner_path, sizeof(g_runner_path), "./fake-llama-cli-evaluator-%ld.sh", (long)getpid());
    snprintf(g_model_path, sizeof(g_model_path), "./fake-model-evaluator-%ld.gguf", (long)getpid());

    FILE *own_model = fopen(g_model_path, "w");
    assert(own_model != NULL);
    fputs("fake-model", own_model);
    fclose(own_model);

    FILE *own_runner = fopen(g_runner_path, "w");
    assert(own_runner != NULL);
    fputs("#!/bin/sh\n", own_runner);
    fputs("printf 'country=Brazil\\ncity=Brasilia\\ncontinent=South America\\nbracey={invalid}\\nspacey= value \\nempty=\\n'\n", own_runner);
    fclose(own_runner);
    assert(chmod(g_runner_path, 0700) == 0);

    assert(setenv("AGERUN_COMPLETE_RUNNER", g_runner_path, 1) == 0);
    assert(setenv("AGERUN_COMPLETE_MODEL", g_model_path, 1) == 0);
}

static void _cleanup_fake_runner(void) {
    unsetenv("AGERUN_COMPLETE_RUNNER");
    unsetenv("AGERUN_COMPLETE_MODEL");
    if (g_runner_path[0] != '\0') {
        remove(g_runner_path);
    }
    if (g_model_path[0] != '\0') {
        remove(g_model_path);
    }
}

typedef struct ar_complete_perf_fixture_s {
    const char *ref_template;
    const char *ref_first_placeholder;
    const char *ref_second_placeholder;
} ar_complete_perf_fixture_t;

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

static int64_t _elapsed_ms_since(const struct timespec *ref_start) {
    struct timespec own_now = {0};
    assert(clock_gettime(CLOCK_MONOTONIC, &own_now) == 0);
    return (int64_t)(own_now.tv_sec - ref_start->tv_sec) * 1000 +
           (int64_t)(own_now.tv_nsec - ref_start->tv_nsec) / 1000000;
}

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

static bool _complete_result_has_fixture_values(ar_data_t *ref_memory, const ar_complete_perf_fixture_t *ref_fixture) {
    ar_data_t *ref_result = ar_data__get_map_data(ref_memory, "ok");
    if (ref_result == NULL || ar_data__get_type(ref_result) != AR_DATA_TYPE__MAP) {
        return false;
    }
    if (ref_fixture->ref_first_placeholder != NULL &&
        ar_data__get_map_string(ref_result, ref_fixture->ref_first_placeholder) == NULL) {
        return false;
    }
    if (ref_fixture->ref_second_placeholder != NULL &&
        ar_data__get_map_string(ref_result, ref_fixture->ref_second_placeholder) == NULL) {
        return false;
    }
    return true;
}

static ar_instruction_ast_t* _create_complete_ast(
    const char *ref_template,
    const char *ref_base_path,
    const char *ref_result_path
) {
    const char *args[2] = {ref_template, ref_base_path};
    size_t arg_count = ref_base_path ? 2U : 1U;
    ar_instruction_ast_t *own_ast = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__COMPLETE,
        "complete",
        args,
        arg_count,
        ref_result_path
    );
    assert(own_ast != NULL);

    ar_list_t *own_arg_asts = ar_list__create();
    assert(own_arg_asts != NULL);
    assert(ar_list__add_last(own_arg_asts, ar_expression_ast__create_literal_string(ref_template)) == true);
    if (ref_base_path != NULL) {
        assert(strcmp(ref_base_path, "memory.location") == 0);
        const char *path[] = {"location"};
        assert(ar_list__add_last(own_arg_asts, ar_expression_ast__create_memory_access("memory", path, 1)) == true);
    }
    assert(ar_instruction_ast__set_function_arg_asts(own_ast, own_arg_asts) == true);
    return own_ast;
}

static ar_instruction_ast_t* _create_complete_ast_with_base_ast(
    const char *ref_template,
    ar_expression_ast_t *own_base_ast,
    const char *ref_result_path
) {
    const char *args[2] = {ref_template, "<custom-base>"};
    ar_instruction_ast_t *own_ast = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__COMPLETE,
        "complete",
        args,
        2U,
        ref_result_path
    );
    assert(own_ast != NULL);

    ar_list_t *own_arg_asts = ar_list__create();
    assert(own_arg_asts != NULL);
    assert(ar_list__add_last(own_arg_asts, ar_expression_ast__create_literal_string(ref_template)) == true);
    assert(ar_list__add_last(own_arg_asts, own_base_ast) == true);
    assert(ar_instruction_ast__set_function_arg_asts(own_ast, own_arg_asts) == true);
    return own_ast;
}

static void test_complete_instruction_evaluator__create_destroy(void) {
    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("test_complete_instruction_evaluator__create_destroy");
    assert(own_fixture != NULL);

    ar_local_completion_t *own_runtime = ar_local_completion__create(ar_evaluator_fixture__get_log(own_fixture));
    assert(own_runtime != NULL);

    ar_complete_instruction_evaluator_t *own_evaluator = ar_complete_instruction_evaluator__create(
        ar_evaluator_fixture__get_log(own_fixture),
        ar_evaluator_fixture__get_expression_evaluator(own_fixture),
        own_runtime
    );
    assert(own_evaluator != NULL);

    ar_complete_instruction_evaluator__destroy(own_evaluator);
    ar_local_completion__destroy(own_runtime);
    ar_evaluator_fixture__destroy(own_fixture);
}

static void test_complete_instruction_evaluator__evaluate_top_level_success(void) {
    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("test_complete_instruction_evaluator__evaluate_top_level_success");
    assert(own_fixture != NULL);
    ar_local_completion_t *own_runtime = ar_local_completion__create(ar_evaluator_fixture__get_log(own_fixture));
    assert(own_runtime != NULL);
    ar_complete_instruction_evaluator_t *own_evaluator = ar_complete_instruction_evaluator__create(
        ar_evaluator_fixture__get_log(own_fixture),
        ar_evaluator_fixture__get_expression_evaluator(own_fixture),
        own_runtime
    );
    assert(own_evaluator != NULL);

    ar_instruction_ast_t *own_ast = _create_complete_ast(
        "The \"largest\" country in South America is {country}.",
        NULL,
        "memory.result"
    );
    ar_frame_t *ref_frame = ar_evaluator_fixture__create_frame(own_fixture);

    bool result = ar_complete_instruction_evaluator__evaluate(own_evaluator, ref_frame, own_ast);
    assert(result == true);

    ar_data_t *ref_result = ar_data__get_map_data(ar_evaluator_fixture__get_memory(own_fixture), "result");
    assert(ref_result != NULL);
    assert(ar_data__get_type(ref_result) == AR_DATA_TYPE__MAP);

    ar_data_t *ref_country = ar_data__get_map_data(ref_result, "country");
    assert(ref_country != NULL);
    assert(ar_data__get_type(ref_country) == AR_DATA_TYPE__STRING);
    assert(strcmp(ar_data__get_string(ref_country), "Brazil") == 0);

    char own_completed_text[160];
    snprintf(
        own_completed_text,
        sizeof(own_completed_text),
        "The \"largest\" country in South America is %s.",
        ar_data__get_string(ref_country)
    );
    assert(strcmp(own_completed_text, "The \"largest\" country in South America is Brazil.") == 0);

    ar_instruction_ast__destroy(own_ast);
    ar_complete_instruction_evaluator__destroy(own_evaluator);
    ar_local_completion__destroy(own_runtime);
    ar_evaluator_fixture__destroy(own_fixture);
}

static void test_complete_instruction_evaluator__evaluate_returns_new_map_with_existing_and_completed_values(void) {
    // Given a complete evaluator and an input values map with one matching placeholder
    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("test_complete_instruction_evaluator__evaluate_returns_new_map_with_existing_and_completed_values");
    assert(own_fixture != NULL);
    ar_local_completion_t *own_runtime = ar_local_completion__create(ar_evaluator_fixture__get_log(own_fixture));
    assert(own_runtime != NULL);
    ar_complete_instruction_evaluator_t *own_evaluator = ar_complete_instruction_evaluator__create(
        ar_evaluator_fixture__get_log(own_fixture),
        ar_evaluator_fixture__get_expression_evaluator(own_fixture),
        own_runtime
    );
    assert(own_evaluator != NULL);

    ar_data_t *own_input = ar_data__create_map();
    assert(own_input != NULL);
    assert(ar_data__set_map_string(own_input, "country", "Brazil") == true);
    assert(ar_data__set_map_string(own_input, "description", "{literal}") == true);
    assert(ar_data__set_map_string(own_input, "extra", "kept") == true);
    ar_data_t *own_nested = ar_data__create_map();
    assert(own_nested != NULL);
    assert(ar_data__set_map_string(own_nested, "note", "nested kept") == true);
    assert(ar_data__set_map_data(own_input, "nested", own_nested) == true);
    assert(ar_data__set_map_data(ar_evaluator_fixture__get_memory(own_fixture), "input", own_input) == true);

    const char *path[] = {"input"};
    ar_instruction_ast_t *own_ast = _create_complete_ast_with_base_ast(
        "The {description} capital of {country} is {city} with {nested} metadata.",
        ar_expression_ast__create_memory_access("memory", path, 1),
        "memory.result"
    );
    ar_frame_t *ref_frame = ar_evaluator_fixture__create_frame(own_fixture);

    // When evaluating complete() with the input values map
    bool result = ar_complete_instruction_evaluator__evaluate(own_evaluator, ref_frame, own_ast);

    // Then evaluation should return a new map and leave the input map unchanged
    assert(result == true);

    ar_data_t *ref_input = ar_data__get_map_data(ar_evaluator_fixture__get_memory(own_fixture), "input");
    assert(ref_input != NULL);
    assert(ar_data__get_map_data(ref_input, "city") == NULL);
    assert(strcmp(ar_data__get_map_string(ref_input, "country"), "Brazil") == 0);
    assert(strcmp(ar_data__get_map_string(ref_input, "description"), "{literal}") == 0);
    assert(strcmp(ar_data__get_map_string(ref_input, "extra"), "kept") == 0);
    ar_data_t *ref_input_nested = ar_data__get_map_data(ref_input, "nested");
    assert(ref_input_nested != NULL);
    assert(strcmp(ar_data__get_map_string(ref_input_nested, "note"), "nested kept") == 0);

    ar_data_t *ref_result = ar_data__get_map_data(ar_evaluator_fixture__get_memory(own_fixture), "result");
    assert(ref_result != NULL);
    assert(ar_data__get_type(ref_result) == AR_DATA_TYPE__MAP);
    assert(strcmp(ar_data__get_map_string(ref_result, "country"), "Brazil") == 0);
    assert(strcmp(ar_data__get_map_string(ref_result, "description"), "{literal}") == 0);
    assert(strcmp(ar_data__get_map_string(ref_result, "city"), "Brasilia") == 0);
    assert(strcmp(ar_data__get_map_string(ref_result, "extra"), "kept") == 0);
    ar_data_t *ref_result_nested = ar_data__get_map_data(ref_result, "nested");
    assert(ref_result_nested != NULL);
    assert(ref_result_nested != ref_input_nested);
    assert(strcmp(ar_data__get_map_string(ref_result_nested, "note"), "nested kept") == 0);

    // Cleanup
    ar_instruction_ast__destroy(own_ast);
    ar_complete_instruction_evaluator__destroy(own_evaluator);
    ar_local_completion__destroy(own_runtime);
    ar_evaluator_fixture__destroy(own_fixture);
}

static void test_complete_instruction_evaluator__evaluate_values_map_preserves_existing_values(void) {
    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("test_complete_instruction_evaluator__evaluate_values_map_preserves_existing_values");
    assert(own_fixture != NULL);
    ar_local_completion_t *own_runtime = ar_local_completion__create(ar_evaluator_fixture__get_log(own_fixture));
    assert(own_runtime != NULL);
    ar_complete_instruction_evaluator_t *own_evaluator = ar_complete_instruction_evaluator__create(
        ar_evaluator_fixture__get_log(own_fixture),
        ar_evaluator_fixture__get_expression_evaluator(own_fixture),
        own_runtime
    );
    assert(own_evaluator != NULL);

    ar_data_t *own_location = ar_data__create_map();
    assert(own_location != NULL);
    assert(ar_data__set_map_data(ar_evaluator_fixture__get_memory(own_fixture), "location", own_location) == true);
    assert(ar_data__set_map_string(ar_evaluator_fixture__get_memory(own_fixture), "location.city", "Old City") == true);

    ar_instruction_ast_t *own_ast = _create_complete_ast("The capital is {city}.", "memory.location", "memory.result");
    ar_frame_t *ref_frame = ar_evaluator_fixture__create_frame(own_fixture);

    bool result = ar_complete_instruction_evaluator__evaluate(own_evaluator, ref_frame, own_ast);
    assert(result == true);

    ar_data_t *ref_location = ar_data__get_map_data(ar_evaluator_fixture__get_memory(own_fixture), "location");
    assert(ref_location != NULL);
    assert(ar_data__get_type(ref_location) == AR_DATA_TYPE__MAP);
    assert(strcmp(ar_data__get_map_string(ref_location, "city"), "Old City") == 0);

    ar_data_t *ref_result = ar_data__get_map_data(ar_evaluator_fixture__get_memory(own_fixture), "result");
    assert(ref_result != NULL);
    assert(ar_data__get_type(ref_result) == AR_DATA_TYPE__MAP);
    assert(strcmp(ar_data__get_map_string(ref_result, "city"), "Old City") == 0);

    ar_instruction_ast__destroy(own_ast);
    ar_complete_instruction_evaluator__destroy(own_evaluator);
    ar_local_completion__destroy(own_runtime);
    ar_evaluator_fixture__destroy(own_fixture);
}

static void test_complete_instruction_evaluator__failure_stores_false_and_preserves_existing_values(void) {
    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("test_complete_instruction_evaluator__failure_stores_false_and_preserves_existing_values");
    assert(own_fixture != NULL);
    ar_local_completion_t *own_runtime = ar_local_completion__create(ar_evaluator_fixture__get_log(own_fixture));
    assert(own_runtime != NULL);
    ar_complete_instruction_evaluator_t *own_evaluator = ar_complete_instruction_evaluator__create(
        ar_evaluator_fixture__get_log(own_fixture),
        ar_evaluator_fixture__get_expression_evaluator(own_fixture),
        own_runtime
    );
    assert(own_evaluator != NULL);

    assert(ar_data__set_map_string(ar_evaluator_fixture__get_memory(own_fixture), "country", "Peru") == true);

    ar_instruction_ast_t *own_ast = _create_complete_ast(
        "The country is {country} and the broken value is {bracey}.",
        NULL,
        "memory.result"
    );
    ar_frame_t *ref_frame = ar_evaluator_fixture__create_frame(own_fixture);

    bool result = ar_complete_instruction_evaluator__evaluate(own_evaluator, ref_frame, own_ast);
    assert(result == true);
    ar_data_t *ref_result = ar_data__get_map_data(ar_evaluator_fixture__get_memory(own_fixture), "result");
    assert(ref_result != NULL);
    assert(ar_data__get_type(ref_result) == AR_DATA_TYPE__MAP);
    assert(ar_data__get_map_data(ref_result, "country") == NULL);
    assert(strcmp(ar_data__get_map_string(ar_evaluator_fixture__get_memory(own_fixture), "country"), "Peru") == 0);
    assert(ar_data__get_map_data(ar_evaluator_fixture__get_memory(own_fixture), "bracey") == NULL);
    assert(ar_log__get_last_error_message(ar_evaluator_fixture__get_log(own_fixture)) != NULL);

    ar_instruction_ast__destroy(own_ast);
    ar_complete_instruction_evaluator__destroy(own_evaluator);
    ar_local_completion__destroy(own_runtime);
    ar_evaluator_fixture__destroy(own_fixture);
}

static void test_complete_instruction_evaluator__whitespace_rejection_keeps_memory_clean(void) {
    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("test_complete_instruction_evaluator__whitespace_rejection_keeps_memory_clean");
    assert(own_fixture != NULL);
    ar_local_completion_t *own_runtime = ar_local_completion__create(ar_evaluator_fixture__get_log(own_fixture));
    assert(own_runtime != NULL);
    ar_complete_instruction_evaluator_t *own_evaluator = ar_complete_instruction_evaluator__create(
        ar_evaluator_fixture__get_log(own_fixture),
        ar_evaluator_fixture__get_expression_evaluator(own_fixture),
        own_runtime
    );
    assert(own_evaluator != NULL);

    ar_instruction_ast_t *own_ast = _create_complete_ast("Value is {spacey}.", NULL, "memory.result");
    ar_frame_t *ref_frame = ar_evaluator_fixture__create_frame(own_fixture);

    bool result = ar_complete_instruction_evaluator__evaluate(own_evaluator, ref_frame, own_ast);
    assert(result == true);
    ar_data_t *ref_result = ar_data__get_map_data(ar_evaluator_fixture__get_memory(own_fixture), "result");
    assert(ref_result != NULL);
    assert(ar_data__get_type(ref_result) == AR_DATA_TYPE__MAP);
    assert(ar_data__get_map_data(ref_result, "spacey") == NULL);
    assert(ar_log__get_last_error_message(ar_evaluator_fixture__get_log(own_fixture)) != NULL);

    ar_instruction_ast__destroy(own_ast);
    ar_complete_instruction_evaluator__destroy(own_evaluator);
    ar_local_completion__destroy(own_runtime);
    ar_evaluator_fixture__destroy(own_fixture);
}

static void test_complete_instruction_evaluator__invalid_template_fast_failure_does_not_initialize_runtime(void) {
    unsetenv("AGERUN_COMPLETE_RUNNER");
    assert(setenv("AGERUN_COMPLETE_RUNNER", "./definitely-missing-llama-cli", 1) == 0);

    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("test_complete_instruction_evaluator__invalid_template_fast_failure_does_not_initialize_runtime");
    assert(own_fixture != NULL);
    ar_local_completion_t *own_runtime = ar_local_completion__create(ar_evaluator_fixture__get_log(own_fixture));
    assert(own_runtime != NULL);
    ar_complete_instruction_evaluator_t *own_evaluator = ar_complete_instruction_evaluator__create(
        ar_evaluator_fixture__get_log(own_fixture),
        ar_evaluator_fixture__get_expression_evaluator(own_fixture),
        own_runtime
    );
    assert(own_evaluator != NULL);

    ar_instruction_ast_t *own_ast = _create_complete_ast("No placeholders here.", NULL, "memory.result");
    ar_frame_t *ref_frame = ar_evaluator_fixture__create_frame(own_fixture);

    bool result = ar_complete_instruction_evaluator__evaluate(own_evaluator, ref_frame, own_ast);
    assert(result == true);
    ar_data_t *ref_result = ar_data__get_map_data(ar_evaluator_fixture__get_memory(own_fixture), "result");
    assert(ref_result != NULL);
    assert(ar_data__get_type(ref_result) == AR_DATA_TYPE__MAP);
    assert(ar_data__get_map_data(ref_result, "country") == NULL);
    assert(ar_local_completion__is_ready(own_runtime) == false);

    ar_instruction_ast__destroy(own_ast);
    ar_complete_instruction_evaluator__destroy(own_evaluator);
    ar_local_completion__destroy(own_runtime);
    ar_evaluator_fixture__destroy(own_fixture);
    assert(setenv("AGERUN_COMPLETE_RUNNER", g_runner_path, 1) == 0);
    assert(setenv("AGERUN_COMPLETE_MODEL", g_model_path, 1) == 0);
}

static void test_complete_instruction_evaluator__non_map_second_argument_returns_empty_map(void) {
    unsetenv("AGERUN_COMPLETE_RUNNER");
    assert(setenv("AGERUN_COMPLETE_RUNNER", "./definitely-missing-llama-cli", 1) == 0);

    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("test_complete_instruction_evaluator__non_map_second_argument_returns_empty_map");
    assert(own_fixture != NULL);
    ar_local_completion_t *own_runtime = ar_local_completion__create(ar_evaluator_fixture__get_log(own_fixture));
    assert(own_runtime != NULL);
    ar_complete_instruction_evaluator_t *own_evaluator = ar_complete_instruction_evaluator__create(
        ar_evaluator_fixture__get_log(own_fixture),
        ar_evaluator_fixture__get_expression_evaluator(own_fixture),
        own_runtime
    );
    assert(own_evaluator != NULL);

    assert(ar_data__set_map_string(ar_evaluator_fixture__get_memory(own_fixture), "city", "Old City") == true);
    ar_instruction_ast_t *own_ast = _create_complete_ast_with_base_ast(
        "The capital is {city}.",
        ar_expression_ast__create_literal_string("not-a-memory-path"),
        "memory.result"
    );
    ar_frame_t *ref_frame = ar_evaluator_fixture__create_frame(own_fixture);

    bool result = ar_complete_instruction_evaluator__evaluate(own_evaluator, ref_frame, own_ast);
    assert(result == true);
    ar_data_t *ref_result = ar_data__get_map_data(ar_evaluator_fixture__get_memory(own_fixture), "result");
    assert(ref_result != NULL);
    assert(ar_data__get_type(ref_result) == AR_DATA_TYPE__MAP);
    assert(strcmp(ar_data__get_map_string(ar_evaluator_fixture__get_memory(own_fixture), "city"), "Old City") == 0);
    assert(ar_local_completion__is_ready(own_runtime) == false);
    assert(strstr(ar_log__get_last_error_message(ar_evaluator_fixture__get_log(own_fixture)), "second argument must evaluate to a map") != NULL);

    ar_instruction_ast__destroy(own_ast);
    ar_complete_instruction_evaluator__destroy(own_evaluator);
    ar_local_completion__destroy(own_runtime);
    ar_evaluator_fixture__destroy(own_fixture);
    assert(setenv("AGERUN_COMPLETE_RUNNER", g_runner_path, 1) == 0);
    assert(setenv("AGERUN_COMPLETE_MODEL", g_model_path, 1) == 0);
}

static void test_complete_instruction_evaluator__missing_placeholder_response_keeps_memory_clean(void) {
    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("test_complete_instruction_evaluator__missing_placeholder_response_keeps_memory_clean");
    assert(own_fixture != NULL);
    ar_local_completion_t *own_runtime = ar_local_completion__create(ar_evaluator_fixture__get_log(own_fixture));
    assert(own_runtime != NULL);
    ar_complete_instruction_evaluator_t *own_evaluator = ar_complete_instruction_evaluator__create(
        ar_evaluator_fixture__get_log(own_fixture),
        ar_evaluator_fixture__get_expression_evaluator(own_fixture),
        own_runtime
    );
    assert(own_evaluator != NULL);

    assert(ar_data__set_map_string(ar_evaluator_fixture__get_memory(own_fixture), "country", "Peru") == true);
    ar_instruction_ast_t *own_ast = _create_complete_ast(
        "The largest country in South America is {country}. The capital is {language}.",
        NULL,
        "memory.result"
    );
    ar_frame_t *ref_frame = ar_evaluator_fixture__create_frame(own_fixture);

    bool result = ar_complete_instruction_evaluator__evaluate(own_evaluator, ref_frame, own_ast);
    assert(result == true);
    ar_data_t *ref_result = ar_data__get_map_data(ar_evaluator_fixture__get_memory(own_fixture), "result");
    assert(ref_result != NULL);
    assert(ar_data__get_type(ref_result) == AR_DATA_TYPE__MAP);
    assert(ar_data__get_map_data(ref_result, "country") == NULL);
    assert(strcmp(ar_data__get_map_string(ar_evaluator_fixture__get_memory(own_fixture), "country"), "Peru") == 0);
    assert(ar_data__get_map_data(ar_evaluator_fixture__get_memory(own_fixture), "language") == NULL);
    assert(strstr(ar_log__get_last_error_message(ar_evaluator_fixture__get_log(own_fixture)), "recovery_hint=") != NULL);

    ar_instruction_ast__destroy(own_ast);
    ar_complete_instruction_evaluator__destroy(own_evaluator);
    ar_local_completion__destroy(own_runtime);
    ar_evaluator_fixture__destroy(own_fixture);
}

static void test_complete_instruction_evaluator__performance_cold_fixture(void) {
    const char *ref_model_path = "../../models/phi-3-mini-q4.gguf";
    const int64_t cold_start_limit_ms = 30000;
    const char *ref_index_text = getenv("AGERUN_COMPLETE_EVALUATOR_FIXTURE_INDEX");
    assert(ref_index_text != NULL);
    long own_index = strtol(ref_index_text, NULL, 10);
    assert(own_index >= 0L);
    assert((size_t)own_index < sizeof(g_complete_perf_fixtures) / sizeof(g_complete_perf_fixtures[0]));

    const ar_complete_perf_fixture_t *ref_fixture = &g_complete_perf_fixtures[(size_t)own_index];
    _assert_short_template_fixture(ref_fixture);

    assert(access(ref_model_path, F_OK) == 0);
    unsetenv("AGERUN_COMPLETE_RUNNER");
    assert(setenv("AGERUN_COMPLETE_MODEL", ref_model_path, 1) == 0);

    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("test_complete_instruction_evaluator__performance_cold_fixture");
    assert(own_fixture != NULL);
    ar_local_completion_t *own_runtime = ar_local_completion__create(ar_evaluator_fixture__get_log(own_fixture));
    assert(own_runtime != NULL);
    ar_complete_instruction_evaluator_t *own_evaluator = ar_complete_instruction_evaluator__create(
        ar_evaluator_fixture__get_log(own_fixture),
        ar_evaluator_fixture__get_expression_evaluator(own_fixture),
        own_runtime
    );
    assert(own_evaluator != NULL);

    const char *ref_base_path = (((size_t)own_index) % 2U) == 0U ? NULL : "memory.location";
    if (ref_base_path != NULL) {
        ar_data_t *own_location = ar_data__create_map();
        assert(own_location != NULL);
        assert(ar_data__set_map_data(ar_evaluator_fixture__get_memory(own_fixture), "location", own_location) == true);
    }
    ar_instruction_ast_t *own_ast = _create_complete_ast(ref_fixture->ref_template, ref_base_path, "memory.ok");
    ar_frame_t *ref_frame = ar_evaluator_fixture__create_frame(own_fixture);
    struct timespec own_start = {0};
    assert(clock_gettime(CLOCK_MONOTONIC, &own_start) == 0);
    bool result = ar_complete_instruction_evaluator__evaluate(own_evaluator, ref_frame, own_ast);
    int64_t elapsed_ms = _elapsed_ms_since(&own_start);
    bool success = result == true && _complete_result_has_fixture_values(
        ar_evaluator_fixture__get_memory(own_fixture),
        ref_fixture
    );

    printf("Cold evaluator fixture %ld elapsed=%" PRId64 " ms success=%s under_limit=%s template=%s\n",
           own_index + 1L,
           elapsed_ms,
           success ? "yes" : "no",
           elapsed_ms <= cold_start_limit_ms ? "yes" : "no",
           ref_fixture->ref_template);

    assert(success == true);
    assert(elapsed_ms <= cold_start_limit_ms);

    ar_instruction_ast__destroy(own_ast);
    ar_complete_instruction_evaluator__destroy(own_evaluator);
    ar_local_completion__destroy(own_runtime);
    ar_evaluator_fixture__destroy(own_fixture);
    unsetenv("AGERUN_COMPLETE_MODEL");
}

static void test_complete_instruction_evaluator__performance_warm_fixture_set(void) {
    const char *ref_model_path = "../../models/phi-3-mini-q4.gguf";
    const int64_t warm_run_limit_ms = 15000;
    size_t warm_success_count = 0U;
    size_t warm_under_limit_count = 0U;
    int64_t warm_max_ms = 0;
    int64_t warm_total_ms = 0;

    assert(access(ref_model_path, F_OK) == 0);
    unsetenv("AGERUN_COMPLETE_RUNNER");
    assert(setenv("AGERUN_COMPLETE_MODEL", ref_model_path, 1) == 0);

    ar_evaluator_fixture_t *own_warm_fixture = ar_evaluator_fixture__create("test_complete_instruction_evaluator__performance_warm_fixture_set");
    assert(own_warm_fixture != NULL);
    ar_local_completion_t *own_warm_runtime = ar_local_completion__create(ar_evaluator_fixture__get_log(own_warm_fixture));
    assert(own_warm_runtime != NULL);
    ar_complete_instruction_evaluator_t *own_warm_evaluator = ar_complete_instruction_evaluator__create(
        ar_evaluator_fixture__get_log(own_warm_fixture),
        ar_evaluator_fixture__get_expression_evaluator(own_warm_fixture),
        own_warm_runtime
    );
    assert(own_warm_evaluator != NULL);

    {
        ar_data_t *own_location = ar_data__create_map();
        assert(own_location != NULL);
        assert(ar_data__set_map_data(ar_evaluator_fixture__get_memory(own_warm_fixture), "location", own_location) == true);

        ar_instruction_ast_t *own_warmup_ast = _create_complete_ast(
            g_complete_perf_fixtures[0].ref_template,
            NULL,
            "memory.ok"
        );
        ar_frame_t *ref_warmup_frame = ar_evaluator_fixture__create_frame(own_warm_fixture);
        bool warmup_result = ar_complete_instruction_evaluator__evaluate(own_warm_evaluator, ref_warmup_frame, own_warmup_ast);
        assert(warmup_result == true);
        assert(_complete_result_has_fixture_values(
            ar_evaluator_fixture__get_memory(own_warm_fixture),
            &g_complete_perf_fixtures[0]
        ) == true);
        ar_instruction_ast__destroy(own_warmup_ast);
    }

    for (size_t index = 0U; index < sizeof(g_complete_perf_fixtures) / sizeof(g_complete_perf_fixtures[0]); index += 1U) {
        const ar_complete_perf_fixture_t *ref_fixture = &g_complete_perf_fixtures[index];
        const char *ref_base_path = (index % 2U) == 0U ? NULL : "memory.location";
        ar_instruction_ast_t *own_ast = _create_complete_ast(ref_fixture->ref_template, ref_base_path, "memory.ok");
        ar_frame_t *ref_frame = ar_evaluator_fixture__create_frame(own_warm_fixture);
        struct timespec own_start = {0};
        assert(clock_gettime(CLOCK_MONOTONIC, &own_start) == 0);
        bool result = ar_complete_instruction_evaluator__evaluate(own_warm_evaluator, ref_frame, own_ast);
        int64_t elapsed_ms = _elapsed_ms_since(&own_start);
        bool success = result == true && _complete_result_has_fixture_values(
            ar_evaluator_fixture__get_memory(own_warm_fixture),
            ref_fixture
        );

        warm_total_ms += elapsed_ms;
        if (elapsed_ms > warm_max_ms) {
            warm_max_ms = elapsed_ms;
        }
        if (success) {
            warm_success_count += 1U;
        }
        if (elapsed_ms <= warm_run_limit_ms) {
            warm_under_limit_count += 1U;
        }

        printf("Warm evaluator fixture %zu elapsed=%" PRId64 " ms success=%s template=%s\n",
               index + 1U,
               elapsed_ms,
               success ? "yes" : "no",
               ref_fixture->ref_template);

        ar_instruction_ast__destroy(own_ast);
    }

    printf("Warm evaluator summary: fixtures=%zu success=%zu under_%" PRId64 "ms=%zu avg=%" PRId64 " ms max=%" PRId64 " ms\n",
           sizeof(g_complete_perf_fixtures) / sizeof(g_complete_perf_fixtures[0]),
           warm_success_count,
           warm_run_limit_ms,
           warm_under_limit_count,
           warm_total_ms / (int64_t)(sizeof(g_complete_perf_fixtures) / sizeof(g_complete_perf_fixtures[0])),
           warm_max_ms);

    assert(warm_success_count >= 18U);
    assert(warm_under_limit_count >= 18U);

    ar_complete_instruction_evaluator__destroy(own_warm_evaluator);
    ar_local_completion__destroy(own_warm_runtime);
    ar_evaluator_fixture__destroy(own_warm_fixture);
    unsetenv("AGERUN_COMPLETE_MODEL");
}

static void test_complete_instruction_evaluator__repeated_placeholders_use_one_consistent_value(void) {
    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("test_complete_instruction_evaluator__repeated_placeholders_use_one_consistent_value");
    assert(own_fixture != NULL);
    ar_local_completion_t *own_runtime = ar_local_completion__create(ar_evaluator_fixture__get_log(own_fixture));
    assert(own_runtime != NULL);
    ar_complete_instruction_evaluator_t *own_evaluator = ar_complete_instruction_evaluator__create(
        ar_evaluator_fixture__get_log(own_fixture),
        ar_evaluator_fixture__get_expression_evaluator(own_fixture),
        own_runtime
    );
    assert(own_evaluator != NULL);

    ar_instruction_ast_t *own_ast = _create_complete_ast(
        "Repeat {country} and then {country} again.",
        NULL,
        "memory.result"
    );
    ar_frame_t *ref_frame = ar_evaluator_fixture__create_frame(own_fixture);

    bool result = ar_complete_instruction_evaluator__evaluate(own_evaluator, ref_frame, own_ast);
    assert(result == true);
    ar_data_t *ref_result = ar_data__get_map_data(ar_evaluator_fixture__get_memory(own_fixture), "result");
    assert(ref_result != NULL);
    assert(ar_data__get_type(ref_result) == AR_DATA_TYPE__MAP);

    const char *ref_country = ar_data__get_map_string(ref_result, "country");
    assert(ref_country != NULL);
    assert(strcmp(ref_country, "Brazil") == 0);

    char own_completed_text[160];
    snprintf(
        own_completed_text,
        sizeof(own_completed_text),
        "Repeat %s and then %s again.",
        ref_country,
        ref_country
    );
    assert(strcmp(own_completed_text, "Repeat Brazil and then Brazil again.") == 0);

    ar_instruction_ast__destroy(own_ast);
    ar_complete_instruction_evaluator__destroy(own_evaluator);
    ar_local_completion__destroy(own_runtime);
    ar_evaluator_fixture__destroy(own_fixture);
}

int main(void) {
    const char *ref_subtest_name = getenv("AGERUN_COMPLETE_EVALUATOR_SUBTEST");

    printf("Running complete instruction evaluator tests...\n");
    if (ref_subtest_name != NULL) {
        if (strcmp(ref_subtest_name, "performance_cold_fixture") == 0) {
            test_complete_instruction_evaluator__performance_cold_fixture();
        } else if (strcmp(ref_subtest_name, "performance_warm_fixture_set") == 0) {
            test_complete_instruction_evaluator__performance_warm_fixture_set();
        } else {
            assert(false && "Unknown AGERUN_COMPLETE_EVALUATOR_SUBTEST value");
        }
        printf("Complete instruction evaluator subtest passed: %s\n", ref_subtest_name);
        return 0;
    }

    _setup_fake_runner();
    test_complete_instruction_evaluator__create_destroy();
    test_complete_instruction_evaluator__evaluate_top_level_success();
    test_complete_instruction_evaluator__evaluate_returns_new_map_with_existing_and_completed_values();
    test_complete_instruction_evaluator__evaluate_values_map_preserves_existing_values();
    test_complete_instruction_evaluator__failure_stores_false_and_preserves_existing_values();
    test_complete_instruction_evaluator__whitespace_rejection_keeps_memory_clean();
    test_complete_instruction_evaluator__invalid_template_fast_failure_does_not_initialize_runtime();
    test_complete_instruction_evaluator__non_map_second_argument_returns_empty_map();
    test_complete_instruction_evaluator__missing_placeholder_response_keeps_memory_clean();
    test_complete_instruction_evaluator__repeated_placeholders_use_one_consistent_value();
    printf("All complete instruction evaluator tests passed!\n");
    _cleanup_fake_runner();
    return 0;
}
