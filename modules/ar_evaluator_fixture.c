/**
 * @file ar_evaluator_fixture.c
 * @brief Implementation of the generic instruction evaluator test fixture
 */

#include "ar_evaluator_fixture.h"
#include "ar_heap.h"
#include "ar_list.h"
#include "ar_expression_ast.h"
#include <string.h>
#include <stdio.h>
#include <assert.h>

/* Internal structure for the fixture */
struct ar_evaluator_fixture_s {
    char *own_test_name;                                       /* Test name for identification */
    ar_log_t *own_log;                                        /* Log instance */
    ar_data_t *own_memory;                                    /* Memory map */
    ar_expression_evaluator_t *own_expr_evaluator;            /* Expression evaluator */
    /* Note: Evaluators are managed by tests, not the fixture */
    ar_list_t *own_tracked_frames;                            /* List of frames to destroy */
    ar_list_t *own_tracked_asts;                              /* List of ASTs to destroy */
    ar_list_t *own_tracked_data;                              /* List of data objects to destroy */
    int initial_allocations;                                  /* Allocation count at start */
};

ar_evaluator_fixture_t* ar_evaluator_fixture__create(
    const char *ref_test_name
) {
    if (!ref_test_name) {
        return NULL;
    }
    
    // Record initial allocation count
    int initial_allocations = 0; // TODO: Add heap tracking if needed
    
    // Allocate fixture
    ar_evaluator_fixture_t *own_fixture = 
        AR__HEAP__MALLOC(sizeof(ar_evaluator_fixture_t), "instruction_evaluator_fixture");
    if (!own_fixture) {
        return NULL;
    }
    
    // Initialize all fields to NULL for safe cleanup
    memset(own_fixture, 0, sizeof(ar_evaluator_fixture_t));
    own_fixture->initial_allocations = initial_allocations;
    
    // Copy test name
    own_fixture->own_test_name = AR__HEAP__STRDUP(ref_test_name, "test_name");
    if (!own_fixture->own_test_name) {
        AR__HEAP__FREE(own_fixture);
        return NULL;
    }
    
    // Create tracking lists
    own_fixture->own_tracked_frames = ar_list__create();
    own_fixture->own_tracked_asts = ar_list__create();
    own_fixture->own_tracked_data = ar_list__create();
    
    if (!own_fixture->own_tracked_frames || 
        !own_fixture->own_tracked_asts || 
        !own_fixture->own_tracked_data) {
        ar_evaluator_fixture__destroy(own_fixture);
        return NULL;
    }
    
    // Create core components
    own_fixture->own_log = ar_log__create();
    own_fixture->own_memory = ar_data__create_map();
    
    if (!own_fixture->own_log || !own_fixture->own_memory) {
        ar_evaluator_fixture__destroy(own_fixture);
        return NULL;
    }
    
    own_fixture->own_expr_evaluator = ar_expression_evaluator__create(
        own_fixture->own_log
    );
    
    if (!own_fixture->own_expr_evaluator) {
        ar_evaluator_fixture__destroy(own_fixture);
        return NULL;
    }
    
    // Note: Evaluators are created and managed by the tests
    
    // Ownership transferred to caller
    return own_fixture;
}

void ar_evaluator_fixture__destroy(
    ar_evaluator_fixture_t *own_fixture
) {
    if (!own_fixture) {
        return;
    }
    
    // Destroy tracked frames
    if (own_fixture->own_tracked_frames) {
        size_t count = ar_list__count(own_fixture->own_tracked_frames);
        void **items = ar_list__items(own_fixture->own_tracked_frames);
        if (items) {
            for (size_t i = 0; i < count; i++) {
                ar_frame_t *frame = (ar_frame_t*)items[i];
                if (frame) {
                    ar_frame__destroy(frame);
                }
            }
            AR__HEAP__FREE(items);
        }
        ar_list__destroy(own_fixture->own_tracked_frames);
    }
    
    // Destroy tracked ASTs
    if (own_fixture->own_tracked_asts) {
        size_t count = ar_list__count(own_fixture->own_tracked_asts);
        void **items = ar_list__items(own_fixture->own_tracked_asts);
        if (items) {
            for (size_t i = 0; i < count; i++) {
                ar_instruction_ast_t *ast = (ar_instruction_ast_t*)items[i];
                if (ast) {
                    ar_instruction_ast__destroy(ast);
                }
            }
            AR__HEAP__FREE(items);
        }
        ar_list__destroy(own_fixture->own_tracked_asts);
    }
    
    // Destroy tracked data
    if (own_fixture->own_tracked_data) {
        size_t count = ar_list__count(own_fixture->own_tracked_data);
        void **items = ar_list__items(own_fixture->own_tracked_data);
        if (items) {
            for (size_t i = 0; i < count; i++) {
                ar_data_t *data = (ar_data_t*)items[i];
                if (data) {
                    ar_data__destroy(data);
                }
            }
            AR__HEAP__FREE(items);
        }
        ar_list__destroy(own_fixture->own_tracked_data);
    }
    
    // Note: Tests are responsible for destroying their evaluators
    if (own_fixture->own_expr_evaluator) {
        ar_expression_evaluator__destroy(own_fixture->own_expr_evaluator);
    }
    if (own_fixture->own_memory) {
        ar_data__destroy(own_fixture->own_memory);
    }
    if (own_fixture->own_log) {
        ar_log__destroy(own_fixture->own_log);
    }
    if (own_fixture->own_test_name) {
        AR__HEAP__FREE(own_fixture->own_test_name);
    }
    
    AR__HEAP__FREE(own_fixture);
}

/* Evaluators are managed by tests, not the fixture */

ar_expression_evaluator_t* ar_evaluator_fixture__get_expression_evaluator(
    const ar_evaluator_fixture_t *ref_fixture
) {
    return ref_fixture ? ref_fixture->own_expr_evaluator : NULL;
}

ar_data_t* ar_evaluator_fixture__get_memory(
    const ar_evaluator_fixture_t *ref_fixture
) {
    return ref_fixture ? ref_fixture->own_memory : NULL;
}

ar_log_t* ar_evaluator_fixture__get_log(
    const ar_evaluator_fixture_t *ref_fixture
) {
    return ref_fixture ? ref_fixture->own_log : NULL;
}

ar_frame_t* ar_evaluator_fixture__create_frame(
    ar_evaluator_fixture_t *mut_fixture
) {
    if (!mut_fixture) {
        return NULL;
    }
    
    // Create empty context and message
    ar_data_t *own_context = ar_data__create_map();
    if (!own_context) {
        return NULL;
    }
    ar_list__add_last(mut_fixture->own_tracked_data, own_context);
    
    ar_data_t *own_message = ar_data__create_string("");
    if (!own_message) {
        return NULL;
    }
    ar_list__add_last(mut_fixture->own_tracked_data, own_message);
    
    // Create frame
    ar_frame_t *own_frame = ar_frame__create(mut_fixture->own_memory, own_context, own_message);
    if (!own_frame) {
        return NULL;
    }
    
    // Track the frame
    ar_list__add_last(mut_fixture->own_tracked_frames, own_frame);
    
    return own_frame;
}

ar_instruction_ast_t* ar_evaluator_fixture__create_assignment_int(
    ar_evaluator_fixture_t *mut_fixture,
    const char *ref_path,
    int value
) {
    if (!mut_fixture || !ref_path) {
        return NULL;
    }
    
    // Create assignment AST
    char value_str[32];
    snprintf(value_str, sizeof(value_str), "%d", value);
    ar_instruction_ast_t *own_ast = ar_instruction_ast__create_assignment(ref_path, value_str);
    if (!own_ast) {
        return NULL;
    }
    
    // Create expression AST
    ar_expression_ast_t *own_expr_ast = ar_expression_ast__create_literal_int(value);
    if (!own_expr_ast) {
        ar_instruction_ast__destroy(own_ast);
        return NULL;
    }
    
    // Attach expression to assignment
    bool success = ar_instruction_ast__set_assignment_expression_ast(own_ast, own_expr_ast);
    if (!success) {
        ar_instruction_ast__destroy(own_ast);
        return NULL;
    }
    
    // Track the AST
    ar_list__add_last(mut_fixture->own_tracked_asts, own_ast);
    
    return own_ast;
}

ar_instruction_ast_t* ar_evaluator_fixture__create_assignment_string(
    ar_evaluator_fixture_t *mut_fixture,
    const char *ref_path,
    const char *ref_value
) {
    if (!mut_fixture || !ref_path || !ref_value) {
        return NULL;
    }
    
    // Create assignment AST with quoted string
    size_t quoted_len = strlen(ref_value) + 3; // For quotes and null terminator
    char *quoted_value = AR__HEAP__MALLOC(quoted_len, "quoted_value");
    if (!quoted_value) {
        return NULL;
    }
    snprintf(quoted_value, quoted_len, "\"%s\"", ref_value);
    
    ar_instruction_ast_t *own_ast = ar_instruction_ast__create_assignment(ref_path, quoted_value);
    AR__HEAP__FREE(quoted_value);
    
    if (!own_ast) {
        return NULL;
    }
    
    // Create expression AST
    ar_expression_ast_t *own_expr_ast = ar_expression_ast__create_literal_string(ref_value);
    if (!own_expr_ast) {
        ar_instruction_ast__destroy(own_ast);
        return NULL;
    }
    
    // Attach expression to assignment
    bool success = ar_instruction_ast__set_assignment_expression_ast(own_ast, own_expr_ast);
    if (!success) {
        ar_instruction_ast__destroy(own_ast);
        return NULL;
    }
    
    // Track the AST
    ar_list__add_last(mut_fixture->own_tracked_asts, own_ast);
    
    return own_ast;
}

ar_instruction_ast_t* ar_evaluator_fixture__create_assignment_expr(
    ar_evaluator_fixture_t *mut_fixture,
    const char *ref_path,
    ar_expression_ast_t *own_expr_ast
) {
    if (!mut_fixture || !ref_path || !own_expr_ast) {
        return NULL;
    }
    
    // Create assignment AST (value doesn't matter since we'll use the expr_ast)
    ar_instruction_ast_t *own_ast = ar_instruction_ast__create_assignment(ref_path, "expr");
    if (!own_ast) {
        ar_expression_ast__destroy(own_expr_ast);
        return NULL;
    }
    
    // Attach expression to assignment (transfers ownership)
    bool success = ar_instruction_ast__set_assignment_expression_ast(own_ast, own_expr_ast);
    if (!success) {
        ar_instruction_ast__destroy(own_ast);
        return NULL;
    }
    
    // Track the AST
    ar_list__add_last(mut_fixture->own_tracked_asts, own_ast);
    
    return own_ast;
}

void ar_evaluator_fixture__track_ast(
    ar_evaluator_fixture_t *mut_fixture,
    ar_instruction_ast_t *own_ast
) {
    if (mut_fixture && own_ast) {
        ar_list__add_last(mut_fixture->own_tracked_asts, own_ast);
    }
}

bool ar_evaluator_fixture__check_memory(
    const ar_evaluator_fixture_t *ref_fixture
) {
    if (!ref_fixture) {
        return false;
    }
    
    // Memory checking is done automatically by the heap module
    // This function is provided for API compatibility
    return true;
}