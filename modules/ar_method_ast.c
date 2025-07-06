#include "ar_method_ast.h"
#include "ar_heap.h"
#include "ar_list.h"
#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * Method AST structure (internal).
 * Contains a list of instruction ASTs.
 */
struct ar_method_ast_s {
    ar_list_t *instructions;  // List of ar_instruction_ast_t*
};

/**
 * Create a new method AST.
 */
ar_method_ast_t* ar_method_ast__create(void) {
    ar_method_ast_t *own_ast = AR__HEAP__MALLOC(sizeof(ar_method_ast_t), "method_ast");
    if (!own_ast) {
        return NULL;
    }
    
    own_ast->instructions = ar_list__create();
    if (!own_ast->instructions) {
        AR__HEAP__FREE(own_ast);
        return NULL;
    }
    
    return own_ast;
    // Ownership transferred to caller
}

/**
 * Destroy a method AST and all its instruction ASTs.
 */
void ar_method_ast__destroy(ar_method_ast_t* own_ast) {
    if (!own_ast) {
        return;
    }
    
    // Destroy all instruction ASTs in the list
    if (own_ast->instructions) {
        // Remove and destroy instructions from the end to avoid shifting
        while (!ar_list__empty(own_ast->instructions)) {
            ar_instruction_ast_t *own_instruction = ar_list__remove_last(own_ast->instructions);
            if (own_instruction) {
                ar_instruction_ast__destroy(own_instruction);
            }
        }
        ar_list__destroy(own_ast->instructions);
    }
    
    AR__HEAP__FREE(own_ast);
}

/**
 * Add an instruction AST to the method AST.
 */
void ar_method_ast__add_instruction(ar_method_ast_t* mut_ast, ar_instruction_ast_t* own_instruction) {
    if (!mut_ast || !own_instruction) {
        // If instruction is provided but AST is NULL, we need to destroy the instruction
        // to prevent memory leak since we're taking ownership
        if (own_instruction) {
            ar_instruction_ast__destroy(own_instruction);
        }
        return;
    }
    
    bool added = ar_list__add_last(mut_ast->instructions, own_instruction);
    if (!added) {
        // Failed to add to list, need to clean up the instruction
        ar_instruction_ast__destroy(own_instruction);
    }
    // Ownership of instruction transferred to list on success
}

/**
 * Get the number of instructions in the method AST.
 */
size_t ar_method_ast__get_instruction_count(const ar_method_ast_t* ref_ast) {
    if (!ref_ast || !ref_ast->instructions) {
        return 0;
    }
    
    return ar_list__count(ref_ast->instructions);
}

/**
 * Get an instruction by line number (1-based).
 */
const ar_instruction_ast_t* ar_method_ast__get_instruction(const ar_method_ast_t* ref_ast, size_t line_no) {
    if (!ref_ast || !ref_ast->instructions || line_no == 0) {
        return NULL;
    }
    
    size_t count = ar_list__count(ref_ast->instructions);
    if (line_no > count) {
        return NULL;
    }
    
    // Get the array of instructions
    void **items = ar_list__items(ref_ast->instructions);
    if (!items) {
        return NULL;
    }
    
    // Get the instruction at the given line (convert 1-based to 0-based index)
    const ar_instruction_ast_t* instruction = (const ar_instruction_ast_t*)items[line_no - 1];
    
    // Free the array (but not the items themselves)
    AR__HEAP__FREE(items);
    
    return instruction;
}