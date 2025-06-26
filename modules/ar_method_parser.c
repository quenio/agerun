#include "ar_method_parser.h"
#include "ar_method_ast.h"
#include "ar_instruction_parser.h"
#include "ar_string.h"
#include "ar_heap.h"
#include <stdlib.h>
#include <string.h>

// Opaque structure definition
struct ar_method_parser_s {
    instruction_parser_t *instruction_parser;
};

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
    
    // Parse the single instruction
    ar_instruction_ast_t *own_instruction = ar_instruction_parser__parse(mut_parser->instruction_parser, mut_trimmed);
    AR__HEAP__FREE(own_copy);
    
    if (!own_instruction) {
        // Parse failed
        ar_method_ast__destroy(own_ast);
        return NULL;
    }
    
    // Add the instruction to the AST
    ar_method_ast__add_instruction(own_ast, own_instruction);
    
    return own_ast;
    // Ownership transferred to caller
}