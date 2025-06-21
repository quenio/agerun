#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdbool.h>

#include "agerun_instruction_parser.h"
#include "agerun_instruction_ast.h"
#include "agerun_heap.h"
#include "agerun_string.h"
#include "agerun_assignment_instruction_parser.h"
#include "agerun_send_instruction_parser.h"
#include "agerun_condition_instruction_parser.h"
#include "agerun_parse_instruction_parser.h"
#include "agerun_build_instruction_parser.h"
#include "agerun_method_instruction_parser.h"
#include "agerun_agent_instruction_parser.h"
#include "agerun_destroy_agent_instruction_parser.h"
#include "agerun_destroy_method_instruction_parser.h"

/**
 * Opaque parser structure.
 */
struct instruction_parser_s {
    char *own_error;         /* Error message if parsing fails */
    size_t error_position;   /* Position where error occurred */
    
    /* Specialized parser instances */
    ar_assignment_instruction_parser_t *own_assignment_parser;
    ar_send_instruction_parser_t *own_send_parser;
    ar_condition_instruction_parser_t *own_condition_parser;
    ar_parse_instruction_parser_t *own_parse_parser;
    ar_build_instruction_parser_t *own_build_parser;
    ar_method_instruction_parser_t *own_method_parser;
    ar_agent_instruction_parser_t *own_agent_parser;
    ar_destroy_agent_instruction_parser_t *own_destroy_agent_parser;
    ar_destroy_method_instruction_parser_t *own_destroy_method_parser;
};

/**
 * Internal helper to destroy all specialized parsers.
 */
static void _destroy_specialized_parsers(instruction_parser_t *mut_parser) {
    if (!mut_parser) {
        return;
    }
    
    if (mut_parser->own_assignment_parser) {
        ar_assignment_instruction_parser__destroy(mut_parser->own_assignment_parser);
    }
    if (mut_parser->own_send_parser) {
        ar_send_instruction_parser__destroy(mut_parser->own_send_parser);
    }
    if (mut_parser->own_condition_parser) {
        ar_condition_instruction_parser__destroy(mut_parser->own_condition_parser);
    }
    if (mut_parser->own_parse_parser) {
        ar_parse_instruction_parser__destroy(mut_parser->own_parse_parser);
    }
    if (mut_parser->own_build_parser) {
        ar_build_instruction_parser__destroy(mut_parser->own_build_parser);
    }
    if (mut_parser->own_method_parser) {
        ar_method_instruction_parser__destroy(mut_parser->own_method_parser);
    }
    if (mut_parser->own_agent_parser) {
        ar_agent_instruction_parser__destroy(mut_parser->own_agent_parser);
    }
    if (mut_parser->own_destroy_agent_parser) {
        ar_destroy_agent_instruction_parser__destroy(mut_parser->own_destroy_agent_parser);
    }
    if (mut_parser->own_destroy_method_parser) {
        ar_destroy_method_instruction_parser__destroy(mut_parser->own_destroy_method_parser);
    }
}

/**
 * Create a new instruction parser instance.
 */
instruction_parser_t* ar__instruction_parser__create(void) {
    instruction_parser_t *own_parser = AR__HEAP__MALLOC(sizeof(instruction_parser_t), "instruction_parser");
    if (!own_parser) {
        return NULL;
    }
    
    // Initialize all fields to NULL first
    memset(own_parser, 0, sizeof(instruction_parser_t));
    
    // Create assignment parser
    own_parser->own_assignment_parser = ar_assignment_instruction_parser__create();
    if (!own_parser->own_assignment_parser) {
        goto error;
    }
    
    // Create send parser
    own_parser->own_send_parser = ar_send_instruction_parser__create();
    if (!own_parser->own_send_parser) {
        goto error;
    }
    
    // Create condition parser
    own_parser->own_condition_parser = ar_condition_instruction_parser__create();
    if (!own_parser->own_condition_parser) {
        goto error;
    }
    
    // Create parse parser
    own_parser->own_parse_parser = ar_parse_instruction_parser__create();
    if (!own_parser->own_parse_parser) {
        goto error;
    }
    
    // Create build parser
    own_parser->own_build_parser = ar_build_instruction_parser__create();
    if (!own_parser->own_build_parser) {
        goto error;
    }
    
    // Create method parser
    own_parser->own_method_parser = ar_method_instruction_parser__create();
    if (!own_parser->own_method_parser) {
        goto error;
    }
    
    // Create agent parser
    own_parser->own_agent_parser = ar_agent_instruction_parser__create();
    if (!own_parser->own_agent_parser) {
        goto error;
    }
    
    // Create destroy agent parser
    own_parser->own_destroy_agent_parser = ar_destroy_agent_instruction_parser__create();
    if (!own_parser->own_destroy_agent_parser) {
        goto error;
    }
    
    // Create destroy method parser
    own_parser->own_destroy_method_parser = ar_destroy_method_instruction_parser__create();
    if (!own_parser->own_destroy_method_parser) {
        goto error;
    }
    
    return own_parser;

error:
    _destroy_specialized_parsers(own_parser);
    AR__HEAP__FREE(own_parser);
    return NULL;
}

/**
 * Destroy an instruction parser instance.
 */
void ar__instruction_parser__destroy(instruction_parser_t *own_parser) {
    if (!own_parser) {
        return;
    }
    
    // Destroy all specialized parsers
    _destroy_specialized_parsers(own_parser);
    
    AR__HEAP__FREE(own_parser->own_error);
    AR__HEAP__FREE(own_parser);
}

/**
 * Get the last error message from the parser.
 */
const char* ar__instruction_parser__get_error(const instruction_parser_t *ref_parser) {
    if (!ref_parser) {
        return NULL;
    }
    return ref_parser->own_error;
}

/**
 * Get the error position from the last parse attempt.
 */
size_t ar__instruction_parser__get_error_position(const instruction_parser_t *ref_parser) {
    if (!ref_parser) {
        return 0;
    }
    return ref_parser->error_position;
}

/**
 * Internal: Set error message and position.
 */
static void _set_error(instruction_parser_t *mut_parser, const char *error, size_t position) {
    if (!mut_parser) {
        return;
    }
    
    AR__HEAP__FREE(mut_parser->own_error);
    mut_parser->own_error = AR__HEAP__STRDUP(error, "parser error message");
    mut_parser->error_position = position;
}

/**
 * Internal: Clear any previous error.
 */
static void _clear_error(instruction_parser_t *mut_parser) {
    if (!mut_parser) {
        return;
    }
    
    AR__HEAP__FREE(mut_parser->own_error);
    mut_parser->own_error = NULL;
    mut_parser->error_position = 0;
}

/**
 * Internal: Skip whitespace in string.
 */
static size_t _skip_whitespace(const char *str, size_t pos) {
    while (str[pos] && isspace((unsigned char)str[pos])) {
        pos++;
    }
    return pos;
}

/**
 * Internal: Check if instruction is a pure assignment (not a function with assignment).
 */
static bool _is_pure_assignment(const char *ref_instruction) {
    if (!ref_instruction) {
        return false;
    }
    
    // Look for := operator (but not inside quotes)
    const char *p = ref_instruction;
    bool in_quotes = false;
    const char *assign_op = NULL;
    
    while (*p) {
        if (*p == '"' && (p == ref_instruction || *(p-1) != '\\')) {
            in_quotes = !in_quotes;
        } else if (!in_quotes && *p == ':' && *(p+1) == '=') {
            assign_op = p;
            break;
        }
        p++;
    }
    
    if (!assign_op) {
        return false;
    }
    
    // Skip whitespace after :=
    const char *expr_start = assign_op + 2;
    while (*expr_start && isspace((unsigned char)*expr_start)) {
        expr_start++;
    }
    
    // Check if the expression is a function call
    // Function calls have the pattern: name(args)
    const char *paren = strchr(expr_start, '(');
    if (paren) {
        // Check if this looks like a function name before the paren
        const char *ptr = expr_start;
        while (ptr < paren && (isalnum((unsigned char)*ptr) || *ptr == '_')) {
            ptr++;
        }
        // Skip whitespace between name and paren
        while (ptr < paren && isspace((unsigned char)*ptr)) {
            ptr++;
        }
        // If we reached the paren, it's a function call
        if (ptr == paren) {
            return false; // Not a pure assignment, it's a function with assignment
        }
    }
    
    return true; // Pure assignment
}

/**
 * Internal: Propagate error from specialized parser.
 */
static void _propagate_error(instruction_parser_t *mut_parser, 
                            const char *ref_error, 
                            size_t error_pos,
                            const char *ref_default_msg) {
    _set_error(mut_parser, ref_error ? ref_error : ref_default_msg, error_pos);
}

/**
 * Internal: Extract result path from instruction if it has assignment.
 * Returns allocated string that must be freed, or NULL if no assignment.
 */
static char* _extract_result_path(const char *ref_instruction) {
    if (!ref_instruction) {
        return NULL;
    }
    
    // Look for := operator (but not inside quotes)
    const char *p = ref_instruction;
    bool in_quotes = false;
    const char *assign_op = NULL;
    
    while (*p) {
        if (*p == '"' && (p == ref_instruction || *(p-1) != '\\')) {
            in_quotes = !in_quotes;
        } else if (!in_quotes && *p == ':' && *(p+1) == '=') {
            assign_op = p;
            break;
        }
        p++;
    }
    
    if (!assign_op) {
        return NULL;
    }
    
    // Find start of instruction
    size_t start_pos = _skip_whitespace(ref_instruction, 0);
    const char *start = ref_instruction + start_pos;
    
    // Calculate length of result path
    size_t len = (size_t)(assign_op - start);
    
    // Trim trailing whitespace
    while (len > 0 && isspace((unsigned char)start[len - 1])) {
        len--;
    }
    
    if (len == 0) {
        return NULL;
    }
    
    // Allocate and copy result path
    char *result_path = AR__HEAP__MALLOC(len + 1, "result path");
    if (!result_path) {
        return NULL;
    }
    
    memcpy(result_path, start, len);
    result_path[len] = '\0';
    
    return result_path;
}

/**
 * Internal: Get function name from instruction.
 * Returns pointer to start of function name or NULL if not a function call.
 * If instruction has assignment, starts search after :=
 */
static const char* _get_function_name(const char *ref_instruction, size_t *out_name_len) {
    if (!ref_instruction || !out_name_len) {
        return NULL;
    }
    
    const char *search_start = ref_instruction;
    
    // If there's an assignment, start search after := (but not inside quotes)
    const char *p = ref_instruction;
    bool in_quotes = false;
    const char *assign_op = NULL;
    
    while (*p) {
        if (*p == '"' && (p == ref_instruction || *(p-1) != '\\')) {
            in_quotes = !in_quotes;
        } else if (!in_quotes && *p == ':' && *(p+1) == '=') {
            assign_op = p;
            break;
        }
        p++;
    }
    
    if (assign_op) {
        search_start = assign_op + 2;
    }
    
    size_t pos = _skip_whitespace(search_start, 0);
    const char *start = search_start + pos;
    
    // Find the opening parenthesis
    const char *paren = strchr(start, '(');
    if (!paren) {
        return NULL;
    }
    
    // Calculate name length (excluding whitespace)
    size_t len = (size_t)(paren - start);
    while (len > 0 && isspace((unsigned char)start[len - 1])) {
        len--;
    }
    
    if (len == 0) {
        return NULL;
    }
    
    *out_name_len = len;
    return start;
}

/**
 * Parse an instruction using the unified parser facade.
 */
instruction_ast_t* ar_instruction_parser__parse(instruction_parser_t *mut_parser, const char *ref_instruction) {
    if (!mut_parser || !ref_instruction) {
        return NULL;
    }
    
    _clear_error(mut_parser);
    
    // Skip leading whitespace
    size_t pos = _skip_whitespace(ref_instruction, 0);
    
    // Check if it's a pure assignment (not a function with assignment)
    if (_is_pure_assignment(ref_instruction)) {
        instruction_ast_t *own_ast = ar_assignment_instruction_parser__parse(
            mut_parser->own_assignment_parser, 
            ref_instruction
        );
        
        if (!own_ast) {
            // Propagate error from specialized parser
            _propagate_error(
                mut_parser,
                ar_assignment_instruction_parser__get_error(mut_parser->own_assignment_parser),
                ar_assignment_instruction_parser__get_error_position(mut_parser->own_assignment_parser),
                "Assignment parsing failed"
            );
        }
        
        return own_ast;
    }
    
    // Check if it's a function call (with or without assignment)
    size_t name_len = 0;
    const char *func_name = _get_function_name(ref_instruction, &name_len);
    char *own_result_path = NULL;
    
    if (func_name) {
        // Extract result path if there's an assignment
        own_result_path = _extract_result_path(ref_instruction);
        
        // Check for send
        if (name_len == 4 && strncmp(func_name, "send", 4) == 0) {
            instruction_ast_t *own_ast = ar_send_instruction_parser__parse(
                mut_parser->own_send_parser,
                ref_instruction,
                own_result_path
            );
            
            if (!own_ast) {
                _propagate_error(
                    mut_parser,
                    ar_send_instruction_parser__get_error(mut_parser->own_send_parser),
                    ar_send_instruction_parser__get_error_position(mut_parser->own_send_parser),
                    "Send parsing failed"
                );
            }
            
            AR__HEAP__FREE(own_result_path);
            return own_ast;
        }
        
        // Check for if
        if (name_len == 2 && strncmp(func_name, "if", 2) == 0) {
            instruction_ast_t *own_ast = ar_condition_instruction_parser__parse(
                mut_parser->own_condition_parser,
                ref_instruction,
                own_result_path
            );
            
            if (!own_ast) {
                _propagate_error(
                    mut_parser,
                    ar_condition_instruction_parser__get_error(mut_parser->own_condition_parser),
                    ar_condition_instruction_parser__get_error_position(mut_parser->own_condition_parser),
                    "If parsing failed"
                );
            }
            
            AR__HEAP__FREE(own_result_path);
            return own_ast;
        }
        
        // Check for parse
        if (name_len == 5 && strncmp(func_name, "parse", 5) == 0) {
            instruction_ast_t *own_ast = ar_parse_instruction_parser__parse(
                mut_parser->own_parse_parser,
                ref_instruction,
                own_result_path
            );
            
            if (!own_ast) {
                _propagate_error(
                    mut_parser,
                    ar_parse_instruction_parser__get_error(mut_parser->own_parse_parser),
                    ar_parse_instruction_parser__get_error_position(mut_parser->own_parse_parser),
                    "Parse parsing failed"
                );
            }
            
            AR__HEAP__FREE(own_result_path);
            return own_ast;
        }
        
        // Check for build
        if (name_len == 5 && strncmp(func_name, "build", 5) == 0) {
            instruction_ast_t *own_ast = ar_build_instruction_parser__parse(
                mut_parser->own_build_parser,
                ref_instruction,
                own_result_path
            );
            
            if (!own_ast) {
                _propagate_error(
                    mut_parser,
                    ar_build_instruction_parser__get_error(mut_parser->own_build_parser),
                    ar_build_instruction_parser__get_error_position(mut_parser->own_build_parser),
                    "Build parsing failed"
                );
            }
            
            AR__HEAP__FREE(own_result_path);
            return own_ast;
        }
        
        // Check for method
        if (name_len == 6 && strncmp(func_name, "method", 6) == 0) {
            instruction_ast_t *own_ast = ar_method_instruction_parser__parse(
                mut_parser->own_method_parser,
                ref_instruction,
                own_result_path
            );
            
            if (!own_ast) {
                _propagate_error(
                    mut_parser,
                    ar_method_instruction_parser__get_error(mut_parser->own_method_parser),
                    ar_method_instruction_parser__get_error_position(mut_parser->own_method_parser),
                    "Method parsing failed"
                );
            }
            
            AR__HEAP__FREE(own_result_path);
            return own_ast;
        }
        
        // Check for agent
        if (name_len == 5 && strncmp(func_name, "agent", 5) == 0) {
            instruction_ast_t *own_ast = ar_agent_instruction_parser__parse(
                mut_parser->own_agent_parser,
                ref_instruction,
                own_result_path
            );
            
            if (!own_ast) {
                _propagate_error(
                    mut_parser,
                    ar_agent_instruction_parser__get_error(mut_parser->own_agent_parser),
                    ar_agent_instruction_parser__get_error_position(mut_parser->own_agent_parser),
                    "Agent parsing failed"
                );
            }
            
            AR__HEAP__FREE(own_result_path);
            return own_ast;
        }
        
        // Check for destroy - need to determine which type
        if (name_len == 7 && strncmp(func_name, "destroy", 7) == 0) {
            // Try to determine which type by looking at arguments
            // First, try destroy method which expects two string arguments
            instruction_ast_t *own_ast = ar_destroy_method_instruction_parser__parse(
                mut_parser->own_destroy_method_parser,
                ref_instruction,
                own_result_path
            );
            
            if (own_ast) {
                AR__HEAP__FREE(own_result_path);
                return own_ast;
            }
            
            // Clear error and try destroy agent
            _clear_error(mut_parser);
            own_ast = ar_destroy_agent_instruction_parser__parse(
                mut_parser->own_destroy_agent_parser,
                ref_instruction,
                own_result_path
            );
            
            if (!own_ast) {
                _propagate_error(
                    mut_parser,
                    ar_destroy_agent_instruction_parser__get_error(mut_parser->own_destroy_agent_parser),
                    ar_destroy_agent_instruction_parser__get_error_position(mut_parser->own_destroy_agent_parser),
                    "Destroy parsing failed"
                );
            }
            
            AR__HEAP__FREE(own_result_path);
            return own_ast;
        }
    }
    
    // Check if it looks like an assignment attempt with wrong operator
    const char *p = ref_instruction;
    while (*p) {
        if (*p == '=' && (p == ref_instruction || *(p-1) != ':') && (p+1 == ref_instruction + strlen(ref_instruction) || *(p+1) != '=')) {
            // Found a single = that's not part of := or ==
            AR__HEAP__FREE(own_result_path);
            _set_error(mut_parser, "Invalid assignment operator, expected ':='", (size_t)(p - ref_instruction));
            return NULL;
        }
        p++;
    }
    
    // Unknown instruction type
    AR__HEAP__FREE(own_result_path);
    _set_error(mut_parser, "Unknown instruction type", pos);
    return NULL;
}


