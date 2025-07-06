#include "ar_instruction.h"
#include "ar_string.h"
#include "ar_data.h"
#include "ar_expression.h"
#include "ar_map.h"
#include "ar_assert.h" // Include for ownership assertions
#include "ar_heap.h" // Include for memory allocation macros

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>

// Instruction context structure definition
struct instruction_context_s {
    ar_data_t *mut_memory;        // Mutable reference to memory, not owned
    const ar_data_t *ref_context; // Borrowed reference to context, not owned
    const ar_data_t *ref_message; // Borrowed reference to message, not owned
    char *own_error_message;   // Owned error message string
    int error_position;        // Position where error occurred (1-based, 0 if no error)
};

// Parsed instruction structure definition
struct parsed_instruction_s {
    ar_instruction_type_t type;
    
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
static parsed_instruction_t* _parse_instruction(ar_instruction_context_t *mut_ctx, const char *ref_instruction, int *mut_pos);
static parsed_instruction_t* _parse_assignment(ar_instruction_context_t *mut_ctx, const char *ref_instruction, int *mut_pos);
static parsed_instruction_t* _parse_function_instruction(ar_instruction_context_t *mut_ctx, const char *ref_instruction, int *mut_pos);
static bool _parse_memory_access(const char *ref_instruction, int *mut_pos, char **path);
static parsed_instruction_t* _parse_function_call(ar_instruction_context_t *mut_ctx, const char *ref_instruction, int *mut_pos, char **out_result_path);
static bool _skip_whitespace(const char *ref_instruction, int *mut_pos);
static bool _extract_identifier(const char *ref_instruction, int *mut_pos, char *mut_identifier, int max_size);

// Error handling helper functions
static void _set_error(ar_instruction_context_t *mut_ctx, const char *ref_message, int position);
static void _clear_error(ar_instruction_context_t *mut_ctx);

// Create a new instruction context
ar_instruction_context_t* ar_instruction__create_context(ar_data_t *mut_memory, const ar_data_t *ref_context, const ar_data_t *ref_message) {
    // Allocate memory for the context
    ar_instruction_context_t *own_ctx = (ar_instruction_context_t*)AR__HEAP__MALLOC(sizeof(ar_instruction_context_t), "Instruction context");
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
void ar_instruction__destroy_context(ar_instruction_context_t *own_ctx) {
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
ar_data_t* ar_instruction__get_memory(const ar_instruction_context_t *ref_ctx) {
    if (!ref_ctx) {
        return NULL;
    }
    return ref_ctx->mut_memory;
}

// Get the context data from the instruction context
const ar_data_t* ar_instruction__get_context(const ar_instruction_context_t *ref_ctx) {
    if (!ref_ctx) {
        return NULL;
    }
    return ref_ctx->ref_context;
}

// Get the message from the instruction context
const ar_data_t* ar_instruction__get_message(const ar_instruction_context_t *ref_ctx) {
    if (!ref_ctx) {
        return NULL;
    }
    return ref_ctx->ref_message;
}


// Parse and execute a single instruction
// This is now the main parse function that builds AST
parsed_instruction_t* ar_instruction__parse(const char *ref_instruction, ar_instruction_context_t *mut_ctx) {
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
void ar_instruction__destroy_parsed(parsed_instruction_t *own_parsed) {
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
ar_instruction_type_t ar_instruction__get_type(const parsed_instruction_t *ref_parsed) {
    if (!ref_parsed) {
        return INST_ASSIGNMENT; // Default, though caller should check for NULL
    }
    return ref_parsed->type;
}

// Gets the memory path for an assignment instruction
const char* ar_instruction__get_assignment_path(const parsed_instruction_t *ref_parsed) {
    if (!ref_parsed || ref_parsed->type != INST_ASSIGNMENT) {
        return NULL;
    }
    return ref_parsed->own_assignment_path;
}

// Gets the expression for an assignment instruction
const char* ar_instruction__get_assignment_expression(const parsed_instruction_t *ref_parsed) {
    if (!ref_parsed || ref_parsed->type != INST_ASSIGNMENT) {
        return NULL;
    }
    return ref_parsed->own_assignment_expression;
}

// Gets function call details from a parsed instruction
bool ar_instruction__get_function_call(const parsed_instruction_t *ref_parsed,
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
static parsed_instruction_t* _parse_instruction(ar_instruction_context_t *mut_ctx, const char *ref_instruction, int *mut_pos) {
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
static parsed_instruction_t* _parse_assignment(ar_instruction_context_t *mut_ctx, const char *ref_instruction, int *mut_pos) {
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
    ar_expression_context_t *own_expr_test = ar_expression__create_context(
        ar_instruction__get_memory(mut_ctx),
        ar_instruction__get_context(mut_ctx), 
        ar_instruction__get_message(mut_ctx),
        ref_instruction + *mut_pos
    );
    
    if (!own_expr_test) {
        AR__HEAP__FREE(own_path);
        return NULL;
    }
    
    // Try to evaluate the expression - if this fails, it's not a valid expression
    const ar_data_t *ref_test_result = ar_expression__evaluate(own_expr_test);
    int expr_offset = ar_expression__offset(own_expr_test);
    ar_expression__destroy_context(own_expr_test);
    
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
        ar_instruction__destroy_parsed(own_result);
        return NULL;
    }
    
    memcpy(own_result->own_assignment_expression, expr_start, expr_len);
    own_result->own_assignment_expression[expr_len] = '\0';
    
    // Update position to end of instruction
    *mut_pos += (int)strlen(expr_start);
    
    return own_result;
}

// <function-instruction> ::= [<memory-access> ':='] <function-call>
static parsed_instruction_t* _parse_function_instruction(ar_instruction_context_t *mut_ctx, const char *ref_instruction, int *mut_pos) {
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
static parsed_instruction_t* _parse_function_call(ar_instruction_context_t *mut_ctx, const char *ref_instruction, int *mut_pos, char **out_result_path) {
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
        ar_instruction__destroy_parsed(own_result);
        return NULL;
    }
    
    // Handle different function types
    if (strcmp(function_name, "send") == 0) {
        own_result->type = INST_SEND;
        // send(agent_id, message) - requires exactly 2 arguments
        
        // Allocate args array for 2 arguments
        own_result->own_args = AR__HEAP__MALLOC(2 * sizeof(char*), "Send arguments");
        if (!own_result->own_args) {
            ar_instruction__destroy_parsed(own_result);
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
            ar_instruction__destroy_parsed(own_result);
            return NULL;
        }
        
        // Extract and store first argument
        int arg_len = *mut_pos - arg_start;
        own_result->own_args[0] = AR__HEAP__MALLOC((size_t)(arg_len + 1), "Agent ID expression");
        if (!own_result->own_args[0]) {
            ar_instruction__destroy_parsed(own_result);
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
            ar_instruction__destroy_parsed(own_result);
            return NULL;
        }
        
        // Extract and store second argument
        arg_len = *mut_pos - arg_start;
        own_result->own_args[1] = AR__HEAP__MALLOC((size_t)(arg_len + 1), "Message expression");
        if (!own_result->own_args[1]) {
            ar_instruction__destroy_parsed(own_result);
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
            ar_instruction__destroy_parsed(own_result);
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
                ar_instruction__destroy_parsed(own_result);
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
                ar_instruction__destroy_parsed(own_result);
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
    else if (strcmp(function_name, "parse") == 0) {
        own_result->type = INST_PARSE;
        // parse(template, input) - requires exactly 2 arguments
        // Parses input string using template into a map of key-value pairs
        
        // Allocate args array for 2 arguments
        own_result->own_args = AR__HEAP__MALLOC(2 * sizeof(char*), "Parse arguments");
        if (!own_result->own_args) {
            ar_instruction__destroy_parsed(own_result);
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
            ar_instruction__destroy_parsed(own_result);
            return NULL;
        }
        
        // Extract and store first argument
        int arg_len = *mut_pos - arg_start;
        own_result->own_args[0] = AR__HEAP__MALLOC((size_t)(arg_len + 1), "Parse template argument");
        if (!own_result->own_args[0]) {
            ar_instruction__destroy_parsed(own_result);
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
            ar_instruction__destroy_parsed(own_result);
            return NULL;
        }
        
        // Extract and store second argument
        arg_len = *mut_pos - arg_start;
        own_result->own_args[1] = AR__HEAP__MALLOC((size_t)(arg_len + 1), "Parse input argument");
        if (!own_result->own_args[1]) {
            ar_instruction__destroy_parsed(own_result);
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
    else if (strcmp(function_name, "build") == 0) {
        own_result->type = INST_BUILD;
        // build(template, map) - requires exactly 2 arguments
        
        // Allocate args array for 2 arguments
        own_result->own_args = AR__HEAP__MALLOC(2 * sizeof(char*), "Build arguments");
        if (!own_result->own_args) {
            ar_instruction__destroy_parsed(own_result);
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
            ar_instruction__destroy_parsed(own_result);
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
            ar_instruction__destroy_parsed(own_result);
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
            ar_instruction__destroy_parsed(own_result);
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
            ar_instruction__destroy_parsed(own_result);
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
    else if (strcmp(function_name, "method") == 0) {
        own_result->type = INST_METHOD;
        // method(name, instructions, version) - requires exactly 3 arguments
        
        // Allocate args array for 3 arguments
        own_result->own_args = AR__HEAP__MALLOC(3 * sizeof(char*), "Method arguments");
        if (!own_result->own_args) {
            ar_instruction__destroy_parsed(own_result);
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
                ar_instruction__destroy_parsed(own_result);
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
            
            own_result->own_args[i] = AR__HEAP__MALLOC((size_t)(arg_len + 1), 
                (i == 0) ? "Method name" : (i == 1) ? "Method instructions" : "Method version");
            if (!own_result->own_args[i]) {
                ar_instruction__destroy_parsed(own_result);
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
    else if (strcmp(function_name, "agent") == 0) {
        own_result->type = INST_AGENT;
        // agent(method_name, version, context) - requires exactly 3 arguments
        
        // Allocate args array for 3 arguments
        own_result->own_args = AR__HEAP__MALLOC(3 * sizeof(char*), "Agent arguments");
        if (!own_result->own_args) {
            ar_instruction__destroy_parsed(own_result);
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
                ar_instruction__destroy_parsed(own_result);
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
            
            own_result->own_args[i] = AR__HEAP__MALLOC((size_t)(arg_len + 1), 
                (i == 0) ? "Method name" : (i == 1) ? "Version" : "Context");
            if (!own_result->own_args[i]) {
                ar_instruction__destroy_parsed(own_result);
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
    else if (strcmp(function_name, "destroy") == 0) {
        // Set instruction type
        own_result->type = INST_DESTROY;
        
        // Allocate args array - destroy can have 1 or 2 arguments
        own_result->own_args = AR__HEAP__MALLOC(sizeof(char*) * 2, "Destroy arguments");
        if (!own_result->own_args) {
            ar_instruction__destroy_parsed(own_result);
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
            ar_instruction__destroy_parsed(own_result);
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
                ar_instruction__destroy_parsed(own_result);
                return NULL;
            }
            memcpy(own_result->own_args[1], ref_instruction + arg_start, (size_t)arg_len);
            own_result->own_args[1][arg_len] = '\0';
            own_result->arg_count = 2;
            
            _skip_whitespace(ref_instruction, mut_pos);
        }
        
        // Expect closing parenthesis
        if (ref_instruction[*mut_pos] != ')') {
            ar_instruction__destroy_parsed(own_result);
            return NULL;
        }
        (*mut_pos)++; // Skip ')'
        
        return own_result;
        
    }
    else {
        // Unknown function - return NULL to indicate parsing error
        ar_instruction__destroy_parsed(own_result);
        return NULL;
    }
    
    return NULL; // Should not reach here
}

// Error reporting functions

// Gets the last error message from the instruction context
const char* ar_instruction__get_last_error(const ar_instruction_context_t *ref_ctx) {
    if (!ref_ctx) {
        return NULL;
    }
    return ref_ctx->own_error_message;
}

// Gets the position in the instruction string where the last error occurred
int ar_instruction__get_error_position(const ar_instruction_context_t *ref_ctx) {
    if (!ref_ctx) {
        return 0;
    }
    return ref_ctx->error_position;
}

// Helper function to set error in context
static void _set_error(ar_instruction_context_t *mut_ctx, const char *ref_message, int position) {
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
static void _clear_error(ar_instruction_context_t *mut_ctx) {
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
