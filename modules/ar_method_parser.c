#include "ar_method_parser.h"
#include "ar_method_ast.h"
#include "ar_instruction_parser.h"
#include "ar_string.h"
#include "ar_heap.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

// Error buffer size for formatting error messages
#define ERROR_BUFFER_SIZE 512

// Opaque structure definition
struct ar_method_parser_s {
    instruction_parser_t *instruction_parser;
    char *own_error_message;
    int error_line;
};

// Helper function to set error message with line number
static void _set_error(ar_method_parser_t *mut_parser, int line_number, const char *ref_instruction_error) {
    if (ref_instruction_error) {
        // Format error message with line number
        char error_buffer[ERROR_BUFFER_SIZE];
        snprintf(error_buffer, sizeof(error_buffer), "Line %d: %s", line_number, ref_instruction_error);
        mut_parser->own_error_message = AR__HEAP__STRDUP(error_buffer, "error message");
    } else {
        mut_parser->own_error_message = AR__HEAP__STRDUP("Unknown parse error", "error message");
    }
    mut_parser->error_line = line_number;
}

// Helper function to parse a single line
static bool _parse_line(ar_method_parser_t *mut_parser, ar_method_ast_t *mut_ast, const char *ref_line) {
    // Make a copy and trim the line
    char *own_line = AR__HEAP__STRDUP(ref_line, "line copy");
    if (!own_line) {
        return false;
    }
    
    char *mut_trimmed = ar_string__trim(own_line);
    
    // Skip empty lines
    if (strlen(mut_trimmed) == 0) {
        AR__HEAP__FREE(own_line);
        return true;
    }
    
    // Skip comment lines (lines starting with #)
    if (mut_trimmed[0] == '#') {
        AR__HEAP__FREE(own_line);
        return true;
    }
    
    // Remove inline comments (everything after # on the line, but not inside quotes)
    char *p = mut_trimmed;
    bool in_quotes = false;
    while (*p) {
        if (*p == '"' && (p == mut_trimmed || *(p-1) != '\\')) {
            in_quotes = !in_quotes;
        } else if (*p == '#' && !in_quotes) {
            // Found a comment marker outside of quotes
            *p = '\0';
            // Trim again to remove trailing whitespace before the comment
            mut_trimmed = ar_string__trim(mut_trimmed);
            
            // If the line is now empty after removing the comment, skip it
            if (strlen(mut_trimmed) == 0) {
                AR__HEAP__FREE(own_line);
                return true;
            }
            break;
        }
        p++;
    }
    
    // Parse the instruction
    ar_instruction_ast_t *own_instruction = ar_instruction_parser__parse(mut_parser->instruction_parser, mut_trimmed);
    AR__HEAP__FREE(own_line);
    
    if (!own_instruction) {
        return false;
    }
    
    ar_method_ast__add_instruction(mut_ast, own_instruction);
    return true;
}

ar_method_parser_t* ar_method_parser__create(void) {
    ar_method_parser_t *own_parser = AR__HEAP__MALLOC(sizeof(ar_method_parser_t), "method_parser");
    if (!own_parser) {
        return NULL;
    }
    
    own_parser->instruction_parser = ar_instruction_parser__create();
    if (!own_parser->instruction_parser) {
        AR__HEAP__FREE(own_parser);
        return NULL;
    }
    
    own_parser->own_error_message = NULL;
    own_parser->error_line = 0;
    
    return own_parser;
    // Ownership transferred to caller
}

void ar_method_parser__destroy(ar_method_parser_t *own_parser) {
    if (!own_parser) {
        return;
    }
    
    if (own_parser->instruction_parser) {
        ar_instruction_parser__destroy(own_parser->instruction_parser);
    }
    
    if (own_parser->own_error_message) {
        AR__HEAP__FREE(own_parser->own_error_message);
    }
    
    AR__HEAP__FREE(own_parser);
}

ar_method_ast_t* ar_method_parser__parse(ar_method_parser_t *mut_parser, const char *ref_source) {
    if (!mut_parser || !ref_source) {
        return NULL;
    }
    
    // Clear any previous error
    if (mut_parser->own_error_message) {
        AR__HEAP__FREE(mut_parser->own_error_message);
        mut_parser->own_error_message = NULL;
    }
    mut_parser->error_line = 0;
    
    // Make a copy and trim the source to remove leading/trailing whitespace
    char *own_copy = AR__HEAP__STRDUP(ref_source, "method source copy");
    if (!own_copy) {
        return NULL;
    }
    
    char *mut_trimmed = ar_string__trim(own_copy);
    
    // Create the AST to populate
    ar_method_ast_t *own_ast = ar_method_ast__create();
    if (!own_ast) {
        AR__HEAP__FREE(own_copy);
        return NULL;
    }
    
    // If the trimmed source is empty, return empty AST
    if (strlen(mut_trimmed) == 0) {
        AR__HEAP__FREE(own_copy);
        return own_ast;
    }
    
    // Split by lines and parse each instruction
    char *mut_current = mut_trimmed;
    char *mut_line_start = mut_current;
    int current_line = 1;
    
    while (*mut_current) {
        // Find the end of the current line
        while (*mut_current && *mut_current != '\n' && *mut_current != '\r') {
            mut_current++;
        }
        
        // Temporarily null-terminate the line
        char saved_char = *mut_current;
        *mut_current = '\0';
        
        // Parse the line
        if (!_parse_line(mut_parser, own_ast, mut_line_start)) {
            // Parse failed - capture error from instruction parser
            const char *ref_inst_error = ar_instruction_parser__get_error(mut_parser->instruction_parser);
            _set_error(mut_parser, current_line, ref_inst_error);
            
            AR__HEAP__FREE(own_copy);
            ar_method_ast__destroy(own_ast);
            return NULL;
        }
        
        // Restore the character
        *mut_current = saved_char;
        
        // Skip past line endings and count lines
        if (*mut_current) {
            // Count the line - handle \n, \r, and \r\n
            if (*mut_current == '\n') {
                current_line++;
                mut_current++;
            } else if (*mut_current == '\r') {
                current_line++;
                mut_current++;
                // Skip \n in \r\n sequence
                if (*mut_current == '\n') {
                    mut_current++;
                }
            }
        }
        
        // Set up for next line
        mut_line_start = mut_current;
    }
    
    AR__HEAP__FREE(own_copy);
    
    return own_ast;
    // Ownership transferred to caller
}

const char* ar_method_parser__get_error(const ar_method_parser_t *ref_parser) {
    if (!ref_parser) {
        return NULL;
    }
    
    return ref_parser->own_error_message;
}

int ar_method_parser__get_error_line(const ar_method_parser_t *ref_parser) {
    if (!ref_parser) {
        return 0;
    }
    
    return ref_parser->error_line;
}