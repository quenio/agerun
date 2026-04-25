#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <inttypes.h>
#include <unistd.h>
#include <sys/stat.h>
#include "ar_instruction_evaluator.h"
#include "ar_instruction_ast.h"
#include "ar_expression_ast.h"
#include "ar_data.h"
#include "ar_methodology.h"
#include "ar_agency.h"
#include "ar_delegation.h"
#include "ar_delegate.h"
#include "ar_system.h"
#include "ar_log.h"
#include "ar_event.h"
#include "ar_frame.h"
#include "ar_list.h"

static char g_complete_runner_path[128] = {0};
static char g_complete_model_path[128] = {0};

static void setup_fake_complete_runner(void) {
    snprintf(g_complete_runner_path, sizeof(g_complete_runner_path), "./fake-llama-cli-instruction-%ld.sh", (long)getpid());
    snprintf(g_complete_model_path, sizeof(g_complete_model_path), "./fake-model-instruction-%ld.gguf", (long)getpid());

    FILE *own_model = fopen(g_complete_model_path, "w");
    assert(own_model != NULL);
    fputs("fake-model", own_model);
    fclose(own_model);

    FILE *own_runner = fopen(g_complete_runner_path, "w");
    assert(own_runner != NULL);
    fputs("#!/bin/sh\n", own_runner);
    fputs("printf 'country=Brazil\\ncity=Brasilia\\ncontinent=South America\\nbracey={invalid}\\nspacey= value \\nempty=\\n'\n", own_runner);
    fclose(own_runner);
    assert(chmod(g_complete_runner_path, 0700) == 0);

    assert(setenv("AGERUN_COMPLETE_RUNNER", g_complete_runner_path, 1) == 0);
    assert(setenv("AGERUN_COMPLETE_MODEL", g_complete_model_path, 1) == 0);
}

static void cleanup_fake_complete_runner(void) {
    unsetenv("AGERUN_COMPLETE_RUNNER");
    unsetenv("AGERUN_COMPLETE_MODEL");
    if (g_complete_runner_path[0] != '\0') {
        remove(g_complete_runner_path);
    }
    if (g_complete_model_path[0] != '\0') {
        remove(g_complete_model_path);
    }
}

static void test_instruction_evaluator__create_destroy(void) {
    // Given a log and system for agency
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    
    ar_system_t *sys = ar_system__create();
    assert(sys != NULL);
    ar_agency_t *agency = ar_system__get_agency(sys);
    ar_delegation_t *delegation = ar_system__get_delegation(sys);

    // When creating an instruction evaluator with log, agency, and delegation
    ar_instruction_evaluator_t *evaluator = ar_instruction_evaluator__create(log, agency, delegation);
    
    // Then it should be created successfully
    assert(evaluator != NULL);
    
    // When destroying the evaluator
    ar_instruction_evaluator__destroy(evaluator);
    
    // Then cleanup
    ar_system__destroy(sys);
    ar_log__destroy(log);
}

static void test_instruction_evaluator__create_with_null_context(void) {
    // Given a log and system
    ar_log_t *log = ar_log__create();
    assert(log != NULL);

    ar_system_t *sys = ar_system__create();
    assert(sys != NULL);
    ar_agency_t *agency = ar_system__get_agency(sys);
    ar_delegation_t *delegation = ar_system__get_delegation(sys);

    // When creating an instruction evaluator
    ar_instruction_evaluator_t *evaluator = ar_instruction_evaluator__create(log, agency, delegation);
    
    // Then it should be created successfully
    assert(evaluator != NULL);
    
    // Cleanup
    ar_instruction_evaluator__destroy(evaluator);
    ar_system__destroy(sys);
    ar_log__destroy(log);
}

static void test_instruction_evaluator__destroy_null(void) {
    // When destroying a NULL evaluator
    ar_instruction_evaluator__destroy(NULL);
    
    // Then it should handle it gracefully (no crash)
    // If we reach here, the test passed
}


static void test_instruction_evaluator__create_with_null_memory(void) {
    // Given an expression evaluator created with dummy memory
    ar_data_t *dummy_memory = ar_data__create_map();
    
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    
    ar_system_t *sys = ar_system__create();
    assert(sys != NULL);
    ar_agency_t *agency = ar_system__get_agency(sys);
    ar_delegation_t *delegation = ar_system__get_delegation(sys);
    
    // When creating an instruction evaluator (memory comes from frame now)
    ar_instruction_evaluator_t *evaluator = ar_instruction_evaluator__create(log, agency, delegation);
    
    // Then it should succeed (memory is no longer required at creation)
    assert(evaluator != NULL);
    
    // Cleanup
    ar_instruction_evaluator__destroy(evaluator);
    ar_system__destroy(sys);
    ar_data__destroy(dummy_memory);
    ar_log__destroy(log);
}

static void test_instruction_evaluator__stores_evaluator_instances_internally(void) {
    // Given an instruction evaluator
    ar_data_t *memory = ar_data__create_map();
    assert(memory != NULL);
    
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    
    ar_system_t *sys = ar_system__create();
    assert(sys != NULL);
    ar_agency_t *agency = ar_system__get_agency(sys);
    ar_delegation_t *delegation = ar_system__get_delegation(sys);
    
    // When creating an instruction evaluator
    ar_instruction_evaluator_t *evaluator = ar_instruction_evaluator__create(log, agency, delegation);
    assert(evaluator != NULL);
    
    // Then it should work with all instruction types through the unified interface
    // (internal evaluator instances are created but not exposed)
    
    // Test assignment instruction
    ar_instruction_ast_t *assignment_ast = ar_instruction_ast__create_assignment("memory.x", "42");
    ar_expression_ast_t *expr_ast = ar_expression_ast__create_literal_int(42);
    ar_instruction_ast__set_assignment_expression_ast(assignment_ast, expr_ast);
    
    // Create a frame for evaluation
    ar_data_t *context = ar_data__create_map();
    ar_data_t *message = ar_data__create_string("");
    ar_frame_t *frame = ar_frame__create(memory, context, message);
    assert(frame != NULL);
    
    bool result = ar_instruction_evaluator__evaluate(evaluator, frame, assignment_ast);
    assert(result == true);
    assert(ar_data__get_map_integer(memory, "x") == 42);
    
    ar_instruction_ast__destroy(assignment_ast);
    
    // Cleanup frame
    ar_frame__destroy(frame);
    ar_data__destroy(context);
    ar_data__destroy(message);
    
    // Cleanup
    ar_instruction_evaluator__destroy(evaluator);
    ar_system__destroy(sys);
    ar_data__destroy(memory);
    ar_log__destroy(log);
}



static void test_instruction_evaluator__unified_evaluate_all_types(void) {
    // Given an evaluator with memory, context, and message
    ar_data_t *memory = ar_data__create_map();
    assert(memory != NULL);
    
    ar_data_t *context = ar_data__create_map();
    assert(context != NULL);
    ar_data__set_map_integer(context, "test_context", 123);
    
    ar_data_t *message = ar_data__create_string("test message");
    assert(message != NULL);
    
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    
    ar_system_t *sys = ar_system__create();
    assert(sys != NULL);
    ar_agency_t *agency = ar_system__get_agency(sys);
    ar_delegation_t *delegation = ar_system__get_delegation(sys);
    
    ar_instruction_evaluator_t *evaluator = ar_instruction_evaluator__create(log, agency, delegation);
    assert(evaluator != NULL);
    
    // Test 1: Send instruction
    {
        const char *args[] = {"0", "\"hello\""};
        ar_instruction_ast_t *ast = ar_instruction_ast__create_function_call(
            AR_INSTRUCTION_AST_TYPE__SEND, "send", args, 2, NULL
        );
        assert(ast != NULL);
        
        // Create argument ASTs
        ar_list_t *arg_asts = ar_list__create();
        ar_expression_ast_t *target_ast = ar_expression_ast__create_literal_int(0);
        ar_expression_ast_t *msg_ast = ar_expression_ast__create_literal_string("hello");
        ar_list__add_last(arg_asts, target_ast);
        ar_list__add_last(arg_asts, msg_ast);
        ar_instruction_ast__set_function_arg_asts(ast, arg_asts);
        
        // Create a frame for evaluation
        ar_data_t *ctx = ar_data__create_map();
        ar_data_t *msg = ar_data__create_string("");
        ar_frame_t *fr = ar_frame__create(memory, ctx, msg);
        
        bool result = ar_instruction_evaluator__evaluate(evaluator, fr, ast);
        
        // Clean up frame
        ar_frame__destroy(fr);
        ar_data__destroy(ctx);
        ar_data__destroy(msg);
        assert(result == true);
        
        ar_instruction_ast__destroy(ast);
    }
    
    // Test 2: If instruction with result assignment
    {
        const char *args[] = {"1", "\"yes\"", "\"no\""};
        ar_instruction_ast_t *ast = ar_instruction_ast__create_function_call(
            AR_INSTRUCTION_AST_TYPE__IF, "if", args, 3, "memory.result"
        );
        assert(ast != NULL);
        
        // Create argument ASTs: if(1, "yes", "no")
        ar_list_t *arg_asts = ar_list__create();
        ar_expression_ast_t *cond_ast = ar_expression_ast__create_literal_int(1);
        ar_expression_ast_t *true_ast = ar_expression_ast__create_literal_string("yes");
        ar_expression_ast_t *false_ast = ar_expression_ast__create_literal_string("no");
        ar_list__add_last(arg_asts, cond_ast);
        ar_list__add_last(arg_asts, true_ast);
        ar_list__add_last(arg_asts, false_ast);
        ar_instruction_ast__set_function_arg_asts(ast, arg_asts);
        
        // Create a frame for evaluation
        ar_data_t *ctx = ar_data__create_map();
        ar_data_t *msg = ar_data__create_string("");
        ar_frame_t *fr = ar_frame__create(memory, ctx, msg);
        
        bool result = ar_instruction_evaluator__evaluate(evaluator, fr, ast);
        
        // Clean up frame
        ar_frame__destroy(fr);
        ar_data__destroy(ctx);
        ar_data__destroy(msg);
        assert(result == true);
        
        // Verify result was stored
        ar_data_t *value = ar_data__get_map_data(memory, "result");
        assert(value != NULL);
        assert(ar_data__get_type(value) == AR_DATA_TYPE__STRING);
        assert(strcmp(ar_data__get_string(value), "yes") == 0);
        
        ar_instruction_ast__destroy(ast);
    }
    
    // Test 3: Parse instruction
    {
        const char *args[] = {"\"user={username}, role={role}\"", "\"user=alice, role=admin\""};
        ar_instruction_ast_t *ast = ar_instruction_ast__create_function_call(
            AR_INSTRUCTION_AST_TYPE__PARSE, "parse", args, 2, "memory.parsed"
        );
        assert(ast != NULL);
        
        // Create argument ASTs: parse("user={username}, role={role}", "user=alice, role=admin")
        ar_list_t *arg_asts = ar_list__create();
        ar_expression_ast_t *template_ast = ar_expression_ast__create_literal_string("user={username}, role={role}");
        ar_expression_ast_t *input_ast = ar_expression_ast__create_literal_string("user=alice, role=admin");
        ar_list__add_last(arg_asts, template_ast);
        ar_list__add_last(arg_asts, input_ast);
        ar_instruction_ast__set_function_arg_asts(ast, arg_asts);
        
        // Create a frame for evaluation
        ar_data_t *ctx = ar_data__create_map();
        ar_data_t *msg = ar_data__create_string("");
        ar_frame_t *fr = ar_frame__create(memory, ctx, msg);
        
        bool result = ar_instruction_evaluator__evaluate(evaluator, fr, ast);
        
        // Clean up frame
        ar_frame__destroy(fr);
        ar_data__destroy(ctx);
        ar_data__destroy(msg);
        assert(result == true);
        
        // Verify result was stored as a map
        ar_data_t *value = ar_data__get_map_data(memory, "parsed");
        assert(value != NULL);
        assert(ar_data__get_type(value) == AR_DATA_TYPE__MAP);
        
        // Check parsed values
        ar_data_t *username_value = ar_data__get_map_data(value, "username");
        assert(username_value != NULL);
        assert(ar_data__get_type(username_value) == AR_DATA_TYPE__STRING);
        assert(strcmp(ar_data__get_string(username_value), "alice") == 0);
        
        ar_data_t *role_value = ar_data__get_map_data(value, "role");
        assert(role_value != NULL);
        assert(ar_data__get_type(role_value) == AR_DATA_TYPE__STRING);
        assert(strcmp(ar_data__get_string(role_value), "admin") == 0);
        
        ar_instruction_ast__destroy(ast);
    }
    
    // Test 4: Build instruction
    {
        // First create a map to use in build
        ar_data__set_map_string(memory, "name", "Alice");
        
        const char *args[] = {"\"Hi {name}\"", "memory"};
        ar_instruction_ast_t *ast = ar_instruction_ast__create_function_call(
            AR_INSTRUCTION_AST_TYPE__BUILD, "build", args, 2, "memory.built"
        );
        assert(ast != NULL);
        
        // Create argument ASTs: build("Hi {name}", memory)
        ar_list_t *arg_asts = ar_list__create();
        ar_expression_ast_t *template_ast = ar_expression_ast__create_literal_string("Hi {name}");
        ar_expression_ast_t *values_ast = ar_expression_ast__create_memory_access("memory", NULL, 0);
        ar_list__add_last(arg_asts, template_ast);
        ar_list__add_last(arg_asts, values_ast);
        ar_instruction_ast__set_function_arg_asts(ast, arg_asts);
        
        // Create a frame for evaluation
        ar_data_t *ctx = ar_data__create_map();
        ar_data_t *msg = ar_data__create_string("");
        ar_frame_t *fr = ar_frame__create(memory, ctx, msg);
        
        bool result = ar_instruction_evaluator__evaluate(evaluator, fr, ast);
        
        // Clean up frame
        ar_frame__destroy(fr);
        ar_data__destroy(ctx);
        ar_data__destroy(msg);
        assert(result == true);
        
        // Verify result was stored
        ar_data_t *value = ar_data__get_map_data(memory, "built");
        assert(value != NULL);
        assert(ar_data__get_type(value) == AR_DATA_TYPE__STRING);
        assert(strcmp(ar_data__get_string(value), "Hi Alice") == 0);
        
        ar_instruction_ast__destroy(ast);
    }

    // Test 5: Complete instruction
    {
        const char *args[] = {"\"The largest country in South America is {country}.\""};
        ar_instruction_ast_t *ast = ar_instruction_ast__create_function_call(
            AR_INSTRUCTION_AST_TYPE__COMPLETE, "complete", args, 1, "memory.ok"
        );
        assert(ast != NULL);

        ar_list_t *arg_asts = ar_list__create();
        ar_expression_ast_t *template_ast = ar_expression_ast__create_literal_string(
            "The largest country in South America is {country}."
        );
        ar_list__add_last(arg_asts, template_ast);
        ar_instruction_ast__set_function_arg_asts(ast, arg_asts);

        ar_data_t *ctx = ar_data__create_map();
        ar_data_t *msg = ar_data__create_string("");
        ar_frame_t *fr = ar_frame__create(memory, ctx, msg);

        bool result = ar_instruction_evaluator__evaluate(evaluator, fr, ast);

        ar_frame__destroy(fr);
        ar_data__destroy(ctx);
        ar_data__destroy(msg);
        assert(result == true);
        ar_data_t *value = ar_data__get_map_data(memory, "ok");
        assert(value != NULL);
        assert(ar_data__get_type(value) == AR_DATA_TYPE__MAP);
        assert(strcmp(ar_data__get_map_string(value, "country"), "Brazil") == 0);

        ar_instruction_ast__destroy(ast);
    }
    
    // Test 6: Method instruction
    {
        
        // Given a compile instruction AST with three string arguments and result assignment
        const char *args[] = {"\"test_method\"", "\"memory.result := 42\"", "\"1.0.0\""};
        ar_instruction_ast_t *ast = ar_instruction_ast__create_function_call(
            AR_INSTRUCTION_AST_TYPE__COMPILE, "compile", args, 3, "memory.method_created"
        );
        assert(ast != NULL);
        
        ar_list_t *arg_asts = ar_list__create();
        ar_expression_ast_t *name_ast = ar_expression_ast__create_literal_string("test_method");
        ar_expression_ast_t *code_ast = ar_expression_ast__create_literal_string("memory.result := 42");
        ar_expression_ast_t *version_ast = ar_expression_ast__create_literal_string("1.0.0");
        ar_list__add_last(arg_asts, name_ast);
        ar_list__add_last(arg_asts, code_ast);
        ar_list__add_last(arg_asts, version_ast);
        ar_instruction_ast__set_function_arg_asts(ast, arg_asts);
        
        ar_data_t *ctx = ar_data__create_map();
        ar_data_t *msg = ar_data__create_string("");
        ar_frame_t *fr = ar_frame__create(memory, ctx, msg);
        
        // When evaluating the method instruction
        bool result = ar_instruction_evaluator__evaluate(evaluator, fr, ast);
        
        // Then evaluation should succeed
        ar_frame__destroy(fr);
        ar_data__destroy(ctx);
        ar_data__destroy(msg);
        assert(result == true);
        
        // And the result should be stored (1 for success)
        ar_data_t *value = ar_data__get_map_data(memory, "method_created");
        assert(value != NULL);
        assert(ar_data__get_type(value) == AR_DATA_TYPE__INTEGER);
        assert(ar_data__get_integer(value) == 1);
        
        // And the method should be registered in methodology
        ar_methodology_t *methodology = ar_agency__get_methodology(agency);
        ar_method_t *method = ar_methodology__get_method(methodology, "test_method", "1.0.0");
        assert(method != NULL);
        assert(strcmp(ar_method__get_name(method), "test_method") == 0);
        assert(strcmp(ar_method__get_version(method), "1.0.0") == 0);
        
        ar_instruction_ast__destroy(ast);
    }
    
    // Test 6: Destroy agent instruction 
    {
        // Use the same agency from the evaluator
        ar_methodology_t *mut_methodology = ar_agency__get_methodology(agency);
        
        // Create a test method and agent first
        ar_methodology__create_method(mut_methodology, "destroy_test_method", "memory.x := 1", "1.0.0");
        int64_t agent_id = ar_agency__create_agent(agency, "destroy_test_method", "1.0.0", NULL);
        assert(agent_id > 0);
        
        // Create destroy agent instruction AST
        char agent_id_str[32];
        snprintf(agent_id_str, sizeof(agent_id_str), "%" PRId64, agent_id);
        const char *args[] = {agent_id_str};
        ar_instruction_ast_t *ast = ar_instruction_ast__create_function_call(
            AR_INSTRUCTION_AST_TYPE__EXIT, "exit", args, 1, "memory.exit_result"
        );
        assert(ast != NULL);
        
        // Create argument AST: destroy(agent_id)
        ar_list_t *arg_asts = ar_list__create();
        ar_expression_ast_t *agent_id_ast = ar_expression_ast__create_literal_int((int)agent_id);
        ar_list__add_last(arg_asts, agent_id_ast);
        ar_instruction_ast__set_function_arg_asts(ast, arg_asts);
        
        // Create a frame for evaluation
        ar_data_t *ctx = ar_data__create_map();
        ar_data_t *msg = ar_data__create_string("");
        ar_frame_t *fr = ar_frame__create(memory, ctx, msg);
        
        // When evaluating through the facade
        bool result = ar_instruction_evaluator__evaluate(evaluator, fr, ast);
        
        // Then it should succeed
        ar_frame__destroy(fr);
        ar_data__destroy(ctx);
        ar_data__destroy(msg);
        assert(result == true);
        
        // And the result should be true (1) indicating successful destruction
        ar_data_t *value = ar_data__get_map_data(memory, "exit_result");
        assert(value != NULL);
        assert(ar_data__get_type(value) == AR_DATA_TYPE__INTEGER);
        assert(ar_data__get_integer(value) == 1);
        
        // And the agent should be destroyed
        ar_agent_registry_t *ref_registry = ar_agency__get_registry(agency);
        assert(ar_agent_registry__is_registered(ref_registry, agent_id) == false);
        
        ar_instruction_ast__destroy(ast);
    }
    
    // Test 7: Destroy method instruction 
    {
        // Use the same methodology from the evaluator
        ar_methodology_t *mut_methodology = ar_agency__get_methodology(agency);
        
        ar_methodology__create_method(mut_methodology, "destroy_method_test", "memory.x := 1", "1.0.0");
        
        // Verify method exists
        ar_method_t *method = ar_methodology__get_method(mut_methodology, "destroy_method_test", "1.0.0");
        assert(method != NULL);
        
        // Create destroy method instruction AST
        const char *args[] = {"\"destroy_method_test\"", "\"1.0.0\""};
        ar_instruction_ast_t *ast = ar_instruction_ast__create_function_call(
            AR_INSTRUCTION_AST_TYPE__DEPRECATE, "deprecate", args, 2, "memory.destroy_method_result"
        );
        assert(ast != NULL);
        
        // Create argument ASTs: destroy("destroy_method_test", "1.0.0")
        ar_list_t *arg_asts = ar_list__create();
        ar_expression_ast_t *name_ast = ar_expression_ast__create_literal_string("destroy_method_test");
        ar_expression_ast_t *version_ast = ar_expression_ast__create_literal_string("1.0.0");
        ar_list__add_last(arg_asts, name_ast);
        ar_list__add_last(arg_asts, version_ast);
        ar_instruction_ast__set_function_arg_asts(ast, arg_asts);
        
        // Create a frame for evaluation
        ar_data_t *ctx = ar_data__create_map();
        ar_data_t *msg = ar_data__create_string("");
        ar_frame_t *fr = ar_frame__create(memory, ctx, msg);
        
        // When evaluating through the facade
        bool result = ar_instruction_evaluator__evaluate(evaluator, fr, ast);
        
        // Then it should succeed
        ar_frame__destroy(fr);
        ar_data__destroy(ctx);
        ar_data__destroy(msg);
        assert(result == true);
        
        // And the result should be true (1) indicating successful destruction
        ar_data_t *value = ar_data__get_map_data(memory, "destroy_method_result");
        assert(value != NULL);
        assert(ar_data__get_type(value) == AR_DATA_TYPE__INTEGER);
        assert(ar_data__get_integer(value) == 1);
        
        // And the method should be destroyed
        ar_method_t *destroyed_method = ar_methodology__get_method(mut_methodology, "destroy_method_test", "1.0.0");
        assert(destroyed_method == NULL);
        
        ar_instruction_ast__destroy(ast);
    }
    
    // Cleanup
    ar_instruction_evaluator__destroy(evaluator);
    ar_system__destroy(sys);
    ar_data__destroy(memory);
    ar_data__destroy(context);
    ar_data__destroy(message);
    ar_log__destroy(log);
}

static void test_instruction_evaluator__complete_failure_returns_boolean_status(void) {
    ar_data_t *memory = ar_data__create_map();
    assert(memory != NULL);

    ar_log_t *log = ar_log__create();
    assert(log != NULL);

    ar_system_t *sys = ar_system__create();
    assert(sys != NULL);
    ar_agency_t *agency = ar_system__get_agency(sys);
    ar_delegation_t *delegation = ar_system__get_delegation(sys);

    ar_instruction_evaluator_t *evaluator = ar_instruction_evaluator__create(log, agency, delegation);
    assert(evaluator != NULL);

    const char *args[] = {"\"Broken value is {bracey}.\""};
    ar_instruction_ast_t *ast = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__COMPLETE, "complete", args, 1, "memory.ok"
    );
    assert(ast != NULL);

    ar_list_t *arg_asts = ar_list__create();
    assert(arg_asts != NULL);
    assert(ar_list__add_last(arg_asts, ar_expression_ast__create_literal_string("Broken value is {bracey}.")) == true);
    assert(ar_instruction_ast__set_function_arg_asts(ast, arg_asts) == true);

    ar_data_t *ctx = ar_data__create_map();
    ar_data_t *msg = ar_data__create_string("");
    ar_frame_t *fr = ar_frame__create(memory, ctx, msg);
    assert(fr != NULL);

    bool result = ar_instruction_evaluator__evaluate(evaluator, fr, ast);
    assert(result == true);
    ar_data_t *ref_result = ar_data__get_map_data(memory, "ok");
    assert(ref_result != NULL);
    assert(ar_data__get_type(ref_result) == AR_DATA_TYPE__MAP);
    assert(ar_data__get_map_data(ref_result, "bracey") == NULL);
    assert(ar_log__get_last_error_message(log) != NULL);

    ar_frame__destroy(fr);
    ar_data__destroy(ctx);
    ar_data__destroy(msg);
    ar_instruction_ast__destroy(ast);
    ar_instruction_evaluator__destroy(evaluator);
    ar_system__destroy(sys);
    ar_data__destroy(memory);
    ar_log__destroy(log);
}

static void test_instruction_evaluator__normal_work_continues_after_complete_failure(void) {
    ar_data_t *memory = ar_data__create_map();
    assert(memory != NULL);
    assert(ar_data__set_map_string(memory, "country", "Peru") == true);

    ar_log_t *log = ar_log__create();
    assert(log != NULL);

    ar_system_t *sys = ar_system__create();
    assert(sys != NULL);
    ar_agency_t *agency = ar_system__get_agency(sys);
    ar_delegation_t *delegation = ar_system__get_delegation(sys);

    ar_instruction_evaluator_t *evaluator = ar_instruction_evaluator__create(log, agency, delegation);
    assert(evaluator != NULL);

    const char *complete_args[] = {"\"Broken value is {bracey}.\""};
    ar_instruction_ast_t *complete_ast = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__COMPLETE, "complete", complete_args, 1, "memory.ok"
    );
    assert(complete_ast != NULL);
    ar_list_t *complete_arg_asts = ar_list__create();
    assert(complete_arg_asts != NULL);
    assert(ar_list__add_last(complete_arg_asts, ar_expression_ast__create_literal_string("Broken value is {bracey}.")) == true);
    assert(ar_instruction_ast__set_function_arg_asts(complete_ast, complete_arg_asts) == true);

    ar_data_t *ctx1 = ar_data__create_map();
    ar_data_t *msg1 = ar_data__create_string("");
    ar_frame_t *frame1 = ar_frame__create(memory, ctx1, msg1);
    assert(frame1 != NULL);

    bool complete_result = ar_instruction_evaluator__evaluate(evaluator, frame1, complete_ast);
    assert(complete_result == true);
    ar_data_t *ref_result = ar_data__get_map_data(memory, "ok");
    assert(ref_result != NULL);
    assert(ar_data__get_type(ref_result) == AR_DATA_TYPE__MAP);
    assert(ar_data__get_map_data(ref_result, "bracey") == NULL);
    assert(strcmp(ar_data__get_map_string(memory, "country"), "Peru") == 0);

    ar_frame__destroy(frame1);
    ar_data__destroy(ctx1);
    ar_data__destroy(msg1);
    ar_instruction_ast__destroy(complete_ast);

    ar_instruction_ast_t *assignment_ast = ar_instruction_ast__create_assignment("memory.after_failure", "42");
    assert(assignment_ast != NULL);
    assert(ar_instruction_ast__set_assignment_expression_ast(assignment_ast, ar_expression_ast__create_literal_int(42)) == true);

    ar_data_t *ctx2 = ar_data__create_map();
    ar_data_t *msg2 = ar_data__create_string("");
    ar_frame_t *frame2 = ar_frame__create(memory, ctx2, msg2);
    assert(frame2 != NULL);

    bool assignment_result = ar_instruction_evaluator__evaluate(evaluator, frame2, assignment_ast);
    assert(assignment_result == true);
    assert(ar_data__get_map_integer(memory, "after_failure") == 42);

    ar_frame__destroy(frame2);
    ar_data__destroy(ctx2);
    ar_data__destroy(msg2);
    ar_instruction_ast__destroy(assignment_ast);

    ar_instruction_evaluator__destroy(evaluator);
    ar_system__destroy(sys);
    ar_data__destroy(memory);
    ar_log__destroy(log);
}

static void test_instruction_evaluator__complete_values_can_feed_later_build_and_send_instructions(void) {
    ar_data_t *memory = ar_data__create_map();
    assert(memory != NULL);

    ar_log_t *log = ar_log__create();
    assert(log != NULL);

    ar_system_t *sys = ar_system__create();
    assert(sys != NULL);
    ar_agency_t *agency = ar_system__get_agency(sys);
    ar_delegation_t *delegation = ar_system__get_delegation(sys);

    ar_delegate_t *own_delegate = ar_delegate__create(log, "test");
    assert(own_delegate != NULL);
    assert(ar_delegation__register_delegate(delegation, -101, own_delegate) == true);

    ar_instruction_evaluator_t *evaluator = ar_instruction_evaluator__create(log, agency, delegation);
    assert(evaluator != NULL);

    {
        const char *args[] = {"\"{country} is in {continent}. {country} remains consistent.\""};
        ar_instruction_ast_t *ast = ar_instruction_ast__create_function_call(
            AR_INSTRUCTION_AST_TYPE__COMPLETE, "complete", args, 1, "memory.values"
        );
        assert(ast != NULL);

        ar_list_t *arg_asts = ar_list__create();
        assert(arg_asts != NULL);
        assert(ar_list__add_last(
            arg_asts,
            ar_expression_ast__create_literal_string("{country} is in {continent}. {country} remains consistent.")
        ) == true);
        assert(ar_instruction_ast__set_function_arg_asts(ast, arg_asts) == true);

        ar_data_t *ctx = ar_data__create_map();
        ar_data_t *msg = ar_data__create_string("");
        ar_frame_t *fr = ar_frame__create(memory, ctx, msg);
        assert(fr != NULL);

        bool result = ar_instruction_evaluator__evaluate(evaluator, fr, ast);
        assert(result == true);
        ar_data_t *ref_values = ar_data__get_map_data(memory, "values");
        assert(ref_values != NULL);
        assert(ar_data__get_type(ref_values) == AR_DATA_TYPE__MAP);
        assert(strcmp(ar_data__get_map_string(ref_values, "country"), "Brazil") == 0);
        assert(strcmp(ar_data__get_map_string(ref_values, "continent"), "South America") == 0);

        ar_frame__destroy(fr);
        ar_data__destroy(ctx);
        ar_data__destroy(msg);
        ar_instruction_ast__destroy(ast);
    }

    {
        const char *args[] = {"\"reply={country}|{continent}\"", "memory.values"};
        ar_instruction_ast_t *ast = ar_instruction_ast__create_function_call(
            AR_INSTRUCTION_AST_TYPE__BUILD, "build", args, 2, "memory.reply"
        );
        assert(ast != NULL);

        ar_list_t *arg_asts = ar_list__create();
        assert(arg_asts != NULL);
        assert(ar_list__add_last(arg_asts, ar_expression_ast__create_literal_string("reply={country}|{continent}")) == true);
        const char *values_path[] = {"values"};
        assert(ar_list__add_last(arg_asts, ar_expression_ast__create_memory_access("memory", values_path, 1)) == true);
        assert(ar_instruction_ast__set_function_arg_asts(ast, arg_asts) == true);

        ar_data_t *ctx = ar_data__create_map();
        ar_data_t *msg = ar_data__create_string("");
        ar_frame_t *fr = ar_frame__create(memory, ctx, msg);
        assert(fr != NULL);

        bool result = ar_instruction_evaluator__evaluate(evaluator, fr, ast);
        assert(result == true);
        assert(strcmp(ar_data__get_map_string(memory, "reply"), "reply=Brazil|South America") == 0);

        ar_frame__destroy(fr);
        ar_data__destroy(ctx);
        ar_data__destroy(msg);
        ar_instruction_ast__destroy(ast);
    }

    {
        const char *args[] = {"-101", "memory.reply"};
        ar_instruction_ast_t *ast = ar_instruction_ast__create_function_call(
            AR_INSTRUCTION_AST_TYPE__SEND, "send", args, 2, NULL
        );
        assert(ast != NULL);

        ar_list_t *arg_asts = ar_list__create();
        assert(arg_asts != NULL);
        assert(ar_list__add_last(arg_asts, ar_expression_ast__create_literal_int(-101)) == true);
        const char *reply_path[] = {"reply"};
        assert(ar_list__add_last(arg_asts, ar_expression_ast__create_memory_access("memory", reply_path, 1)) == true);
        assert(ar_instruction_ast__set_function_arg_asts(ast, arg_asts) == true);

        ar_data_t *ctx = ar_data__create_map();
        ar_data_t *msg = ar_data__create_string("");
        ar_frame_t *fr = ar_frame__create(memory, ctx, msg);
        assert(fr != NULL);

        bool result = ar_instruction_evaluator__evaluate(evaluator, fr, ast);
        assert(result == true);
        assert(ar_delegation__delegate_has_messages(delegation, -101) == true);

        ar_data_t *own_sent = ar_delegation__take_delegate_message(delegation, -101);
        assert(own_sent != NULL);
        assert(ar_data__get_type(own_sent) == AR_DATA_TYPE__STRING);
        assert(strcmp(ar_data__get_string(own_sent), "reply=Brazil|South America") == 0);
        ar_data__destroy(own_sent);

        ar_frame__destroy(fr);
        ar_data__destroy(ctx);
        ar_data__destroy(msg);
        ar_instruction_ast__destroy(ast);
    }

    ar_instruction_evaluator__destroy(evaluator);
    ar_system__destroy(sys);
    ar_data__destroy(memory);
    ar_log__destroy(log);
}

static void test_instruction_evaluator__only_unified_interface_exposed(void) {
    // This test verifies that only the unified evaluate method is exposed
    // and that individual evaluate functions are not accessible
    
    // Given an evaluator
    ar_data_t *memory = ar_data__create_map();
    assert(memory != NULL);
    
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    
    ar_system_t *sys = ar_system__create();
    assert(sys != NULL);
    ar_agency_t *agency = ar_system__get_agency(sys);
    ar_delegation_t *delegation = ar_system__get_delegation(sys);
    
    ar_instruction_evaluator_t *evaluator = ar_instruction_evaluator__create(log, agency, delegation);
    assert(evaluator != NULL);
    
    // When we have various instruction ASTs
    ar_instruction_ast_t *assignment_ast = ar_instruction_ast__create_assignment("memory.x", "42");
    assert(assignment_ast != NULL);
    ar_expression_ast_t *expr_ast = ar_expression_ast__create_literal_int(42);
    assert(expr_ast != NULL);
    bool ast_set = ar_instruction_ast__set_assignment_expression_ast(assignment_ast, expr_ast);
    assert(ast_set == true);
    
    const char *send_args[] = {"0", "\"hello\""};
    ar_instruction_ast_t *send_ast = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__SEND, "send", send_args, 2, NULL
    );
    assert(send_ast != NULL);
    
    // Create argument ASTs for send
    ar_list_t *arg_asts = ar_list__create();
    ar_expression_ast_t *target_ast = ar_expression_ast__create_literal_int(0);
    ar_expression_ast_t *msg_ast = ar_expression_ast__create_literal_string("hello");
    ar_list__add_last(arg_asts, target_ast);
    ar_list__add_last(arg_asts, msg_ast);
    ar_instruction_ast__set_function_arg_asts(send_ast, arg_asts);
    
    // Then we should only be able to evaluate them through the unified interface
    // The following should compile and work:
    
    // Create frames for evaluation
    ar_data_t *ctx1 = ar_data__create_map();
    ar_data_t *msg1 = ar_data__create_string("");
    ar_frame_t *frame1 = ar_frame__create(memory, ctx1, msg1);
    
    bool result1 = ar_instruction_evaluator__evaluate(evaluator, frame1, assignment_ast);
    assert(result1 == true);
    
    ar_frame__destroy(frame1);
    ar_data__destroy(ctx1);
    ar_data__destroy(msg1);
    
    ar_data_t *ctx2 = ar_data__create_map();
    ar_data_t *msg2 = ar_data__create_string("");
    ar_frame_t *frame2 = ar_frame__create(memory, ctx2, msg2);
    
    bool result2 = ar_instruction_evaluator__evaluate(evaluator, frame2, send_ast);
    
    ar_frame__destroy(frame2);
    ar_data__destroy(ctx2);
    ar_data__destroy(msg2);
    assert(result2 == true);
    
    // And the individual evaluate functions should not be available
    // (This is checked by removing them from the header - if they're still there,
    // the test will compile but we'll know we need to remove them)
    
    // Cleanup
    ar_instruction_ast__destroy(assignment_ast);
    ar_instruction_ast__destroy(send_ast);
    ar_instruction_evaluator__destroy(evaluator);
    ar_system__destroy(sys);
    ar_data__destroy(memory);
    ar_log__destroy(log);
}

static void test_instruction_evaluator__unified_evaluate_assignment(void) {
    // Given an evaluator with memory and an assignment AST
    ar_data_t *memory = ar_data__create_map();
    assert(memory != NULL);
    
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    
    ar_system_t *sys = ar_system__create();
    assert(sys != NULL);
    ar_agency_t *agency = ar_system__get_agency(sys);
    ar_delegation_t *delegation = ar_system__get_delegation(sys);
    
    ar_instruction_evaluator_t *evaluator = ar_instruction_evaluator__create(log, agency, delegation);
    assert(evaluator != NULL);
    
    // Create an assignment AST: memory.x := 42
    ar_instruction_ast_t *ast = ar_instruction_ast__create_assignment("memory.x", "42");
    assert(ast != NULL);
    
    ar_expression_ast_t *expr_ast = ar_expression_ast__create_literal_int(42);
    assert(expr_ast != NULL);
    bool set_result = ar_instruction_ast__set_assignment_expression_ast(ast, expr_ast);
    assert(set_result == true);
    
    // Verify the expression AST was stored
    const ar_expression_ast_t *stored_ast = ar_instruction_ast__get_assignment_expression_ast(ast);
    assert(stored_ast != NULL);
    
    // When evaluating using the unified evaluate method
    ar_data_t *ctx = ar_data__create_map();
    ar_data_t *msg = ar_data__create_string("");
    ar_frame_t *fr = ar_frame__create(memory, ctx, msg);
    
    bool result = ar_instruction_evaluator__evaluate(evaluator, fr, ast);
    
    ar_frame__destroy(fr);
    ar_data__destroy(ctx);
    ar_data__destroy(msg);
    
    // Then evaluation should succeed
    assert(result == true);
    
    // And the value should be stored in memory
    ar_data_t *value = ar_data__get_map_data(memory, "x");
    assert(value != NULL);
    assert(ar_data__get_type(value) == AR_DATA_TYPE__INTEGER);
    assert(ar_data__get_integer(value) == 42);
    
    // Cleanup
    ar_instruction_ast__destroy(ast);
    ar_instruction_evaluator__destroy(evaluator);
    ar_system__destroy(sys);
    ar_data__destroy(memory);
    ar_log__destroy(log);
}

static void test_instruction_evaluator__rejects_protected_self_result_paths(void) {
    // Given an instruction evaluator with agency-managed memory.self
    ar_data_t *memory = ar_data__create_map();
    assert(memory != NULL);
    ar_data__set_map_integer(memory, "self", 7);

    ar_log_t *log = ar_log__create();
    assert(log != NULL);

    ar_system_t *sys = ar_system__create();
    assert(sys != NULL);
    ar_agency_t *agency = ar_system__get_agency(sys);
    ar_delegation_t *delegation = ar_system__get_delegation(sys);

    ar_instruction_evaluator_t *evaluator = ar_instruction_evaluator__create(log, agency, delegation);
    assert(evaluator != NULL);

    ar_data_t *ctx = ar_data__create_map();
    ar_data_t *msg = ar_data__create_string("");
    ar_frame_t *frame = ar_frame__create(memory, ctx, msg);
    assert(frame != NULL);

    struct instruction_case {
        ar_instruction_ast_type_t type;
        const char *function_name;
    } cases[] = {
        {AR_INSTRUCTION_AST_TYPE__SEND, "send"},
        {AR_INSTRUCTION_AST_TYPE__IF, "if"},
        {AR_INSTRUCTION_AST_TYPE__BUILD, "build"},
        {AR_INSTRUCTION_AST_TYPE__COMPLETE, "complete"},
        {AR_INSTRUCTION_AST_TYPE__COMPILE, "compile"},
        {AR_INSTRUCTION_AST_TYPE__SPAWN, "spawn"},
        {AR_INSTRUCTION_AST_TYPE__EXIT, "exit"},
        {AR_INSTRUCTION_AST_TYPE__DEPRECATE, "deprecate"}
    };

    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
        ar_instruction_ast_t *ast = ar_instruction_ast__create_function_call(
            cases[i].type, cases[i].function_name, NULL, 0, "memory.self.anything"
        );
        assert(ast != NULL);

        bool result = ar_instruction_evaluator__evaluate(evaluator, frame, ast);

        // Then every non-parse function instruction should reject protected self result paths
        assert(result == false);
        assert(ar_data__get_map_integer(memory, "self") == 7);

        ar_event_t *error_event = ar_log__get_last_error(log);
        assert(error_event != NULL);
        const char *error_msg = ar_event__get_message(error_event);
        assert(error_msg != NULL);
        assert(strstr(error_msg, "memory.self is agency-managed") != NULL);

        ar_instruction_ast__destroy(ast);
    }

    ar_frame__destroy(frame);
    ar_data__destroy(ctx);
    ar_data__destroy(msg);
    ar_instruction_evaluator__destroy(evaluator);
    ar_system__destroy(sys);
    ar_data__destroy(memory);
    ar_log__destroy(log);
}

int main(void) {
    setup_fake_complete_runner();
    printf("Starting instruction_evaluator create/destroy tests...\n");
    
    test_instruction_evaluator__create_destroy();
    printf("test_instruction_evaluator__create_destroy passed!\n");
    
    test_instruction_evaluator__create_with_null_context();
    printf("test_instruction_evaluator__create_with_null_context passed!\n");
    
    test_instruction_evaluator__destroy_null();
    printf("test_instruction_evaluator__destroy_null passed!\n");
    
    test_instruction_evaluator__create_with_null_memory();
    printf("test_instruction_evaluator__create_with_null_memory passed!\n");
    
    test_instruction_evaluator__stores_evaluator_instances_internally();
    printf("test_instruction_evaluator__stores_evaluator_instances_internally passed!\n");
    
    test_instruction_evaluator__unified_evaluate_assignment();
    printf("test_instruction_evaluator__unified_evaluate_assignment passed!\n");
    
    test_instruction_evaluator__unified_evaluate_all_types();
    printf("test_instruction_evaluator__unified_evaluate_all_types passed!\n");

    test_instruction_evaluator__rejects_protected_self_result_paths();
    printf("test_instruction_evaluator__rejects_protected_self_result_paths passed!\n");

    test_instruction_evaluator__complete_failure_returns_boolean_status();
    printf("test_instruction_evaluator__complete_failure_returns_boolean_status passed!\n");

    test_instruction_evaluator__normal_work_continues_after_complete_failure();
    printf("test_instruction_evaluator__normal_work_continues_after_complete_failure passed!\n");

    test_instruction_evaluator__complete_values_can_feed_later_build_and_send_instructions();
    printf("test_instruction_evaluator__complete_values_can_feed_later_build_and_send_instructions passed!\n");
    
    test_instruction_evaluator__only_unified_interface_exposed();
    printf("test_instruction_evaluator__only_unified_interface_exposed passed!\n");
    
    printf("All instruction_evaluator create/destroy tests passed!\n");
    
    cleanup_fake_complete_runner();
    return 0;
}
