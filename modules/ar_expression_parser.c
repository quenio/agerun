#include "ar_expression_parser.h"
#include "ar_heap.h"
#include "ar_string.h"
#include "ar_list.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>

/**
 * Parser state structure.
 * Tracks the expression being parsed and current position.
 */
struct ar_expression_parser_s {
    ar_log_t *ref_log;         /* Log instance for error reporting (borrowed) */
    char *own_expression;      /* Copy of the expression string */
    size_t position;           /* Current parsing position */
};

/* Forward declarations for internal parsing functions */
static void _skip_whitespace(ar_expression_parser_t *mut_parser);
static bool _peek_char(const ar_expression_parser_t *ref_parser, char expected);
static bool _consume_char(ar_expression_parser_t *mut_parser, char expected);
static char _current_char(const ar_expression_parser_t *ref_parser);
static void _advance(ar_expression_parser_t *mut_parser);
static void _set_error(ar_expression_parser_t *mut_parser, const char *ref_message);
static ar_expression_ast_t* _parse_primary(ar_expression_parser_t *mut_parser);
static ar_expression_ast_t* _parse_term(ar_expression_parser_t *mut_parser);
static ar_expression_ast_t* _parse_additive(ar_expression_parser_t *mut_parser);
static ar_expression_ast_t* _parse_relational(ar_expression_parser_t *mut_parser);
static ar_expression_ast_t* _parse_equality(ar_expression_parser_t *mut_parser);

/**
 * Create a new expression parser instance.
 */
ar_expression_parser_t* ar_expression_parser__create(ar_log_t *ref_log, const char *ref_expression) {
    if (!ref_expression) {
        if (ref_log) {
            ar_log__error(ref_log, "NULL expression provided to expression parser");
        }
        return NULL;
    }
    
    ar_expression_parser_t *own_parser = AR__HEAP__MALLOC(sizeof(ar_expression_parser_t), "Expression parser");
    if (!own_parser) {
        if (ref_log) {
            ar_log__error(ref_log, "Failed to allocate memory for expression parser");
        }
        return NULL;
    }
    
    own_parser->ref_log = ref_log;
    own_parser->own_expression = AR__HEAP__STRDUP(ref_expression, "Parser expression copy");
    if (!own_parser->own_expression) {
        if (ref_log) {
            ar_log__error(ref_log, "Failed to allocate memory for expression copy");
        }
        AR__HEAP__FREE(own_parser);
        return NULL;
    }
    
    own_parser->position = 0;
    
    return own_parser; // Ownership transferred to caller
}

/**
 * Destroy an expression parser instance.
 */
void ar_expression_parser__destroy(ar_expression_parser_t *own_parser) {
    if (!own_parser) {
        return;
    }
    
    if (own_parser->own_expression) {
        AR__HEAP__FREE(own_parser->own_expression);
    }
    
    AR__HEAP__FREE(own_parser);
}

/**
 * Get the current position in the expression being parsed.
 */
size_t ar_expression_parser__get_position(const ar_expression_parser_t *ref_parser) {
    if (!ref_parser) {
        return 0;
    }
    return ref_parser->position;
}

/**
 * Get the last error message from the parser.
 * DEPRECATED: This function always returns NULL. Use ar_log for error reporting.
 */
const char* ar_expression_parser__get_error(const ar_expression_parser_t *ref_parser) {
    (void)ref_parser; // Suppress unused parameter warning
    return NULL;
}


/**
 * Skip whitespace characters.
 */
static void _skip_whitespace(ar_expression_parser_t *mut_parser) {
    if (!mut_parser || !mut_parser->own_expression) {
        return;
    }
    
    while (isspace((unsigned char)mut_parser->own_expression[mut_parser->position])) {
        mut_parser->position++;
    }
}

/**
 * Get the current character without advancing.
 */
static char _current_char(const ar_expression_parser_t *ref_parser) {
    if (!ref_parser || !ref_parser->own_expression) {
        return '\0';
    }
    return ref_parser->own_expression[ref_parser->position];
}

/**
 * Check if the current character matches expected without consuming.
 */
static bool _peek_char(const ar_expression_parser_t *ref_parser, char expected) {
    return _current_char(ref_parser) == expected;
}

/**
 * Advance the position by one character.
 */
static void _advance(ar_expression_parser_t *mut_parser) {
    if (!mut_parser || !mut_parser->own_expression) {
        return;
    }
    
    if (mut_parser->own_expression[mut_parser->position] != '\0') {
        mut_parser->position++;
    }
}

/**
 * Consume a character if it matches expected.
 */
static bool _consume_char(ar_expression_parser_t *mut_parser, char expected) {
    if (_peek_char(mut_parser, expected)) {
        _advance(mut_parser);
        return true;
    }
    return false;
}

/**
 * Set an error message.
 */
static void _set_error(ar_expression_parser_t *mut_parser, const char *ref_message) {
    if (!mut_parser || !ref_message) {
        return;
    }
    
    // Log the error with position
    if (mut_parser->ref_log) {
        ar_log__error_at(mut_parser->ref_log, ref_message, (int)mut_parser->position);
    }
}

/**
 * Parse a literal (integer, double, or string).
 */
ar_expression_ast_t* ar_expression_parser__parse_literal(ar_expression_parser_t *mut_parser) {
    if (!mut_parser) {
        return NULL;
    }
    
    _skip_whitespace(mut_parser);
    char current = _current_char(mut_parser);
    
    // String literal
    if (current == '"') {
        _advance(mut_parser); // Skip opening quote
        
        size_t start = mut_parser->position;
        while (_current_char(mut_parser) != '"' && _current_char(mut_parser) != '\0') {
            _advance(mut_parser);
        }
        
        if (_current_char(mut_parser) != '"') {
            _set_error(mut_parser, "Unterminated string literal");
            return NULL;
        }
        
        // Extract string content
        size_t length = mut_parser->position - start;
        char *own_string = AR__HEAP__MALLOC(length + 1, "String literal content");
        if (!own_string) {
            _set_error(mut_parser, "Out of memory");
            return NULL;
        }
        
        strncpy(own_string, mut_parser->own_expression + start, length);
        own_string[length] = '\0';
        
        _advance(mut_parser); // Skip closing quote
        
        ar_expression_ast_t *own_node = ar_expression_ast__create_literal_string(own_string);
        AR__HEAP__FREE(own_string);
        
        if (!own_node) {
            _set_error(mut_parser, "Failed to create string literal AST node");
        }
        
        return own_node;
    }
    
    // Number literal (integer or double)
    if (isdigit((unsigned char)current) || current == '-') {
        size_t start = mut_parser->position;
        bool has_dot = false;
        
        // Handle negative sign
        if (current == '-') {
            _advance(mut_parser);
        }
        
        // Parse digits before decimal point
        while (isdigit((unsigned char)_current_char(mut_parser))) {
            _advance(mut_parser);
        }
        
        // Check for decimal point
        if (_peek_char(mut_parser, '.')) {
            has_dot = true;
            _advance(mut_parser); // Skip dot
            
            // Parse digits after decimal point
            while (isdigit((unsigned char)_current_char(mut_parser))) {
                _advance(mut_parser);
            }
        }
        
        // Extract number string
        size_t length = mut_parser->position - start;
        char *own_number = AR__HEAP__MALLOC(length + 1, "Number literal string");
        if (!own_number) {
            _set_error(mut_parser, "Out of memory");
            return NULL;
        }
        
        strncpy(own_number, mut_parser->own_expression + start, length);
        own_number[length] = '\0';
        
        ar_expression_ast_t *own_node = NULL;
        
        if (has_dot) {
            // Parse as double
            char *endptr;
            errno = 0;
            double value = strtod(own_number, &endptr);
            
            if (errno != 0 || *endptr != '\0') {
                AR__HEAP__FREE(own_number);
                _set_error(mut_parser, "Invalid double literal");
                return NULL;
            }
            
            own_node = ar_expression_ast__create_literal_double(value);
        } else {
            // Parse as integer
            char *endptr;
            errno = 0;
            long value = strtol(own_number, &endptr, 10);
            
            if (errno != 0 || *endptr != '\0') {
                AR__HEAP__FREE(own_number);
                _set_error(mut_parser, "Invalid integer literal");
                return NULL;
            }
            
            own_node = ar_expression_ast__create_literal_int((int)value);
        }
        
        AR__HEAP__FREE(own_number);
        
        if (!own_node) {
            _set_error(mut_parser, "Failed to create number literal AST node");
        }
        
        return own_node;
    }
    
    _set_error(mut_parser, "Expected literal (string or number)");
    return NULL;
}

/**
 * Parse a memory access expression.
 */
ar_expression_ast_t* ar_expression_parser__parse_memory_access(ar_expression_parser_t *mut_parser) {
    if (!mut_parser) {
        return NULL;
    }
    
    _skip_whitespace(mut_parser);
    
    // Check for valid base accessor
    const char *bases[] = {"memory", "message", "context"};
    const char *base = NULL;
    size_t base_len = 0;
    
    for (int i = 0; i < 3; i++) {
        size_t len = strlen(bases[i]);
        if (strncmp(mut_parser->own_expression + mut_parser->position, bases[i], len) == 0) {
            // Check that it's not part of a larger identifier
            char next = mut_parser->own_expression[mut_parser->position + len];
            if (!isalnum((unsigned char)next) && next != '_') {
                base = bases[i];
                base_len = len;
                break;
            }
        }
    }
    
    if (!base) {
        // Not a memory access - return NULL without setting error
        // This allows the parser to try other expression types
        return NULL;
    }
    
    // Advance past the base
    mut_parser->position += base_len;
    
    // Parse path components if present
    ar_list_t *own_path_list = ar_list__create();
    if (!own_path_list) {
        _set_error(mut_parser, "Out of memory");
        return NULL;
    }
    
    while (_consume_char(mut_parser, '.')) {
        // Parse identifier
        size_t start = mut_parser->position;
        
        if (!isalpha((unsigned char)_current_char(mut_parser)) && _current_char(mut_parser) != '_') {
            // Clean up path components before destroying list
            void *item;
            while ((item = ar_list__remove_first(own_path_list)) != NULL) {
                AR__HEAP__FREE(item);
            }
            ar_list__destroy(own_path_list);
            _set_error(mut_parser, "Expected identifier after '.'");
            return NULL;
        }
        
        while (isalnum((unsigned char)_current_char(mut_parser)) || _current_char(mut_parser) == '_') {
            _advance(mut_parser);
        }
        
        // Extract identifier
        size_t length = mut_parser->position - start;
        char *own_component = AR__HEAP__MALLOC(length + 1, "Path component");
        if (!own_component) {
            // Clean up path components before destroying list
            void *item;
            while ((item = ar_list__remove_first(own_path_list)) != NULL) {
                AR__HEAP__FREE(item);
            }
            ar_list__destroy(own_path_list);
            _set_error(mut_parser, "Out of memory");
            return NULL;
        }
        
        strncpy(own_component, mut_parser->own_expression + start, length);
        own_component[length] = '\0';
        
        ar_list__add_last(own_path_list, own_component);
    }
    
    // Convert list to array for AST node creation
    size_t path_count = ar_list__count(own_path_list);
    const char **path_array = NULL;
    void **own_items = NULL;
    
    if (path_count > 0) {
        own_items = ar_list__items(own_path_list);
        if (!own_items) {
            // Clean up path components before destroying list
            void *item;
            while ((item = ar_list__remove_first(own_path_list)) != NULL) {
                AR__HEAP__FREE(item);
            }
            ar_list__destroy(own_path_list);
            _set_error(mut_parser, "Failed to get path items");
            return NULL;
        }
        
        // Create a const char** array from the void** array
        path_array = AR__HEAP__MALLOC(sizeof(const char*) * path_count, "Path array");
        if (!path_array) {
            AR__HEAP__FREE(own_items);
            // Clean up path components before destroying list
            void *item;
            while ((item = ar_list__remove_first(own_path_list)) != NULL) {
                AR__HEAP__FREE(item);
            }
            ar_list__destroy(own_path_list);
            _set_error(mut_parser, "Out of memory");
            return NULL;
        }
        
        for (size_t i = 0; i < path_count; i++) {
            path_array[i] = (const char *)own_items[i];
        }
    }
    
    // Create AST node
    ar_expression_ast_t *own_node = ar_expression_ast__create_memory_access(base, path_array, path_count);
    
    // Clean up the arrays
    if (own_items) {
        AR__HEAP__FREE(own_items);
    }
    if (path_array) {
        AR__HEAP__FREE(path_array);
    }
    
    // Clean up path components and destroy list
    void *item;
    while ((item = ar_list__remove_first(own_path_list)) != NULL) {
        AR__HEAP__FREE(item);
    }
    ar_list__destroy(own_path_list);
    
    if (!own_node) {
        _set_error(mut_parser, "Failed to create memory access AST node");
    }
    
    return own_node;
}

/**
 * Parse a primary expression (literal, memory access, or parenthesized expression).
 */
static ar_expression_ast_t* _parse_primary(ar_expression_parser_t *mut_parser) {
    if (!mut_parser) {
        return NULL;
    }
    
    _skip_whitespace(mut_parser);
    
    // Parenthesized expression
    if (_consume_char(mut_parser, '(')) {
        ar_expression_ast_t *own_expr = _parse_equality(mut_parser);
        if (!own_expr) {
            return NULL;
        }
        
        _skip_whitespace(mut_parser);
        if (!_consume_char(mut_parser, ')')) {
            ar_expression_ast__destroy(own_expr);
            _set_error(mut_parser, "Expected ')' after expression");
            return NULL;
        }
        
        return own_expr;
    }
    
    // Try memory access first
    ar_expression_ast_t *own_node = ar_expression_parser__parse_memory_access(mut_parser);
    if (own_node) {
        return own_node;
    }
    
    // Note: Error from failed memory access attempt is already logged to ar_log
    
    // Try literal
    return ar_expression_parser__parse_literal(mut_parser);
}

/**
 * Parse a term (multiplication and division).
 */
static ar_expression_ast_t* _parse_term(ar_expression_parser_t *mut_parser) {
    if (!mut_parser) {
        return NULL;
    }
    
    ar_expression_ast_t *own_left = _parse_primary(mut_parser);
    if (!own_left) {
        return NULL;
    }
    
    while (true) {
        _skip_whitespace(mut_parser);
        
        ar_binary_operator_t op;
        if (_peek_char(mut_parser, '*')) {
            op = AR_BINARY_OPERATOR__MULTIPLY;
        } else if (_peek_char(mut_parser, '/')) {
            op = AR_BINARY_OPERATOR__DIVIDE;
        } else {
            break;
        }
        
        _advance(mut_parser); // Consume operator
        
        ar_expression_ast_t *own_right = _parse_primary(mut_parser);
        if (!own_right) {
            ar_expression_ast__destroy(own_left);
            _set_error(mut_parser, op == AR_BINARY_OPERATOR__MULTIPLY ? 
                      "Failed to parse right operand of multiplication" :
                      "Failed to parse right operand of division");
            return NULL;
        }
        
        ar_expression_ast_t *own_new_left = ar_expression_ast__create_binary_op(op, own_left, own_right);
        if (!own_new_left) {
            // own_left and own_right are already destroyed by create_binary_op
            _set_error(mut_parser, "Failed to create binary operation AST node");
            return NULL;
        }
        
        own_left = own_new_left;
    }
    
    return own_left;
}

/**
 * Parse an additive expression (addition and subtraction).
 */
static ar_expression_ast_t* _parse_additive(ar_expression_parser_t *mut_parser) {
    if (!mut_parser) {
        return NULL;
    }
    
    ar_expression_ast_t *own_left = _parse_term(mut_parser);
    if (!own_left) {
        return NULL;
    }
    
    while (true) {
        _skip_whitespace(mut_parser);
        
        ar_binary_operator_t op;
        if (_peek_char(mut_parser, '+')) {
            op = AR_BINARY_OPERATOR__ADD;
        } else if (_peek_char(mut_parser, '-')) {
            op = AR_BINARY_OPERATOR__SUBTRACT;
        } else {
            break;
        }
        
        _advance(mut_parser); // Consume operator
        
        ar_expression_ast_t *own_right = _parse_term(mut_parser);
        if (!own_right) {
            ar_expression_ast__destroy(own_left);
            _set_error(mut_parser, op == AR_BINARY_OPERATOR__ADD ? 
                      "Failed to parse right operand of addition" :
                      "Failed to parse right operand of subtraction");
            return NULL;
        }
        
        ar_expression_ast_t *own_new_left = ar_expression_ast__create_binary_op(op, own_left, own_right);
        if (!own_new_left) {
            // own_left and own_right are already destroyed by create_binary_op
            _set_error(mut_parser, "Failed to create binary operation AST node");
            return NULL;
        }
        
        own_left = own_new_left;
    }
    
    return own_left;
}

/**
 * Parse a relational expression (<, <=, >, >=).
 */
static ar_expression_ast_t* _parse_relational(ar_expression_parser_t *mut_parser) {
    if (!mut_parser) {
        return NULL;
    }
    
    ar_expression_ast_t *own_left = _parse_additive(mut_parser);
    if (!own_left) {
        return NULL;
    }
    
    while (true) {
        _skip_whitespace(mut_parser);
        
        ar_binary_operator_t op;
        if (_peek_char(mut_parser, '<')) {
            _advance(mut_parser);
            if (_consume_char(mut_parser, '=')) {
                op = AR_BINARY_OPERATOR__LESS_EQ;
            } else if (_consume_char(mut_parser, '>')) {
                // <> for not equal
                op = AR_BINARY_OPERATOR__NOT_EQUAL;
            } else {
                op = AR_BINARY_OPERATOR__LESS;
            }
        } else if (_peek_char(mut_parser, '>')) {
            _advance(mut_parser);
            if (_consume_char(mut_parser, '=')) {
                op = AR_BINARY_OPERATOR__GREATER_EQ;
            } else {
                op = AR_BINARY_OPERATOR__GREATER;
            }
        } else {
            break;
        }
        
        ar_expression_ast_t *own_right = _parse_additive(mut_parser);
        if (!own_right) {
            ar_expression_ast__destroy(own_left);
            const char *op_name = (op == AR_BINARY_OPERATOR__LESS) ? "less than" :
                                  (op == AR_BINARY_OPERATOR__LESS_EQ) ? "less than or equal" :
                                  (op == AR_BINARY_OPERATOR__GREATER_EQ) ? "greater than or equal" :
                                  "greater than";
            char error_msg[128];
            snprintf(error_msg, sizeof(error_msg), "Failed to parse right operand of %s comparison", op_name);
            _set_error(mut_parser, error_msg);
            return NULL;
        }
        
        ar_expression_ast_t *own_new_left = ar_expression_ast__create_binary_op(op, own_left, own_right);
        if (!own_new_left) {
            // own_left and own_right are already destroyed by create_binary_op
            _set_error(mut_parser, "Failed to create binary operation AST node");
            return NULL;
        }
        
        own_left = own_new_left;
    }
    
    return own_left;
}

/**
 * Parse an equality expression (=, <>).
 */
static ar_expression_ast_t* _parse_equality(ar_expression_parser_t *mut_parser) {
    if (!mut_parser) {
        return NULL;
    }
    
    ar_expression_ast_t *own_left = _parse_relational(mut_parser);
    if (!own_left) {
        return NULL;
    }
    
    while (true) {
        _skip_whitespace(mut_parser);
        
        ar_binary_operator_t op;
        if (_peek_char(mut_parser, '=')) {
            _advance(mut_parser);
            op = AR_BINARY_OPERATOR__EQUAL;
        } else {
            break;
        }
        
        ar_expression_ast_t *own_right = _parse_relational(mut_parser);
        if (!own_right) {
            ar_expression_ast__destroy(own_left);
            _set_error(mut_parser, op == AR_BINARY_OPERATOR__EQUAL ? 
                      "Failed to parse right operand of equality comparison" :
                      "Failed to parse right operand of inequality comparison");
            return NULL;
        }
        
        ar_expression_ast_t *own_new_left = ar_expression_ast__create_binary_op(op, own_left, own_right);
        if (!own_new_left) {
            // own_left and own_right are already destroyed by create_binary_op
            _set_error(mut_parser, "Failed to create binary operation AST node");
            return NULL;
        }
        
        own_left = own_new_left;
    }
    
    return own_left;
}

/**
 * Parse an arithmetic expression.
 */
ar_expression_ast_t* ar_expression_parser__parse_arithmetic(ar_expression_parser_t *mut_parser) {
    // Arithmetic expressions are handled by the additive parser
    return _parse_additive(mut_parser);
}

/**
 * Parse a comparison expression.
 */
ar_expression_ast_t* ar_expression_parser__parse_comparison(ar_expression_parser_t *mut_parser) {
    // Comparison expressions are handled by the equality parser
    return _parse_equality(mut_parser);
}

/**
 * Parse an expression (main entry point).
 */
ar_expression_ast_t* ar_expression_parser__parse_expression(ar_expression_parser_t *mut_parser) {
    if (!mut_parser) {
        return NULL;
    }
    
    // Start with equality (lowest precedence)
    ar_expression_ast_t *own_expr = _parse_equality(mut_parser);
    if (!own_expr) {
        return NULL;
    }
    
    // Check for unexpected characters after expression
    _skip_whitespace(mut_parser);
    if (_current_char(mut_parser) != '\0') {
        ar_expression_ast__destroy(own_expr);
        _set_error(mut_parser, "Unexpected characters after expression");
        return NULL;
    }
    
    return own_expr;
}
