#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
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
        const char *path[] = {"location"};
        assert(ar_list__add_last(own_arg_asts, ar_expression_ast__create_memory_access("memory", path, 1)) == true);
    }
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
        "memory.ok"
    );
    ar_frame_t *ref_frame = ar_evaluator_fixture__create_frame(own_fixture);

    bool result = ar_complete_instruction_evaluator__evaluate(own_evaluator, ref_frame, own_ast);
    assert(result == true);

    ar_data_t *ref_country = ar_data__get_map_data(ar_evaluator_fixture__get_memory(own_fixture), "country");
    assert(ref_country != NULL);
    assert(ar_data__get_type(ref_country) == AR_DATA_TYPE__STRING);
    assert(strcmp(ar_data__get_string(ref_country), "Brazil") == 0);

    ar_data_t *ref_ok = ar_data__get_map_data(ar_evaluator_fixture__get_memory(own_fixture), "ok");
    assert(ref_ok != NULL);
    assert(ar_data__get_type(ref_ok) == AR_DATA_TYPE__INTEGER);
    assert(ar_data__get_integer(ref_ok) == 1);

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

static void test_complete_instruction_evaluator__evaluate_nested_success_overwrites_existing_values(void) {
    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("test_complete_instruction_evaluator__evaluate_nested_success_overwrites_existing_values");
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

    ar_instruction_ast_t *own_ast = _create_complete_ast("The capital is {city}.", "memory.location", "memory.ok");
    ar_frame_t *ref_frame = ar_evaluator_fixture__create_frame(own_fixture);

    bool result = ar_complete_instruction_evaluator__evaluate(own_evaluator, ref_frame, own_ast);
    assert(result == true);

    ar_data_t *ref_location = ar_data__get_map_data(ar_evaluator_fixture__get_memory(own_fixture), "location");
    assert(ref_location != NULL);
    assert(ar_data__get_type(ref_location) == AR_DATA_TYPE__MAP);
    assert(strcmp(ar_data__get_map_string(ref_location, "city"), "Brasilia") == 0);
    assert(ar_data__get_map_integer(ar_evaluator_fixture__get_memory(own_fixture), "ok") == 1);

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
        "memory.ok"
    );
    ar_frame_t *ref_frame = ar_evaluator_fixture__create_frame(own_fixture);

    bool result = ar_complete_instruction_evaluator__evaluate(own_evaluator, ref_frame, own_ast);
    assert(result == true);
    assert(ar_data__get_map_integer(ar_evaluator_fixture__get_memory(own_fixture), "ok") == 0);
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

    ar_instruction_ast_t *own_ast = _create_complete_ast("Value is {spacey}.", NULL, "memory.ok");
    ar_frame_t *ref_frame = ar_evaluator_fixture__create_frame(own_fixture);

    bool result = ar_complete_instruction_evaluator__evaluate(own_evaluator, ref_frame, own_ast);
    assert(result == true);
    assert(ar_data__get_map_integer(ar_evaluator_fixture__get_memory(own_fixture), "ok") == 0);
    assert(ar_data__get_map_data(ar_evaluator_fixture__get_memory(own_fixture), "spacey") == NULL);
    assert(ar_log__get_last_error_message(ar_evaluator_fixture__get_log(own_fixture)) != NULL);

    ar_instruction_ast__destroy(own_ast);
    ar_complete_instruction_evaluator__destroy(own_evaluator);
    ar_local_completion__destroy(own_runtime);
    ar_evaluator_fixture__destroy(own_fixture);
}

int main(void) {
    _setup_fake_runner();
    printf("Running complete instruction evaluator tests...\n");
    test_complete_instruction_evaluator__create_destroy();
    test_complete_instruction_evaluator__evaluate_top_level_success();
    test_complete_instruction_evaluator__evaluate_nested_success_overwrites_existing_values();
    test_complete_instruction_evaluator__failure_stores_false_and_preserves_existing_values();
    test_complete_instruction_evaluator__whitespace_rejection_keeps_memory_clean();
    printf("All complete instruction evaluator tests passed!\n");
    _cleanup_fake_runner();
    return 0;
}
