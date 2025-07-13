#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdbool.h>

#include "ar_instruction_parser.h"
#include "ar_instruction_ast.h"
#include "ar_heap.h"
#include "ar_string.h"
#include "ar_assignment_instruction_parser.h"
#include "ar_send_instruction_parser.h"
#include "ar_condition_instruction_parser.h"
#include "ar_parse_instruction_parser.h"
#include "ar_build_instruction_parser.h"
#include "ar_compile_instruction_parser.h"
#include "ar_create_instruction_parser.h"
#include "ar_destroy_agent_instruction_parser.h"
#include "ar_deprecate_instruction_parser.h"

/**
 * Opaque parser structure.
 */
struct ar_instruction_parser_s {
    ar_log_t *ref_log;       /* Log instance for error reporting (borrowed) */
    
    /* Specialized parser instances */
    ar_assignment_instruction_parser_t *own_assignment_parser;
    ar_send_instruction_parser_t *own_send_parser;
    ar_condition_instruction_parser_t *own_condition_parser;
    ar_parse_instruction_parser_t *own_parse_parser;
    ar_build_instruction_parser_t *own_build_parser;
    ar_compile_instruction_parser_t *own_method_parser;
    ar_create_instruction_parser_t *own_create_parser;
    ar_destroy_agent_instruction_parser_t *own_destroy_agent_parser;
    ar_deprecate_instruction_parser_t *own_deprecate_parser;
};

/**
 * Internal helper to destroy all specialized parsers.
 */
static void _destroy_specialized_parsers(ar_instruction_parser_t *mut_parser) {
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
        ar_compile_instruction_parser__destroy(mut_parser->own_method_parser);
    }
    if (mut_parser->own_create_parser) {
        ar_create_instruction_parser__destroy(mut_parser->own_create_parser);
    }
    if (mut_parser->own_destroy_agent_parser) {
        ar_destroy_agent_instruction_parser__destroy(mut_parser->own_destroy_agent_parser);
    }
    if (mut_parser->own_deprecate_parser) {
        ar_deprecate_instruction_parser__destroy(mut_parser->own_deprecate_parser);
    }
}

/**
 * Create a new instruction parser instance.
 */
ar_instruction_parser_t* ar_instruction_parser__create(ar_log_t *ref_log) {
    ar_instruction_parser_t *own_parser = AR__HEAP__MALLOC(sizeof(ar_instruction_parser_t), "instruction_parser");
    if (!own_parser) {
        return NULL;
    }
    
    // Initialize all fields to NULL first
    memset(own_parser, 0, sizeof(ar_instruction_parser_t));
    
    // Store the log reference
    own_parser->ref_log = ref_log;
    
    // Create assignment parser
    own_parser->own_assignment_parser = ar_assignment_instruction_parser__create(ref_log);
    if (!own_parser->own_assignment_parser) {
        goto error;
    }
    
    // Create send parser
    own_parser->own_send_parser = ar_send_instruction_parser__create(ref_log);
    if (!own_parser->own_send_parser) {
        goto error;
    }
    
    // Create condition parser
    own_parser->own_condition_parser = ar_condition_instruction_parser__create(ref_log);
    if (!own_parser->own_condition_parser) {
        goto error;
    }
    
    // Create parse parser
    own_parser->own_parse_parser = ar_parse_instruction_parser__create(ref_log);
    if (!own_parser->own_parse_parser) {
        goto error;
    }
    
    // Create build parser
    own_parser->own_build_parser = ar_build_instruction_parser__create(ref_log);
    if (!own_parser->own_build_parser) {
        goto error;
    }
    
    // Create method parser
    own_parser->own_method_parser = ar_compile_instruction_parser__create(ref_log);
    if (!own_parser->own_method_parser) {
        goto error;
    }
    
    // Create agent parser
    own_parser->own_create_parser = ar_create_instruction_parser__create(ref_log);
    if (!own_parser->own_create_parser) {
        goto error;
    }
    
    // Create destroy agent parser
    own_parser->own_destroy_agent_parser = ar_destroy_agent_instruction_parser__create(ref_log);
    if (!own_parser->own_destroy_agent_parser) {
        goto error;
    }
    
    // Create destroy method parser
    own_parser->own_deprecate_parser = ar_deprecate_instruction_parser__create(ref_log);
    if (!own_parser->own_deprecate_parser) {
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
void ar_instruction_parser__destroy(ar_instruction_parser_t *own_parser) {
    if (!own_parser) {
        return;
    }
    
    // Destroy all specialized parsers
    _destroy_specialized_parsers(own_parser);
    
    AR__HEAP__FREE(own_parser);
}

/**
 * Get the last error message from the parser.
 * DEPRECATED: This function always returns NULL. Use ar_log for error reporting.
 */
const char* ar_instruction_parser__get_error(const ar_instruction_parser_t *ref_parser) {
    (void)ref_parser; // Suppress unused parameter warning
    return NULL;
}

/**
 * Get the error position from the last parse attempt.
 * DEPRECATED: This function always returns 0. Use ar_log for error reporting.
 */
size_t ar_instruction_parser__get_error_position(const ar_instruction_parser_t *ref_parser) {
    (void)ref_parser; // Suppress unused parameter warning
    return 0;
}

/**
 * Internal: Log error message with position.
 */
static void _log_error(ar_instruction_parser_t *mut_parser, const char *error, size_t position) {
    if (!mut_parser) {
        return;
    }
    
    /* Log the error with position */
    if (mut_parser->ref_log) {
        ar_log__error_at(mut_parser->ref_log, error, (int)position);
    }
}


// TODO: Uncomment when implementing proper dispatch
// /**
//  * Internal: Propagate error from specialized parser.
//  */
// static void _propagate_error(ar_instruction_parser_t *mut_parser, 
//                             const char *ref_error, 
//                             size_t error_pos,
//                             const char *ref_default_msg) {
//     _set_error(mut_parser, ref_error ? ref_error : ref_default_msg, error_pos);
// }


/**
 * Internal: Extract and dispatch to appropriate parser based on function name.
 */
static ar_instruction_ast_t* _dispatch_function(ar_instruction_parser_t *mut_parser, 
                                           const char *ref_instruction,
                                           const char *func_name, 
                                           size_t func_len,
                                           char *own_result_path) {
    // Check for send
    if (func_len == 4 && strncmp(func_name, "send", 4) == 0) {
        ar_instruction_ast_t *own_ast = ar_send_instruction_parser__parse(
            mut_parser->own_send_parser,
            ref_instruction,
            own_result_path
        );
        
        /* Error already logged by send parser to shared log if parsing failed */
        return own_ast;
    }
    
    // Check for if
    if (func_len == 2 && strncmp(func_name, "if", 2) == 0) {
        ar_instruction_ast_t *own_ast = ar_condition_instruction_parser__parse(
            mut_parser->own_condition_parser,
            ref_instruction,
            own_result_path
        );
        
        /* Error already logged by condition parser to shared log if parsing failed */
        return own_ast;
    }
    
    // Check for parse
    if (func_len == 5 && strncmp(func_name, "parse", 5) == 0) {
        ar_instruction_ast_t *own_ast = ar_parse_instruction_parser__parse(
            mut_parser->own_parse_parser,
            ref_instruction,
            own_result_path
        );
        
        /* Error already logged by parse parser to shared log if parsing failed */
        return own_ast;
    }
    
    // Check for build
    if (func_len == 5 && strncmp(func_name, "build", 5) == 0) {
        ar_instruction_ast_t *own_ast = ar_build_instruction_parser__parse(
            mut_parser->own_build_parser,
            ref_instruction,
            own_result_path
        );
        
        /* Error already logged by build parser to shared log if parsing failed */
        return own_ast;
    }
    
    // Check for compile
    if (func_len == 7 && strncmp(func_name, "compile", 7) == 0) {
        ar_instruction_ast_t *own_ast = ar_compile_instruction_parser__parse(
            mut_parser->own_method_parser,
            ref_instruction,
            own_result_path
        );
        
        /* Error already logged by method parser to shared log if parsing failed */
        return own_ast;
    }
    
    // Check for create
    if (func_len == 6 && strncmp(func_name, "create", 6) == 0) {
        ar_instruction_ast_t *own_ast = ar_create_instruction_parser__parse(
            mut_parser->own_create_parser,
            ref_instruction,
            own_result_path
        );
        
        /* Error already logged by agent parser to shared log if parsing failed */
        return own_ast;
    }
    
    // Check for deprecate
    if (func_len == 9 && strncmp(func_name, "deprecate", 9) == 0) {
        ar_instruction_ast_t *own_ast = ar_deprecate_instruction_parser__parse(
            mut_parser->own_deprecate_parser,
            ref_instruction,
            own_result_path
        );
        
        /* Error already logged by deprecate parser to shared log if parsing failed */
        return own_ast;
    }
    
    // Check for destroy (only destroy agent now)
    if (func_len == 7 && strncmp(func_name, "destroy", 7) == 0) {
        ar_instruction_ast_t *own_ast = ar_destroy_agent_instruction_parser__parse(
            mut_parser->own_destroy_agent_parser,
            ref_instruction,
            own_result_path
        );
        
        /* Error already logged by destroy agent parser to shared log if parsing failed */
        return own_ast;
    }
    
    // Unknown function
    _log_error(mut_parser, "Unknown function type", 0);
    return NULL;
}

/**
 * Parse an instruction using the unified parser facade.
 */
ar_instruction_ast_t* ar_instruction_parser__parse(ar_instruction_parser_t *mut_parser, const char *ref_instruction) {
    if (!mut_parser || !ref_instruction) {
        return NULL;
    }
    
    
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
                _log_error(mut_parser, "Memory allocation failed", 0);
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
            ar_instruction_ast_t *own_ast = _dispatch_function(mut_parser, ref_instruction, 
                                                           func_start, func_len, own_result_path);
            AR__HEAP__FREE(own_result_path);
            return own_ast;
        } else {
            // Pure assignment
            ar_instruction_ast_t *own_ast = ar_assignment_instruction_parser__parse(
                mut_parser->own_assignment_parser, 
                ref_instruction
            );
            
            /* Error already logged by assignment parser to shared log if parsing failed */
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
            _log_error(mut_parser, "Invalid assignment operator, expected ':='", (size_t)(p - ref_instruction));
            return NULL;
        }
        p++;
    }
    
    // Unknown instruction type
    _log_error(mut_parser, "Unknown instruction type", 0);
    return NULL;
}


