/**
 * @file ar_method_evaluator_tests.c
 * @brief Tests for the method evaluator module
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include "ar_method_evaluator.h"
#include "ar_heap.h"
#include "ar_log.h"
#include "ar_data.h"
#include "ar_frame.h"
#include "ar_method_ast.h"
#include "ar_method_parser.h"
#include "ar_expression_ast.h"
#include "ar_instruction_ast.h"
#include "ar_system.h"
#include "ar_agency.h"
#include "ar_assert.h"

static void test_method_evaluator__create_destroy(void) {
    printf("Testing method evaluator create/destroy...\n");
    
    // Given a system and log
    ar_system_t *own_system = ar_system__create();
    assert(own_system != NULL);
    ar_agency_t *ref_agency = ar_system__get_agency(own_system);
    assert(ref_agency != NULL);
    ar_delegation_t *ref_delegation = ar_system__get_delegation(own_system);
    assert(ref_delegation != NULL);

    // Given evaluator resources
    ar_log_t *own_log = ar_log__create();
    assert(own_log != NULL);

    // When creating a method evaluator with log, agency, and delegation
    ar_method_evaluator_t *own_evaluator = ar_method_evaluator__create(own_log, ref_agency, ref_delegation);
    
    // Then it should be created successfully
    assert(own_evaluator != NULL);
    
    // Cleanup
    ar_method_evaluator__destroy(own_evaluator);
    ar_log__destroy(own_log);
    ar_system__destroy(own_system);
    
    printf("  ✓ Method evaluator created and destroyed successfully\n");
}

static void test_method_evaluator__evaluate_empty_method(void) {
    printf("Testing method evaluator with empty method...\n");
    
    // Given a system and method evaluator
    ar_system_t *own_system = ar_system__create();
    assert(own_system != NULL);
    ar_agency_t *ref_agency = ar_system__get_agency(own_system);
    assert(ref_agency != NULL);
    ar_delegation_t *ref_delegation = ar_system__get_delegation(own_system);
    assert(ref_delegation != NULL);

    // Given evaluator resources
    ar_log_t *own_log = ar_log__create();
    ar_data_t *own_memory = ar_data__create_map();
    ar_data_t *own_context = ar_data__create_map();
    ar_method_evaluator_t *own_evaluator = ar_method_evaluator__create(own_log, ref_agency, ref_delegation);
    
    // Given a frame
    ar_data_t *own_message = ar_data__create_string("test message");
    ar_frame_t *own_frame = ar_frame__create(own_memory, own_context, own_message);
    assert(own_frame != NULL);
    
    // Given an empty method AST
    ar_method_ast_t *own_ast = ar_method_ast__create();
    assert(own_ast != NULL);
    
    // Given the method has no instructions
    assert(ar_method_ast__get_instruction_count(own_ast) == 0);
    
    // When evaluating the empty method
    bool result = ar_method_evaluator__evaluate(own_evaluator, own_frame, own_ast);
    
    // Then it should succeed (empty method has no instructions to fail)
    assert(result == true);
    
    // Cleanup
    ar_method_ast__destroy(own_ast);
    ar_frame__destroy(own_frame);
    ar_method_evaluator__destroy(own_evaluator);
    ar_data__destroy(own_message);
    ar_data__destroy(own_context);
    ar_data__destroy(own_memory);
    ar_log__destroy(own_log);
    ar_system__destroy(own_system);
    
    printf("  ✓ Empty method evaluated successfully\n");
}

static void test_method_evaluator__evaluate_single_instruction_method(void) {
    printf("Testing method evaluator with single instruction...\n");
    
    // Given a system and method evaluator
    ar_system_t *own_system = ar_system__create();
    assert(own_system != NULL);
    ar_agency_t *ref_agency = ar_system__get_agency(own_system);
    assert(ref_agency != NULL);
    ar_delegation_t *ref_delegation = ar_system__get_delegation(own_system);
    assert(ref_delegation != NULL);

    // Given evaluator resources
    ar_log_t *own_log = ar_log__create();
    ar_data_t *own_memory = ar_data__create_map();
    ar_data_t *own_context = ar_data__create_map();
    ar_method_evaluator_t *own_evaluator = ar_method_evaluator__create(own_log, ref_agency, ref_delegation);
    
    // Given a frame
    ar_data_t *own_message = ar_data__create_string("test message");
    ar_frame_t *own_frame = ar_frame__create(own_memory, own_context, own_message);
    
    // Given a method AST with one instruction
    ar_method_ast_t *own_ast = ar_method_ast__create();
    
    // Given a proper assignment instruction with expression AST
    ar_instruction_ast_t *own_instr = ar_instruction_ast__create_assignment("memory.x", "42");
    ar_expression_ast_t *own_expr = ar_expression_ast__create_literal_int(42);
    ar_instruction_ast__set_assignment_expression_ast(own_instr, own_expr);
    ar_method_ast__add_instruction(own_ast, own_instr);
    
    // Given the method has one instruction
    assert(ar_method_ast__get_instruction_count(own_ast) == 1);
    
    // When evaluating the method with one instruction
    bool result = ar_method_evaluator__evaluate(own_evaluator, own_frame, own_ast);
    
    // Then it should succeed
    assert(result == true);
    
    // Then the value should be stored in memory
    assert(ar_data__get_map_integer(own_memory, "x") == 42);
    
    // Cleanup
    ar_method_ast__destroy(own_ast);
    ar_frame__destroy(own_frame);
    ar_method_evaluator__destroy(own_evaluator);
    ar_data__destroy(own_message);
    ar_data__destroy(own_context);
    ar_data__destroy(own_memory);
    ar_log__destroy(own_log);
    ar_system__destroy(own_system);
    
    printf("  ✓ Single instruction method evaluated successfully\n");
}

static void test_method_evaluator__evaluate_multiple_instructions(void) {
    printf("Testing method evaluator with multiple instructions...\n");
    
    // Given a system and method evaluator
    ar_system_t *own_system = ar_system__create();
    assert(own_system != NULL);
    ar_agency_t *ref_agency = ar_system__get_agency(own_system);
    assert(ref_agency != NULL);
    ar_delegation_t *ref_delegation = ar_system__get_delegation(own_system);
    assert(ref_delegation != NULL);

    // Given evaluator resources
    ar_log_t *own_log = ar_log__create();
    ar_data_t *own_memory = ar_data__create_map();
    ar_data_t *own_context = ar_data__create_map();
    ar_method_evaluator_t *own_evaluator = ar_method_evaluator__create(own_log, ref_agency, ref_delegation);
    
    // Given a frame
    ar_data_t *own_message = ar_data__create_string("test message");
    ar_frame_t *own_frame = ar_frame__create(own_memory, own_context, own_message);
    
    // Given a method AST with three instructions
    ar_method_ast_t *own_ast = ar_method_ast__create();
    
    // Given the first instruction: memory.x := 10
    ar_instruction_ast_t *own_instr1 = ar_instruction_ast__create_assignment("memory.x", "10");
    ar_expression_ast_t *own_expr1 = ar_expression_ast__create_literal_int(10);
    ar_instruction_ast__set_assignment_expression_ast(own_instr1, own_expr1);
    ar_method_ast__add_instruction(own_ast, own_instr1);
    
    // Given the second instruction: memory.y := 20
    ar_instruction_ast_t *own_instr2 = ar_instruction_ast__create_assignment("memory.y", "20");
    ar_expression_ast_t *own_expr2 = ar_expression_ast__create_literal_int(20);
    ar_instruction_ast__set_assignment_expression_ast(own_instr2, own_expr2);
    ar_method_ast__add_instruction(own_ast, own_instr2);
    
    // Given the third instruction: memory.z := 30
    ar_instruction_ast_t *own_instr3 = ar_instruction_ast__create_assignment("memory.z", "30");
    ar_expression_ast_t *own_expr3 = ar_expression_ast__create_literal_int(30);
    ar_instruction_ast__set_assignment_expression_ast(own_instr3, own_expr3);
    ar_method_ast__add_instruction(own_ast, own_instr3);
    
    // Given the method has three instructions
    assert(ar_method_ast__get_instruction_count(own_ast) == 3);
    
    // When evaluating the method with multiple instructions
    bool result = ar_method_evaluator__evaluate(own_evaluator, own_frame, own_ast);
    
    // Then it should succeed
    assert(result == true);
    
    // Then all values should be stored in memory
    assert(ar_data__get_map_integer(own_memory, "x") == 10);
    assert(ar_data__get_map_integer(own_memory, "y") == 20);
    assert(ar_data__get_map_integer(own_memory, "z") == 30);
    
    // Cleanup
    ar_method_ast__destroy(own_ast);
    ar_frame__destroy(own_frame);
    ar_method_evaluator__destroy(own_evaluator);
    ar_data__destroy(own_message);
    ar_data__destroy(own_context);
    ar_data__destroy(own_memory);
    ar_log__destroy(own_log);
    ar_system__destroy(own_system);
    
    printf("  ✓ Multiple instruction method evaluated successfully\n");
}

static void test_method_evaluator__evaluate_null_parameters(void) {
    printf("Testing method evaluator with NULL parameters...\n");
    
    // Given a system and method evaluator
    ar_system_t *own_system = ar_system__create();
    assert(own_system != NULL);
    ar_agency_t *ref_agency = ar_system__get_agency(own_system);
    assert(ref_agency != NULL);
    ar_delegation_t *ref_delegation = ar_system__get_delegation(own_system);
    assert(ref_delegation != NULL);

    // Given evaluator resources
    ar_log_t *own_log = ar_log__create();
    ar_data_t *own_memory = ar_data__create_map();
    ar_data_t *own_context = ar_data__create_map();
    ar_method_evaluator_t *own_evaluator = ar_method_evaluator__create(own_log, ref_agency, ref_delegation);
    
    // Given valid frame and AST inputs for testing
    ar_data_t *own_message = ar_data__create_string("test message");
    ar_frame_t *own_frame = ar_frame__create(own_memory, own_context, own_message);
    ar_method_ast_t *own_ast = ar_method_ast__create();
    
    // When evaluating with NULL evaluator
    bool result1 = ar_method_evaluator__evaluate(NULL, own_frame, own_ast);
    
    // Then it should fail
    assert(result1 == false);
    
    // When evaluating with NULL frame
    bool result2 = ar_method_evaluator__evaluate(own_evaluator, NULL, own_ast);
    
    // Then it should fail
    assert(result2 == false);
    
    // When evaluating with NULL AST
    bool result3 = ar_method_evaluator__evaluate(own_evaluator, own_frame, NULL);
    
    // Then it should fail
    assert(result3 == false);
    
    // Cleanup
    ar_method_ast__destroy(own_ast);
    ar_frame__destroy(own_frame);
    ar_method_evaluator__destroy(own_evaluator);
    ar_data__destroy(own_message);
    ar_data__destroy(own_context);
    ar_data__destroy(own_memory);
    ar_log__destroy(own_log);
    ar_system__destroy(own_system);
    
    printf("  ✓ NULL parameter handling verified\n");
}

static void test_method_evaluator__evaluate_with_failing_instruction(void) {
    printf("Testing method evaluator with failing instruction...\n");
    
    // Given a system and method evaluator
    ar_system_t *own_system = ar_system__create();
    assert(own_system != NULL);
    ar_agency_t *ref_agency = ar_system__get_agency(own_system);
    assert(ref_agency != NULL);
    ar_delegation_t *ref_delegation = ar_system__get_delegation(own_system);
    assert(ref_delegation != NULL);

    // Given evaluator resources
    ar_log_t *own_log = ar_log__create();
    ar_data_t *own_memory = ar_data__create_map();
    ar_data_t *own_context = ar_data__create_map();
    ar_method_evaluator_t *own_evaluator = ar_method_evaluator__create(own_log, ref_agency, ref_delegation);
    
    // Given a frame
    ar_data_t *own_message = ar_data__create_string("test message");
    ar_frame_t *own_frame = ar_frame__create(own_memory, own_context, own_message);
    
    // Given a method AST with a failing instruction followed by another valid instruction
    ar_method_ast_t *own_ast = ar_method_ast__create();
    
    // Given a valid first instruction
    ar_instruction_ast_t *own_instr1 = ar_instruction_ast__create_assignment("memory.x", "10");
    ar_expression_ast_t *own_expr1 = ar_expression_ast__create_literal_int(10);
    ar_instruction_ast__set_assignment_expression_ast(own_instr1, own_expr1);
    ar_method_ast__add_instruction(own_ast, own_instr1);
    
    // Given an invalid second instruction that assigns to a non-memory target
    ar_instruction_ast_t *own_instr2 = ar_instruction_ast__create_assignment("invalid.target", "20");
    ar_expression_ast_t *own_expr2 = ar_expression_ast__create_literal_int(20);
    ar_instruction_ast__set_assignment_expression_ast(own_instr2, own_expr2);
    ar_method_ast__add_instruction(own_ast, own_instr2);

    // Given another valid instruction after the failure
    ar_instruction_ast_t *own_instr3 = ar_instruction_ast__create_assignment("memory.y", "30");
    ar_expression_ast_t *own_expr3 = ar_expression_ast__create_literal_int(30);
    ar_instruction_ast__set_assignment_expression_ast(own_instr3, own_expr3);
    ar_method_ast__add_instruction(own_ast, own_instr3);
    
    // When evaluating the method with a failing instruction
    bool result = ar_method_evaluator__evaluate(own_evaluator, own_frame, own_ast);
    
    // Then it should fail
    assert(result == false);
    
    // Then the first instruction should have succeeded
    assert(ar_data__get_map_integer(own_memory, "x") == 10);

    // Then the later instruction should still run after the failure
    assert(ar_data__get_map_integer(own_memory, "y") == 30);
    
    // Then an error should be logged by the assignment evaluator
    ar_event_t *ref_error = ar_log__get_last_error(own_log);
    assert(ref_error != NULL);
    const char *ref_message = ar_event__get_message(ref_error);
    assert(ref_message != NULL);
    
    // Then the error message should mention that it failed at line 2
    assert(strstr(ref_message, "line 2") != NULL);
    
    // Cleanup
    ar_method_ast__destroy(own_ast);
    ar_frame__destroy(own_frame);
    ar_method_evaluator__destroy(own_evaluator);
    ar_data__destroy(own_message);
    ar_data__destroy(own_context);
    ar_data__destroy(own_memory);
    ar_log__destroy(own_log);
    ar_system__destroy(own_system);
    
    printf("  ✓ Failing instruction handling verified\n");
}

static void test_method_evaluator__evaluates_parsed_append_instruction(void) {
    printf("Testing method evaluator with parsed append instruction...\n");

    // Given a method evaluator with agency dependencies
    ar_system_t *own_system = ar_system__create();
    assert(own_system != NULL);
    ar_agency_t *ref_agency = ar_system__get_agency(own_system);
    ar_delegation_t *ref_delegation = ar_system__get_delegation(own_system);
    assert(ref_agency != NULL);
    assert(ref_delegation != NULL);

    // Given method parser and evaluator facades
    ar_log_t *own_log = ar_log__create();
    assert(own_log != NULL);
    ar_method_parser_t *own_parser = ar_method_parser__create(own_log);
    assert(own_parser != NULL);
    ar_method_evaluator_t *own_evaluator = ar_method_evaluator__create(
        own_log,
        ref_agency,
        ref_delegation
    );
    assert(own_evaluator != NULL);

    // Given a method with standalone compatibility append
    const char *ref_source =
        "memory.results := []\n"
        "append(memory.results, message.value)\n";
    ar_method_ast_t *own_ast = ar_method_parser__parse(own_parser, ref_source);
    assert(own_ast != NULL);
    assert(ar_method_ast__get_instruction_count(own_ast) == 2);

    // Given a frame with a message value to append
    ar_data_t *own_memory = ar_data__create_map();
    ar_data_t *own_context = ar_data__create_map();
    ar_data_t *own_message = ar_data__create_map();
    assert(own_memory != NULL);
    assert(own_context != NULL);
    assert(own_message != NULL);
    assert(ar_data__set_map_string(own_message, "value", "from method"));
    ar_frame_t *own_frame = ar_frame__create(own_memory, own_context, own_message);
    assert(own_frame != NULL);

    // When evaluating the parsed method
    bool result = ar_method_evaluator__evaluate(own_evaluator, own_frame, own_ast);

    // Then standalone append should mutate the memory-owned list
    assert(result == true);
    ar_data_t *ref_results = ar_data__get_map_data(own_memory, "results");
    assert(ref_results != NULL);
    assert(ar_data__get_type(ref_results) == AR_DATA_TYPE__LIST);
    assert(ar_data__list_count(ref_results) == 1);
    ar_data_t *ref_appended = ar_data__list_first(ref_results);
    assert(ref_appended != NULL);
    assert(strcmp(ar_data__get_string(ref_appended), "from method") == 0);

    // Cleanup
    ar_frame__destroy(own_frame);
    ar_data__destroy(own_message);
    ar_data__destroy(own_context);
    ar_data__destroy(own_memory);
    ar_method_ast__destroy(own_ast);
    ar_method_evaluator__destroy(own_evaluator);
    ar_method_parser__destroy(own_parser);
    ar_log__destroy(own_log);
    ar_system__destroy(own_system);

    printf("  ✓ Parsed append instruction evaluated successfully\n");
}

static void test_method_evaluator__evaluates_multiline_map_local_references(void) {
    printf("Testing method evaluator with multi-line map local references...\n");

    // Given a method evaluator with agency dependencies
    ar_system_t *own_system = ar_system__create();
    AR_ASSERT(own_system != NULL, "System should be created");
    ar_agency_t *ref_agency = ar_system__get_agency(own_system);
    ar_delegation_t *ref_delegation = ar_system__get_delegation(own_system);
    AR_ASSERT(ref_agency != NULL, "Agency should exist");
    AR_ASSERT(ref_delegation != NULL, "Delegation should exist");

    // Given method parser and evaluator facades
    ar_log_t *own_log = ar_log__create();
    AR_ASSERT(own_log != NULL, "Log should be created");
    ar_method_parser_t *own_parser = ar_method_parser__create(own_log);
    AR_ASSERT(own_parser != NULL, "Method parser should be created");
    ar_method_evaluator_t *own_evaluator = ar_method_evaluator__create(
        own_log,
        ref_agency,
        ref_delegation
    );
    AR_ASSERT(own_evaluator != NULL, "Method evaluator should be created");

    // Given a multi-line map assignment whose second entry reads the first entry
    const char *ref_source =
        "memory.profile := {\n"
        "  base := 2\n"
        "  doubled := .base * 2\n"
        "  fallback := .missing\n"
        "}\n";
    ar_method_ast_t *own_ast = ar_method_parser__parse(own_parser, ref_source);
    AR_ASSERT(own_ast != NULL, "Multi-line map with local reference should parse");
    AR_ASSERT(ar_method_ast__get_instruction_count(own_ast) == 1, "Method should contain one instruction");

    // Given an empty frame
    ar_data_t *own_memory = ar_data__create_map();
    ar_data_t *own_context = ar_data__create_map();
    ar_data_t *own_message = ar_data__create_map();
    AR_ASSERT(own_memory != NULL, "Memory should be created");
    AR_ASSERT(own_context != NULL, "Context should be created");
    AR_ASSERT(own_message != NULL, "Message should be created");
    ar_frame_t *own_frame = ar_frame__create(own_memory, own_context, own_message);
    AR_ASSERT(own_frame != NULL, "Frame should be created");

    // When evaluating the parsed method
    bool result = ar_method_evaluator__evaluate(own_evaluator, own_frame, own_ast);

    // Then the local reference should read the prior key in the same map block
    AR_ASSERT(result == true, "Method evaluation should succeed");
    ar_data_t *ref_profile = ar_data__get_map_data(own_memory, "profile");
    AR_ASSERT(ref_profile != NULL, "Profile map should be stored");
    AR_ASSERT(ar_data__get_type(ref_profile) == AR_DATA_TYPE__MAP, "Profile should be a map");
    AR_ASSERT(ar_data__get_map_integer(ref_profile, "base") == 2, "Base key should be stored");
    AR_ASSERT(ar_data__get_map_integer(ref_profile, "doubled") == 4, "Doubled key should use .base");
    AR_ASSERT(ar_data__get_map_integer(ref_profile, "fallback") == 0,
              "Missing .key should use integer 0 sentinel");

    // Cleanup
    ar_frame__destroy(own_frame);
    ar_data__destroy(own_message);
    ar_data__destroy(own_context);
    ar_data__destroy(own_memory);
    ar_method_ast__destroy(own_ast);
    ar_method_evaluator__destroy(own_evaluator);
    ar_method_parser__destroy(own_parser);
    ar_log__destroy(own_log);
    ar_system__destroy(own_system);

    printf("  ✓ Multi-line map local references evaluated successfully\n");
}

static void test_method_evaluator__evaluates_frame_references_in_multiline_map(void) {
    printf("Testing method evaluator with frame references in multi-line map...\n");

    // Given a method evaluator with agency dependencies
    ar_system_t *own_system = ar_system__create();
    AR_ASSERT(own_system != NULL, "System should be created");
    ar_agency_t *ref_agency = ar_system__get_agency(own_system);
    ar_delegation_t *ref_delegation = ar_system__get_delegation(own_system);
    AR_ASSERT(ref_agency != NULL, "Agency should exist");
    AR_ASSERT(ref_delegation != NULL, "Delegation should exist");

    // Given method parser and evaluator facades
    ar_log_t *own_log = ar_log__create();
    AR_ASSERT(own_log != NULL, "Log should be created");
    ar_method_parser_t *own_parser = ar_method_parser__create(own_log);
    AR_ASSERT(own_parser != NULL, "Method parser should be created");
    ar_method_evaluator_t *own_evaluator = ar_method_evaluator__create(
        own_log,
        ref_agency,
        ref_delegation
    );
    AR_ASSERT(own_evaluator != NULL, "Method evaluator should be created");

    // Given a multi-line map assignment that reads all frame roots
    const char *ref_source =
        "memory.profile := {\n"
        "  from_memory := memory.seed\n"
        "  from_message := message.count\n"
        "  from_context := context.limit\n"
        "}\n";
    ar_method_ast_t *own_ast = ar_method_parser__parse(own_parser, ref_source);
    AR_ASSERT(own_ast != NULL, "Multi-line map with frame references should parse");
    AR_ASSERT(ar_method_ast__get_instruction_count(own_ast) == 1, "Method should contain one instruction");

    // Given a frame with memory, message, and context values
    ar_data_t *own_memory = ar_data__create_map();
    ar_data_t *own_context = ar_data__create_map();
    ar_data_t *own_message = ar_data__create_map();
    AR_ASSERT(own_memory != NULL, "Memory should be created");
    AR_ASSERT(own_context != NULL, "Context should be created");
    AR_ASSERT(own_message != NULL, "Message should be created");
    AR_ASSERT(ar_data__set_map_integer(own_memory, "seed", 3), "Memory seed should be set");
    AR_ASSERT(ar_data__set_map_integer(own_message, "count", 5), "Message count should be set");
    AR_ASSERT(ar_data__set_map_integer(own_context, "limit", 7), "Context limit should be set");
    ar_frame_t *own_frame = ar_frame__create(own_memory, own_context, own_message);
    AR_ASSERT(own_frame != NULL, "Frame should be created");

    // When evaluating the parsed method
    bool result = ar_method_evaluator__evaluate(own_evaluator, own_frame, own_ast);

    // Then frame-root references should keep their ordinary meaning inside the map block
    AR_ASSERT(result == true, "Method evaluation should succeed");
    ar_data_t *ref_profile = ar_data__get_map_data(own_memory, "profile");
    AR_ASSERT(ref_profile != NULL, "Profile map should be stored");
    AR_ASSERT(ar_data__get_type(ref_profile) == AR_DATA_TYPE__MAP, "Profile should be a map");
    AR_ASSERT(ar_data__get_map_integer(ref_profile, "from_memory") == 3, "memory.seed should be read");
    AR_ASSERT(ar_data__get_map_integer(ref_profile, "from_message") == 5, "message.count should be read");
    AR_ASSERT(ar_data__get_map_integer(ref_profile, "from_context") == 7, "context.limit should be read");

    // Cleanup
    ar_frame__destroy(own_frame);
    ar_data__destroy(own_message);
    ar_data__destroy(own_context);
    ar_data__destroy(own_memory);
    ar_method_ast__destroy(own_ast);
    ar_method_evaluator__destroy(own_evaluator);
    ar_method_parser__destroy(own_parser);
    ar_log__destroy(own_log);
    ar_system__destroy(own_system);

    printf("  ✓ Frame references in multi-line map evaluated successfully\n");
}

static void test_method_evaluator__evaluates_local_references_inside_nested_one_line_literals(void) {
    printf("Testing method evaluator with local references inside nested one-line literals...\n");

    // Given a method evaluator with agency dependencies
    ar_system_t *own_system = ar_system__create();
    AR_ASSERT(own_system != NULL, "System should be created");
    ar_agency_t *ref_agency = ar_system__get_agency(own_system);
    ar_delegation_t *ref_delegation = ar_system__get_delegation(own_system);
    AR_ASSERT(ref_agency != NULL, "Agency should exist");
    AR_ASSERT(ref_delegation != NULL, "Delegation should exist");

    // Given method parser and evaluator facades
    ar_log_t *own_log = ar_log__create();
    AR_ASSERT(own_log != NULL, "Log should be created");
    ar_method_parser_t *own_parser = ar_method_parser__create(own_log);
    AR_ASSERT(own_parser != NULL, "Method parser should be created");
    ar_method_evaluator_t *own_evaluator = ar_method_evaluator__create(
        own_log,
        ref_agency,
        ref_delegation
    );
    AR_ASSERT(own_evaluator != NULL, "Method evaluator should be created");

    // Given nested one-line literals inside a multi-line map assignment
    const char *ref_source =
        "memory.profile := {\n"
        "  base := 3\n"
        "  nested_map := {base: 100, copied: .base, doubled: .base * 2}\n"
        "  nested_list := [.base, .base + 1]\n"
        "}\n";
    ar_method_ast_t *own_ast = ar_method_parser__parse(own_parser, ref_source);
    AR_ASSERT(own_ast != NULL, "Nested one-line local references should parse");
    AR_ASSERT(ar_method_ast__get_instruction_count(own_ast) == 1, "Method should contain one instruction");

    // Given an empty frame
    ar_data_t *own_memory = ar_data__create_map();
    ar_data_t *own_context = ar_data__create_map();
    ar_data_t *own_message = ar_data__create_map();
    AR_ASSERT(own_memory != NULL, "Memory should be created");
    AR_ASSERT(own_context != NULL, "Context should be created");
    AR_ASSERT(own_message != NULL, "Message should be created");
    ar_frame_t *own_frame = ar_frame__create(own_memory, own_context, own_message);
    AR_ASSERT(own_frame != NULL, "Frame should be created");

    // When evaluating the parsed method
    bool result = ar_method_evaluator__evaluate(own_evaluator, own_frame, own_ast);

    // Then nested one-line literals should read from the outer multi-line map block
    AR_ASSERT(result == true, "Method evaluation should succeed");
    ar_data_t *ref_profile = ar_data__get_map_data(own_memory, "profile");
    AR_ASSERT(ref_profile != NULL, "Profile map should be stored");
    ar_data_t *ref_nested_map = ar_data__get_map_data(ref_profile, "nested_map");
    AR_ASSERT(ref_nested_map != NULL, "Nested map should be stored");
    AR_ASSERT(ar_data__get_map_integer(ref_nested_map, "base") == 100, "Nested map base should be local data");
    AR_ASSERT(ar_data__get_map_integer(ref_nested_map, "copied") == 3, ".base should read outer block base");
    AR_ASSERT(ar_data__get_map_integer(ref_nested_map, "doubled") == 6, ".base arithmetic should read outer block base");
    ar_data_t *ref_nested_list = ar_data__get_map_data(ref_profile, "nested_list");
    AR_ASSERT(ref_nested_list != NULL, "Nested list should be stored");
    AR_ASSERT(ar_data__list_count(ref_nested_list) == 2, "Nested list should have two items");
    ar_data_t **own_items = ar_data__list_items(ref_nested_list);
    AR_ASSERT(own_items != NULL, "Nested list items should be readable");
    AR_ASSERT(ar_data__get_integer(own_items[0]) == 3, "List .base item should read outer block base");
    AR_ASSERT(ar_data__get_integer(own_items[1]) == 4, "List .base arithmetic item should read outer block base");
    AR__HEAP__FREE(own_items);

    // Cleanup
    ar_frame__destroy(own_frame);
    ar_data__destroy(own_message);
    ar_data__destroy(own_context);
    ar_data__destroy(own_memory);
    ar_method_ast__destroy(own_ast);
    ar_method_evaluator__destroy(own_evaluator);
    ar_method_parser__destroy(own_parser);
    ar_log__destroy(own_log);
    ar_system__destroy(own_system);

    printf("  ✓ Local references inside nested one-line literals evaluated successfully\n");
}

static void test_method_evaluator__memory_stress_test(void) {
    printf("Testing method evaluator memory handling with many instructions...\n");
    
    // Given a system and method evaluator
    ar_system_t *own_system = ar_system__create();
    assert(own_system != NULL);
    ar_agency_t *ref_agency = ar_system__get_agency(own_system);
    assert(ref_agency != NULL);
    ar_delegation_t *ref_delegation = ar_system__get_delegation(own_system);
    assert(ref_delegation != NULL);

    ar_log_t *own_log = ar_log__create();
    ar_data_t *own_memory = ar_data__create_map();
    ar_data_t *own_context = ar_data__create_map();
    ar_method_evaluator_t *own_evaluator = ar_method_evaluator__create(own_log, ref_agency, ref_delegation);
    
    // Create a frame
    ar_data_t *own_message = ar_data__create_string("test message");
    ar_frame_t *own_frame = ar_frame__create(own_memory, own_context, own_message);
    
    // Create a method AST with many instructions
    ar_method_ast_t *own_ast = ar_method_ast__create();
    
    // Add 50 instructions to stress test memory handling
    for (int i = 0; i < 50; i++) {
        char path[32];
        char value[32];
        snprintf(path, sizeof(path), "memory.var%d", i);
        snprintf(value, sizeof(value), "%d", i);
        
        ar_instruction_ast_t *own_instr = ar_instruction_ast__create_assignment(path, value);
        ar_expression_ast_t *own_expr = ar_expression_ast__create_literal_int(i);
        ar_instruction_ast__set_assignment_expression_ast(own_instr, own_expr);
        ar_method_ast__add_instruction(own_ast, own_instr);
    }
    
    // Verify the method has 50 instructions
    assert(ar_method_ast__get_instruction_count(own_ast) == 50);
    
    // When evaluating the method with many instructions
    bool result = ar_method_evaluator__evaluate(own_evaluator, own_frame, own_ast);
    
    // Then it should succeed
    assert(result == true);
    
    // And all values should be stored in memory
    for (int i = 0; i < 50; i++) {
        char key[32];
        snprintf(key, sizeof(key), "var%d", i);
        assert(ar_data__get_map_integer(own_memory, key) == i);
    }
    
    // Cleanup
    ar_method_ast__destroy(own_ast);
    ar_frame__destroy(own_frame);
    ar_method_evaluator__destroy(own_evaluator);
    ar_data__destroy(own_message);
    ar_data__destroy(own_context);
    ar_data__destroy(own_memory);
    ar_log__destroy(own_log);
    ar_system__destroy(own_system);
    
    printf("  ✓ Memory handling verified with 50 instructions\n");
}

int main(void) {
    // Directory check - must be run from bin/*-tests/
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        const char *bin_pos = strstr(cwd, "/bin/");
        if (!bin_pos || !strstr(bin_pos + 5, "-tests")) {
            fprintf(stderr, "ERROR: Tests must be run from a bin/*-tests directory!\n");
            fprintf(stderr, "Current directory: %s\n", cwd);
            fprintf(stderr, "Tests are automatically run by make from the correct directory.\n");
            return 1;
        }
    }
    
    printf("Running method evaluator tests...\n");
    
    test_method_evaluator__create_destroy();
    test_method_evaluator__evaluate_empty_method();
    test_method_evaluator__evaluate_single_instruction_method();
    test_method_evaluator__evaluate_multiple_instructions();
    test_method_evaluator__evaluate_null_parameters();
    test_method_evaluator__evaluate_with_failing_instruction();
    test_method_evaluator__evaluates_parsed_append_instruction();
    test_method_evaluator__evaluates_multiline_map_local_references();
    test_method_evaluator__evaluates_frame_references_in_multiline_map();
    test_method_evaluator__evaluates_local_references_inside_nested_one_line_literals();
    test_method_evaluator__memory_stress_test();
    
    // Check for memory leaks
    ar_heap__memory_report();
    
    printf("All 11 tests passed!\n");
    return 0;
}
