#ifndef AGERUN_METHOD_H
#define AGERUN_METHOD_H

#include <stdbool.h>
#include <stdint.h>
#include "ar_data.h"
#include "ar_method_ast.h"
#include "ar_log.h"

/* Method Definition (opaque type) */
typedef struct method_s method_t;

/**
 * Creates a new method object with the given parameters
 * @param ref_name Method name (borrowed reference)
 * @param ref_instructions The method implementation code (borrowed reference)
 * @param ref_version Semantic version string for this method (e.g., "1.0.0")
 * @return Newly created method object, or NULL on failure
 * @note Ownership: Returns an owned object that the caller must destroy with ar_method__destroy.
 *       The method copies the name, instructions, and version. The original strings remain owned by the caller.
 */
method_t* ar_method__create(const char *ref_name, const char *ref_instructions, 
                           const char *ref_version);

/**
 * Creates a new method object with the given parameters and log support
 * @param ref_name Method name (borrowed reference)
 * @param ref_instructions The method implementation code (borrowed reference)
 * @param ref_version Semantic version string for this method (e.g., "1.0.0")
 * @param ref_log Optional log instance for error reporting (borrowed reference, may be NULL)
 * @return Newly created method object, or NULL on failure
 * @note Ownership: Returns an owned object that the caller must destroy with ar_method__destroy.
 *       The method copies the name, instructions, and version. The original strings remain owned by the caller.
 */
method_t* ar_method__create_with_log(const char *ref_name, const char *ref_instructions, 
                                    const char *ref_version, ar_log_t *ref_log);

/**
 * Get the name of a method
 * @param ref_method Method reference (borrowed reference)
 * @return Method name (borrowed reference)
 * @note Ownership: Returns a borrowed reference. The caller should not free the result.
 */
const char* ar_method__get_name(const method_t *ref_method);

/**
 * Get the version of a method
 * @param ref_method Method reference (borrowed reference)
 * @return Method version string (borrowed reference)
 * @note Ownership: Returns a borrowed reference. The caller should not free the result.
 */
const char* ar_method__get_version(const method_t *ref_method);

// Removed ar_method_is_backward_compatible and ar_method_is_persistent

/**
 * Get the instructions of a method
 * @param ref_method Method reference (borrowed reference)
 * @return Method instructions (borrowed reference)
 * @note Ownership: Returns a borrowed reference. The caller should not free the result.
 */
const char* ar_method__get_instructions(const method_t *ref_method);

/**
 * Destroys a method object and frees its resources
 * @param own_method The method to destroy (owned reference, will be freed)
 * @note Ownership: This function takes ownership of the method and frees it.
 *       The pointer will be invalid after this call.
 */
void ar_method__destroy(method_t *own_method);

/**
 * Get the parsed AST of a method
 * @param ref_method Method reference (borrowed reference)
 * @return Method AST (borrowed reference), or NULL if not parsed
 * @note Ownership: Returns a borrowed reference. The caller should not free the result.
 */
const ar_method_ast_t* ar_method__get_ast(const method_t *ref_method);

#endif /* AGERUN_METHOD_H */
