/**
 * @file ar_function_call_parser.h
 * @brief Parser for AgeRun function-call argument lists.
 *
 * This module owns the shared grammar rule for splitting function-call
 * arguments while respecting expression nesting in parentheses, list literals,
 * map literals, and quoted strings.
 */

#ifndef AGERUN_FUNCTION_CALL_PARSER_H
#define AGERUN_FUNCTION_CALL_PARSER_H

#include <stdbool.h>
#include <stddef.h>

#include "ar_list.h"
#include "ar_log.h"

/**
 * Extract one function-call argument up to the requested delimiter.
 *
 * @param ref_log Log instance for parse diagnostics (borrowed reference, nullable)
 * @param ref_str Source instruction text (borrowed reference)
 * @param mut_pos Current parse position, updated to the delimiter position
 * @param delimiter Delimiter that terminates the argument, usually ',' or ')'
 * @return Newly allocated argument string (owned by caller), or NULL on failure
 * @note Ownership: Returns an owned string that must be destroyed with
 *       ar_function_call_parser__destroy_args() or ar_function_call_parser__destroy_arg().
 */
char* ar_function_call_parser__extract_argument(
    ar_log_t *ref_log,
    const char *ref_str,
    size_t *mut_pos,
    char delimiter
);

/**
 * Parse a fixed-size function-call argument list.
 *
 * @param ref_log Log instance for parse diagnostics (borrowed reference, nullable)
 * @param ref_str Source instruction text (borrowed reference)
 * @param mut_pos Current parse position, updated to the closing parenthesis
 * @param out_args Receives an owned array of owned argument strings
 * @param out_count Receives the number of parsed arguments
 * @param expected_count Exact argument count to parse
 * @return true on success, false on parse or allocation failure
 * @note Ownership: On success, caller owns the returned array and all contained strings.
 *       Destroy them with ar_function_call_parser__destroy_args().
 */
bool ar_function_call_parser__parse_exact(
    ar_log_t *ref_log,
    const char *ref_str,
    size_t *mut_pos,
    char ***out_args,
    size_t *out_count,
    size_t expected_count
);

/**
 * Parse argument strings into expression ASTs.
 *
 * @param ref_log Log instance for parse diagnostics (borrowed reference, nullable)
 * @param ref_args Array of argument strings (borrowed references)
 * @param arg_count Number of argument strings
 * @param error_offset Offset to use for wrapper-level parse diagnostics
 * @return Owned list of owned ar_expression_ast_t nodes, or NULL on failure
 * @note Ownership: Returns an owned list and transfers ownership of the ASTs to it.
 *       Destroy failure results with ar_function_call_parser__destroy_arg_asts().
 */
ar_list_t* ar_function_call_parser__parse_arg_asts(
    ar_log_t *ref_log,
    char **ref_args,
    size_t arg_count,
    size_t error_offset
);

/**
 * Destroy a single argument string.
 *
 * @param own_arg Argument string to destroy (ownership transferred)
 */
void ar_function_call_parser__destroy_arg(char *own_arg);

/**
 * Destroy an owned argument string array.
 *
 * @param own_args Argument string array to destroy (ownership transferred)
 * @param arg_count Number of strings in the array
 */
void ar_function_call_parser__destroy_args(char **own_args, size_t arg_count);

/**
 * Destroy an owned argument AST list.
 *
 * @param own_arg_asts Argument AST list to destroy (ownership transferred)
 */
void ar_function_call_parser__destroy_arg_asts(ar_list_t *own_arg_asts);

#endif /* AGERUN_FUNCTION_CALL_PARSER_H */
