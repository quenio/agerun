#include "ar_method_parser.h"
#include "ar_method_ast.h"
#include "ar_instruction_parser.h"
#include "ar_string.h"
#include "ar_heap.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <ctype.h>

// Error buffer size for formatting error messages
#define ERROR_BUFFER_SIZE 512
static const char *MULTILINE_MAP_EXPRESSION_PREFIX = "__agerun_multiline_map__(";

// Opaque structure definition
struct ar_method_parser_s {
    ar_log_t *ref_log;                       /* Log instance for error reporting (borrowed) */
    ar_instruction_parser_t *instruction_parser;
};

// Helper function to log error with line number
static void _log_error(ar_method_parser_t *mut_parser, int line_number, const char *ref_instruction_error) {
    if (mut_parser->ref_log) {
        if (ref_instruction_error) {
            // Format error message with line number
            char error_buffer[ERROR_BUFFER_SIZE];
            snprintf(error_buffer, sizeof(error_buffer), "Line %d: %s", line_number, ref_instruction_error);
            ar_log__error_at(mut_parser->ref_log, error_buffer, line_number);
        } else {
            ar_log__error_at(mut_parser->ref_log, "Unknown parse error", line_number);
        }
    }
}

static bool _append_text(char **own_output, size_t *mut_length, size_t *mut_capacity, const char *ref_text, size_t text_length) {
    if (!own_output || !mut_length || !mut_capacity || !ref_text) {
        return false;
    }

    while (*mut_length + text_length + 1 > *mut_capacity) {
        size_t new_capacity = (*mut_capacity == 0) ? 128 : (*mut_capacity * 2);
        char *own_resized = AR__HEAP__REALLOC(*own_output, new_capacity, "canonical method source");
        if (!own_resized) {
            return false;
        }
        *own_output = own_resized;
        *mut_capacity = new_capacity;
    }

    memcpy(*own_output + *mut_length, ref_text, text_length);
    *mut_length += text_length;
    (*own_output)[*mut_length] = '\0';
    return true;
}

static bool _append_cstr(char **own_output, size_t *mut_length, size_t *mut_capacity, const char *ref_text) {
    return _append_text(own_output, mut_length, mut_capacity, ref_text, strlen(ref_text));
}

static bool _append_char(char **own_output, size_t *mut_length, size_t *mut_capacity, char c) {
    return _append_text(own_output, mut_length, mut_capacity, &c, 1);
}

static size_t _count_indent(const char *ref_line) {
    size_t indent = 0;
    while (ref_line[indent] == ' ' || ref_line[indent] == '\t') {
        indent++;
    }
    return indent;
}

static char* _strip_inline_comments_and_trim(char *mut_line) {
    char *mut_trimmed = ar_string__trim(mut_line);
    char *p = mut_trimmed;
    bool in_quotes = false;

    while (*p) {
        if (*p == '"' && (p == mut_trimmed || *(p - 1) != '\\')) {
            in_quotes = !in_quotes;
        } else if (*p == '#' && !in_quotes) {
            *p = '\0';
            mut_trimmed = ar_string__trim(mut_trimmed);
            break;
        }
        p++;
    }

    return mut_trimmed;
}

static bool _is_multiline_literal_opener(char *mut_trimmed, char *out_open_delimiter, char *out_close_delimiter) {
    char *assignment = strstr(mut_trimmed, ":=");
    if (!assignment || assignment == mut_trimmed) {
        return false;
    }

    char *rhs = ar_string__trim(assignment + 2);
    if (strcmp(rhs, "[") == 0) {
        *out_open_delimiter = '[';
        *out_close_delimiter = ']';
        return true;
    }
    if (strcmp(rhs, "{") == 0) {
        *out_open_delimiter = '{';
        *out_close_delimiter = '}';
        return true;
    }

    return false;
}

static bool _validate_one_line_literal_item(const char *ref_item) {
    bool in_quotes = false;
    int bracket_depth = 0;
    int brace_depth = 0;
    int parenthesis_depth = 0;

    for (size_t i = 0; ref_item[i] != '\0'; i++) {
        char c = ref_item[i];
        if (c == '"' && (i == 0 || ref_item[i - 1] != '\\')) {
            in_quotes = !in_quotes;
            continue;
        }
        if (in_quotes) {
            continue;
        }

        if (c == '[') {
            bracket_depth++;
        } else if (c == ']') {
            bracket_depth--;
            if (bracket_depth < 0) {
                return false;
            }
        } else if (c == '{') {
            brace_depth++;
        } else if (c == '}') {
            brace_depth--;
            if (brace_depth < 0) {
                return false;
            }
        } else if (c == '(') {
            parenthesis_depth++;
        } else if (c == ')') {
            parenthesis_depth--;
            if (parenthesis_depth < 0) {
                return false;
            }
        } else if (c == ',' && bracket_depth == 0 && brace_depth == 0 &&
                   parenthesis_depth == 0) {
            return false;
        }
    }

    return !in_quotes && bracket_depth == 0 && brace_depth == 0 && parenthesis_depth == 0;
}

static bool _is_identifier(const char *ref_text) {
    if (!ref_text || (!isalpha((unsigned char)ref_text[0]) && ref_text[0] != '_')) {
        return false;
    }

    for (size_t i = 1; ref_text[i] != '\0'; i++) {
        if (!isalnum((unsigned char)ref_text[i]) && ref_text[i] != '_') {
            return false;
        }
    }

    return true;
}

static char* _find_top_level_assignment_operator(char *mut_item) {
    bool in_quotes = false;
    int bracket_depth = 0;
    int brace_depth = 0;
    int parenthesis_depth = 0;

    for (size_t i = 0; mut_item[i] != '\0'; i++) {
        char c = mut_item[i];
        if (c == '"' && (i == 0 || mut_item[i - 1] != '\\')) {
            in_quotes = !in_quotes;
            continue;
        }
        if (in_quotes) {
            continue;
        }

        if (c == '[') {
            bracket_depth++;
        } else if (c == ']') {
            bracket_depth--;
        } else if (c == '{') {
            brace_depth++;
        } else if (c == '}') {
            brace_depth--;
        } else if (c == '(') {
            parenthesis_depth++;
        } else if (c == ')') {
            parenthesis_depth--;
        } else if (c == ':' && mut_item[i + 1] == '=' &&
                   bracket_depth == 0 && brace_depth == 0 && parenthesis_depth == 0) {
            return &mut_item[i];
        }
    }

    return NULL;
}

static bool _append_canonical_literal_block(
    ar_method_parser_t *mut_parser,
    char **own_output,
    size_t *mut_output_length,
    size_t *mut_output_capacity,
    const char *ref_lhs,
    char open_delimiter,
    char close_delimiter,
    char **mut_cursor,
    int *mut_line_number,
    size_t opener_indent
) {
    size_t item_indent = (size_t)-1;
    size_t consumed_lines = 1;
    bool first_item = true;
    bool is_map_literal_block = open_delimiter == '{';

    if (!_append_cstr(own_output, mut_output_length, mut_output_capacity, ref_lhs) ||
        !_append_cstr(own_output, mut_output_length, mut_output_capacity, " := ")) {
        _log_error(mut_parser, *mut_line_number, "Out of memory");
        return false;
    }

    if (is_map_literal_block &&
        !_append_cstr(own_output, mut_output_length, mut_output_capacity,
                      MULTILINE_MAP_EXPRESSION_PREFIX)) {
        _log_error(mut_parser, *mut_line_number, "Out of memory");
        return false;
    }

    if (!_append_char(own_output, mut_output_length, mut_output_capacity, open_delimiter)) {
        _log_error(mut_parser, *mut_line_number, "Out of memory");
        return false;
    }

    while (**mut_cursor) {
        char *line_start = *mut_cursor;
        char *line_end = line_start;
        while (*line_end && *line_end != '\n' && *line_end != '\r') {
            line_end++;
        }

        char saved_char = *line_end;
        *line_end = '\0';
        size_t raw_indent = _count_indent(line_start);
        char *own_line = AR__HEAP__STRDUP(line_start, "literal item line");
        *line_end = saved_char;
        if (!own_line) {
            _log_error(mut_parser, *mut_line_number, "Out of memory");
            return false;
        }

        char *trimmed = _strip_inline_comments_and_trim(own_line);
        int item_line_number = *mut_line_number;

        char *next_line = line_end;
        if (*next_line == '\n') {
            next_line++;
        } else if (*next_line == '\r') {
            next_line++;
            if (*next_line == '\n') {
                next_line++;
            }
        }

        (*mut_line_number)++;
        consumed_lines++;

        if (strlen(trimmed) == 1 && trimmed[0] == close_delimiter) {
            if (raw_indent != opener_indent) {
                AR__HEAP__FREE(own_line);
                _log_error(mut_parser, item_line_number, "Multi-line literal closing delimiter indentation mismatch");
                return false;
            }

            if (!_append_char(own_output, mut_output_length, mut_output_capacity, close_delimiter)) {
                AR__HEAP__FREE(own_line);
                _log_error(mut_parser, item_line_number, "Out of memory");
                return false;
            }

            if (is_map_literal_block &&
                !_append_char(own_output, mut_output_length, mut_output_capacity, ')')) {
                AR__HEAP__FREE(own_line);
                _log_error(mut_parser, item_line_number, "Out of memory");
                return false;
            }

            for (size_t i = 0; i < consumed_lines; i++) {
                if (!_append_char(own_output, mut_output_length, mut_output_capacity, '\n')) {
                    AR__HEAP__FREE(own_line);
                    _log_error(mut_parser, item_line_number, "Out of memory");
                    return false;
                }
            }

            AR__HEAP__FREE(own_line);
            *mut_cursor = next_line;
            return true;
        }

        if (strlen(trimmed) == 0) {
            AR__HEAP__FREE(own_line);
            _log_error(mut_parser, item_line_number, "Multi-line literal item line cannot be empty");
            return false;
        }

        if (raw_indent <= opener_indent) {
            AR__HEAP__FREE(own_line);
            _log_error(mut_parser, item_line_number, "Multi-line literal item indentation must be greater than assignment indentation");
            return false;
        }

        if (item_indent == (size_t)-1) {
            item_indent = raw_indent;
        } else if (raw_indent != item_indent) {
            AR__HEAP__FREE(own_line);
            _log_error(mut_parser, item_line_number, "Multi-line literal item indentation must be consistent");
            return false;
        }

        bool is_map_literal = open_delimiter == '{';
        char *map_entry_value = NULL;
        if (is_map_literal) {
            char *assignment = _find_top_level_assignment_operator(trimmed);
            if (!assignment) {
                AR__HEAP__FREE(own_line);
                _log_error(mut_parser, item_line_number, "Multi-line map entries must use identifier := expression");
                return false;
            }

            *assignment = '\0';
            char *map_entry_key = ar_string__trim(trimmed);
            map_entry_value = ar_string__trim(assignment + 2);
            if (!_is_identifier(map_entry_key) || strlen(map_entry_value) == 0 ||
                !_validate_one_line_literal_item(map_entry_value)) {
                AR__HEAP__FREE(own_line);
                _log_error(mut_parser, item_line_number, "Multi-line map entries must use identifier := expression");
                return false;
            }

            trimmed = map_entry_key;
        } else if (!_validate_one_line_literal_item(trimmed)) {
            AR__HEAP__FREE(own_line);
            _log_error(mut_parser, item_line_number, "Multi-line literal items must be one complete item per line");
            return false;
        }

        if (!first_item) {
            if (!_append_cstr(own_output, mut_output_length, mut_output_capacity, ", ")) {
                AR__HEAP__FREE(own_line);
                _log_error(mut_parser, item_line_number, "Out of memory");
                return false;
            }
        }
        first_item = false;

        bool append_success = false;
        if (is_map_literal) {
            append_success =
                _append_cstr(own_output, mut_output_length, mut_output_capacity, trimmed) &&
                _append_cstr(own_output, mut_output_length, mut_output_capacity, ": ") &&
                _append_cstr(own_output, mut_output_length, mut_output_capacity, map_entry_value);
        } else {
            append_success = _append_cstr(own_output, mut_output_length, mut_output_capacity, trimmed);
        }

        if (!append_success) {
            AR__HEAP__FREE(own_line);
            _log_error(mut_parser, item_line_number, "Out of memory");
            return false;
        }

        AR__HEAP__FREE(own_line);
        *mut_cursor = next_line;
    }

    _log_error(mut_parser, *mut_line_number, "Unterminated multi-line literal assignment");
    return false;
}

static char* _canonicalize_multiline_literals(ar_method_parser_t *mut_parser, const char *ref_source) {
    size_t output_length = 0;
    size_t output_capacity = strlen(ref_source) + 1;
    char *own_output = AR__HEAP__MALLOC(output_capacity, "canonical method source");
    if (!own_output) {
        return NULL;
    }
    own_output[0] = '\0';

    char *own_scan = AR__HEAP__STRDUP(ref_source, "method source scan");
    if (!own_scan) {
        AR__HEAP__FREE(own_output);
        return NULL;
    }

    char *cursor = own_scan;
    int line_number = 1;

    while (*cursor) {
        char *line_start = cursor;
        char *line_end = cursor;
        while (*line_end && *line_end != '\n' && *line_end != '\r') {
            line_end++;
        }

        char saved_char = *line_end;
        *line_end = '\0';
        size_t opener_indent = _count_indent(line_start);
        char *own_line = AR__HEAP__STRDUP(line_start, "method source line");
        *line_end = saved_char;
        if (!own_line) {
            AR__HEAP__FREE(own_scan);
            AR__HEAP__FREE(own_output);
            return NULL;
        }

        char *trimmed = _strip_inline_comments_and_trim(own_line);
        char open_delimiter = '\0';
        char close_delimiter = '\0';
        bool is_opener = _is_multiline_literal_opener(trimmed, &open_delimiter, &close_delimiter);

        char *next_line = line_end;
        if (*next_line == '\n') {
            next_line++;
        } else if (*next_line == '\r') {
            next_line++;
            if (*next_line == '\n') {
                next_line++;
            }
        }

        if (is_opener) {
            char *assignment = strstr(trimmed, ":=");
            *assignment = '\0';
            char *lhs = ar_string__trim(trimmed);

            cursor = next_line;
            line_number++;
            bool success = _append_canonical_literal_block(
                mut_parser,
                &own_output,
                &output_length,
                &output_capacity,
                lhs,
                open_delimiter,
                close_delimiter,
                &cursor,
                &line_number,
                opener_indent
            );
            AR__HEAP__FREE(own_line);
            if (!success) {
                AR__HEAP__FREE(own_scan);
                AR__HEAP__FREE(own_output);
                return NULL;
            }
            continue;
        }

        if (!_append_text(&own_output, &output_length, &output_capacity, line_start, (size_t)(line_end - line_start))) {
            AR__HEAP__FREE(own_line);
            AR__HEAP__FREE(own_scan);
            AR__HEAP__FREE(own_output);
            return NULL;
        }

        AR__HEAP__FREE(own_line);

        if (saved_char != '\0') {
            if (!_append_char(&own_output, &output_length, &output_capacity, '\n')) {
                AR__HEAP__FREE(own_scan);
                AR__HEAP__FREE(own_output);
                return NULL;
            }
            line_number++;
        }

        cursor = next_line;
    }

    AR__HEAP__FREE(own_scan);
    return own_output;
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

ar_method_parser_t* ar_method_parser__create(ar_log_t *ref_log) {
    ar_method_parser_t *own_parser = AR__HEAP__MALLOC(sizeof(ar_method_parser_t), "method_parser");
    if (!own_parser) {
        return NULL;
    }
    
    own_parser->ref_log = ref_log;
    own_parser->instruction_parser = ar_instruction_parser__create(ref_log);
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
        ar_instruction_parser__destroy(own_parser->instruction_parser);
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

    char *own_canonical_source = _canonicalize_multiline_literals(mut_parser, mut_trimmed);
    if (!own_canonical_source) {
        AR__HEAP__FREE(own_copy);
        ar_method_ast__destroy(own_ast);
        return NULL;
    }
    
    // Split by lines and parse each instruction
    char *mut_current = own_canonical_source;
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
            _log_error(mut_parser, current_line, ref_inst_error);
            
            AR__HEAP__FREE(own_canonical_source);
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
    
    AR__HEAP__FREE(own_canonical_source);
    AR__HEAP__FREE(own_copy);
    
    return own_ast;
    // Ownership transferred to caller
}
