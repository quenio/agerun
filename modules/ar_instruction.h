#ifndef AGERUN_INSTRUCTION_H
#define AGERUN_INSTRUCTION_H

#include <stdbool.h>
#include <stdint.h>
#include "ar_data.h"

/**
 * Context structure for instruction parsing and execution.
 * Contains all the information needed during instruction parsing and execution.
 * 
 * This is an opaque type. Clients should use the provided functions to create,
 * manipulate, and destroy instruction contexts.
 */
typedef struct ar_instruction_context_s ar_instruction_context_t;

/**
 * Instruction type enumeration
 */
typedef enum {
    AR_INSTRUCTION_TYPE__ASSIGNMENT,      // memory.x := expression
    AR_INSTRUCTION_TYPE__SEND,           // send(target, message) or result := send(...)
    AR_INSTRUCTION_TYPE__IF,             // if(condition, true_val, false_val)
    AR_INSTRUCTION_TYPE__COMPILE,        // compile(name, instructions, version)
    AR_INSTRUCTION_TYPE__CREATE,         // create(method, version, context)
    AR_INSTRUCTION_TYPE__DESTROY,        // destroy(type, arg1, arg2)
    AR_INSTRUCTION_TYPE__PARSE,          // parse(string)
    INST_BUILD           // build(template, map)
} ar_instruction_type_t;

/**
 * Parsed instruction structure (opaque type)
 * Contains the parsed representation of an instruction
 */
typedef struct ar_parsed_instruction_s ar_parsed_instruction_t;

/**
 * Creates a new instruction context for parsing and executing instructions.
 *
 * @param mut_memory The memory to use for the instruction (mutable reference, can be NULL if not needed)
 * @param ref_context The context data (borrowed reference, can be NULL if not needed)
 * @param ref_message The message being processed (borrowed reference, can be NULL if not needed)
 * @return Newly created instruction context (owned by caller), or NULL on failure
 * @note Ownership: Returns an owned value that caller must destroy.
 *       The function does not take ownership of the memory, context, or message parameters.
 */
ar_instruction_context_t* ar_instruction__create_context(ar_data_t *mut_memory, const ar_data_t *ref_context, const ar_data_t *ref_message);

/**
 * Destroys an instruction context.
 *
 * @param own_ctx The instruction context to destroy (ownership transferred to function)
 * @note Ownership: Takes ownership of the context parameter and destroys it.
 *       This only frees the context structure itself,
 *       not the memory, context, or message data structures which are owned by the caller.
 */
void ar_instruction__destroy_context(ar_instruction_context_t *own_ctx);


/**
 * Parses an instruction without executing it
 * 
 * @param ref_instruction The instruction string to parse (borrowed reference)
 * @param mut_ctx The instruction context for error reporting (mutable reference)
 * @return Parsed instruction structure, or NULL on parse error
 * @note Ownership: Returns an owned value that caller must destroy with ar_instruction__destroy_parsed.
 *       The function does not take ownership of the instruction string.
 *       Parse errors are reported through the context's error mechanism.
 */
ar_parsed_instruction_t* ar_instruction__parse(const char *ref_instruction, ar_instruction_context_t *mut_ctx);

/**
 * Destroys a parsed instruction and frees its resources
 * 
 * @param own_parsed The parsed instruction to destroy (owned reference, will be freed)
 * @note Ownership: This function takes ownership of the parsed instruction and frees it.
 *       The pointer will be invalid after this call.
 */
void ar_instruction__destroy_parsed(ar_parsed_instruction_t *own_parsed);

/**
 * Gets the type of a parsed instruction
 * 
 * @param ref_parsed The parsed instruction (borrowed reference)
 * @return The instruction type
 * @note Ownership: Does not take ownership of the parsed instruction.
 */
ar_instruction_type_t ar_instruction__get_type(const ar_parsed_instruction_t *ref_parsed);

/**
 * Gets the memory path for an assignment instruction
 * 
 * @param ref_parsed The parsed instruction (borrowed reference)
 * @return The memory path (e.g., "memory.x.y"), or NULL if not an assignment
 * @note Ownership: Returns a borrowed reference. The caller should not free the result.
 */
const char* ar_instruction__get_assignment_path(const ar_parsed_instruction_t *ref_parsed);

/**
 * Gets the expression for an assignment instruction
 * 
 * @param ref_parsed The parsed instruction (borrowed reference)
 * @return The expression string, or NULL if not an assignment
 * @note Ownership: Returns a borrowed reference. The caller should not free the result.
 */
const char* ar_instruction__get_assignment_expression(const ar_parsed_instruction_t *ref_parsed);

/**
 * Gets function call details from a parsed instruction
 * 
 * @param ref_parsed The parsed instruction (borrowed reference)
 * @param out_function_name Output: the function name (borrowed reference)
 * @param out_args Output: array of argument expressions (borrowed references)
 * @param out_arg_count Output: number of arguments
 * @param out_result_path Output: result assignment path if any (borrowed reference, may be NULL)
 * @return true if this is a function call instruction, false otherwise
 * @note Ownership: All output parameters are borrowed references that remain owned by the parsed instruction.
 */
bool ar_instruction__get_function_call(const ar_parsed_instruction_t *ref_parsed,
                                        const char **out_function_name,
                                        const char ***out_args,
                                        int *out_arg_count,
                                        const char **out_result_path);

/**
 * Gets the memory from the instruction context.
 *
 * @param ref_ctx The instruction context (borrowed reference)
 * @return Mutable reference to the memory (not owned by caller)
 * @note Ownership: Does not take ownership of the context parameter.
 *       The returned memory is still owned by the context.
 */
ar_data_t* ar_instruction__get_memory(const ar_instruction_context_t *ref_ctx);

/**
 * Gets the context data from the instruction context.
 *
 * @param ref_ctx The instruction context (borrowed reference)
 * @return Borrowed reference to the context data (not owned by caller)
 * @note Ownership: Does not take ownership of the context parameter.
 *       The returned context data is still owned by the context owner.
 */
const ar_data_t* ar_instruction__get_context(const ar_instruction_context_t *ref_ctx);

/**
 * Gets the message from the instruction context.
 *
 * @param ref_ctx The instruction context (borrowed reference)
 * @return Borrowed reference to the message (not owned by caller)
 * @note Ownership: Does not take ownership of the context parameter.
 *       The returned message is still owned by the context owner.
 */
const ar_data_t* ar_instruction__get_message(const ar_instruction_context_t *ref_ctx);


/**
 * Gets the last error message from the instruction context.
 *
 * @param ref_ctx The instruction context (borrowed reference)
 * @return The last error message, or NULL if no error
 * @note Ownership: Does not take ownership of the context parameter.
 *       The returned string is owned by the context and should not be freed.
 */
const char* ar_instruction__get_last_error(const ar_instruction_context_t *ref_ctx);

/**
 * Gets the position in the instruction string where the last error occurred.
 *
 * @param ref_ctx The instruction context (borrowed reference)
 * @return The error position (1-based column), or 0 if no error
 * @note Ownership: Does not take ownership of the context parameter.
 */
int ar_instruction__get_error_position(const ar_instruction_context_t *ref_ctx);

#endif /* AGERUN_INSTRUCTION_H */
