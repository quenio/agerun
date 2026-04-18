/**
 * @file ar_complete_instruction_parser.h
 * @brief Parser for complete() function instructions in the AgeRun language
 */

#ifndef AGERUN_COMPLETE_INSTRUCTION_PARSER_H
#define AGERUN_COMPLETE_INSTRUCTION_PARSER_H

#include <stddef.h>
#include "ar_instruction_ast.h"
#include "ar_log.h"

typedef struct ar_complete_instruction_parser_s ar_complete_instruction_parser_t;

ar_complete_instruction_parser_t* ar_complete_instruction_parser__create(ar_log_t *ref_log);
void ar_complete_instruction_parser__destroy(ar_complete_instruction_parser_t *own_parser);
ar_instruction_ast_t* ar_complete_instruction_parser__parse(
    ar_complete_instruction_parser_t *mut_parser,
    const char *ref_instruction,
    const char *ref_result_path
);
const char* ar_complete_instruction_parser__get_error(const ar_complete_instruction_parser_t *ref_parser);
size_t ar_complete_instruction_parser__get_error_position(const ar_complete_instruction_parser_t *ref_parser);

#endif
