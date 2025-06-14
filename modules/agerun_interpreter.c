#include "agerun_interpreter.h"
#include "agerun_heap.h"
#include "agerun_agency.h"
#include "agerun_agent.h"
#include "agerun_string.h"
#include "agerun_data.h"
#include "agerun_expression.h"
#include "agerun_map.h"
#include "agerun_methodology.h"
#include "agerun_assert.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

/**
 * Interpreter structure (private implementation)
 */
struct interpreter_s {
    // Future: Could add configuration options, optimization flags, etc.
    // For now, keeping it simple as we extract functionality
    int placeholder; // C doesn't allow empty structs
};

// Forward declarations for execution functions
static bool _execute_assignment(interpreter_t *mut_interpreter, instruction_context_t *mut_context, const parsed_instruction_t *ref_parsed);
static bool _execute_send(interpreter_t *mut_interpreter, instruction_context_t *mut_context, const parsed_instruction_t *ref_parsed);
static bool _execute_if(interpreter_t *mut_interpreter, instruction_context_t *mut_context, const parsed_instruction_t *ref_parsed);
static bool _execute_parse(interpreter_t *mut_interpreter, instruction_context_t *mut_context, const parsed_instruction_t *ref_parsed);
static bool _execute_build(interpreter_t *mut_interpreter, instruction_context_t *mut_context, const parsed_instruction_t *ref_parsed);
static bool _execute_method(interpreter_t *mut_interpreter, instruction_context_t *mut_context, const parsed_instruction_t *ref_parsed);
static bool _execute_agent(interpreter_t *mut_interpreter, instruction_context_t *mut_context, const parsed_instruction_t *ref_parsed);
static bool _execute_destroy(interpreter_t *mut_interpreter, instruction_context_t *mut_context, const parsed_instruction_t *ref_parsed);

// Helper function to send messages
static bool _send_message(int64_t target_id, data_t *own_message);

/**
 * Creates a new interpreter instance
 */
interpreter_t* ar__interpreter__create(void) {
    interpreter_t *own_interpreter = AR__HEAP__MALLOC(sizeof(interpreter_t), "interpreter");
    if (!own_interpreter) {
        return NULL;
    }
    
    own_interpreter->placeholder = 0;
    
    return own_interpreter;
    // Ownership transferred to caller
}

/**
 * Destroys an interpreter instance and frees its resources
 */
void ar__interpreter__destroy(interpreter_t *own_interpreter) {
    if (!own_interpreter) {
        return;
    }
    
    AR__HEAP__FREE(own_interpreter);
}

/**
 * Executes a single instruction in the given context
 */
bool ar__interpreter__execute_instruction(interpreter_t *mut_interpreter, 
                                         instruction_context_t *mut_context, 
                                         const char *ref_instruction) {
    if (!mut_interpreter || !mut_context || !ref_instruction) {
        return false;
    }
    
    // Parse the instruction to get an AST
    parsed_instruction_t *own_parsed = ar__instruction__parse(ref_instruction, mut_context);
    if (!own_parsed) {
        return false;
    }
    
    // Execute based on instruction type
    bool result = false;
    instruction_type_t type = ar__instruction__get_type(own_parsed);
    
    fprintf(stderr, "DEBUG: Instruction type: %d\n", type);
    
    switch (type) {
        case INST_ASSIGNMENT:
            result = _execute_assignment(mut_interpreter, mut_context, own_parsed);
            break;
        case INST_SEND:
            result = _execute_send(mut_interpreter, mut_context, own_parsed);
            fprintf(stderr, "DEBUG: _execute_send returned: %s\n", result ? "true" : "false");
            break;
        case INST_IF:
            result = _execute_if(mut_interpreter, mut_context, own_parsed);
            break;
        case INST_PARSE:
            result = _execute_parse(mut_interpreter, mut_context, own_parsed);
            break;
        case INST_BUILD:
            result = _execute_build(mut_interpreter, mut_context, own_parsed);
            break;
        case INST_METHOD:
            result = _execute_method(mut_interpreter, mut_context, own_parsed);
            break;
        case INST_AGENT:
            result = _execute_agent(mut_interpreter, mut_context, own_parsed);
            break;
        case INST_DESTROY:
            result = _execute_destroy(mut_interpreter, mut_context, own_parsed);
            break;
        default:
            // Unknown instruction type
            result = false;
            break;
    }
    
    // Clean up parsed instruction
    ar__instruction__destroy_parsed(own_parsed);
    
    return result;
}

/**
 * Executes a method in the context of an agent
 */
bool ar__interpreter__execute_method(interpreter_t *mut_interpreter,
                                    int64_t agent_id, 
                                    const data_t *ref_message, 
                                    const method_t *ref_method) {
    if (!mut_interpreter || !ref_method) {
        return false;
    }
    
    // Get agent memory and context
    data_t *mut_memory = ar__agency__get_agent_mutable_memory(agent_id);
    const data_t *ref_context = ar__agency__get_agent_context(agent_id);
    
    if (!mut_memory) {
        fprintf(stderr, "DEBUG: Agent %lld has no memory\n", (long long)agent_id);
        return false;
    }
    
    // Context can be NULL - that's valid
    fprintf(stderr, "DEBUG: execute_method - agent %lld, memory=%p, context=%p, message=%p\n", 
            (long long)agent_id, (void*)mut_memory, (const void*)ref_context, (const void*)ref_message);
    
    // Create an instruction context
    instruction_context_t *own_ctx = ar__instruction__create_context(
        mut_memory,
        ref_context,
        ref_message
    );
    
    if (!own_ctx) {
        return false;
    }
    
    // Get method instructions
    const char *ref_instructions = ar__method__get_instructions(ref_method);
    if (!ref_instructions) {
        ar__instruction__destroy_context(own_ctx);
        return false;
    }
    
    // Make a copy of the instructions for tokenization
    char *own_instructions_copy = AR__HEAP__STRDUP(ref_instructions, "Method instructions copy");
    if (!own_instructions_copy) {
        ar__instruction__destroy_context(own_ctx);
        return false;
    }
    
    // Split instructions by newlines and execute each one
    char *mut_instruction = strtok(own_instructions_copy, "\n");
    bool result = true;
    
    while (mut_instruction != NULL) {
        mut_instruction = ar__string__trim(mut_instruction);
        
        // Skip empty lines and comments
        if (strlen(mut_instruction) > 0 && mut_instruction[0] != '#') {
            if (!ar__interpreter__execute_instruction(mut_interpreter, own_ctx, mut_instruction)) {
                result = false;
                break;
            }
        }
        
        mut_instruction = strtok(NULL, "\n");
    }
    
    // Clean up
    ar__instruction__destroy_context(own_ctx);
    AR__HEAP__FREE(own_instructions_copy);
    own_instructions_copy = NULL; // Mark as freed
    
    return result;
}

// Helper function to send messages
static bool _send_message(int64_t target_id, data_t *own_message) {
    if (target_id == 0) {
        // Special case: agent_id 0 is a no-op that always returns true
        ar__data__destroy(own_message);
        return true;
    }
    
    // Send message (ownership of own_message is transferred to ar__agency__send_to_agent)
    return ar__agency__send_to_agent(target_id, own_message);
}

// Execute an assignment instruction
static bool _execute_assignment(interpreter_t *mut_interpreter, instruction_context_t *mut_context, const parsed_instruction_t *ref_parsed) {
    (void)mut_interpreter; // Unused for now
    
    // Get assignment details
    const char *ref_path = ar__instruction__get_assignment_path(ref_parsed);
    const char *ref_expression = ar__instruction__get_assignment_expression(ref_parsed);
    
    if (!ref_path || !ref_expression) {
        return false;
    }
    
    // Get the memory from context
    data_t *mut_memory = ar__instruction__get_memory(mut_context);
    const data_t *ref_context_data = ar__instruction__get_context(mut_context);
    const data_t *ref_message = ar__instruction__get_message(mut_context);
    
    // Create expression context
    expression_context_t *own_expr_ctx = ar__expression__create_context(
        mut_memory, ref_context_data, ref_message, ref_expression
    );
    if (!own_expr_ctx) {
        return false;
    }
    
    // Evaluate the expression and take ownership (old code always did this)
    const data_t *ref_result = ar__expression__evaluate(own_expr_ctx);
    if (!ref_result) {
        fprintf(stderr, "DEBUG: Expression evaluation returned NULL for: %s\n", ref_expression);
        ar__expression__destroy_context(own_expr_ctx);
        return false;
    }
    
    // Debug: Check what type we got
    fprintf(stderr, "DEBUG: Expression evaluated to type %d\n", ar__data__get_type(ref_result));
    if (ar__data__get_type(ref_result) == DATA_STRING) {
        fprintf(stderr, "DEBUG: Expression result string: '%s'\n", ar__data__get_string(ref_result));
    }
    
    data_t *own_value = ar__expression__take_ownership(own_expr_ctx, ref_result);
    if (!own_value) {
        fprintf(stderr, "DEBUG: Failed to take ownership of expression result (this matches old behavior)\n");
        ar__expression__destroy_context(own_expr_ctx);
        return false;
    }
    
    // The path from parse_memory_access already has "memory." stripped
    // So ref_path is like "x" or "x.y.z"
    // ar__data__set_map_data supports deep paths like "x.y.z"
    bool success = false;
    
    if (own_value) {
        // Store result in agent's memory (transfers ownership of value)
        success = ar__data__set_map_data(mut_memory, ref_path, own_value);
        if (!success) {
            ar__data__destroy(own_value);
        }
    }
    
    // Clean up
    ar__expression__destroy_context(own_expr_ctx);
    
    return success;
}

// Execute a send instruction
static bool _execute_send(interpreter_t *mut_interpreter, instruction_context_t *mut_context, const parsed_instruction_t *ref_parsed) {
    (void)mut_interpreter; // Unused for now
    
    // Get function call details
    const char *ref_function_name;
    const char **ref_args;
    int arg_count;
    const char *ref_result_path;
    
    if (!ar__instruction__get_function_call(ref_parsed, &ref_function_name, &ref_args, &arg_count, &ref_result_path)) {
        return false;
    }
    
    fprintf(stderr, "DEBUG: _execute_send - result_path: %s\n", ref_result_path ? ref_result_path : "NULL");
    
    // Verify this is a send function with 2 arguments
    if (strcmp(ref_function_name, "send") != 0 || arg_count != 2) {
        return false;
    }
    
    // Get context data
    data_t *mut_memory = ar__instruction__get_memory(mut_context);
    const data_t *ref_context_data = ar__instruction__get_context(mut_context);
    const data_t *ref_message = ar__instruction__get_message(mut_context);
    
    // Evaluate first argument (agent ID)
    expression_context_t *own_expr_ctx = ar__expression__create_context(
        mut_memory, ref_context_data, ref_message, ref_args[0]
    );
    if (!own_expr_ctx) {
        return false;
    }
    
    const data_t *ref_agent_id_data = ar__expression__evaluate(own_expr_ctx);
    data_t *own_agent_id = ar__expression__take_ownership(own_expr_ctx, ref_agent_id_data);
    
    // Clean up context immediately
    ar__expression__destroy_context(own_expr_ctx);
    
    if (!own_agent_id) {
        return false;
    }
    
    // Extract agent_id
    int64_t agent_id = 0;
    if (ar__data__get_type(own_agent_id) == DATA_INTEGER) {
        agent_id = (int64_t)ar__data__get_integer(own_agent_id);
    }
    
    ar__data__destroy(own_agent_id);
    
    // Evaluate second argument (message)
    printf("DEBUG: Evaluating send arg[1]: '%s'\n", ref_args[1]);
    own_expr_ctx = ar__expression__create_context(
        mut_memory, ref_context_data, ref_message, ref_args[1]
    );
    if (!own_expr_ctx) {
        printf("DEBUG: Failed to create expression context for: '%s'\n", ref_args[1]);
        return false;
    }
    
    const data_t *ref_msg_data = ar__expression__evaluate(own_expr_ctx);
    data_t *own_msg = ar__expression__take_ownership(own_expr_ctx, ref_msg_data);
    
    // Clean up context immediately
    ar__expression__destroy_context(own_expr_ctx);
    
    if (!own_msg) {
        fprintf(stderr, "DEBUG: Failed to evaluate message expression\n");
        return false;
    }
    
    // Send the message (ownership transferred)
    bool send_result = _send_message(agent_id, own_msg);
    fprintf(stderr, "DEBUG: _send_message(%lld, ...) returned: %s\n", (long long)agent_id, send_result ? "true" : "false");
    
    // If there's a result path, store the result
    if (ref_result_path) {
        data_t *own_result = ar__data__create_integer(send_result ? 1 : 0);
        // The result_path from parsing already has "memory." stripped
        ar__data__set_map_data(mut_memory, ref_result_path, own_result);
        fprintf(stderr, "DEBUG: Stored result in memory.%s, returning true for assignment\n", ref_result_path);
        // For assignments, return true to indicate the instruction succeeded
        return true;
    }
    
    return send_result;
}

// Execute an if instruction
static bool _execute_if(interpreter_t *mut_interpreter, instruction_context_t *mut_context, const parsed_instruction_t *ref_parsed) {
    (void)mut_interpreter; // Unused for now
    
    // Get function call details
    const char *ref_function_name;
    const char **ref_args;
    int arg_count;
    const char *ref_result_path;
    
    if (!ar__instruction__get_function_call(ref_parsed, &ref_function_name, &ref_args, &arg_count, &ref_result_path)) {
        return false;
    }
    
    // Verify this is an if function with 3 arguments
    if (strcmp(ref_function_name, "if") != 0 || arg_count != 3) {
        return false;
    }
    
    // Get context data
    data_t *mut_memory = ar__instruction__get_memory(mut_context);
    const data_t *ref_context_data = ar__instruction__get_context(mut_context);
    const data_t *ref_message = ar__instruction__get_message(mut_context);
    
    // Evaluate condition
    expression_context_t *own_expr_ctx = ar__expression__create_context(
        mut_memory, ref_context_data, ref_message, ref_args[0]
    );
    if (!own_expr_ctx) {
        return false;
    }
    
    const data_t *ref_cond_eval = ar__expression__evaluate(own_expr_ctx);
    data_t *own_cond = ar__expression__take_ownership(own_expr_ctx, ref_cond_eval);
    
    // Clean up context immediately
    ar__expression__destroy_context(own_expr_ctx);
    
    // Handle both owned values and references
    const data_t *cond_to_use = own_cond ? own_cond : ref_cond_eval;
    if (!cond_to_use) {
        return false;
    }
    
    // Evaluate true_value expression
    own_expr_ctx = ar__expression__create_context(
        mut_memory, ref_context_data, ref_message, ref_args[1]
    );
    if (!own_expr_ctx) {
        if (own_cond) {
            ar__data__destroy(own_cond);
        }
        return false;
    }
    
    const data_t *ref_true_eval = ar__expression__evaluate(own_expr_ctx);
    data_t *own_true = ar__expression__take_ownership(own_expr_ctx, ref_true_eval);
    
    // Clean up context immediately
    ar__expression__destroy_context(own_expr_ctx);
    
    // Handle both owned values and references
    const data_t *true_to_use = own_true ? own_true : ref_true_eval;
    if (!true_to_use) {
        if (own_cond) {
            ar__data__destroy(own_cond);
        }
        return false;
    }
    
    // Evaluate false_value expression
    own_expr_ctx = ar__expression__create_context(
        mut_memory, ref_context_data, ref_message, ref_args[2]
    );
    if (!own_expr_ctx) {
        if (own_cond) {
            ar__data__destroy(own_cond);
        }
        if (own_true) {
            ar__data__destroy(own_true);
        }
        return false;
    }
    
    const data_t *ref_false_eval = ar__expression__evaluate(own_expr_ctx);
    data_t *own_false = ar__expression__take_ownership(own_expr_ctx, ref_false_eval);
    
    // Clean up context immediately
    ar__expression__destroy_context(own_expr_ctx);
    
    // Handle both owned values and references
    const data_t *false_to_use = own_false ? own_false : ref_false_eval;
    if (!false_to_use) {
        if (own_cond) {
            ar__data__destroy(own_cond);
        }
        if (own_true) {
            ar__data__destroy(own_true);
        }
        return false;
    }
    
    // Check condition - any non-zero integer or non-empty string is true
    bool condition_is_true = false;
    if (ar__data__get_type(cond_to_use) == DATA_INTEGER) {
        condition_is_true = (ar__data__get_integer(cond_to_use) != 0);
    } else if (ar__data__get_type(cond_to_use) == DATA_STRING) {
        const char *str = ar__data__get_string(cond_to_use);
        condition_is_true = (str && strlen(str) > 0);
    } else if (ar__data__get_type(cond_to_use) == DATA_DOUBLE) {
        condition_is_true = (ar__data__get_double(cond_to_use) != 0.0);
    }
    
    // Create result based on condition
    data_t *own_result = NULL;
    if (condition_is_true) {
        // If we own the true value, transfer ownership
        if (own_true) {
            own_result = own_true;
            own_true = NULL; // Mark as transferred
        } else {
            // Create a copy of the reference
            if (ar__data__get_type(true_to_use) == DATA_INTEGER) {
                own_result = ar__data__create_integer(ar__data__get_integer(true_to_use));
            } else if (ar__data__get_type(true_to_use) == DATA_DOUBLE) {
                own_result = ar__data__create_double(ar__data__get_double(true_to_use));
            } else if (ar__data__get_type(true_to_use) == DATA_STRING) {
                own_result = ar__data__create_string(ar__data__get_string(true_to_use));
            } else {
                // For maps and other types, we can't easily copy, so return 0
                own_result = ar__data__create_integer(0);
            }
        }
        // Clean up false value if we own it
        if (own_false) {
            ar__data__destroy(own_false);
        }
    } else {
        // If we own the false value, transfer ownership
        if (own_false) {
            own_result = own_false;
            own_false = NULL; // Mark as transferred
        } else {
            // Create a copy of the reference
            if (ar__data__get_type(false_to_use) == DATA_INTEGER) {
                own_result = ar__data__create_integer(ar__data__get_integer(false_to_use));
            } else if (ar__data__get_type(false_to_use) == DATA_DOUBLE) {
                own_result = ar__data__create_double(ar__data__get_double(false_to_use));
            } else if (ar__data__get_type(false_to_use) == DATA_STRING) {
                own_result = ar__data__create_string(ar__data__get_string(false_to_use));
            } else {
                // For maps and other types, we can't easily copy, so return 0
                own_result = ar__data__create_integer(0);
            }
        }
        // Clean up true value if we own it
        if (own_true) {
            ar__data__destroy(own_true);
        }
    }
    
    // Clean up condition if we own it
    if (own_cond) {
        ar__data__destroy(own_cond);
    }
    
    // Store result if there's a result path
    if (ref_result_path && own_result) {
        ar__data__set_map_data(mut_memory, ref_result_path, own_result);
    } else if (own_result) {
        ar__data__destroy(own_result);
    }
    
    return true;
}

// Execute a parse instruction
static bool _execute_parse(interpreter_t *mut_interpreter, instruction_context_t *mut_context, const parsed_instruction_t *ref_parsed) {
    (void)mut_interpreter; // Unused for now
    
    // Get function call details
    const char *ref_function_name;
    const char **ref_args;
    int arg_count;
    const char *ref_result_path;
    
    if (!ar__instruction__get_function_call(ref_parsed, &ref_function_name, &ref_args, &arg_count, &ref_result_path)) {
        return false;
    }
    
    // Verify this is a parse function with 2 arguments
    if (strcmp(ref_function_name, "parse") != 0 || arg_count != 2) {
        return false;
    }
    
    // Get context data
    data_t *mut_memory = ar__instruction__get_memory(mut_context);
    const data_t *ref_context_data = ar__instruction__get_context(mut_context);
    const data_t *ref_message = ar__instruction__get_message(mut_context);
    
    // Evaluate template argument
    expression_context_t *own_expr_ctx = ar__expression__create_context(
        mut_memory, ref_context_data, ref_message, ref_args[0]
    );
    if (!own_expr_ctx) {
        return false;
    }
    
    const data_t *ref_eval_result = ar__expression__evaluate(own_expr_ctx);
    data_t *own_template = ar__expression__take_ownership(own_expr_ctx, ref_eval_result);
    
    // Clean up context after getting the value
    ar__expression__destroy_context(own_expr_ctx);
    
    // Get template string
    const char *template_str = NULL;
    bool owns_template = (own_template != NULL);
    
    if (owns_template) {
        // We own the value
        if (ar__data__get_type(own_template) != DATA_STRING) {
            ar__data__destroy(own_template);
            return false;
        }
        template_str = ar__data__get_string(own_template);
    } else {
        // It's a reference - use the evaluation result directly
        if (!ref_eval_result || ar__data__get_type(ref_eval_result) != DATA_STRING) {
            return false;
        }
        template_str = ar__data__get_string(ref_eval_result);
    }
    
    // Evaluate input argument
    own_expr_ctx = ar__expression__create_context(
        mut_memory, ref_context_data, ref_message, ref_args[1]
    );
    if (!own_expr_ctx) {
        if (owns_template && own_template) {
            ar__data__destroy(own_template);
        }
        return false;
    }
    
    const data_t *ref_input_eval = ar__expression__evaluate(own_expr_ctx);
    data_t *own_input = ar__expression__take_ownership(own_expr_ctx, ref_input_eval);
    
    // Clean up context immediately
    ar__expression__destroy_context(own_expr_ctx);
    
    // Handle both owned values and references for input
    const char *input_str = NULL;
    bool owns_input = (own_input != NULL);
    
    if (owns_input) {
        // We own the value
        if (ar__data__get_type(own_input) != DATA_STRING) {
            ar__data__destroy(own_input);
            if (owns_template && own_template) {
                ar__data__destroy(own_template);
            }
            return false;
        }
        input_str = ar__data__get_string(own_input);
    } else {
        // It's a reference - use the evaluation result directly
        if (!ref_input_eval || ar__data__get_type(ref_input_eval) != DATA_STRING) {
            if (owns_template && own_template) {
                ar__data__destroy(own_template);
            }
            return false;
        }
        input_str = ar__data__get_string(ref_input_eval);
    }
    
    // Create result map (owned by us)
    data_t *own_result = ar__data__create_map();
    if (!own_result) {
        if (owns_input && own_input) {
            ar__data__destroy(own_input);
        }
        if (owns_template && own_template) {
            ar__data__destroy(own_template);
        }
        return false;
    }
    
    // Parse the template and input to extract values
    // Template format: "key1={var1}, key2={var2}"
    // Input format: "key1=value1, key2=value2"
    
    // Simple implementation: look for {variable} patterns in template
    const char *template_ptr = template_str;
    const char *input_ptr = input_str;
    
    while (*template_ptr) {
        // Look for {variable} pattern
        const char *var_start = strchr(template_ptr, '{');
        if (var_start) {
            const char *var_end = strchr(var_start + 1, '}');
            if (var_end) {
                // Extract variable name
                size_t var_len = (size_t)(var_end - var_start - 1);
                char *var_name = AR__HEAP__MALLOC(var_len + 1, "Parse variable name");
                if (!var_name) {
                    ar__data__destroy(own_result);
                    if (owns_input && own_input) {
                        ar__data__destroy(own_input);
                    }
                    if (owns_template && own_template) {
                        ar__data__destroy(own_template);
                    }
                    return false;
                }
                
                strncpy(var_name, var_start + 1, var_len);
                var_name[var_len] = '\0';
                
                // Get the literal text between current position and {
                size_t literal_len = (size_t)(var_start - template_ptr);
                
                // Find this literal in the input
                if (literal_len > 0) {
                    // Extract the literal text from template
                    char *literal_text = AR__HEAP__MALLOC(literal_len + 1, "Parse literal");
                    if (!literal_text) {
                        ar__data__destroy(own_result);
                        AR__HEAP__FREE(var_name);
                        if (owns_input && own_input) {
                            ar__data__destroy(own_input);
                        }
                        if (owns_template && own_template) {
                            ar__data__destroy(own_template);
                        }
                        return false;
                    }
                    strncpy(literal_text, template_ptr, literal_len);
                    literal_text[literal_len] = '\0';
                    
                    // Look for the literal text in input
                    const char *literal_pos = strstr(input_ptr, literal_text);
                    if (literal_pos && literal_pos == input_ptr) {
                        // Move input pointer past the literal
                        input_ptr += literal_len;
                    }
                    AR__HEAP__FREE(literal_text);
                }
                
                // Now extract the value until we hit the next literal or end
                const char *next_literal_start = var_end + 1;
                size_t next_literal_len = 0;
                
                // Find next literal (non-variable) text
                const char *next_var_start = strchr(next_literal_start, '{');
                if (next_var_start) {
                    next_literal_len = (size_t)(next_var_start - next_literal_start);
                } else {
                    next_literal_len = strlen(next_literal_start);
                }
                
                // Extract value from input
                const char *value_end;
                if (next_literal_len > 0) {
                    // Extract the next literal text
                    char *next_literal = AR__HEAP__MALLOC(next_literal_len + 1, "Parse next literal");
                    if (!next_literal) {
                        ar__data__destroy(own_result);
                        AR__HEAP__FREE(var_name);
                        if (owns_input && own_input) {
                            ar__data__destroy(own_input);
                        }
                        if (owns_template && own_template) {
                            ar__data__destroy(own_template);
                        }
                        return false;
                    }
                    strncpy(next_literal, next_literal_start, next_literal_len);
                    next_literal[next_literal_len] = '\0';
                    
                    // Find where the next literal starts in input
                    const char *next_literal_in_input = strstr(input_ptr, next_literal);
                    if (next_literal_in_input) {
                        value_end = next_literal_in_input;
                    } else {
                        value_end = input_ptr + strlen(input_ptr);
                    }
                    AR__HEAP__FREE(next_literal);
                } else {
                    // No more literals, take rest of input
                    value_end = input_ptr + strlen(input_ptr);
                }
                
                // Extract the value
                if (value_end > input_ptr) {
                    size_t value_len = (size_t)(value_end - input_ptr);
                    char *value_str = AR__HEAP__MALLOC(value_len + 1, "Parse value");
                    if (value_str) {
                        strncpy(value_str, input_ptr, value_len);
                        value_str[value_len] = '\0';
                        
                        // Try to parse as integer first, then double, then string
                        data_t *own_value = NULL;
                        char *endptr;
                        
                        // Try integer
                        long int_val = strtol(value_str, &endptr, 10);
                        if (*endptr == '\0' && value_str[0] != '\0') {
                            own_value = ar__data__create_integer((int)int_val);
                        } else {
                            // Try double
                            double double_val = strtod(value_str, &endptr);
                            if (*endptr == '\0' && value_str[0] != '\0' && strchr(value_str, '.')) {
                                own_value = ar__data__create_double(double_val);
                            } else {
                                // Use as string
                                own_value = ar__data__create_string(value_str);
                            }
                        }
                        
                        // Store in result map
                        if (own_value) {
                            ar__data__set_map_data(own_result, var_name, own_value);
                            // Ownership of own_value is transferred
                        }
                        
                        AR__HEAP__FREE(value_str);
                    }
                    
                    // Move input pointer past the value
                    input_ptr = value_end;
                } else {
                    // Could not find the next literal - parsing failed
                    ar__data__destroy(own_result);
                    own_result = ar__data__create_map();
                    if (!own_result) {
                        AR__HEAP__FREE(var_name);
                        if (owns_input && own_input) {
                            ar__data__destroy(own_input);
                        }
                        if (owns_template && own_template) {
                            ar__data__destroy(own_template);
                        }
                        return false;
                    }
                    AR__HEAP__FREE(var_name);
                    break;
                }
                
                AR__HEAP__FREE(var_name);
                
                // Move template pointer past the variable
                template_ptr = var_end + 1;
            } else {
                // No closing }, stop parsing
                break;
            }
        } else {
            // No more variables
            break;
        }
    }
    
    // Clean up input data
    if (owns_input && own_input) {
        ar__data__destroy(own_input);
    }
    if (owns_template && own_template) {
        ar__data__destroy(own_template);
    }
    
    // Store result if there's a result path
    if (ref_result_path) {
        ar__data__set_map_data(mut_memory, ref_result_path, own_result);
    } else {
        ar__data__destroy(own_result);
    }
    
    return true;
}

// Execute a build instruction
static bool _execute_build(interpreter_t *mut_interpreter, instruction_context_t *mut_context, const parsed_instruction_t *ref_parsed) {
    (void)mut_interpreter; // Unused for now
    
    // Get function call details
    const char *ref_function_name;
    const char **ref_args;
    int arg_count;
    const char *ref_result_path;
    
    if (!ar__instruction__get_function_call(ref_parsed, &ref_function_name, &ref_args, &arg_count, &ref_result_path)) {
        return false;
    }
    
    // Verify this is a build function with 2 arguments
    if (strcmp(ref_function_name, "build") != 0 || arg_count != 2) {
        return false;
    }
    
    // Get context data
    data_t *mut_memory = ar__instruction__get_memory(mut_context);
    const data_t *ref_context_data = ar__instruction__get_context(mut_context);
    const data_t *ref_message = ar__instruction__get_message(mut_context);
    
    // Evaluate template argument
    expression_context_t *own_expr_ctx = ar__expression__create_context(
        mut_memory, ref_context_data, ref_message, ref_args[0]
    );
    if (!own_expr_ctx) {
        return false;
    }
    
    const data_t *ref_eval_result = ar__expression__evaluate(own_expr_ctx);
    data_t *own_template = ar__expression__take_ownership(own_expr_ctx, ref_eval_result);
    
    // Clean up context after getting the value
    ar__expression__destroy_context(own_expr_ctx);
    
    // Get template string
    const char *template_str = NULL;
    bool owns_template = (own_template != NULL);
    
    if (owns_template) {
        // We own the value
        if (ar__data__get_type(own_template) != DATA_STRING) {
            ar__data__destroy(own_template);
            return false;
        }
        template_str = ar__data__get_string(own_template);
    } else {
        // It's a reference - use the evaluation result directly
        if (!ref_eval_result || ar__data__get_type(ref_eval_result) != DATA_STRING) {
            return false;
        }
        template_str = ar__data__get_string(ref_eval_result);
    }
    
    // Evaluate values argument (should be a map)
    own_expr_ctx = ar__expression__create_context(
        mut_memory, ref_context_data, ref_message, ref_args[1]
    );
    if (!own_expr_ctx) {
        if (owns_template && own_template) {
            ar__data__destroy(own_template);
        }
        return false;
    }
    
    const data_t *ref_values = ar__expression__evaluate(own_expr_ctx);
    
    // Try to take ownership. If it fails, the value is a reference to existing data
    data_t *own_values = ar__expression__take_ownership(own_expr_ctx, ref_values);
    
    // Clean up context immediately
    ar__expression__destroy_context(own_expr_ctx);
    
    // Use ref_values if we couldn't take ownership
    const data_t *values_to_use = own_values ? own_values : ref_values;
    
    // Ensure values is a map
    if (ar__data__get_type(values_to_use) != DATA_MAP) {
        if (own_values) {
            ar__data__destroy(own_values);
        }
        if (owns_template && own_template) {
            ar__data__destroy(own_template);
        }
        return false;
    }
    
    // Build the string by replacing placeholders in template
    // Template format: "Hello {name}, you are {age} years old"
    // Values: map with keys "name" and "age"
    
    // Create a string builder for the result
    size_t result_size = strlen(template_str) * 2 + 256; // Start with a reasonable size
    char *own_result_str = AR__HEAP__MALLOC(result_size, "Build result string");
    if (!own_result_str) {
        if (own_values) {
            ar__data__destroy(own_values);
        }
        if (owns_template && own_template) {
            ar__data__destroy(own_template);
        }
        return false;
    }
    
    size_t result_pos = 0;
    const char *template_ptr = template_str;
    
    while (*template_ptr) {
        if (*template_ptr == '{') {
            // Look for the closing brace
            const char *placeholder_end = strchr(template_ptr + 1, '}');
            if (placeholder_end) {
                // Extract variable name
                size_t var_len = (size_t)(placeholder_end - template_ptr - 1);
                char *var_name = AR__HEAP__MALLOC(var_len + 1, "Build variable name");
                if (!var_name) {
                    AR__HEAP__FREE(own_result_str);
                    if (own_values) {
                        ar__data__destroy(own_values);
                    }
                    if (owns_template && own_template) {
                        ar__data__destroy(own_template);
                    }
                    return false;
                }
                
                strncpy(var_name, template_ptr + 1, var_len);
                var_name[var_len] = '\0';
                
                // Look up value in the map
                const data_t *ref_value = ar__data__get_map_data(values_to_use, var_name);
                if (ref_value) {
                    // Convert value to string
                    char value_buffer[256];
                    const char *value_str = NULL;
                    
                    if (ar__data__get_type(ref_value) == DATA_STRING) {
                        value_str = ar__data__get_string(ref_value);
                    } else if (ar__data__get_type(ref_value) == DATA_INTEGER) {
                        snprintf(value_buffer, sizeof(value_buffer), "%d", ar__data__get_integer(ref_value));
                        value_str = value_buffer;
                    } else if (ar__data__get_type(ref_value) == DATA_DOUBLE) {
                        snprintf(value_buffer, sizeof(value_buffer), "%g", ar__data__get_double(ref_value));
                        value_str = value_buffer;
                    }
                    
                    if (value_str) {
                        // Ensure we have enough space
                        size_t value_len = strlen(value_str);
                        while (result_pos + value_len >= result_size - 1) {
                            result_size *= 2;
                            char *new_result = AR__HEAP__MALLOC(result_size, "Build result resize");
                            if (!new_result) {
                                AR__HEAP__FREE(var_name);
                                AR__HEAP__FREE(own_result_str);
                                if (own_values) {
                                    ar__data__destroy(own_values);
                                }
                                if (owns_template && own_template) {
                                    ar__data__destroy(own_template);
                                }
                                return false;
                            }
                            strcpy(new_result, own_result_str);
                            AR__HEAP__FREE(own_result_str);
                            own_result_str = new_result;
                        }
                        
                        // Copy value to result
                        strcpy(own_result_str + result_pos, value_str);
                        result_pos += value_len;
                    }
                } else {
                    // Variable not found - preserve the placeholder
                    size_t placeholder_len = var_len + 2; // Length of {var_name}
                    while (result_pos + placeholder_len >= result_size - 1) {
                        result_size *= 2;
                        char *new_result = AR__HEAP__MALLOC(result_size, "Build result resize");
                        if (!new_result) {
                            AR__HEAP__FREE(var_name);
                            AR__HEAP__FREE(own_result_str);
                            if (own_values) {
                                ar__data__destroy(own_values);
                            }
                            if (owns_template && own_template) {
                                ar__data__destroy(own_template);
                            }
                            return false;
                        }
                        strcpy(new_result, own_result_str);
                        AR__HEAP__FREE(own_result_str);
                        own_result_str = new_result;
                    }
                    
                    // Copy the placeholder {var_name} to result
                    own_result_str[result_pos++] = '{';
                    strcpy(own_result_str + result_pos, var_name);
                    result_pos += var_len;
                    own_result_str[result_pos++] = '}';
                }
                
                AR__HEAP__FREE(var_name);
                
                // Move template pointer past the placeholder
                template_ptr = placeholder_end + 1;
            } else {
                // No closing brace, copy the { and continue
                if (result_pos >= result_size - 1) {
                    result_size *= 2;
                    char *new_result = AR__HEAP__MALLOC(result_size, "Build result resize");
                    if (!new_result) {
                        AR__HEAP__FREE(own_result_str);
                        if (own_values) {
                            ar__data__destroy(own_values);
                        }
                        if (owns_template && own_template) {
                            ar__data__destroy(own_template);
                        }
                        return false;
                    }
                    strcpy(new_result, own_result_str);
                    AR__HEAP__FREE(own_result_str);
                    own_result_str = new_result;
                }
                own_result_str[result_pos++] = *template_ptr++;
            }
        } else {
            // Regular character, copy it
            if (result_pos >= result_size - 1) {
                result_size *= 2;
                char *new_result = AR__HEAP__MALLOC(result_size, "Build result resize");
                if (!new_result) {
                    AR__HEAP__FREE(own_result_str);
                    if (own_values) {
                        ar__data__destroy(own_values);
                    }
                    if (owns_template && own_template) {
                        ar__data__destroy(own_template);
                    }
                    return false;
                }
                strcpy(new_result, own_result_str);
                AR__HEAP__FREE(own_result_str);
                own_result_str = new_result;
            }
            own_result_str[result_pos++] = *template_ptr++;
        }
    }
    
    // Null-terminate the result
    own_result_str[result_pos] = '\0';
    
    // Clean up input data
    if (own_values) {
        ar__data__destroy(own_values);
    }
    if (owns_template && own_template) {
        ar__data__destroy(own_template);
    }
    
    // Create result string data
    data_t *own_result = ar__data__create_string(own_result_str);
    AR__HEAP__FREE(own_result_str);
    
    if (!own_result) {
        return false;
    }
    
    // Store result if there's a result path
    if (ref_result_path) {
        ar__data__set_map_data(mut_memory, ref_result_path, own_result);
    } else {
        ar__data__destroy(own_result);
    }
    
    return true;
}

// Execute a method instruction
static bool _execute_method(interpreter_t *mut_interpreter, instruction_context_t *mut_context, const parsed_instruction_t *ref_parsed) {
    (void)mut_interpreter; // Unused for now
    
    // Get function call details
    const char *ref_function_name;
    const char **ref_args;
    int arg_count;
    const char *ref_result_path;
    
    if (!ar__instruction__get_function_call(ref_parsed, &ref_function_name, &ref_args, &arg_count, &ref_result_path)) {
        return false;
    }
    
    // Verify this is a method function with 3 arguments
    if (strcmp(ref_function_name, "method") != 0 || arg_count != 3) {
        return false;
    }
    
    // Get context data
    data_t *mut_memory = ar__instruction__get_memory(mut_context);
    const data_t *ref_context_data = ar__instruction__get_context(mut_context);
    const data_t *ref_message = ar__instruction__get_message(mut_context);
    
    // Evaluate name argument
    expression_context_t *own_context = ar__expression__create_context(
        mut_memory, ref_context_data, ref_message, ref_args[0]
    );
    if (!own_context) {
        return false;
    }
    
    const data_t *ref_name_eval = ar__expression__evaluate(own_context);
    data_t *own_name = ar__expression__take_ownership(own_context, ref_name_eval);
    
    // Clean up context immediately
    ar__expression__destroy_context(own_context);
    
    // Handle both owned values and references
    const char *method_name = NULL;
    bool owns_name = (own_name != NULL);
    
    if (owns_name) {
        // We own the value
        if (ar__data__get_type(own_name) != DATA_STRING) {
            ar__data__destroy(own_name);
            return false;
        }
        method_name = ar__data__get_string(own_name);
    } else {
        // It's a reference - use the evaluation result directly
        if (!ref_name_eval || ar__data__get_type(ref_name_eval) != DATA_STRING) {
            return false;
        }
        method_name = ar__data__get_string(ref_name_eval);
    }
    
    // Evaluate instructions argument
    own_context = ar__expression__create_context(
        mut_memory, ref_context_data, ref_message, ref_args[1]
    );
    if (!own_context) {
        if (owns_name && own_name) {
            ar__data__destroy(own_name);
        }
        return false;
    }
    
    const data_t *ref_instr_eval = ar__expression__evaluate(own_context);
    data_t *own_instr = ar__expression__take_ownership(own_context, ref_instr_eval);
    
    // Clean up context immediately
    ar__expression__destroy_context(own_context);
    
    // Handle both owned values and references
    const char *instructions = NULL;
    bool owns_instr = (own_instr != NULL);
    
    if (owns_instr) {
        // We own the value
        if (ar__data__get_type(own_instr) != DATA_STRING) {
            ar__data__destroy(own_instr);
            if (owns_name && own_name) {
                ar__data__destroy(own_name);
            }
            return false;
        }
        instructions = ar__data__get_string(own_instr);
    } else {
        // It's a reference - use the evaluation result directly
        if (!ref_instr_eval || ar__data__get_type(ref_instr_eval) != DATA_STRING) {
            if (owns_name && own_name) {
                ar__data__destroy(own_name);
            }
            return false;
        }
        instructions = ar__data__get_string(ref_instr_eval);
    }
    
    // Evaluate version argument
    own_context = ar__expression__create_context(
        mut_memory, ref_context_data, ref_message, ref_args[2]
    );
    if (!own_context) {
        if (owns_instr && own_instr) {
            ar__data__destroy(own_instr);
        }
        if (owns_name && own_name) {
            ar__data__destroy(own_name);
        }
        return false;
    }
    
    const data_t *ref_version_eval = ar__expression__evaluate(own_context);
    data_t *own_version = ar__expression__take_ownership(own_context, ref_version_eval);
    
    // Clean up context immediately
    ar__expression__destroy_context(own_context);
    
    // Handle both owned values and references for version
    const char *version_str = "1.0.0"; // Default
    bool owns_version = (own_version != NULL);
    
    if (owns_version) {
        // We own the value
        if (ar__data__get_type(own_version) == DATA_STRING) {
            version_str = ar__data__get_string(own_version);
        } else if (ar__data__get_type(own_version) == DATA_INTEGER) {
            // If version is provided as a number, convert it to a string "X.0.0"
            static char version_buffer[16]; // Buffer for conversion
            snprintf(version_buffer, sizeof(version_buffer), "%d.0.0", ar__data__get_integer(own_version));
            version_str = version_buffer;
        }
    } else {
        // It's a reference - use the evaluation result directly
        if (ref_version_eval) {
            if (ar__data__get_type(ref_version_eval) == DATA_STRING) {
                version_str = ar__data__get_string(ref_version_eval);
            } else if (ar__data__get_type(ref_version_eval) == DATA_INTEGER) {
                // If version is provided as a number, convert it to a string "X.0.0"
                static char version_buffer[16]; // Buffer for conversion
                snprintf(version_buffer, sizeof(version_buffer), "%d.0.0", ar__data__get_integer(ref_version_eval));
                version_str = version_buffer;
            }
        }
    }
    
    // Call methodology module directly to create method with just 3 parameters:
    // name, instructions, version
    bool success = ar__methodology__create_method(method_name, instructions, version_str);
    
    // Clean up input data now that we're done with it
    if (owns_version && own_version) {
        ar__data__destroy(own_version);
    }
    if (owns_instr && own_instr) {
        ar__data__destroy(own_instr);
    }
    if (owns_name && own_name) {
        ar__data__destroy(own_name);
    }
    
    // Return success indicator
    data_t *own_result = ar__data__create_integer(success ? 1 : 0);
    
    // Store result if there's a result path
    if (ref_result_path && own_result) {
        ar__data__set_map_data(mut_memory, ref_result_path, own_result);
    } else if (own_result) {
        ar__data__destroy(own_result);
    }
    
    return true;
}

// Execute an agent instruction
static bool _execute_agent(interpreter_t *mut_interpreter, instruction_context_t *mut_context, const parsed_instruction_t *ref_parsed) {
    (void)mut_interpreter; // Unused for now
    
    // Get function call details
    const char *ref_function_name;
    const char **ref_args;
    int arg_count;
    const char *ref_result_path;
    
    if (!ar__instruction__get_function_call(ref_parsed, &ref_function_name, &ref_args, &arg_count, &ref_result_path)) {
        return false;
    }
    
    // Verify this is an agent function with 3 arguments
    if (strcmp(ref_function_name, "agent") != 0 || arg_count != 3) {
        return false;
    }
    
    // Get context data
    data_t *mut_memory = ar__instruction__get_memory(mut_context);
    const data_t *ref_context_data = ar__instruction__get_context(mut_context);
    const data_t *ref_message = ar__instruction__get_message(mut_context);
    
    // Evaluate method name argument
    expression_context_t *own_context = ar__expression__create_context(
        mut_memory, ref_context_data, ref_message, ref_args[0]
    );
    if (!own_context) {
        return false;
    }
    
    const data_t *ref_method_eval = ar__expression__evaluate(own_context);
    data_t *own_method_name = ar__expression__take_ownership(own_context, ref_method_eval);
    
    // Clean up context immediately
    ar__expression__destroy_context(own_context);
    
    // Handle both owned values and references
    const char *method_name = NULL;
    bool owns_method_name = (own_method_name != NULL);
    
    if (owns_method_name) {
        // We own the value
        if (ar__data__get_type(own_method_name) != DATA_STRING) {
            ar__data__destroy(own_method_name);
            return false;
        }
        method_name = ar__data__get_string(own_method_name);
    } else {
        // It's a reference - use the evaluation result directly
        if (!ref_method_eval || ar__data__get_type(ref_method_eval) != DATA_STRING) {
            return false;
        }
        method_name = ar__data__get_string(ref_method_eval);
    }
    
    // Evaluate version argument
    own_context = ar__expression__create_context(
        mut_memory, ref_context_data, ref_message, ref_args[1]
    );
    if (!own_context) {
        if (owns_method_name && own_method_name) {
            ar__data__destroy(own_method_name);
        }
        return false;
    }
    
    const data_t *ref_version_eval = ar__expression__evaluate(own_context);
    data_t *own_version = ar__expression__take_ownership(own_context, ref_version_eval);
    
    // Clean up context immediately
    ar__expression__destroy_context(own_context);
    
    // Handle both owned values and references for version
    const char *version_str = "1.0.0"; // Default
    bool owns_version = (own_version != NULL);
    
    if (owns_version) {
        // We own the value
        if (ar__data__get_type(own_version) == DATA_STRING) {
            version_str = ar__data__get_string(own_version);
        } else if (ar__data__get_type(own_version) == DATA_INTEGER) {
            // If version is provided as a number, convert it to a string "X.0.0"
            static char version_buffer[16]; // Buffer for conversion
            snprintf(version_buffer, sizeof(version_buffer), "%d.0.0", ar__data__get_integer(own_version));
            version_str = version_buffer;
        }
    } else {
        // It's a reference - use the evaluation result directly
        if (ref_version_eval) {
            if (ar__data__get_type(ref_version_eval) == DATA_STRING) {
                version_str = ar__data__get_string(ref_version_eval);
            } else if (ar__data__get_type(ref_version_eval) == DATA_INTEGER) {
                // If version is provided as a number, convert it to a string "X.0.0"
                static char version_buffer[16]; // Buffer for conversion
                snprintf(version_buffer, sizeof(version_buffer), "%d.0.0", ar__data__get_integer(ref_version_eval));
                version_str = version_buffer;
            }
        }
    }
    
    // Evaluate context argument
    own_context = ar__expression__create_context(
        mut_memory, ref_context_data, ref_message, ref_args[2]
    );
    if (!own_context) {
        if (owns_version && own_version) {
            ar__data__destroy(own_version);
        }
        if (owns_method_name && own_method_name) {
            ar__data__destroy(own_method_name);
        }
        return false;
    }
    
    const data_t *ref_ctx_eval = ar__expression__evaluate(own_context);
    data_t *own_ctx = ar__expression__take_ownership(own_context, ref_ctx_eval);
    
    // Clean up context immediately
    ar__expression__destroy_context(own_context);
    
    // Handle both owned values and references for context
    const data_t *ctx_to_use = own_ctx ? own_ctx : ref_ctx_eval;
    bool owns_ctx = (own_ctx != NULL);
    
    // Default to empty map if context is null
    data_t *empty_context = NULL;
    if (!ctx_to_use) {
        empty_context = ar__data__create_map();
        ctx_to_use = empty_context;
    }
    
    // Create the agent
    int64_t agent_id = ar__agency__create_agent(method_name, version_str, ctx_to_use);
    
    // Clean up input data
    if (empty_context) {
        ar__data__destroy(empty_context);
    }
    if (owns_ctx && own_ctx) {
        ar__data__destroy(own_ctx);
    }
    if (owns_version && own_version) {
        ar__data__destroy(own_version);
    }
    if (owns_method_name && own_method_name) {
        ar__data__destroy(own_method_name);
    }
    
    // Return agent ID as result (0 if creation failed)
    data_t *own_result = ar__data__create_integer((int)agent_id);
    
    // Store result if there's a result path
    if (ref_result_path && own_result) {
        ar__data__set_map_data(mut_memory, ref_result_path, own_result);
    } else if (own_result) {
        ar__data__destroy(own_result);
    }
    
    return true;
}

// Execute a destroy instruction
static bool _execute_destroy(interpreter_t *mut_interpreter, instruction_context_t *mut_context, const parsed_instruction_t *ref_parsed) {
    (void)mut_interpreter; // Unused for now
    
    // Get function call details
    const char *ref_function_name;
    const char **ref_args;
    int arg_count;
    const char *ref_result_path;
    
    if (!ar__instruction__get_function_call(ref_parsed, &ref_function_name, &ref_args, &arg_count, &ref_result_path)) {
        return false;
    }
    
    // Verify this is a destroy function with 1 or 2 arguments
    if (strcmp(ref_function_name, "destroy") != 0 || (arg_count != 1 && arg_count != 2)) {
        return false;
    }
    
    // Get context data
    data_t *mut_memory = ar__instruction__get_memory(mut_context);
    const data_t *ref_context_data = ar__instruction__get_context(mut_context);
    const data_t *ref_message = ar__instruction__get_message(mut_context);
    
    // Evaluate first argument
    expression_context_t *own_context = ar__expression__create_context(
        mut_memory, ref_context_data, ref_message, ref_args[0]
    );
    if (!own_context) {
        return false;
    }
    
    const data_t *ref_arg1_eval = ar__expression__evaluate(own_context);
    data_t *own_arg1 = ar__expression__take_ownership(own_context, ref_arg1_eval);
    
    // Clean up context immediately
    ar__expression__destroy_context(own_context);
    
    // Handle both owned values and references
    const data_t *arg1_to_use = own_arg1 ? own_arg1 : ref_arg1_eval;
    if (!arg1_to_use) {
        return false;
    }
    
    bool success = false;
    
    if (arg_count == 2) {
        // destroy(method_name, version)
        // First argument must be a string (method name)
        if (ar__data__get_type(arg1_to_use) != DATA_STRING) {
            if (own_arg1) {
                ar__data__destroy(own_arg1);
            }
            return false;
        }
        const char *method_name = ar__data__get_string(arg1_to_use);
        
        // Evaluate version argument
        own_context = ar__expression__create_context(
            mut_memory, ref_context_data, ref_message, ref_args[1]
        );
        if (!own_context) {
            if (own_arg1) {
                ar__data__destroy(own_arg1);
            }
            return false;
        }
        
        const data_t *ref_version_eval = ar__expression__evaluate(own_context);
        data_t *own_version = ar__expression__take_ownership(own_context, ref_version_eval);
        
        // Clean up context immediately
        ar__expression__destroy_context(own_context);
        
        // Handle both owned values and references for version
        const char *version_str = "1.0.0"; // Default
        bool owns_version = (own_version != NULL);
        
        if (owns_version) {
            // We own the value
            if (ar__data__get_type(own_version) == DATA_STRING) {
                version_str = ar__data__get_string(own_version);
            } else if (ar__data__get_type(own_version) == DATA_INTEGER) {
                // If version is provided as a number, convert it to a string "X.0.0"
                static char version_buffer[16]; // Buffer for conversion
                snprintf(version_buffer, sizeof(version_buffer), "%d.0.0", ar__data__get_integer(own_version));
                version_str = version_buffer;
            }
        } else {
            // It's a reference - use the evaluation result directly
            if (ref_version_eval) {
                if (ar__data__get_type(ref_version_eval) == DATA_STRING) {
                    version_str = ar__data__get_string(ref_version_eval);
                } else if (ar__data__get_type(ref_version_eval) == DATA_INTEGER) {
                    // If version is provided as a number, convert it to a string "X.0.0"
                    static char version_buffer[16]; // Buffer for conversion
                    snprintf(version_buffer, sizeof(version_buffer), "%d.0.0", ar__data__get_integer(ref_version_eval));
                    version_str = version_buffer;
                }
            }
        }
        
        // Call methodology module to unregister the method
        success = ar__methodology__unregister_method(method_name, version_str);
        
        // Clean up
        if (owns_version && own_version) {
            ar__data__destroy(own_version);
        }
    } else {
        // destroy(agent_id)
        // Argument must be an integer (agent ID)
        if (ar__data__get_type(arg1_to_use) != DATA_INTEGER) {
            if (own_arg1) {
                ar__data__destroy(own_arg1);
            }
            return false;
        }
        int64_t agent_id = (int64_t)ar__data__get_integer(arg1_to_use);
        
        // Destroy the agent
        success = ar__agency__destroy_agent(agent_id);
    }
    
    // Clean up first argument
    if (own_arg1) {
        ar__data__destroy(own_arg1);
    }
    
    // Return success indicator
    data_t *own_result = ar__data__create_integer(success ? 1 : 0);
    
    // Store result if there's a result path
    if (ref_result_path && own_result) {
        ar__data__set_map_data(mut_memory, ref_result_path, own_result);
    } else if (own_result) {
        ar__data__destroy(own_result);
    }
    
    return true;
}

