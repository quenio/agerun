# Observable Internal State Pattern

## Learning
Objects need ways to query their validity beyond mere existence. Provide APIs that allow external code to observe critical internal state for validation purposes. Existence checks alone are insufficient when objects can be in invalid states.

## Importance
Without observable internal state, errors can be masked by successful object creation. This pattern enables proper validation and prevents propagation of invalid objects through the system.

## Example
```c
// Method object exists but might have invalid AST
ar_method_t* method = ar_method__create_with_log(name, instructions, version, log);

// Observable internal state API allows validation
const ar_method_ast_t* ast = ar_method__get_ast(method);
if (!ast) {
    // Method parsing failed - internal state is invalid
    ar_method__destroy(method);
    return false;
}

// Another example with agent validation through agency
const ar_data_t* agent_memory = ar_agency__get_agent_memory(agency, agent_id);
if (!agent_memory) {
    return false;  // Agent doesn't exist or has no memory
}

// Check if agent has required method
const ar_method_t* agent_method = ar_agency__get_agent_method(agency, agent_id);
if (!agent_method) {
    // Agent has no method assigned
    return false;
}
```

## Generalization
Design objects with queryable state by:
1. Providing getter functions for critical internal components
2. Exposing state enum values (valid/invalid/error)
3. Offering validation methods that check invariants
4. Making internal consistency checkable from outside

## Implementation
```c
// Real AgeRun APIs for observable state
// Agents have observable components
const ar_data_t* ar_agent__get_memory(const ar_agent_t* agent);
const ar_data_t* ar_agent__get_context(const ar_agent_t* agent);
int64_t ar_agent__get_id(const ar_agent_t* agent);

// Methods have observable AST
const ar_method_ast_t* ar_method__get_ast(const ar_method_t* method);
const char* ar_method__get_name(const ar_method_t* method);
const char* ar_method__get_version(const ar_method_t* method);

// Data objects have observable type
ar_data_type_t ar_data__get_type(const ar_data_t* data);
int64_t ar_data__get_integer(const ar_data_t* data);
const char* ar_data__get_string(const ar_data_t* data);

// Usage for validation
bool validate_method_compilation(ar_method_t* method) {
    if (!method) return false;
    
    // Check if parsing succeeded by examining AST
    const ar_method_ast_t* ast = ar_method__get_ast(method);
    if (!ast) return false;
    
    // Further validation possible
    if (ar_method_ast__get_instruction_count(ast) == 0) return false;
    
    return true;
}
```

## Related Patterns
- [Validation at Creation Time](validation-at-creation-time.md)
- [Opaque Types Principle](opaque-types-principle.md)
- [Minimal Interfaces Principle](minimal-interfaces-principle.md)