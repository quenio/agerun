# Validation at Object Creation Time

## Learning
Creating an object doesn't guarantee its validity. Functions that create objects with potentially invalid internal state can mask errors and lead to false success returns. Always validate critical internal state, not just object existence.

## Importance
This pattern prevents subtle bugs where code appears to succeed but contains invalid data. It's especially critical for functions that parse or compile input - the object might exist but contain parsing errors.

## Example
```c
// BAD: Only checking object existence
ar_method_t* own_method = ar_method__create_with_log(name, instructions, version, log);
if (own_method != null) {
    ar_methodology__register_method_with_instance(methodology, own_method);
    success = true;  // BAD: Method might have invalid AST
}

// GOOD: Validating internal state
ar_method_t* own_method = ar_method__create_with_log(name, instructions, version, log);
if (own_method != null) {
    // Check if the method has a valid AST (parsing succeeded)
    const ar_method_ast_t* ref_ast = ar_method__get_ast(own_method);
    if (ref_ast != null) {
        ar_methodology__register_method_with_instance(methodology, own_method);
        success = true;
    } else {
        // Parsing failed, destroy the invalid method
        ar_method__destroy(own_method);
        success = false;
    }
}
```

## Generalization
Apply validation-at-creation pattern for any object that:
1. Contains parsed or processed data
2. Can exist in an invalid state
3. Has creation separate from validation
4. Represents compiled or interpreted content

## Implementation
```c
// Pattern for safe object creation with validation
ar_method_t* create_and_validate_method(const char* name, const char* instructions, const char* version) {
    // Create the object
    ar_method_t* own_method = ar_method__create(name, instructions, version);
    if (!own_method) return NULL;
    
    // Validate internal state by checking AST
    const ar_method_ast_t* ref_ast = ar_method__get_ast(own_method);
    if (!ref_ast) {
        ar_method__destroy(own_method);
        return NULL;
    }
    
    // Additional validation if needed
    if (ar_method_ast__get_instruction_count(ref_ast) == 0) {
        ar_method__destroy(own_method);
        return NULL;
    }
    
    return own_method;  // Only return if fully valid
}
```

## Related Patterns
- [Observable Internal State](observable-internal-state.md)
- [API Behavior Verification](api-behavior-verification.md)
- [Error Propagation Pattern](error-propagation-pattern.md)