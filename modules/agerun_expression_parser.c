#include "agerun_expression_parser.h"
#include "agerun_heap.h"
#include "agerun_string.h"
#include "agerun_list.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>

/**
 * Parser state structure.
 * Tracks the expression being parsed and current position.
 */
struct expression_parser_s {
    char *own_expression;      /* Copy of the expression string */
    size_t position;           /* Current parsing position */
    char *own_error_message;   /* Last error message (if any) */
};

/* Forward declarations for internal parsing functions */
static void _skip_whitespace(expression_parser_t *mut_parser);
static bool _peek_char(const expression_parser_t *ref_parser, char expected);
static bool _consume_char(expression_parser_t *mut_parser, char expected);
static char _current_char(const expression_parser_t *ref_parser);
static void _advance(expression_parser_t *mut_parser);
static void _set_error(expression_parser_t *mut_parser, const char *ref_message);
static expression_ast_t* _parse_primary(expression_parser_t *mut_parser);
static expression_ast_t* _parse_term(expression_parser_t *mut_parser);
static expression_ast_t* _parse_additive(expression_parser_t *mut_parser);
static expression_ast_t* _parse_relational(expression_parser_t *mut_parser);
static expression_ast_t* _parse_equality(expression_parser_t *mut_parser);

/**
 * Create a new expression parser instance.
 */
expression_parser_t* ar__expression_parser__create(const char *ref_expression) {
    if (!ref_expression) {
        return NULL;
    }
    
    expression_parser_t *own_parser = AR__HEAP__MALLOC(sizeof(expression_parser_t), "Expression parser");
    if (!own_parser) {
        return NULL;
    }
    
    own_parser->own_expression = AR__HEAP__STRDUP(ref_expression, "Parser expression copy");
    if (!own_parser->own_expression) {
        AR__HEAP__FREE(own_parser);
        return NULL;
    }
    
    own_parser->position = 0;
    own_parser->own_error_message = NULL;
    
    return own_parser; // Ownership transferred to caller
}

/**
 * Destroy an expression parser instance.
 */
void ar__expression_parser__destroy(expression_parser_t *own_parser) {
    if (!own_parser) {
        return;
    }
    
    if (own_parser->own_expression) {
        AR__HEAP__FREE(own_parser->own_expression);
    }
    
    if (own_parser->own_error_message) {
        AR__HEAP__FREE(own_parser->own_error_message);
    }
    
    AR__HEAP__FREE(own_parser);
}

/**
 * Get the current position in the expression being parsed.
 */
size_t ar__expression_parser__get_position(const expression_parser_t *ref_parser) {
    if (!ref_parser) {
        return 0;
    }
    return ref_parser->position;
}

/**
 * Get the last error message from the parser.
 */
const char* ar__expression_parser__get_error(const expression_parser_t *ref_parser) {
    if (!ref_parser) {
        return NULL;
    }
    return ref_parser->own_error_message;
}

/**
 * Skip whitespace characters.
 */
static void _skip_whitespace(expression_parser_t *mut_parser) {
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
static char _current_char(const expression_parser_t *ref_parser) {
    if (!ref_parser || !ref_parser->own_expression) {
        return '\0';
    }
    return ref_parser->own_expression[ref_parser->position];
}

/**
 * Check if the current character matches expected without consuming.
 */
static bool _peek_char(const expression_parser_t *ref_parser, char expected) {
    return _current_char(ref_parser) == expected;
}

/**
 * Advance the position by one character.
 */
static void _advance(expression_parser_t *mut_parser) {
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
static bool _consume_char(expression_parser_t *mut_parser, char expected) {
    if (_peek_char(mut_parser, expected)) {
        _advance(mut_parser);
        return true;
    }
    return false;
}

/**
 * Set an error message.
 */
static void _set_error(expression_parser_t *mut_parser, const char *ref_message) {
    if (!mut_parser || !ref_message) {
        return;
    }
    
    // Free any existing error message
    if (mut_parser->own_error_message) {
        AR__HEAP__FREE(mut_parser->own_error_message);
    }
    
    // Create formatted error message with position
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "Error at position %zu: %s", 
             mut_parser->position, ref_message);
    
    mut_parser->own_error_message = AR__HEAP__STRDUP(buffer, "Parser error message");
}

/**
 * Parse a literal (integer, double, or string).
 */
expression_ast_t* ar__expression_parser__parse_literal(expression_parser_t *mut_parser) {
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
        
        expression_ast_t *own_node = ar__expression_ast__create_literal_string(own_string);
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
        
        expression_ast_t *own_node = NULL;
        
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
            
            own_node = ar__expression_ast__create_literal_double(value);
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
            
            own_node = ar__expression_ast__create_literal_int((int)value);
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
expression_ast_t* ar__expression_parser__parse_memory_access(expression_parser_t *mut_parser) {
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
        _set_error(mut_parser, "Expected memory, message, or context");
        return NULL;
    }
    
    // Advance past the base
    mut_parser->position += base_len;
    
    // Parse path components if present
    list_t *own_path_list = ar__list__create();
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
            while ((item = ar__list__remove_first(own_path_list)) != NULL) {
                AR__HEAP__FREE(item);
            }
            ar__list__destroy(own_path_list);
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
            while ((item = ar__list__remove_first(own_path_list)) != NULL) {
                AR__HEAP__FREE(item);
            }
            ar__list__destroy(own_path_list);
            _set_error(mut_parser, "Out of memory");
            return NULL;
        }
        
        strncpy(own_component, mut_parser->own_expression + start, length);
        own_component[length] = '\0';
        
        ar__list__add_last(own_path_list, own_component);
    }
    
    // Convert list to array for AST node creation
    size_t path_count = ar__list__count(own_path_list);
    const char **path_array = NULL;
    void **own_items = NULL;
    
    if (path_count > 0) {
        own_items = ar__list__items(own_path_list);
        if (!own_items) {
            // Clean up path components before destroying list
            void *item;
            while ((item = ar__list__remove_first(own_path_list)) != NULL) {
                AR__HEAP__FREE(item);
            }
            ar__list__destroy(own_path_list);
            _set_error(mut_parser, "Failed to get path items");
            return NULL;
        }
        
        // Create a const char** array from the void** array
        path_array = AR__HEAP__MALLOC(sizeof(const char*) * path_count, "Path array");
        if (!path_array) {
            AR__HEAP__FREE(own_items);
            // Clean up path components before destroying list
            void *item;
            while ((item = ar__list__remove_first(own_path_list)) != NULL) {
                AR__HEAP__FREE(item);
            }
            ar__list__destroy(own_path_list);
            _set_error(mut_parser, "Out of memory");
            return NULL;
        }
        
        for (size_t i = 0; i < path_count; i++) {
            path_array[i] = (const char *)own_items[i];
        }
    }
    
    // Create AST node
    expression_ast_t *own_node = ar__expression_ast__create_memory_access(base, path_array, path_count);
    
    // Clean up the arrays
    if (own_items) {
        AR__HEAP__FREE(own_items);
    }
    if (path_array) {
        AR__HEAP__FREE(path_array);
    }
    
    // Clean up path components and destroy list
    void *item;
    while ((item = ar__list__remove_first(own_path_list)) != NULL) {
        AR__HEAP__FREE(item);
    }
    ar__list__destroy(own_path_list);
    
    if (!own_node) {
        _set_error(mut_parser, "Failed to create memory access AST node");
    }
    
    return own_node;
}

/**
 * Parse a primary expression (literal, memory access, or parenthesized expression).
 */
static expression_ast_t* _parse_primary(expression_parser_t *mut_parser) {
    if (!mut_parser) {
        return NULL;
    }
    
    _skip_whitespace(mut_parser);
    
    // Parenthesized expression
    if (_consume_char(mut_parser, '(')) {
        expression_ast_t *own_expr = _parse_equality(mut_parser);
        if (!own_expr) {
            return NULL;
        }
        
        _skip_whitespace(mut_parser);
        if (!_consume_char(mut_parser, ')')) {
            ar__expression_ast__destroy(own_expr);
            _set_error(mut_parser, "Expected ')' after expression");
            return NULL;
        }
        
        return own_expr;
    }
    
    // Try memory access first
    expression_ast_t *own_node = ar__expression_parser__parse_memory_access(mut_parser);
    if (own_node) {
        return own_node;
    }
    
    // Clear error from failed memory access attempt
    if (mut_parser->own_error_message) {
        AR__HEAP__FREE(mut_parser->own_error_message);
        mut_parser->own_error_message = NULL;
    }
    
    // Try literal
    return ar__expression_parser__parse_literal(mut_parser);
}

/**
 * Parse a term (multiplication and division).
 */
static expression_ast_t* _parse_term(expression_parser_t *mut_parser) {
    if (!mut_parser) {
        return NULL;
    }
    
    expression_ast_t *own_left = _parse_primary(mut_parser);
    if (!own_left) {
        return NULL;
    }
    
    while (true) {
        _skip_whitespace(mut_parser);
        
        binary_operator_t op;
        if (_peek_char(mut_parser, '*')) {
            op = OP_MULTIPLY;
        } else if (_peek_char(mut_parser, '/')) {
            op = OP_DIVIDE;
        } else {
            break;
        }
        
        _advance(mut_parser); // Consume operator
        
        expression_ast_t *own_right = _parse_primary(mut_parser);
        if (!own_right) {
            ar__expression_ast__destroy(own_left);
            return NULL;
        }
        
        expression_ast_t *own_new_left = ar__expression_ast__create_binary_op(op, own_left, own_right);
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
static expression_ast_t* _parse_additive(expression_parser_t *mut_parser) {
    if (!mut_parser) {
        return NULL;
    }
    
    expression_ast_t *own_left = _parse_term(mut_parser);
    if (!own_left) {
        return NULL;
    }
    
    while (true) {
        _skip_whitespace(mut_parser);
        
        binary_operator_t op;
        if (_peek_char(mut_parser, '+')) {
            op = OP_ADD;
        } else if (_peek_char(mut_parser, '-')) {
            op = OP_SUBTRACT;
        } else {
            break;
        }
        
        _advance(mut_parser); // Consume operator
        
        expression_ast_t *own_right = _parse_term(mut_parser);
        if (!own_right) {
            ar__expression_ast__destroy(own_left);
            return NULL;
        }
        
        expression_ast_t *own_new_left = ar__expression_ast__create_binary_op(op, own_left, own_right);
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
static expression_ast_t* _parse_relational(expression_parser_t *mut_parser) {
    if (!mut_parser) {
        return NULL;
    }
    
    expression_ast_t *own_left = _parse_additive(mut_parser);
    if (!own_left) {
        return NULL;
    }
    
    while (true) {
        _skip_whitespace(mut_parser);
        
        binary_operator_t op;
        if (_peek_char(mut_parser, '<')) {
            _advance(mut_parser);
            if (_consume_char(mut_parser, '=')) {
                op = OP_LESS_EQ;
            } else if (_consume_char(mut_parser, '>')) {
                // <> for not equal
                op = OP_NOT_EQUAL;
            } else {
                op = OP_LESS;
            }
        } else if (_peek_char(mut_parser, '>')) {
            _advance(mut_parser);
            if (_consume_char(mut_parser, '=')) {
                op = OP_GREATER_EQ;
            } else {
                op = OP_GREATER;
            }
        } else {
            break;
        }
        
        expression_ast_t *own_right = _parse_additive(mut_parser);
        if (!own_right) {
            ar__expression_ast__destroy(own_left);
            return NULL;
        }
        
        expression_ast_t *own_new_left = ar__expression_ast__create_binary_op(op, own_left, own_right);
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
static expression_ast_t* _parse_equality(expression_parser_t *mut_parser) {
    if (!mut_parser) {
        return NULL;
    }
    
    expression_ast_t *own_left = _parse_relational(mut_parser);
    if (!own_left) {
        return NULL;
    }
    
    while (true) {
        _skip_whitespace(mut_parser);
        
        binary_operator_t op;
        if (_peek_char(mut_parser, '=')) {
            _advance(mut_parser);
            op = OP_EQUAL;
        } else {
            break;
        }
        
        expression_ast_t *own_right = _parse_relational(mut_parser);
        if (!own_right) {
            ar__expression_ast__destroy(own_left);
            return NULL;
        }
        
        expression_ast_t *own_new_left = ar__expression_ast__create_binary_op(op, own_left, own_right);
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
expression_ast_t* ar__expression_parser__parse_arithmetic(expression_parser_t *mut_parser) {
    // Arithmetic expressions are handled by the additive parser
    return _parse_additive(mut_parser);
}

/**
 * Parse a comparison expression.
 */
expression_ast_t* ar__expression_parser__parse_comparison(expression_parser_t *mut_parser) {
    // Comparison expressions are handled by the equality parser
    return _parse_equality(mut_parser);
}

/**
 * Parse an expression (main entry point).
 */
expression_ast_t* ar__expression_parser__parse_expression(expression_parser_t *mut_parser) {
    if (!mut_parser) {
        return NULL;
    }
    
    // Start with equality (lowest precedence)
    expression_ast_t *own_expr = _parse_equality(mut_parser);
    if (!own_expr) {
        return NULL;
    }
    
    // Check for unexpected characters after expression
    _skip_whitespace(mut_parser);
    if (_current_char(mut_parser) != '\0') {
        ar__expression_ast__destroy(own_expr);
        _set_error(mut_parser, "Unexpected characters after expression");
        return NULL;
    }
    
    return own_expr;
}
