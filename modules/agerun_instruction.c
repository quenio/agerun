#include "agerun_instruction.h"
#include "agerun_string.h"
#include "agerun_data.h"
#include "agerun_expression.h"
#include "agerun_map.h"
#include "agerun_assert.h" // Include for ownership assertions
#include "agerun_heap.h" // Include for memory allocation macros

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>

// Instruction context structure definition
struct instruction_context_s {
    data_t *mut_memory;        // Mutable reference to memory, not owned
    const data_t *ref_context; // Borrowed reference to context, not owned
    const data_t *ref_message; // Borrowed reference to message, not owned
    char *own_error_message;   // Owned error message string
    int error_position;        // Position where error occurred (1-based, 0 if no error)
};

// Parsed instruction structure definition
struct parsed_instruction_s {
    instruction_type_t type;
    
    // For assignments
    char *own_assignment_path;       // Owned: e.g., "memory.x.y"
    char *own_assignment_expression; // Owned: the expression to evaluate
    
    // For function calls
    char *own_function_name;         // Owned: function name
    char **own_args;                 // Owned: array of owned argument strings
    int arg_count;                   // Number of arguments
    char *own_result_path;           // Owned: optional result assignment path (may be NULL)
};

// Function prototypes for recursive descent parsing
static parsed_instruction_t* _parse_instruction(instruction_context_t *mut_ctx, const char *ref_instruction, int *mut_pos);
static parsed_instruction_t* _parse_assignment(instruction_context_t *mut_ctx, const char *ref_instruction, int *mut_pos);
static parsed_instruction_t* _parse_function_instruction(instruction_context_t *mut_ctx, const char *ref_instruction, int *mut_pos);
static bool _parse_memory_access(const char *ref_instruction, int *mut_pos, char **path);
static parsed_instruction_t* _parse_function_call(instruction_context_t *mut_ctx, const char *ref_instruction, int *mut_pos, char **out_result_path);
static bool _skip_whitespace(const char *ref_instruction, int *mut_pos);
static bool _extract_identifier(const char *ref_instruction, int *mut_pos, char *mut_identifier, int max_size);

// Error handling helper functions
static void _set_error(instruction_context_t *mut_ctx, const char *ref_message, int position);
static void _clear_error(instruction_context_t *mut_ctx);

// Create a new instruction context
instruction_context_t* ar__instruction__create_context(data_t *mut_memory, const data_t *ref_context, const data_t *ref_message) {
    // Allocate memory for the context
    instruction_context_t *own_ctx = (instruction_context_t*)AR__HEAP__MALLOC(sizeof(instruction_context_t), "Instruction context");
    if (!own_ctx) {
        return NULL;
    }
    
    // Initialize the context
    own_ctx->mut_memory = mut_memory;
    own_ctx->ref_context = ref_context;
    own_ctx->ref_message = ref_message;
    own_ctx->own_error_message = NULL;
    own_ctx->error_position = 0;
    
    // Return the newly created context
    return own_ctx;
}

// Destroy an instruction context
void ar__instruction__destroy_context(instruction_context_t *own_ctx) {
    if (own_ctx) {
        // Free the owned error message if any
        if (own_ctx->own_error_message) {
            AR__HEAP__FREE(own_ctx->own_error_message);
        }
        // The context doesn't own memory, context, or message, so we just free the structure
        AR__HEAP__FREE(own_ctx);
    }
}

// Get the memory from the instruction context
data_t* ar__instruction__get_memory(const instruction_context_t *ref_ctx) {
    if (!ref_ctx) {
        return NULL;
    }
    return ref_ctx->mut_memory;
}

// Get the context data from the instruction context
const data_t* ar__instruction__get_context(const instruction_context_t *ref_ctx) {
    if (!ref_ctx) {
        return NULL;
    }
    return ref_ctx->ref_context;
}

// Get the message from the instruction context
const data_t* ar__instruction__get_message(const instruction_context_t *ref_ctx) {
    if (!ref_ctx) {
        return NULL;
    }
    return ref_ctx->ref_message;
}


// Parse and execute a single instruction
// This is now the main parse function that builds AST
parsed_instruction_t* ar__instruction__parse(const char *ref_instruction, instruction_context_t *mut_ctx) {
    if (!mut_ctx || !ref_instruction) {
        return NULL;
    }
    
    int pos = 0;
    parsed_instruction_t *result = _parse_instruction(mut_ctx, ref_instruction, &pos);
    
    if (result) {
        // Clear error state on success
        _clear_error(mut_ctx);
    }
    
    return result;
}

// Creates a new empty parsed instruction
static parsed_instruction_t* _create_parsed_instruction(void) {
    parsed_instruction_t *own_parsed = AR__HEAP__MALLOC(sizeof(parsed_instruction_t), "Parsed instruction");
    if (!own_parsed) {
        return NULL;
    }
    
    // Initialize all fields to NULL/0
    memset(own_parsed, 0, sizeof(parsed_instruction_t));
    return own_parsed;
}

// Destroys a parsed instruction and frees its resources
void ar__instruction__destroy_parsed(parsed_instruction_t *own_parsed) {
    if (!own_parsed) {
        return;
    }
    
    // Free assignment fields
    if (own_parsed->own_assignment_path) {
        AR__HEAP__FREE(own_parsed->own_assignment_path);
    }
    if (own_parsed->own_assignment_expression) {
        AR__HEAP__FREE(own_parsed->own_assignment_expression);
    }
    
    // Free function call fields
    if (own_parsed->own_function_name) {
        AR__HEAP__FREE(own_parsed->own_function_name);
    }
    if (own_parsed->own_result_path) {
        AR__HEAP__FREE(own_parsed->own_result_path);
    }
    
    // Free arguments array
    if (own_parsed->own_args) {
        for (int i = 0; i < own_parsed->arg_count; i++) {
            if (own_parsed->own_args[i]) {
                AR__HEAP__FREE(own_parsed->own_args[i]);
            }
        }
        AR__HEAP__FREE(own_parsed->own_args);
    }
    
    AR__HEAP__FREE(own_parsed);
}

// Gets the type of a parsed instruction
instruction_type_t ar__instruction__get_type(const parsed_instruction_t *ref_parsed) {
    if (!ref_parsed) {
        return INST_ASSIGNMENT; // Default, though caller should check for NULL
    }
    return ref_parsed->type;
}

// Gets the memory path for an assignment instruction
const char* ar__instruction__get_assignment_path(const parsed_instruction_t *ref_parsed) {
    if (!ref_parsed || ref_parsed->type != INST_ASSIGNMENT) {
        return NULL;
    }
    return ref_parsed->own_assignment_path;
}

// Gets the expression for an assignment instruction
const char* ar__instruction__get_assignment_expression(const parsed_instruction_t *ref_parsed) {
    if (!ref_parsed || ref_parsed->type != INST_ASSIGNMENT) {
        return NULL;
    }
    return ref_parsed->own_assignment_expression;
}

// Gets function call details from a parsed instruction
bool ar__instruction__get_function_call(const parsed_instruction_t *ref_parsed,
                                        const char **out_function_name,
                                        const char ***out_args,
                                        int *out_arg_count,
                                        const char **out_result_path) {
    if (!ref_parsed || 
        (ref_parsed->type == INST_ASSIGNMENT)) {
        return false;
    }
    
    if (out_function_name) {
        *out_function_name = ref_parsed->own_function_name;
    }
    if (out_args) {
        *out_args = (const char **)(void *)ref_parsed->own_args;
    }
    if (out_arg_count) {
        *out_arg_count = ref_parsed->arg_count;
    }
    if (out_result_path) {
        *out_result_path = ref_parsed->own_result_path;
    }
    
    return true;
}


// <instruction> ::= <assignment> | <function-instruction>
static parsed_instruction_t* _parse_instruction(instruction_context_t *mut_ctx, const char *ref_instruction, int *mut_pos) {
    _skip_whitespace(ref_instruction, mut_pos);
    
    // Check for assignment or function instruction
    // Save the current position to backtrack if needed
    int save_pos = *mut_pos;
    
    // Try to parse as assignment first
    parsed_instruction_t *result = _parse_assignment(mut_ctx, ref_instruction, mut_pos);
    if (result) {
        return result;
    }
    
    // Backtrack and try as function instruction
    *mut_pos = save_pos;
    return _parse_function_instruction(mut_ctx, ref_instruction, mut_pos);
}

// <assignment> ::= <memory-access> ':=' <expression>
static parsed_instruction_t* _parse_assignment(instruction_context_t *mut_ctx, const char *ref_instruction, int *mut_pos) {
    char *own_path = NULL;
    
    // Parse memory access (left side)
    if (!_parse_memory_access(ref_instruction, mut_pos, &own_path)) {
        return NULL;
    }
    
    _skip_whitespace(ref_instruction, mut_pos);
    
    // Check for ':=' operator
    if (ref_instruction[*mut_pos] != ':' || ref_instruction[*mut_pos + 1] != '=') {
        AR__HEAP__FREE(own_path);
        return NULL;
    }
    
    *mut_pos += 2; // Skip ':='
    _skip_whitespace(ref_instruction, mut_pos);
    
    // Check if there's an expression after ':='
    if (!ref_instruction[*mut_pos] || ref_instruction[*mut_pos] == '\0') {
        _set_error(mut_ctx, "Expected expression after ':='", *mut_pos);
        AR__HEAP__FREE(own_path);
        return NULL;
    }
    
    // Validate that the right-hand side is a valid expression by trying to parse it
    // This follows the original implementation approach
    expression_context_t *own_expr_test = ar__expression__create_context(
        ar__instruction__get_memory(mut_ctx),
        ar__instruction__get_context(mut_ctx), 
        ar__instruction__get_message(mut_ctx),
        ref_instruction + *mut_pos
    );
    
    if (!own_expr_test) {
        AR__HEAP__FREE(own_path);
        return NULL;
    }
    
    // Try to evaluate the expression - if this fails, it's not a valid expression
    const data_t *ref_test_result = ar__expression__evaluate(own_expr_test);
    int expr_offset = ar__expression__offset(own_expr_test);
    ar__expression__destroy_context(own_expr_test);
    
    if (!ref_test_result || expr_offset == 0) {
        // Expression evaluation failed - this should be handled by function instruction parser
        fprintf(stderr, "DEBUG: Assignment parser rejecting '%s' - not a valid expression\n", ref_instruction + *mut_pos);
        AR__HEAP__FREE(own_path);
        return NULL;
    }
    
    // Extract the expression (everything from current position to end)
    const char *expr_start = ref_instruction + *mut_pos;
    size_t expr_len = strlen(expr_start);
    
    // Trim trailing whitespace
    while (expr_len > 0 && isspace((unsigned char)expr_start[expr_len - 1])) {
        expr_len--;
    }
    
    if (expr_len == 0) {
        _set_error(mut_ctx, "Empty expression in assignment", *mut_pos);
        AR__HEAP__FREE(own_path);
        return NULL;
    }
    
    // Create the parsed instruction
    parsed_instruction_t *own_result = _create_parsed_instruction();
    if (!own_result) {
        AR__HEAP__FREE(own_path);
        return NULL;
    }
    
    own_result->type = INST_ASSIGNMENT;
    own_result->own_assignment_path = own_path; // Transfer ownership
    
    // Copy the expression
    own_result->own_assignment_expression = AR__HEAP__MALLOC(expr_len + 1, "Assignment expression");
    if (!own_result->own_assignment_expression) {
        ar__instruction__destroy_parsed(own_result);
        return NULL;
    }
    
    memcpy(own_result->own_assignment_expression, expr_start, expr_len);
    own_result->own_assignment_expression[expr_len] = '\0';
    
    // Update position to end of instruction
    *mut_pos += (int)strlen(expr_start);
    
    return own_result;
}

// <function-instruction> ::= [<memory-access> ':='] <function-call>
static parsed_instruction_t* _parse_function_instruction(instruction_context_t *mut_ctx, const char *ref_instruction, int *mut_pos) {
    char *own_result_path = NULL;
    bool has_assignment = false;
    
    fprintf(stderr, "DEBUG: Parsing function instruction starting at: '%s'\n", ref_instruction + *mut_pos);
    
    // Check if there's an assignment part
    int save_pos = *mut_pos;
    if (_parse_memory_access(ref_instruction, mut_pos, &own_result_path)) {
        _skip_whitespace(ref_instruction, mut_pos);
        
        if (ref_instruction[*mut_pos] == ':' && ref_instruction[*mut_pos + 1] == '=') {
            has_assignment = true;
            *mut_pos += 2; // Skip ':='
            _skip_whitespace(ref_instruction, mut_pos);
        } else {
            // Not an assignment, backtrack
            *mut_pos = save_pos;
            AR__HEAP__FREE(own_result_path);
            own_result_path = NULL;
        }
    } else {
        // Not a memory access, backtrack
        *mut_pos = save_pos;
    }
    
    // Parse function call and build AST
    parsed_instruction_t *own_result = _parse_function_call(mut_ctx, ref_instruction, mut_pos, 
                                                           has_assignment ? &own_result_path : NULL);
    
    if (!own_result) {
        if (own_result_path) {
            AR__HEAP__FREE(own_result_path);
        }
        return NULL;
    }
    
    // If we had an assignment, transfer the path ownership to the AST node
    if (has_assignment && own_result_path) {
        own_result->own_result_path = own_result_path;
    }
    
    return own_result;
}

// <memory-access> ::= 'memory' {'.' <identifier>}
// Note: According to the spec, in assignments, only 'memory' paths can be used on the left side
static bool _parse_memory_access(const char *ref_instruction, int *mut_pos, char **path) {
    _skip_whitespace(ref_instruction, mut_pos);
    
    // Check for 'memory' identifier
    const char *memory_str = "memory";
    size_t memory_len = strlen(memory_str);
    
    if (strncmp(ref_instruction + *mut_pos, memory_str, memory_len) != 0) {
        return false;
    }
    
    // Make sure the 'memory' is a complete token
    if (isalnum((unsigned char)ref_instruction[*mut_pos + (int)memory_len]) || 
        ref_instruction[*mut_pos + (int)memory_len] == '_') {
        return false;
    }
    
    *mut_pos += (int)memory_len;
    
    // Start building the path
    char buffer[1024] = "";
    int buffer_pos = 0;
    
    // Check for dot-separated identifiers
    while (ref_instruction[*mut_pos] == '.') {
        (*mut_pos)++; // Skip '.'
        
        char identifier[256];
        if (!_extract_identifier(ref_instruction, mut_pos, identifier, sizeof(identifier))) {
            return false;
        }
        
        // Add to buffer
        if (buffer_pos > 0) {
            buffer[buffer_pos++] = '.';
        }
        // Use safe string copy with bounds checking
        size_t id_len = strlen(identifier);
        size_t remaining_space = sizeof(buffer) - (size_t)buffer_pos;
        
        if (id_len < remaining_space) {
            strncpy(buffer + buffer_pos, identifier, remaining_space - 1);
            size_t index = (size_t)buffer_pos + id_len;
            buffer[index] = '\0';
            buffer_pos += (int)id_len;
        } else {
            // Not enough space - truncate safely
            strncpy(buffer + buffer_pos, identifier, remaining_space - 1);
            buffer[sizeof(buffer) - 1] = '\0';
            buffer_pos = (int)sizeof(buffer) - 1;
        }
    }
    
    // Allocate and return the path
    *path = AR__HEAP__STRDUP(buffer, "Memory path");
    return *path != NULL;
}

// Parse function call and execute it
// <function-call> ::= <send-function> | <parse-function> | <build-function> | <method-function> |
//                     <agent-function> | <destroy-function> | <if-function>
static parsed_instruction_t* _parse_function_call(instruction_context_t *mut_ctx, const char *ref_instruction, int *mut_pos, char **out_result_path) {
    fprintf(stderr, "DEBUG: _parse_function_call starting at: '%s'\n", ref_instruction + *mut_pos);
    
    // Extract function name
    char function_name[32];
    if (!_extract_identifier(ref_instruction, mut_pos, function_name, sizeof(function_name))) {
        fprintf(stderr, "DEBUG: Failed to extract function name\n");
        return NULL;
    }
    
    fprintf(stderr, "DEBUG: Parsed function name: '%s'\n", function_name);
    
    _skip_whitespace(ref_instruction, mut_pos);
    
    // Expect opening parenthesis
    if (ref_instruction[*mut_pos] != '(') {
        _set_error(mut_ctx, "Expected '(' after function name", *mut_pos);
        return NULL;
    }
    (*mut_pos)++; // Skip '('
    
    // Create the parsed instruction
    parsed_instruction_t *own_result = _create_parsed_instruction();
    if (!own_result) {
        return NULL;
    }
    
    // Set function name
    own_result->own_function_name = AR__HEAP__STRDUP(function_name, "Function name");
    if (!own_result->own_function_name) {
        ar__instruction__destroy_parsed(own_result);
        return NULL;
    }
    
    // Handle different function types
    if (strcmp(function_name, "send") == 0) {
        own_result->type = INST_SEND;
        // send(agent_id, message) - requires exactly 2 arguments
        
        // Allocate args array for 2 arguments
        own_result->own_args = AR__HEAP__MALLOC(2 * sizeof(char*), "Send arguments");
        if (!own_result->own_args) {
            ar__instruction__destroy_parsed(own_result);
            return NULL;
        }
        own_result->arg_count = 0;
        
        _skip_whitespace(ref_instruction, mut_pos);
        
        // Parse first argument (agent_id expression)
        int arg_start = *mut_pos;
        int paren_depth = 0;
        bool in_quotes = false;
        
        // Find the comma that separates arguments
        while (ref_instruction[*mut_pos] && 
               (ref_instruction[*mut_pos] != ',' || paren_depth > 0 || in_quotes)) {
            if (ref_instruction[*mut_pos] == '"' && 
                (*mut_pos == 0 || ref_instruction[*mut_pos - 1] != '\\')) {
                in_quotes = !in_quotes;
            } else if (!in_quotes) {
                if (ref_instruction[*mut_pos] == '(') paren_depth++;
                else if (ref_instruction[*mut_pos] == ')') paren_depth--;
            }
            (*mut_pos)++;
        }
        
        if (ref_instruction[*mut_pos] != ',') {
            _set_error(mut_ctx, "send() requires exactly 2 arguments", *mut_pos);
            ar__instruction__destroy_parsed(own_result);
            return NULL;
        }
        
        // Extract and store first argument
        int arg_len = *mut_pos - arg_start;
        own_result->own_args[0] = AR__HEAP__MALLOC((size_t)(arg_len + 1), "Agent ID expression");
        if (!own_result->own_args[0]) {
            ar__instruction__destroy_parsed(own_result);
            return NULL;
        }
        memcpy(own_result->own_args[0], ref_instruction + arg_start, (size_t)arg_len);
        own_result->own_args[0][arg_len] = '\0';
        own_result->arg_count = 1;
        
        (*mut_pos)++; // Skip ','
        _skip_whitespace(ref_instruction, mut_pos);
        
        // Parse second argument (message expression)
        arg_start = *mut_pos;
        paren_depth = 0;
        in_quotes = false;
        
        // Find the closing parenthesis
        while (ref_instruction[*mut_pos] && 
               (ref_instruction[*mut_pos] != ')' || paren_depth > 0 || in_quotes)) {
            if (ref_instruction[*mut_pos] == '"' && 
                (*mut_pos == 0 || ref_instruction[*mut_pos - 1] != '\\')) {
                in_quotes = !in_quotes;
            } else if (!in_quotes) {
                if (ref_instruction[*mut_pos] == '(') paren_depth++;
                else if (ref_instruction[*mut_pos] == ')') {
                    if (paren_depth > 0) paren_depth--;
                    else break;
                }
            }
            (*mut_pos)++;
        }
        
        if (ref_instruction[*mut_pos] != ')') {
            _set_error(mut_ctx, "Expected ')' after send() arguments", *mut_pos);
            ar__instruction__destroy_parsed(own_result);
            return NULL;
        }
        
        // Extract and store second argument
        arg_len = *mut_pos - arg_start;
        own_result->own_args[1] = AR__HEAP__MALLOC((size_t)(arg_len + 1), "Message expression");
        if (!own_result->own_args[1]) {
            ar__instruction__destroy_parsed(own_result);
            return NULL;
        }
        memcpy(own_result->own_args[1], ref_instruction + arg_start, (size_t)arg_len);
        own_result->own_args[1][arg_len] = '\0';
        own_result->arg_count = 2;
        
        (*mut_pos)++; // Skip ')'
        
        // Transfer result path ownership if provided
        if (out_result_path && *out_result_path) {
            own_result->own_result_path = *out_result_path;
            *out_result_path = NULL;
        }
        
        return own_result;
    }
    else if (strcmp(function_name, "if") == 0) {
        own_result->type = INST_IF;
        // if(condition, true_value, false_value) - requires exactly 3 arguments
        
        // Allocate args array for 3 arguments
        own_result->own_args = AR__HEAP__MALLOC(3 * sizeof(char*), "If arguments");
        if (!own_result->own_args) {
            ar__instruction__destroy_parsed(own_result);
            return NULL;
        }
        own_result->arg_count = 0;
        
        _skip_whitespace(ref_instruction, mut_pos);
        
        // Parse three arguments separated by commas
        for (int i = 0; i < 3; i++) {
            int arg_start = *mut_pos;
            int paren_depth = 0;
            bool in_quotes = false;
            char target_char = (i < 2) ? ',' : ')';
            
            // Find the comma or closing paren
            while (ref_instruction[*mut_pos]) {
                char c = ref_instruction[*mut_pos];
                
                if (c == '"' && (*mut_pos == 0 || ref_instruction[*mut_pos - 1] != '\\')) {
                    in_quotes = !in_quotes;
                } else if (!in_quotes) {
                    if (c == '(') paren_depth++;
                    else if (c == ')') {
                        if (paren_depth > 0) paren_depth--;
                        else if (target_char == ')') break;
                    }
                    else if (c == ',' && paren_depth == 0 && target_char == ',') break;
                }
                (*mut_pos)++;
            }
            
            if (ref_instruction[*mut_pos] != target_char) {
                _set_error(mut_ctx, "if() requires exactly 3 arguments", *mut_pos);
                ar__instruction__destroy_parsed(own_result);
                return NULL;
            }
            
            // Extract and store argument
            int arg_len = *mut_pos - arg_start;
            while (arg_len > 0 && isspace((unsigned char)ref_instruction[arg_start])) {
                arg_start++;
                arg_len--;
            }
            while (arg_len > 0 && isspace((unsigned char)ref_instruction[arg_start + arg_len - 1])) {
                arg_len--;
            }
            
            own_result->own_args[i] = AR__HEAP__MALLOC((size_t)(arg_len + 1), "If argument");
            if (!own_result->own_args[i]) {
                ar__instruction__destroy_parsed(own_result);
                return NULL;
            }
            memcpy(own_result->own_args[i], ref_instruction + arg_start, (size_t)arg_len);
            own_result->own_args[i][arg_len] = '\0';
            own_result->arg_count++;
            
            if (i < 2) {
                (*mut_pos)++; // Skip ','
                _skip_whitespace(ref_instruction, mut_pos);
            }
        }
        
        (*mut_pos)++; // Skip ')'
        
        // Transfer result path ownership if provided
        if (out_result_path && *out_result_path) {
            own_result->own_result_path = *out_result_path;
            *out_result_path = NULL;
        }
        
        return own_result;
    }
#if 0
    // OLD IF IMPLEMENTATION - TO BE REMOVED
                                              mut_ctx->ref_message, 
                                              ref_instruction + *mut_pos);
        if (!own_context) {
            if (own_cond) {
                ar__data__destroy(own_cond);
                own_cond = NULL; // Mark as destroyed
            }
            return false;
        }
        const data_t *ref_true_eval = ar__expression__evaluate(own_context);
        data_t *own_true = ar__expression__take_ownership(own_context, ref_true_eval);
        *mut_pos += ar__expression__offset(own_context);
        
        // Clean up context immediately
        ar__expression__destroy_context(own_context);
        own_context = NULL; // Mark as destroyed
        
        // Handle both owned values and references
        const data_t *true_to_use = own_true ? own_true : ref_true_eval;
        if (!true_to_use) {
            if (own_cond) {
                ar__data__destroy(own_cond);
                own_cond = NULL; // Mark as destroyed
            }
            return false;
        }
        
        _skip_whitespace(ref_instruction, mut_pos);
        
        // Expect comma
        if (ref_instruction[*mut_pos] != ',') {
            ar__data__destroy(own_cond);
            own_cond = NULL; // Mark as destroyed
            ar__data__destroy(own_true);
            own_true = NULL; // Mark as destroyed
            return false;
        }
        (*mut_pos)++; // Skip ','
        _skip_whitespace(ref_instruction, mut_pos);
        
        // Parse false_value expression - reusing the context variable
        own_context = ar__expression__create_context(mut_ctx->mut_memory, 
                                              mut_ctx->ref_context, 
                                              mut_ctx->ref_message, 
                                              ref_instruction + *mut_pos);
        if (!own_context) {
            if (own_cond) {
                ar__data__destroy(own_cond);
                own_cond = NULL; // Mark as destroyed
            }
            if (own_true) {
                ar__data__destroy(own_true);
                own_true = NULL; // Mark as destroyed
            }
            return false;
        }
        const data_t *ref_false_eval = ar__expression__evaluate(own_context);
        data_t *own_false = ar__expression__take_ownership(own_context, ref_false_eval);
        *mut_pos += ar__expression__offset(own_context);
        
        // Clean up context immediately
        ar__expression__destroy_context(own_context);
        own_context = NULL; // Mark as destroyed
        
        // Handle both owned values and references
        const data_t *false_to_use = own_false ? own_false : ref_false_eval;
        if (!false_to_use) {
            if (own_cond) {
                ar__data__destroy(own_cond);
                own_cond = NULL; // Mark as destroyed
            }
            if (own_true) {
                ar__data__destroy(own_true);
                own_true = NULL; // Mark as destroyed
            }
            return false;
        }
        
        _skip_whitespace(ref_instruction, mut_pos);
        
        // Expect closing parenthesis
        if (ref_instruction[*mut_pos] != ')') {
            ar__data__destroy(own_cond);
            own_cond = NULL; // Mark as destroyed
            ar__data__destroy(own_true);
            own_true = NULL; // Mark as destroyed
            ar__data__destroy(own_false);
            own_false = NULL; // Mark as destroyed
            return false;
        }
        (*mut_pos)++; // Skip ')'
        
        // Evaluate condition
        bool condition = false;
        data_type_t cond_type = ar__data__get_type(cond_to_use);
        
        if (cond_type == DATA_INTEGER) {
            condition = (ar__data__get_integer(cond_to_use) != 0);
        } else if (cond_type == DATA_DOUBLE) {
            condition = (ar__data__get_double(cond_to_use) != 0.0);
        } else if (cond_type == DATA_STRING) {
            const char *str = ar__data__get_string(cond_to_use);
            condition = (str && *str); // True if non-empty string
        }
        
        // Select the result based on condition
        if (condition) {
            // If we own the true value, transfer ownership
            if (own_true) {
                *own_result = own_true;
                own_true = NULL; // Mark as transferred
            } else {
                // Create a copy of the reference
                if (ar__data__get_type(true_to_use) == DATA_INTEGER) {
                    *own_result = ar__data__create_integer(ar__data__get_integer(true_to_use));
                } else if (ar__data__get_type(true_to_use) == DATA_DOUBLE) {
                    *own_result = ar__data__create_double(ar__data__get_double(true_to_use));
                } else if (ar__data__get_type(true_to_use) == DATA_STRING) {
                    *own_result = ar__data__create_string(ar__data__get_string(true_to_use));
                } else {
                    // For maps and other types, we can't easily copy, so return 0
                    *own_result = ar__data__create_integer(0);
                }
            }
            // Clean up false value if we own it
            if (own_false) {
                ar__data__destroy(own_false);
                own_false = NULL; // Mark as destroyed
            }
        } else {
            // If we own the false value, transfer ownership
            if (own_false) {
                *own_result = own_false;
                own_false = NULL; // Mark as transferred
            } else {
                // Create a copy of the reference
                if (ar__data__get_type(false_to_use) == DATA_INTEGER) {
                    *own_result = ar__data__create_integer(ar__data__get_integer(false_to_use));
                } else if (ar__data__get_type(false_to_use) == DATA_DOUBLE) {
                    *own_result = ar__data__create_double(ar__data__get_double(false_to_use));
                } else if (ar__data__get_type(false_to_use) == DATA_STRING) {
                    *own_result = ar__data__create_string(ar__data__get_string(false_to_use));
                } else {
                    // For maps and other types, we can't easily copy, so return 0
                    *own_result = ar__data__create_integer(0);
                }
            }
            // Clean up true value if we own it
            if (own_true) {
                ar__data__destroy(own_true);
                own_true = NULL; // Mark as destroyed
            }
        }
        
        // Clean up condition if we own it
        if (own_cond) {
            ar__data__destroy(own_cond);
            own_cond = NULL; // Mark as destroyed
        }
        return true;
    }
#endif
    else if (strcmp(function_name, "parse") == 0) {
        own_result->type = INST_PARSE;
        // parse(template, input) - requires exactly 2 arguments
        // Parses input string using template into a map of key-value pairs
        
        // Allocate args array for 2 arguments
        own_result->own_args = AR__HEAP__MALLOC(2 * sizeof(char*), "Parse arguments");
        if (!own_result->own_args) {
            ar__instruction__destroy_parsed(own_result);
            return NULL;
        }
        own_result->arg_count = 0;
        
        _skip_whitespace(ref_instruction, mut_pos);
        
        // Parse first argument (template)
        int arg_start = *mut_pos;
        int paren_depth = 0;
        bool in_quotes = false;
        
        // Find the comma that separates arguments
        while (ref_instruction[*mut_pos] && 
               (ref_instruction[*mut_pos] != ',' || paren_depth > 0 || in_quotes)) {
            if (ref_instruction[*mut_pos] == '"' && 
                (*mut_pos == 0 || ref_instruction[*mut_pos - 1] != '\\')) {
                in_quotes = !in_quotes;
            } else if (!in_quotes) {
                if (ref_instruction[*mut_pos] == '(') {
                    paren_depth++;
                } else if (ref_instruction[*mut_pos] == ')') {
                    paren_depth--;
                }
            }
            (*mut_pos)++;
        }
        
        if (ref_instruction[*mut_pos] != ',') {
            _set_error(mut_ctx, "Expected ',' after first parse() argument", *mut_pos);
            ar__instruction__destroy_parsed(own_result);
            return NULL;
        }
        
        // Extract and store first argument
        int arg_len = *mut_pos - arg_start;
        own_result->own_args[0] = AR__HEAP__MALLOC((size_t)(arg_len + 1), "Parse template argument");
        if (!own_result->own_args[0]) {
            ar__instruction__destroy_parsed(own_result);
            return NULL;
        }
        memcpy(own_result->own_args[0], ref_instruction + arg_start, (size_t)arg_len);
        own_result->own_args[0][arg_len] = '\0';
        own_result->arg_count = 1;
        
        (*mut_pos)++; // Skip ','
        _skip_whitespace(ref_instruction, mut_pos);
        
        // Parse second argument (input)
        arg_start = *mut_pos;
        paren_depth = 0;
        in_quotes = false;
        
        // Find the closing parenthesis
        while (ref_instruction[*mut_pos] && 
               (ref_instruction[*mut_pos] != ')' || paren_depth > 0 || in_quotes)) {
            if (ref_instruction[*mut_pos] == '"' && 
                (*mut_pos == 0 || ref_instruction[*mut_pos - 1] != '\\')) {
                in_quotes = !in_quotes;
            } else if (!in_quotes) {
                if (ref_instruction[*mut_pos] == '(') {
                    paren_depth++;
                } else if (ref_instruction[*mut_pos] == ')') {
                    paren_depth--;
                }
            }
            (*mut_pos)++;
        }
        
        if (ref_instruction[*mut_pos] != ')') {
            _set_error(mut_ctx, "Expected ')' after second parse() argument", *mut_pos);
            ar__instruction__destroy_parsed(own_result);
            return NULL;
        }
        
        // Extract and store second argument
        arg_len = *mut_pos - arg_start;
        own_result->own_args[1] = AR__HEAP__MALLOC((size_t)(arg_len + 1), "Parse input argument");
        if (!own_result->own_args[1]) {
            ar__instruction__destroy_parsed(own_result);
            return NULL;
        }
        memcpy(own_result->own_args[1], ref_instruction + arg_start, (size_t)arg_len);
        own_result->own_args[1][arg_len] = '\0';
        own_result->arg_count = 2;
        
        (*mut_pos)++; // Skip ')'
        
        // Transfer result path ownership if provided
        if (out_result_path && *out_result_path) {
            own_result->own_result_path = *out_result_path;
            *out_result_path = NULL;
        }
        
        return own_result;
#if 0
        // OLD PARSE IMPLEMENTATION
        
        if (owns_template) {
            // We own the value
            if (ar__data__get_type(own_template) != DATA_STRING) {
                ar__expression__destroy_context(own_context);
                ar__data__destroy(own_template);
                return false;
            }
            template_str = ar__data__get_string(own_template);
        } else {
            // It's a reference - use the evaluation result directly
            if (!ref_eval_result || ar__data__get_type(ref_eval_result) != DATA_STRING) {
                ar__expression__destroy_context(own_context);
                return false;
            }
            template_str = ar__data__get_string(ref_eval_result);
        }
        
        // Clean up context after getting the string
        ar__expression__destroy_context(own_context);
        own_context = NULL; // Mark as destroyed
        
        _skip_whitespace(ref_instruction, mut_pos);
        
        // Expect comma
        if (ref_instruction[*mut_pos] != ',') {
            if (owns_template && own_template) {
                ar__data__destroy(own_template);
                own_template = NULL; // Mark as destroyed
            }
            return false;
        }
        (*mut_pos)++; // Skip ','
        _skip_whitespace(ref_instruction, mut_pos);
        
        // Parse input expression - reusing the context variable
        own_context = ar__expression__create_context(mut_ctx->mut_memory, 
                                              mut_ctx->ref_context, 
                                              mut_ctx->ref_message, 
                                              ref_instruction + *mut_pos);
        if (!own_context) {
            ar__data__destroy(own_template);
            own_template = NULL; // Mark as destroyed
            return false;
        }
        const data_t *ref_input_eval = ar__expression__evaluate(own_context);
        data_t *own_input = ar__expression__take_ownership(own_context, ref_input_eval);
        *mut_pos += ar__expression__offset(own_context);
        
        // Handle both owned values and references for input
        const char *input_str = NULL;
        bool owns_input = (own_input != NULL);
        
        if (owns_input) {
            // We own the value
            if (ar__data__get_type(own_input) != DATA_STRING) {
                ar__expression__destroy_context(own_context);
                ar__data__destroy(own_input);
                if (owns_template && own_template) {
                    ar__data__destroy(own_template);
                    own_template = NULL; // Mark as destroyed
                }
                return false;
            }
            input_str = ar__data__get_string(own_input);
        } else {
            // It's a reference - use the evaluation result directly
            if (!ref_input_eval || ar__data__get_type(ref_input_eval) != DATA_STRING) {
                ar__expression__destroy_context(own_context);
                if (owns_template && own_template) {
                    ar__data__destroy(own_template);
                    own_template = NULL; // Mark as destroyed
                }
                return false;
            }
            input_str = ar__data__get_string(ref_input_eval);
        }
        
        // Clean up context after getting the string
        ar__expression__destroy_context(own_context);
        own_context = NULL; // Mark as destroyed
        
        _skip_whitespace(ref_instruction, mut_pos);
        
        // Expect closing parenthesis
        if (ref_instruction[*mut_pos] != ')') {
            if (owns_input && own_input) {
                ar__data__destroy(own_input);
                own_input = NULL; // Mark as destroyed
            }
            if (owns_template && own_template) {
                ar__data__destroy(own_template);
                own_template = NULL; // Mark as destroyed
            }
            return false;
        }
        (*mut_pos)++; // Skip ')'
        
        // Create result map (owned by us)
        *own_result = ar__data__create_map();
        if (!*own_result) {
            if (owns_input && own_input) {
                ar__data__destroy(own_input);
                own_input = NULL; // Mark as destroyed
            }
            if (owns_template && own_template) {
                ar__data__destroy(own_template);
                own_template = NULL; // Mark as destroyed
            }
            return false;
        }
        
        // Parse the template and input to extract values
        // Template format: "key1={var1}, key2={var2}"
        // Input format: "key1=value1, key2=value2"
        
        // Simple implementation: look for {variable} patterns in template
        const char *template_ptr = template_str;
        const char *input_ptr = input_str;
        
        while (*template_ptr && *input_ptr) {
            // Look for placeholder start
            const char *placeholder_start = strchr(template_ptr, '{');
            if (!placeholder_start) {
                // No more placeholders, check if remaining template matches input
                if (strcmp(template_ptr, input_ptr) != 0) {
                    // Mismatch - parsing failed, return empty map
                    ar__data__destroy(*own_result);
                    *own_result = ar__data__create_map();
                }
                break;
            }
            
            // Check if the literal part before placeholder matches
            size_t literal_len = (size_t)(placeholder_start - template_ptr);
            if (strncmp(template_ptr, input_ptr, literal_len) != 0) {
                // Mismatch - parsing failed, return empty map
                ar__data__destroy(*own_result);
                *own_result = ar__data__create_map();
                break;
            }
            
            // Move past the literal part
            template_ptr = placeholder_start + 1; // Skip '{'
            input_ptr += literal_len;
            
            // Find the end of the placeholder
            const char *placeholder_end = strchr(template_ptr, '}');
            if (!placeholder_end) {
                // Invalid template - no closing brace
                ar__data__destroy(*own_result);
                *own_result = ar__data__create_map();
                break;
            }
            
            // Extract variable name
            size_t var_len = (size_t)(placeholder_end - template_ptr);
            char *var_name = (char*)AR__HEAP__MALLOC(var_len + 1, "Parse variable name");
            if (!var_name) {
                ar__data__destroy(*own_result);
                *own_result = ar__data__create_map();
                break;
            }
            strncpy(var_name, template_ptr, var_len);
            var_name[var_len] = '\0';
            
            // Move past the placeholder
            template_ptr = placeholder_end + 1; // Skip '}'
            
            // Find the next literal part or end of template
            const char *next_literal_start = template_ptr;
            const char *next_placeholder = strchr(template_ptr, '{');
            size_t next_literal_len = next_placeholder ? 
                (size_t)(next_placeholder - next_literal_start) : strlen(next_literal_start);
            
            // Extract the value from input
            const char *value_end = NULL;
            if (next_literal_len > 0) {
                // Create a temporary string with just the literal part (not including next placeholder)
                char *literal_to_find = (char*)AR__HEAP__MALLOC(next_literal_len + 1, "Parse literal");
                if (literal_to_find) {
                    strncpy(literal_to_find, next_literal_start, next_literal_len);
                    literal_to_find[next_literal_len] = '\0';
                    
                    // Look for the next literal part in the input
                    value_end = strstr(input_ptr, literal_to_find);
                    AR__HEAP__FREE(literal_to_find);
                }
            } else {
                // No more literals, take the rest of the input
                value_end = input_ptr + strlen(input_ptr);
            }
            
            if (value_end) {
                // Extract the value
                size_t value_len = (size_t)(value_end - input_ptr);
                char *value_str = (char*)AR__HEAP__MALLOC(value_len + 1, "Parse value string");
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
                        ar__data__set_map_data(*own_result, var_name, own_value);
                        // Ownership of own_value is transferred
                    }
                    
                    AR__HEAP__FREE(value_str);
                }
                
                // Move input pointer past the value
                input_ptr = value_end;
            } else {
                // Could not find the next literal - parsing failed
                ar__data__destroy(*own_result);
                *own_result = ar__data__create_map();
                AR__HEAP__FREE(var_name);
                break;
            }
            
            AR__HEAP__FREE(var_name);
        }
        
        // Clean up
        if (owns_input && own_input) {
            ar__data__destroy(own_input);
            own_input = NULL; // Mark as destroyed
        }
        if (owns_template && own_template) {
            ar__data__destroy(own_template);
            own_template = NULL; // Mark as destroyed
        }
        
        return true;
#endif
    }
    else if (strcmp(function_name, "build") == 0) {
        own_result->type = INST_BUILD;
        // build(template, map) - requires exactly 2 arguments
        
        // Allocate args array for 2 arguments
        own_result->own_args = AR__HEAP__MALLOC(2 * sizeof(char*), "Build arguments");
        if (!own_result->own_args) {
            ar__instruction__destroy_parsed(own_result);
            return NULL;
        }
        own_result->arg_count = 0;
        
        _skip_whitespace(ref_instruction, mut_pos);
        
        // Parse first argument (template expression)
        int arg_start = *mut_pos;
        int paren_depth = 0;
        bool in_quotes = false;
        
        // Find the comma that separates arguments
        while (ref_instruction[*mut_pos] && 
               (ref_instruction[*mut_pos] != ',' || paren_depth > 0 || in_quotes)) {
            if (ref_instruction[*mut_pos] == '"' && 
                (*mut_pos == 0 || ref_instruction[*mut_pos - 1] != '\\')) {
                in_quotes = !in_quotes;
            } else if (!in_quotes) {
                if (ref_instruction[*mut_pos] == '(') paren_depth++;
                else if (ref_instruction[*mut_pos] == ')') paren_depth--;
            }
            (*mut_pos)++;
        }
        
        if (ref_instruction[*mut_pos] != ',') {
            _set_error(mut_ctx, "build() requires exactly 2 arguments", *mut_pos);
            ar__instruction__destroy_parsed(own_result);
            return NULL;
        }
        
        // Extract and store first argument
        int arg_len = *mut_pos - arg_start;
        while (arg_len > 0 && isspace((unsigned char)ref_instruction[arg_start])) {
            arg_start++;
            arg_len--;
        }
        while (arg_len > 0 && isspace((unsigned char)ref_instruction[arg_start + arg_len - 1])) {
            arg_len--;
        }
        
        own_result->own_args[0] = AR__HEAP__MALLOC((size_t)(arg_len + 1), "Template expression");
        if (!own_result->own_args[0]) {
            ar__instruction__destroy_parsed(own_result);
            return NULL;
        }
        memcpy(own_result->own_args[0], ref_instruction + arg_start, (size_t)arg_len);
        own_result->own_args[0][arg_len] = '\0';
        own_result->arg_count = 1;
        
        (*mut_pos)++; // Skip ','
        _skip_whitespace(ref_instruction, mut_pos);
        
        // Parse second argument (map expression)
        arg_start = *mut_pos;
        paren_depth = 0;
        in_quotes = false;
        
        // Find the closing parenthesis
        while (ref_instruction[*mut_pos] && 
               (ref_instruction[*mut_pos] != ')' || paren_depth > 0 || in_quotes)) {
            if (ref_instruction[*mut_pos] == '"' && 
                (*mut_pos == 0 || ref_instruction[*mut_pos - 1] != '\\')) {
                in_quotes = !in_quotes;
            } else if (!in_quotes) {
                if (ref_instruction[*mut_pos] == '(') paren_depth++;
                else if (ref_instruction[*mut_pos] == ')') {
                    if (paren_depth > 0) paren_depth--;
                    else break;
                }
            }
            (*mut_pos)++;
        }
        
        if (ref_instruction[*mut_pos] != ')') {
            _set_error(mut_ctx, "Expected ')' after build() arguments", *mut_pos);
            ar__instruction__destroy_parsed(own_result);
            return NULL;
        }
        
        // Extract and store second argument
        arg_len = *mut_pos - arg_start;
        while (arg_len > 0 && isspace((unsigned char)ref_instruction[arg_start])) {
            arg_start++;
            arg_len--;
        }
        while (arg_len > 0 && isspace((unsigned char)ref_instruction[arg_start + arg_len - 1])) {
            arg_len--;
        }
        
        own_result->own_args[1] = AR__HEAP__MALLOC((size_t)(arg_len + 1), "Map expression");
        if (!own_result->own_args[1]) {
            ar__instruction__destroy_parsed(own_result);
            return NULL;
        }
        memcpy(own_result->own_args[1], ref_instruction + arg_start, (size_t)arg_len);
        own_result->own_args[1][arg_len] = '\0';
        own_result->arg_count = 2;
        
        (*mut_pos)++; // Skip ')'
        
        // Transfer result path ownership if provided
        if (out_result_path && *out_result_path) {
            own_result->own_result_path = *out_result_path;
            *out_result_path = NULL;
        }
        
        return own_result;
#if 0
        // OLD BUILD IMPLEMENTATION
        own_context = ar__expression__create_context(mut_ctx->mut_memory, 
                                              mut_ctx->ref_context, 
                                              mut_ctx->ref_message, 
                                              ref_instruction + *mut_pos);
        if (!own_context) {
            return false;
        }
        const data_t *ref_template_eval = ar__expression__evaluate(own_context);
        data_t *own_template = ar__expression__take_ownership(own_context, ref_template_eval);
        *mut_pos += ar__expression__offset(own_context);
        
        // Handle both owned values and references for template
        const char *template_str = NULL;
        bool owns_template = (own_template != NULL);
        
        if (owns_template) {
            // We own the value
            if (ar__data__get_type(own_template) != DATA_STRING) {
                ar__expression__destroy_context(own_context);
                ar__data__destroy(own_template);
                return false;
            }
            template_str = ar__data__get_string(own_template);
        } else {
            // It's a reference - use the evaluation result directly
            if (!ref_template_eval || ar__data__get_type(ref_template_eval) != DATA_STRING) {
                ar__expression__destroy_context(own_context);
                return false;
            }
            template_str = ar__data__get_string(ref_template_eval);
        }
        
        // Clean up context after getting the string
        ar__expression__destroy_context(own_context);
        own_context = NULL; // Mark as destroyed
        
        _skip_whitespace(ref_instruction, mut_pos);
        
        // Expect comma
        if (ref_instruction[*mut_pos] != ',') {
            if (owns_template && own_template) {
                ar__data__destroy(own_template);
                own_template = NULL; // Mark as destroyed
            }
            return false;
        }
        (*mut_pos)++; // Skip ','
        _skip_whitespace(ref_instruction, mut_pos);
        
        // Parse values expression - reusing the context variable
        own_context = ar__expression__create_context(mut_ctx->mut_memory, 
                                              mut_ctx->ref_context, 
                                              mut_ctx->ref_message, 
                                              ref_instruction + *mut_pos);
        if (!own_context) {
            if (owns_template && own_template) {
                ar__data__destroy(own_template);
                own_template = NULL; // Mark as destroyed
            }
            return false;
        }
        const data_t *ref_values = ar__expression__evaluate(own_context);
        *mut_pos += ar__expression__offset(own_context);
        
        if (!ref_values) {
            ar__expression__destroy_context(own_context);
            own_context = NULL; // Mark as destroyed
            if (owns_template && own_template) {
                ar__data__destroy(own_template);
                own_template = NULL; // Mark as destroyed
            }
            return false;
        }
        
        // Try to take ownership. If it fails, the value is a reference to existing data
        data_t *own_values = ar__expression__take_ownership(own_context, ref_values);
        
        // Clean up context immediately
        ar__expression__destroy_context(own_context);
        own_context = NULL; // Mark as destroyed
        
        // Use ref_values if we couldn't take ownership
        const data_t *values_to_use = own_values ? own_values : ref_values;
        
        // Ensure values is a map
        if (ar__data__get_type(values_to_use) != DATA_MAP) {
            if (own_values) {
                ar__data__destroy(own_values);
                own_values = NULL; // Mark as destroyed
            }
            if (owns_template && own_template) {
                ar__data__destroy(own_template);
                own_template = NULL; // Mark as destroyed
            }
            return false;
        }
        
        _skip_whitespace(ref_instruction, mut_pos);
        
        // Expect closing parenthesis
        if (ref_instruction[*mut_pos] != ')') {
            if (own_values) {
                ar__data__destroy(own_values);
                own_values = NULL; // Mark as destroyed
            }
            if (owns_template && own_template) {
                ar__data__destroy(own_template);
                own_template = NULL; // Mark as destroyed
            }
            return false;
        }
        (*mut_pos)++; // Skip ')'
        
        // Build the string by replacing placeholders in template
        // Template format: "Hello {name}, you are {age} years old"
        // Values: map with keys "name" and "age"
        
        // Create a string builder for the result
        size_t result_size = strlen(template_str) * 2 + 256; // Start with a reasonable size
        char *own_result_str = (char*)AR__HEAP__MALLOC(result_size, "Build result string");
        if (!own_result_str) {
            ar__data__destroy(own_values);
            own_values = NULL; // Mark as destroyed
            ar__data__destroy(own_template);
            own_template = NULL; // Mark as destroyed
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
                    char *var_name = (char*)AR__HEAP__MALLOC(var_len + 1, "Build variable name");
                    if (!var_name) {
                        AR__HEAP__FREE(own_result_str);
                        if (own_values) {
                            ar__data__destroy(own_values);
                            own_values = NULL; // Mark as destroyed
                        }
                        ar__data__destroy(own_template);
                        own_template = NULL; // Mark as destroyed
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
                                char *new_result = (char*)AR__HEAP__MALLOC(result_size, "Build result resize");
                                if (!new_result) {
                                    AR__HEAP__FREE(var_name);
                                    AR__HEAP__FREE(own_result_str);
                                    if (own_values) {
                                        ar__data__destroy(own_values);
                                        own_values = NULL; // Mark as destroyed
                                    }
                                    ar__data__destroy(own_template);
                                    own_template = NULL; // Mark as destroyed
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
                    }
                    
                    AR__HEAP__FREE(var_name);
                    
                    // Move past the placeholder
                    template_ptr = placeholder_end + 1;
                } else {
                    // No closing brace found, copy the '{' literally
                    if (result_pos >= result_size - 1) {
                        result_size *= 2;
                        char *new_result = (char*)AR__HEAP__MALLOC(result_size, "Build result resize");
                        if (!new_result) {
                            AR__HEAP__FREE(own_result_str);
                            if (own_values) {
                                ar__data__destroy(own_values);
                                own_values = NULL; // Mark as destroyed
                            }
                            if (owns_template && own_template) {
                                ar__data__destroy(own_template);
                                own_template = NULL; // Mark as destroyed
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
                    char *new_result = (char*)AR__HEAP__MALLOC(result_size, "Build result resize");
                    if (!new_result) {
                        AR__HEAP__FREE(own_result_str);
                        if (own_values) {
                            ar__data__destroy(own_values);
                            own_values = NULL; // Mark as destroyed
                        }
                        if (owns_template && own_template) {
                            ar__data__destroy(own_template);
                            own_template = NULL; // Mark as destroyed
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
        
        // Create the result string data object
        *own_result = ar__data__create_string(own_result_str);
        
        // Clean up
        AR__HEAP__FREE(own_result_str);
        if (own_values) {
            ar__data__destroy(own_values);
            own_values = NULL; // Mark as destroyed
        }
        if (owns_template && own_template) {
            ar__data__destroy(own_template);
            own_template = NULL; // Mark as destroyed
        }
        
        return (*own_result != NULL);
#endif
    }
    else if (strcmp(function_name, "method") == 0) {
        own_result->type = INST_METHOD;
        // method(name, instructions, version) - requires exactly 3 arguments
        
        // Allocate args array for 3 arguments
        own_result->own_args = AR__HEAP__MALLOC(3 * sizeof(char*), "Method arguments");
        if (!own_result->own_args) {
            ar__instruction__destroy_parsed(own_result);
            return NULL;
        }
        own_result->arg_count = 0;
        
        _skip_whitespace(ref_instruction, mut_pos);
        
        // Parse three arguments separated by commas
        for (int i = 0; i < 3; i++) {
            int arg_start = *mut_pos;
            int paren_depth = 0;
            bool in_quotes = false;
            char target_char = (i < 2) ? ',' : ')';
            
            // Find the comma or closing paren
            while (ref_instruction[*mut_pos]) {
                char c = ref_instruction[*mut_pos];
                
                if (c == '"' && (*mut_pos == 0 || ref_instruction[*mut_pos - 1] != '\\')) {
                    in_quotes = !in_quotes;
                } else if (!in_quotes) {
                    if (c == '(') paren_depth++;
                    else if (c == ')') {
                        if (paren_depth > 0) paren_depth--;
                        else if (target_char == ')') break;
                    }
                    else if (c == ',' && paren_depth == 0 && target_char == ',') break;
                }
                (*mut_pos)++;
            }
            
            if (ref_instruction[*mut_pos] != target_char) {
                _set_error(mut_ctx, "method() requires exactly 3 arguments", *mut_pos);
                ar__instruction__destroy_parsed(own_result);
                return NULL;
            }
            
            // Extract and store argument
            int arg_len = *mut_pos - arg_start;
            while (arg_len > 0 && isspace((unsigned char)ref_instruction[arg_start])) {
                arg_start++;
                arg_len--;
            }
            while (arg_len > 0 && isspace((unsigned char)ref_instruction[arg_start + arg_len - 1])) {
                arg_len--;
            }
            
            const char *arg_name = (i == 0) ? "Method name" : 
                                   (i == 1) ? "Method instructions" : "Method version";
            own_result->own_args[i] = AR__HEAP__MALLOC((size_t)(arg_len + 1), arg_name);
            if (!own_result->own_args[i]) {
                ar__instruction__destroy_parsed(own_result);
                return NULL;
            }
            memcpy(own_result->own_args[i], ref_instruction + arg_start, (size_t)arg_len);
            own_result->own_args[i][arg_len] = '\0';
            own_result->arg_count++;
            
            if (i < 2) {
                (*mut_pos)++; // Skip ','
                _skip_whitespace(ref_instruction, mut_pos);
            }
        }
        
        (*mut_pos)++; // Skip ')'
        
        // Transfer result path ownership if provided
        if (out_result_path && *out_result_path) {
            own_result->own_result_path = *out_result_path;
            *out_result_path = NULL;
        }
        
        return own_result;
#if 0
        // OLD METHOD IMPLEMENTATION
        
        // Parse method name expression
        own_context = ar__expression__create_context(mut_ctx->mut_memory, 
                                              mut_ctx->ref_context, 
                                              mut_ctx->ref_message, 
                                              ref_instruction + *mut_pos);
        if (!own_context) {
            return false;
        }
        const data_t *ref_name_eval = ar__expression__evaluate(own_context);
        data_t *own_name = ar__expression__take_ownership(own_context, ref_name_eval);
        *mut_pos += ar__expression__offset(own_context);
        
        // Handle both owned values and references for method name
        const char *method_name = NULL;
        bool owns_name = (own_name != NULL);
        
        if (owns_name) {
            // We own the value
            if (ar__data__get_type(own_name) != DATA_STRING) {
                ar__expression__destroy_context(own_context);
                ar__data__destroy(own_name);
                return false;
            }
            method_name = ar__data__get_string(own_name);
        } else {
            // It's a reference - use the evaluation result directly
            if (!ref_name_eval || ar__data__get_type(ref_name_eval) != DATA_STRING) {
                ar__expression__destroy_context(own_context);
                return false;
            }
            method_name = ar__data__get_string(ref_name_eval);
        }
        
        // Clean up context after getting the string
        ar__expression__destroy_context(own_context);
        own_context = NULL; // Mark as destroyed
        
        _skip_whitespace(ref_instruction, mut_pos);
        
        // Expect comma
        if (ref_instruction[*mut_pos] != ',') {
            if (owns_name && own_name) {
                ar__data__destroy(own_name);
                own_name = NULL; // Mark as destroyed
            }
            return false;
        }
        (*mut_pos)++; // Skip ','
        _skip_whitespace(ref_instruction, mut_pos);
        
        // Parse instructions expression - reusing the context variable
        own_context = ar__expression__create_context(mut_ctx->mut_memory, 
                                              mut_ctx->ref_context, 
                                              mut_ctx->ref_message, 
                                              ref_instruction + *mut_pos);
        if (!own_context) {
            if (owns_name && own_name) {
                ar__data__destroy(own_name);
                own_name = NULL; // Mark as destroyed
            }
            return false;
        }
        const data_t *ref_instr_eval = ar__expression__evaluate(own_context);
        data_t *own_instr = ar__expression__take_ownership(own_context, ref_instr_eval);
        *mut_pos += ar__expression__offset(own_context);
        
        // Handle both owned values and references for instructions
        const char *instructions = NULL;
        bool owns_instr = (own_instr != NULL);
        
        if (owns_instr) {
            // We own the value
            if (ar__data__get_type(own_instr) != DATA_STRING) {
                ar__expression__destroy_context(own_context);
                ar__data__destroy(own_instr);
                if (owns_name && own_name) {
                    ar__data__destroy(own_name);
                    own_name = NULL; // Mark as destroyed
                }
                return false;
            }
            instructions = ar__data__get_string(own_instr);
        } else {
            // It's a reference - use the evaluation result directly
            if (!ref_instr_eval || ar__data__get_type(ref_instr_eval) != DATA_STRING) {
                ar__expression__destroy_context(own_context);
                if (owns_name && own_name) {
                    ar__data__destroy(own_name);
                    own_name = NULL; // Mark as destroyed
                }
                return false;
            }
            instructions = ar__data__get_string(ref_instr_eval);
        }
        
        // Clean up context after getting the string
        ar__expression__destroy_context(own_context);
        own_context = NULL; // Mark as destroyed
        
        _skip_whitespace(ref_instruction, mut_pos);
        
        // Expect comma
        if (ref_instruction[*mut_pos] != ',') {
            if (owns_instr && own_instr) {
                ar__data__destroy(own_instr);
                own_instr = NULL; // Mark as destroyed
            }
            if (owns_name && own_name) {
                ar__data__destroy(own_name);
                own_name = NULL; // Mark as destroyed
            }
            return false;
        }
        (*mut_pos)++; // Skip ','
        _skip_whitespace(ref_instruction, mut_pos);
        
        // Parse version expression - reusing the context variable
        own_context = ar__expression__create_context(mut_ctx->mut_memory, 
                                              mut_ctx->ref_context, 
                                              mut_ctx->ref_message, 
                                              ref_instruction + *mut_pos);
        if (!own_context) {
            if (owns_instr && own_instr) {
                ar__data__destroy(own_instr);
                own_instr = NULL; // Mark as destroyed
            }
            if (owns_name && own_name) {
                ar__data__destroy(own_name);
                own_name = NULL; // Mark as destroyed
            }
            return false;
        }
        const data_t *ref_version_eval = ar__expression__evaluate(own_context);
        data_t *own_version = ar__expression__take_ownership(own_context, ref_version_eval);
        *mut_pos += ar__expression__offset(own_context);
        
        // Handle both owned values and references for version
        const char *version_str = "1.0.0";
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
        
        // Clean up context after getting the version
        ar__expression__destroy_context(own_context);
        own_context = NULL; // Mark as destroyed
        
        _skip_whitespace(ref_instruction, mut_pos);
        
        // Expect closing parenthesis
        if (ref_instruction[*mut_pos] != ')') {
            if (owns_version && own_version) {
                ar__data__destroy(own_version);
                own_version = NULL; // Mark as destroyed
            }
            if (owns_instr && own_instr) {
                ar__data__destroy(own_instr);
                own_instr = NULL; // Mark as destroyed
            }
            if (owns_name && own_name) {
                ar__data__destroy(own_name);
                own_name = NULL; // Mark as destroyed
            }
            return false;
        }
        (*mut_pos)++; // Skip ')'
        
        // Call methodology module directly to create method with just 3 parameters:
        // name, instructions, version
        bool success = ar__methodology__create_method(method_name, instructions, version_str);
        
        // Clean up input data now that we're done with it
        if (owns_version && own_version) {
            ar__data__destroy(own_version);
            own_version = NULL; // Mark as destroyed
        }
        if (owns_instr && own_instr) {
            ar__data__destroy(own_instr);
            own_instr = NULL; // Mark as destroyed
        }
        if (owns_name && own_name) {
            ar__data__destroy(own_name);
            own_name = NULL; // Mark as destroyed
        }
        
        // Return success indicator
        *own_result = ar__data__create_integer(success ? 1 : 0);
        return true;
#endif
    }
    else if (strcmp(function_name, "agent") == 0) {
        own_result->type = INST_AGENT;
        // agent(method_name, version, context) - requires exactly 3 arguments
        
        // Allocate args array for 3 arguments
        own_result->own_args = AR__HEAP__MALLOC(3 * sizeof(char*), "Agent arguments");
        if (!own_result->own_args) {
            ar__instruction__destroy_parsed(own_result);
            return NULL;
        }
        own_result->arg_count = 0;
        
        _skip_whitespace(ref_instruction, mut_pos);
        
        // Parse three arguments separated by commas
        for (int i = 0; i < 3; i++) {
            int arg_start = *mut_pos;
            int paren_depth = 0;
            bool in_quotes = false;
            char target_char = (i < 2) ? ',' : ')';
            
            // Find the comma or closing paren
            while (ref_instruction[*mut_pos]) {
                char c = ref_instruction[*mut_pos];
                
                if (c == '"' && (*mut_pos == 0 || ref_instruction[*mut_pos - 1] != '\\')) {
                    in_quotes = !in_quotes;
                } else if (!in_quotes) {
                    if (c == '(') paren_depth++;
                    else if (c == ')') {
                        if (paren_depth > 0) paren_depth--;
                        else if (target_char == ')') break;
                    }
                    else if (c == ',' && paren_depth == 0 && target_char == ',') break;
                }
                (*mut_pos)++;
            }
            
            if (ref_instruction[*mut_pos] != target_char) {
                _set_error(mut_ctx, "agent() requires exactly 3 arguments", *mut_pos);
                ar__instruction__destroy_parsed(own_result);
                return NULL;
            }
            
            // Extract and store argument
            int arg_len = *mut_pos - arg_start;
            while (arg_len > 0 && isspace((unsigned char)ref_instruction[arg_start])) {
                arg_start++;
                arg_len--;
            }
            while (arg_len > 0 && isspace((unsigned char)ref_instruction[arg_start + arg_len - 1])) {
                arg_len--;
            }
            
            const char *arg_name = (i == 0) ? "Method name" : 
                                   (i == 1) ? "Version" : "Context";
            own_result->own_args[i] = AR__HEAP__MALLOC((size_t)(arg_len + 1), arg_name);
            if (!own_result->own_args[i]) {
                ar__instruction__destroy_parsed(own_result);
                return NULL;
            }
            memcpy(own_result->own_args[i], ref_instruction + arg_start, (size_t)arg_len);
            own_result->own_args[i][arg_len] = '\0';
            own_result->arg_count++;
            
            if (i < 2) {
                (*mut_pos)++; // Skip ','
                _skip_whitespace(ref_instruction, mut_pos);
            }
        }
        
        (*mut_pos)++; // Skip ')'
        
        // Transfer result path ownership if provided
        if (out_result_path && *out_result_path) {
            own_result->own_result_path = *out_result_path;
            *out_result_path = NULL;
        }
        
        return own_result;
#if 0
        // OLD AGENT IMPLEMENTATION
        
        // Parse method name expression
        own_context = ar__expression__create_context(mut_ctx->mut_memory, 
                                              mut_ctx->ref_context, 
                                              mut_ctx->ref_message, 
                                              ref_instruction + *mut_pos);
        if (!own_context) {
            return false;
        }
        const data_t *ref_method_name_eval = ar__expression__evaluate(own_context);
        if (!ref_method_name_eval) {
            ar__expression__destroy_context(own_context);
            return false;
        }
        data_t *own_method_name = ar__expression__take_ownership(own_context, ref_method_name_eval);
        *mut_pos += ar__expression__offset(own_context);
        
        // Handle both owned values and references for method name
        const char *method_name = NULL;
        bool owns_method_name = (own_method_name != NULL);
        
        if (owns_method_name) {
            // We own the value
            if (ar__data__get_type(own_method_name) != DATA_STRING) {
                ar__expression__destroy_context(own_context);
                ar__data__destroy(own_method_name);
                return false;
            }
            method_name = ar__data__get_string(own_method_name);
        } else {
            // It's a reference - use the evaluation result directly
            if (!ref_method_name_eval || ar__data__get_type(ref_method_name_eval) != DATA_STRING) {
                ar__expression__destroy_context(own_context);
                return false;
            }
            method_name = ar__data__get_string(ref_method_name_eval);
        }
        
        // Clean up context after getting the string
        ar__expression__destroy_context(own_context);
        own_context = NULL; // Mark as destroyed
        
        _skip_whitespace(ref_instruction, mut_pos);
        
        // Expect comma
        if (ref_instruction[*mut_pos] != ',') {
            if (owns_method_name && own_method_name) {
                ar__data__destroy(own_method_name);
                own_method_name = NULL; // Mark as destroyed
            }
            return false;
        }
        (*mut_pos)++; // Skip ','
        
        _skip_whitespace(ref_instruction, mut_pos);
        
        // Parse version expression
        own_context = ar__expression__create_context(mut_ctx->mut_memory,
                                              mut_ctx->ref_context,
                                              mut_ctx->ref_message,
                                              ref_instruction + *mut_pos);
        if (!own_context) {
            if (owns_method_name && own_method_name) {
                ar__data__destroy(own_method_name);
                own_method_name = NULL; // Mark as destroyed
            }
            return false;
        }
        const data_t *ref_version_eval = ar__expression__evaluate(own_context);
        if (!ref_version_eval) {
            ar__expression__destroy_context(own_context);
            if (owns_method_name && own_method_name) {
                ar__data__destroy(own_method_name);
                own_method_name = NULL; // Mark as destroyed
            }
            return false;
        }
        data_t *own_version = ar__expression__take_ownership(own_context, ref_version_eval);
        *mut_pos += ar__expression__offset(own_context);
        
        // Handle both owned values and references for version
        const char *version_str = NULL;
        bool owns_version = (own_version != NULL);
        
        if (owns_version) {
            // We own the value
            if (ar__data__get_type(own_version) != DATA_STRING) {
                ar__expression__destroy_context(own_context);
                ar__data__destroy(own_version);
                if (owns_method_name && own_method_name) {
                    ar__data__destroy(own_method_name);
                    own_method_name = NULL; // Mark as destroyed
                }
                return false;
            }
            version_str = ar__data__get_string(own_version);
        } else {
            // It's a reference - use the evaluation result directly
            if (!ref_version_eval || ar__data__get_type(ref_version_eval) != DATA_STRING) {
                ar__expression__destroy_context(own_context);
                if (owns_method_name && own_method_name) {
                    ar__data__destroy(own_method_name);
                    own_method_name = NULL; // Mark as destroyed
                }
                return false;
            }
            version_str = ar__data__get_string(ref_version_eval);
        }
        
        // Clean up context after getting the string
        ar__expression__destroy_context(own_context);
        own_context = NULL; // Mark as destroyed
        
        _skip_whitespace(ref_instruction, mut_pos);
        
        // Expect comma
        if (ref_instruction[*mut_pos] != ',') {
            if (owns_version && own_version) {
                ar__data__destroy(own_version);
                own_version = NULL; // Mark as destroyed
            }
            if (owns_method_name && own_method_name) {
                ar__data__destroy(own_method_name);
                own_method_name = NULL; // Mark as destroyed
            }
            return false;
        }
        (*mut_pos)++; // Skip ','
        
        _skip_whitespace(ref_instruction, mut_pos);
        
        // Parse context expression
        own_context = ar__expression__create_context(mut_ctx->mut_memory,
                                              mut_ctx->ref_context,
                                              mut_ctx->ref_message,
                                              ref_instruction + *mut_pos);
        if (!own_context) {
            if (owns_version && own_version) {
                ar__data__destroy(own_version);
                own_version = NULL; // Mark as destroyed
            }
            if (owns_method_name && own_method_name) {
                ar__data__destroy(own_method_name);
                own_method_name = NULL; // Mark as destroyed
            }
            return false;
        }
        const data_t *ref_agent_context = ar__expression__evaluate(own_context);
        *mut_pos += ar__expression__offset(own_context);
        
        // Check if we need to take ownership of the context
        data_t *own_agent_context = ar__expression__take_ownership(own_context, ref_agent_context);
        bool owns_context = (own_agent_context != NULL);
        
        // Clean up expression context
        ar__expression__destroy_context(own_context);
        own_context = NULL; // Mark as destroyed
        
        if (!ref_agent_context) {
            if (owns_version && own_version) {
                ar__data__destroy(own_version);
                own_version = NULL; // Mark as destroyed
            }
            if (owns_method_name && own_method_name) {
                ar__data__destroy(own_method_name);
                own_method_name = NULL; // Mark as destroyed
            }
            return false;
        }
        
        _skip_whitespace(ref_instruction, mut_pos);
        
        // Expect closing parenthesis
        if (ref_instruction[*mut_pos] != ')') {
            if (owns_context && own_agent_context) {
                ar__data__destroy(own_agent_context);
                own_agent_context = NULL; // Mark as destroyed
            }
            if (owns_version && own_version) {
                ar__data__destroy(own_version);
                own_version = NULL; // Mark as destroyed
            }
            if (owns_method_name && own_method_name) {
                ar__data__destroy(own_method_name);
                own_method_name = NULL; // Mark as destroyed
            }
            return false;
        }
        (*mut_pos)++; // Skip ')'
        
        // Create the agent
        int64_t agent_id = ar__agency__create_agent(method_name, version_str, ref_agent_context);
        
        // Check if agent creation failed (method not found)
        if (agent_id == 0) {
            char error_msg[512];
            snprintf(error_msg, sizeof(error_msg), "Method '%s' version '%s' not found", method_name, version_str);
            _set_error(mut_ctx, error_msg, *mut_pos - 1); // Position at closing parenthesis
        }
        
        // Clean up input data now that we're done with it
        if (owns_context && own_agent_context) {
            ar__data__destroy(own_agent_context);
            own_agent_context = NULL; // Mark as destroyed
        }
        if (owns_version && own_version) {
            ar__data__destroy(own_version);
            own_version = NULL; // Mark as destroyed
        }
        if (owns_method_name && own_method_name) {
            ar__data__destroy(own_method_name);
            own_method_name = NULL; // Mark as destroyed
        }
        
        // Return agent ID as result (0 if creation failed)
        *own_result = ar__data__create_integer((int)agent_id);
        return true;
#endif
    }
    else if (strcmp(function_name, "destroy") == 0) {
        // Set instruction type
        own_result->type = INST_DESTROY;
        
        // Allocate args array - destroy can have 1 or 2 arguments
        own_result->own_args = AR__HEAP__MALLOC(sizeof(char*) * 2, "Destroy arguments");
        if (!own_result->own_args) {
            ar__instruction__destroy_parsed(own_result);
            return NULL;
        }
        own_result->own_args[0] = NULL;
        own_result->own_args[1] = NULL;
        own_result->arg_count = 0;
        
        // Parse first argument
        _skip_whitespace(ref_instruction, mut_pos);
        
        int arg_start = *mut_pos;
        int paren_depth = 0;
        bool in_quotes = false;
        
        // Find the comma or closing parenthesis
        while (ref_instruction[*mut_pos] && 
               (ref_instruction[*mut_pos] != ',' || paren_depth > 0 || in_quotes) &&
               (ref_instruction[*mut_pos] != ')' || paren_depth > 0 || in_quotes)) {
            if (ref_instruction[*mut_pos] == '"' && 
                (*mut_pos == 0 || ref_instruction[*mut_pos - 1] != '\\')) {
                in_quotes = !in_quotes;
            } else if (!in_quotes) {
                if (ref_instruction[*mut_pos] == '(') {
                    paren_depth++;
                } else if (ref_instruction[*mut_pos] == ')') {
                    paren_depth--;
                }
            }
            (*mut_pos)++;
        }
        
        // Extract and store first argument
        int arg_len = *mut_pos - arg_start;
        own_result->own_args[0] = AR__HEAP__MALLOC((size_t)(arg_len + 1), "Destroy first argument");
        if (!own_result->own_args[0]) {
            ar__instruction__destroy_parsed(own_result);
            return NULL;
        }
        memcpy(own_result->own_args[0], ref_instruction + arg_start, (size_t)arg_len);
        own_result->own_args[0][arg_len] = '\0';
        own_result->arg_count = 1;
        
        _skip_whitespace(ref_instruction, mut_pos);
        
        // Check if there's a comma (indicates method destruction)
        if (ref_instruction[*mut_pos] == ',') {
            // This is destroy(method_name, version)
            (*mut_pos)++; // Skip ','
            _skip_whitespace(ref_instruction, mut_pos);
            
            // Parse version argument
            arg_start = *mut_pos;
            paren_depth = 0;
            in_quotes = false;
            
            // Find the closing parenthesis
            while (ref_instruction[*mut_pos] && 
                   (ref_instruction[*mut_pos] != ')' || paren_depth > 0 || in_quotes)) {
                if (ref_instruction[*mut_pos] == '"' && 
                    (*mut_pos == 0 || ref_instruction[*mut_pos - 1] != '\\')) {
                    in_quotes = !in_quotes;
                } else if (!in_quotes) {
                    if (ref_instruction[*mut_pos] == '(') {
                        paren_depth++;
                    } else if (ref_instruction[*mut_pos] == ')') {
                        paren_depth--;
                    }
                }
                (*mut_pos)++;
            }
            
            // Extract and store second argument
            arg_len = *mut_pos - arg_start;
            own_result->own_args[1] = AR__HEAP__MALLOC((size_t)(arg_len + 1), "Destroy version argument");
            if (!own_result->own_args[1]) {
                ar__instruction__destroy_parsed(own_result);
                return NULL;
            }
            memcpy(own_result->own_args[1], ref_instruction + arg_start, (size_t)arg_len);
            own_result->own_args[1][arg_len] = '\0';
            own_result->arg_count = 2;
            
            _skip_whitespace(ref_instruction, mut_pos);
        }
        
        // Expect closing parenthesis
        if (ref_instruction[*mut_pos] != ')') {
            ar__instruction__destroy_parsed(own_result);
            return NULL;
        }
        (*mut_pos)++; // Skip ')'
        
        return own_result;
        
#if 0
        // destroy(agent_id) or destroy(method_name, version)
        _skip_whitespace(ref_instruction, mut_pos);
        
        // Create a single context for all expressions
        expression_context_t *own_context = NULL;
        
        // Parse first argument
        own_context = ar__expression__create_context(mut_ctx->mut_memory,
                                              mut_ctx->ref_context,
                                              mut_ctx->ref_message,
                                              ref_instruction + *mut_pos);
        if (!own_context) {
            return false;
        }
        const data_t *ref_arg1 = ar__expression__evaluate(own_context);
        
        // Check if we got a valid result
        bool has_result = (ref_arg1 != NULL);
        data_t *own_arg1 = NULL;
        if (has_result) {
            own_arg1 = ar__expression__take_ownership(own_context, ref_arg1);
        }
        *mut_pos += ar__expression__offset(own_context);
        
        // Clean up context immediately
        ar__expression__destroy_context(own_context);
        own_context = NULL; // Mark as destroyed
        
        // If we didn't get a result, parsing failed
        if (!has_result) {
            return false;
        }
        
        _skip_whitespace(ref_instruction, mut_pos);
        
        // Check if there's a comma (indicates method destruction)
        if (ref_instruction[*mut_pos] == ',') {
            // This is destroy(method_name, version)
            (*mut_pos)++; // Skip ','
            _skip_whitespace(ref_instruction, mut_pos);
            
            // Ensure first argument is a string (method name)
            const data_t *arg_to_use = own_arg1 ? own_arg1 : ref_arg1;
            if (ar__data__get_type(arg_to_use) != DATA_STRING) {
                if (own_arg1) {
                    ar__data__destroy(own_arg1);
                    own_arg1 = NULL; // Mark as destroyed
                }
                return false;
            }
            const char *method_name = ar__data__get_string(arg_to_use);
            
            // Parse version expression
            own_context = ar__expression__create_context(mut_ctx->mut_memory,
                                                  mut_ctx->ref_context,
                                                  mut_ctx->ref_message,
                                                  ref_instruction + *mut_pos);
            if (!own_context) {
                if (own_arg1) {
                    ar__data__destroy(own_arg1);
                    own_arg1 = NULL; // Mark as destroyed
                }
                return false;
            }
            const data_t *ref_version_eval = ar__expression__evaluate(own_context);
            data_t *own_version = ar__expression__take_ownership(own_context, ref_version_eval);
            *mut_pos += ar__expression__offset(own_context);
            
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
            
            // Clean up context immediately
            ar__expression__destroy_context(own_context);
            own_context = NULL; // Mark as destroyed
            
            _skip_whitespace(ref_instruction, mut_pos);
            
            // Expect closing parenthesis
            if (ref_instruction[*mut_pos] != ')') {
                if (owns_version && own_version) {
                    ar__data__destroy(own_version);
                    own_version = NULL; // Mark as destroyed
                }
                if (own_arg1) {
                    ar__data__destroy(own_arg1);
                    own_arg1 = NULL; // Mark as destroyed
                }
                return false;
            }
            (*mut_pos)++; // Skip ')'
            
            // Call methodology module to unregister the method
            bool success = ar__methodology__unregister_method(method_name, version_str);
            
            // Clean up
            if (owns_version && own_version) {
                ar__data__destroy(own_version);
                own_version = NULL; // Mark as destroyed
            }
            if (own_arg1) {
                ar__data__destroy(own_arg1);
                own_arg1 = NULL; // Mark as destroyed
            }
            
            // Return success indicator
            *own_result = ar__data__create_integer(success ? 1 : 0);
            return true;
        }
        else if (ref_instruction[*mut_pos] == ')') {
            // This is destroy(agent_id)
            (*mut_pos)++; // Skip ')'
            
            // Ensure argument is an integer (agent ID)
            const data_t *arg_to_use = own_arg1 ? own_arg1 : ref_arg1;
            if (ar__data__get_type(arg_to_use) != DATA_INTEGER) {
                if (own_arg1) {
                    ar__data__destroy(own_arg1);
                    own_arg1 = NULL; // Mark as destroyed
                }
                return false;
            }
            int64_t agent_id = (int64_t)ar__data__get_integer(arg_to_use);
            
            // Destroy the agent
            bool success = ar__agency__destroy_agent(agent_id);
            
            // Clean up
            if (own_arg1) {
                ar__data__destroy(own_arg1);
                own_arg1 = NULL; // Mark as destroyed
            }
            
            // Return success indicator
            *own_result = ar__data__create_integer(success ? 1 : 0);
            return true;
        }
        else {
            // Invalid syntax
            if (own_arg1) {
                ar__data__destroy(own_arg1);
                own_arg1 = NULL; // Mark as destroyed
            }
            return false;
        }
#endif
    }
    else {
        // Unknown function - return NULL to indicate parsing error
        ar__instruction__destroy_parsed(own_result);
        return NULL;
    }
    
    return NULL; // Should not reach here
}

// Error reporting functions

// Gets the last error message from the instruction context
const char* ar__instruction__get_last_error(const instruction_context_t *ref_ctx) {
    if (!ref_ctx) {
        return NULL;
    }
    return ref_ctx->own_error_message;
}

// Gets the position in the instruction string where the last error occurred
int ar__instruction__get_error_position(const instruction_context_t *ref_ctx) {
    if (!ref_ctx) {
        return 0;
    }
    return ref_ctx->error_position;
}

// Helper function to set error in context
static void _set_error(instruction_context_t *mut_ctx, const char *ref_message, int position) {
    if (!mut_ctx) {
        return;
    }
    
    // Free existing error message if any
    if (mut_ctx->own_error_message) {
        AR__HEAP__FREE(mut_ctx->own_error_message);
    }
    
    // Duplicate the new error message
    mut_ctx->own_error_message = AR__HEAP__STRDUP(ref_message, "Error message");
    mut_ctx->error_position = position + 1; // Convert 0-based to 1-based
}

// Helper function to clear error state
static void _clear_error(instruction_context_t *mut_ctx) {
    if (!mut_ctx) {
        return;
    }
    
    // Free existing error message if any
    if (mut_ctx->own_error_message) {
        AR__HEAP__FREE(mut_ctx->own_error_message);
        mut_ctx->own_error_message = NULL;
    }
    mut_ctx->error_position = 0;
}

// Utility functions

static bool _skip_whitespace(const char *ref_instruction, int *mut_pos) {
    while (ref_instruction[*mut_pos] && isspace((unsigned char)ref_instruction[*mut_pos])) {
        (*mut_pos)++;
    }
    return true;
}

static bool _extract_identifier(const char *ref_instruction, int *mut_pos, char *mut_identifier, int max_size) {
    _skip_whitespace(ref_instruction, mut_pos);
    
    int i = 0;
    // First character must be a letter
    if (!isalpha((unsigned char)ref_instruction[*mut_pos])) {
        return false;
    }
    
    // Extract the identifier
    while (i < max_size - 1 && 
           (isalnum((unsigned char)ref_instruction[*mut_pos]) || ref_instruction[*mut_pos] == '_')) {
        mut_identifier[i++] = ref_instruction[*mut_pos];
        (*mut_pos)++;
    }
    
    mut_identifier[i] = '\0';
    return i > 0;
}
