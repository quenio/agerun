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


// TODO: Uncomment when implementing proper dispatch
// /**
//  * Internal: Propagate error from specialized parser.
//  */
// static void _propagate_error(instruction_parser_t *mut_parser, 
//                             const char *ref_error, 
//                             size_t error_pos,
//                             const char *ref_default_msg) {
//     _set_error(mut_parser, ref_error ? ref_error : ref_default_msg, error_pos);
// }


/**
 * Internal: Extract and dispatch to appropriate parser based on function name.
 */
static instruction_ast_t* _dispatch_function(instruction_parser_t *mut_parser, 
                                           const char *ref_instruction,
                                           const char *func_name, 
                                           size_t func_len,
                                           char *own_result_path) {
    // Check for send
    if (func_len == 4 && strncmp(func_name, "send", 4) == 0) {
        instruction_ast_t *own_ast = ar_send_instruction_parser__parse(
            mut_parser->own_send_parser,
            ref_instruction,
            own_result_path
        );
        
        if (!own_ast) {
            const char *ref_error = ar_send_instruction_parser__get_error(mut_parser->own_send_parser);
            size_t error_pos = ar_send_instruction_parser__get_error_position(mut_parser->own_send_parser);
            _set_error(mut_parser, ref_error ? ref_error : "Send parsing failed", error_pos);
        }
        
        return own_ast;
    }
    
    // Check for if
    if (func_len == 2 && strncmp(func_name, "if", 2) == 0) {
        instruction_ast_t *own_ast = ar_condition_instruction_parser__parse(
            mut_parser->own_condition_parser,
            ref_instruction,
            own_result_path
        );
        
        if (!own_ast) {
            const char *ref_error = ar_condition_instruction_parser__get_error(mut_parser->own_condition_parser);
            size_t error_pos = ar_condition_instruction_parser__get_error_position(mut_parser->own_condition_parser);
            _set_error(mut_parser, ref_error ? ref_error : "If parsing failed", error_pos);
        }
        
        return own_ast;
    }
    
    // Check for parse
    if (func_len == 5 && strncmp(func_name, "parse", 5) == 0) {
        instruction_ast_t *own_ast = ar_parse_instruction_parser__parse(
            mut_parser->own_parse_parser,
            ref_instruction,
            own_result_path
        );
        
        if (!own_ast) {
            const char *ref_error = ar_parse_instruction_parser__get_error(mut_parser->own_parse_parser);
            size_t error_pos = ar_parse_instruction_parser__get_error_position(mut_parser->own_parse_parser);
            _set_error(mut_parser, ref_error ? ref_error : "Parse parsing failed", error_pos);
        }
        
        return own_ast;
    }
    
    // Check for build
    if (func_len == 5 && strncmp(func_name, "build", 5) == 0) {
        instruction_ast_t *own_ast = ar_build_instruction_parser__parse(
            mut_parser->own_build_parser,
            ref_instruction,
            own_result_path
        );
        
        if (!own_ast) {
            const char *ref_error = ar_build_instruction_parser__get_error(mut_parser->own_build_parser);
            size_t error_pos = ar_build_instruction_parser__get_error_position(mut_parser->own_build_parser);
            _set_error(mut_parser, ref_error ? ref_error : "Build parsing failed", error_pos);
        }
        
        return own_ast;
    }
    
    // Check for method
    if (func_len == 6 && strncmp(func_name, "method", 6) == 0) {
        instruction_ast_t *own_ast = ar_method_instruction_parser__parse(
            mut_parser->own_method_parser,
            ref_instruction,
            own_result_path
        );
        
        if (!own_ast) {
            const char *ref_error = ar_method_instruction_parser__get_error(mut_parser->own_method_parser);
            size_t error_pos = ar_method_instruction_parser__get_error_position(mut_parser->own_method_parser);
            _set_error(mut_parser, ref_error ? ref_error : "Method parsing failed", error_pos);
        }
        
        return own_ast;
    }
    
    // Check for agent
    if (func_len == 5 && strncmp(func_name, "agent", 5) == 0) {
        instruction_ast_t *own_ast = ar_agent_instruction_parser__parse(
            mut_parser->own_agent_parser,
            ref_instruction,
            own_result_path
        );
        
        if (!own_ast) {
            const char *ref_error = ar_agent_instruction_parser__get_error(mut_parser->own_agent_parser);
            size_t error_pos = ar_agent_instruction_parser__get_error_position(mut_parser->own_agent_parser);
            _set_error(mut_parser, ref_error ? ref_error : "Agent parsing failed", error_pos);
        }
        
        return own_ast;
    }
    
    // Check for destroy
    if (func_len == 7 && strncmp(func_name, "destroy", 7) == 0) {
        // Try destroy method first
        instruction_ast_t *own_ast = ar_destroy_method_instruction_parser__parse(
            mut_parser->own_destroy_method_parser,
            ref_instruction,
            own_result_path
        );
        
        if (own_ast) {
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
            const char *ref_error = ar_destroy_agent_instruction_parser__get_error(mut_parser->own_destroy_agent_parser);
            size_t error_pos = ar_destroy_agent_instruction_parser__get_error_position(mut_parser->own_destroy_agent_parser);
            _set_error(mut_parser, ref_error ? ref_error : "Destroy parsing failed", error_pos);
        }
        
        return own_ast;
    }
    
    // Unknown function
    _set_error(mut_parser, "Unknown function type", 0);
    return NULL;
}

/**
 * Parse an instruction using the unified parser facade.
 */
instruction_ast_t* ar_instruction_parser__parse(instruction_parser_t *mut_parser, const char *ref_instruction) {
    if (!mut_parser || !ref_instruction) {
        return NULL;
    }
    
    _clear_error(mut_parser);
    
    // Minimal lookahead to detect instruction type
    const char *p = ref_instruction;
    bool in_quotes = false;
    const char *assign_pos = NULL;
    const char *paren_pos = NULL;
    
    // First pass: find := and ( positions
    while (*p) {
        if (*p == '"' && (p == ref_instruction || *(p-1) != '\\')) {
            in_quotes = !in_quotes;
        } else if (!in_quotes) {
            if (*p == ':' && *(p+1) == '=' && !assign_pos) {
                assign_pos = p;
            } else if (*p == '(' && !paren_pos) {
                paren_pos = p;
            }
        }
        p++;
    }
    
    // Determine instruction type based on what we found
    if (assign_pos) {
        // Check if there's a function call after :=
        if (paren_pos && paren_pos > assign_pos) {
            // This is a function with assignment
            // Extract the result path (everything before :=)
            const char *path_start = ref_instruction;
            while (*path_start && isspace((unsigned char)*path_start)) {
                path_start++;
            }
            size_t path_len = (size_t)(assign_pos - path_start);
            while (path_len > 0 && isspace((unsigned char)path_start[path_len - 1])) {
                path_len--;
            }
            
            char *own_result_path = AR__HEAP__MALLOC(path_len + 1, "result path");
            if (!own_result_path) {
                _set_error(mut_parser, "Memory allocation failed", 0);
                return NULL;
            }
            memcpy(own_result_path, path_start, path_len);
            own_result_path[path_len] = '\0';
            
            // Extract function name (between := and ()
            const char *func_start = assign_pos + 2;
            while (*func_start && isspace((unsigned char)*func_start)) {
                func_start++;
            }
            size_t func_len = (size_t)(paren_pos - func_start);
            while (func_len > 0 && isspace((unsigned char)func_start[func_len - 1])) {
                func_len--;
            }
            
            // Dispatch to appropriate parser
            instruction_ast_t *own_ast = _dispatch_function(mut_parser, ref_instruction, 
                                                           func_start, func_len, own_result_path);
            AR__HEAP__FREE(own_result_path);
            return own_ast;
        } else {
            // Pure assignment
            instruction_ast_t *own_ast = ar_assignment_instruction_parser__parse(
                mut_parser->own_assignment_parser, 
                ref_instruction
            );
            
            if (!own_ast) {
                // Get error from specialized parser
                const char *ref_error = ar_assignment_instruction_parser__get_error(mut_parser->own_assignment_parser);
                size_t error_pos = ar_assignment_instruction_parser__get_error_position(mut_parser->own_assignment_parser);
                _set_error(mut_parser, ref_error ? ref_error : "Assignment parsing failed", error_pos);
            }
            
            return own_ast;
        }
    }
    
    // Check for function calls
    if (paren_pos) {
        // Extract function name (everything before the '(')
        const char *name_start = ref_instruction;
        while (*name_start && isspace((unsigned char)*name_start)) {
            name_start++;
        }
        
        size_t name_len = (size_t)(paren_pos - name_start);
        while (name_len > 0 && isspace((unsigned char)name_start[name_len - 1])) {
            name_len--;
        }
        
        // Dispatch to appropriate parser
        return _dispatch_function(mut_parser, ref_instruction, name_start, name_len, NULL);
    }
    
    // Check for invalid assignment operator (= instead of :=)
    p = ref_instruction;
    in_quotes = false;
    while (*p) {
        if (*p == '"' && (p == ref_instruction || *(p-1) != '\\')) {
            in_quotes = !in_quotes;
        } else if (!in_quotes && *p == '=' && 
                  (p == ref_instruction || *(p-1) != ':') && 
                  (*(p+1) != '=')) {
            // Found a single = that's not part of := or ==
            _set_error(mut_parser, "Invalid assignment operator, expected ':='", (size_t)(p - ref_instruction));
            return NULL;
        }
        p++;
    }
    
    // Unknown instruction type
    _set_error(mut_parser, "Unknown instruction type", 0);
    return NULL;
}


