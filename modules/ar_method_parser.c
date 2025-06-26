#include "ar_method_parser.h"
#include "ar_method_ast.h"
#include "ar_instruction_parser.h"
#include "ar_string.h"
#include "ar_heap.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Opaque structure definition
struct ar_method_parser_s {
    instruction_parser_t *instruction_parser;
};

// Helper function to parse a single line
static bool _parse_line(ar_method_parser_t *mut_parser, ar_method_ast_t *mut_ast, const char *ref_line) {
    // Make a copy and trim the line
    char *own_line = AR__HEAP__STRDUP(ref_line, "line copy");
    if (!own_line) {
        return false;
    }
    
    char *mut_trimmed = ar__string__trim(own_line);
    
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
            mut_trimmed = ar__string__trim(mut_trimmed);
            
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
    
    own_parser->instruction_parser = ar__instruction_parser__create();
    if (!own_parser->instruction_parser) {
        AR__HEAP__FREE(own_parser);
        return NULL;
    }
    
    return own_parser;
    // Ownership transferred to caller
}

void ar_method_parser__destroy(ar_method_parser_t *own_parser) {
    if (!own_parser) {
        return;
    }
    
    if (own_parser->instruction_parser) {
        ar__instruction_parser__destroy(own_parser->instruction_parser);
    }
    
    AR__HEAP__FREE(own_parser);
}

ar_method_ast_t* ar_method_parser__parse(ar_method_parser_t *mut_parser, const char *ref_source) {
    if (!mut_parser || !ref_source) {
        return NULL;
    }
    
    // Make a copy and trim the source to remove leading/trailing whitespace
    char *own_copy = AR__HEAP__STRDUP(ref_source, "method source copy");
    if (!own_copy) {
        return NULL;
    }
    
    char *mut_trimmed = ar__string__trim(own_copy);
    
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
            // Parse failed
            AR__HEAP__FREE(own_copy);
            ar_method_ast__destroy(own_ast);
            return NULL;
        }
        
        // Restore the character
        *mut_current = saved_char;
        
        // Skip past line endings
        while (*mut_current && (*mut_current == '\n' || *mut_current == '\r')) {
            mut_current++;
        }
        
        // Set up for next line
        mut_line_start = mut_current;
    }
    
    AR__HEAP__FREE(own_copy);
    
    return own_ast;
    // Ownership transferred to caller
}