# Module Development Patterns

## Learning

Module development in AgeRun follows specific architectural patterns that separate concerns, manage dependencies, and ensure maintainability.

## Importance

Following these patterns prevents circular dependencies, reduces coupling, and creates modules that are easy to test, understand, and modify.

## Example

```c
// Parser responsibility - data owner parses
// In ar_methodology.c
ar_method_t* ar_methodology__parse_method(const char* ref_source) {  // EXAMPLE: Hypothetical function showing pattern
    // Methodology owns methods, so it parses them
    ar_method_t* own_method = _create_method();
    _parse_method_content(own_method, ref_source);
    return own_method;  // Ownership transferred to caller
}

// Consumer evaluates - interpreter uses ASTs
// In ar_interpreter.c  
ar_data_t* ar_interpreter__evaluate_method(  // EXAMPLE: Hypothetical function showing pattern
    ar_interpreter_t* mut_interpreter,
    ar_method_t* ref_method,  // Borrowed reference
    ar_data_t* ref_params) {
    // Interpreter evaluates but doesn't own methods
    return _evaluate_ast(ref_method->ast, ref_params);
}

// Module size management - split at ~850 lines
// Original: ar_agency.c (1200 lines)
// Split into:
// - ar_agency.c (300 lines) - Public API
// - ar_agent_registry.c (400 lines) - Agent storage
// - ar_agent_resolver.c (300 lines) - Agent lookup
// - ar_agent_lifecycle.c (200 lines) - Creation/destruction
```

## Generalization

Core patterns:
- **Parse once, evaluate many**: Store ASTs not source text
- **Data owner parses**: Module that owns data type does parsing
- **Consumer evaluates**: Module using data does evaluation  
- **Dynamic collections**: Use ar_list_t/ar_map_t not fixed arrays
- **String IDs**: More reliable than numeric IDs for persistence
- **Read first**: Check interface before guessing function names
- **No platform code**: No `#ifdef __linux__` etc.
- **Delegate errors**: Pass ar_log_t through layers

Module size guidelines:
- Split at ~850 lines into focused modules
- Each module has single responsibility
- Internal modules (registries) vs external (stores)
- Support function name variants in parsers

## Implementation

```c
// Creating a new module
// 1. Define opaque type in header
typedef struct ar_module_s ar_module_t;  // EXAMPLE: Hypothetical module type

// 2. Hide implementation in .c file
struct ar_module_s {
    ar_dependency_t* ref_dependency;  // EXAMPLE: Hypothetical dependency type
    ar_list_t* own_items;            // Real type - owned collection
    char* own_name;                  // Owned string
};

// 3. Provide clean interface
ar_module_t* ar_module__create(ar_dependency_t* ref_dep);  // EXAMPLE: Hypothetical function
void ar_module__destroy(ar_module_t* own_module);  // EXAMPLE: Hypothetical function
const char* ar_module__get_name(ar_module_t* ref_module);  // EXAMPLE: Hypothetical function

// 4. Make instantiable (bottom-up by dependencies)
typedef struct {
    ar_io_t* ref_io;              // EXAMPLE: Hypothetical IO type
    ar_allocator_t* ref_allocator; // EXAMPLE: Hypothetical allocator type
} ar_module_config_t;  // EXAMPLE: Hypothetical config type

ar_module_t* ar_module__create_with_config(  // EXAMPLE: Hypothetical function
    const ar_module_config_t* ref_config);  // EXAMPLE: Hypothetical parameter type
```

## Related Patterns
- [Opaque Types Pattern](opaque-types-pattern.md)
- [Internal vs External Module Pattern](internal-vs-external-module-pattern.md)
- [Parser Function Alias Support](parser-function-alias-support.md)
- [Module Delegation Error Propagation](module-delegation-error-propagation.md)
- [Module Quality Checklist](module-quality-checklist.md)
- [Module Instantiation Prerequisites](module-instantiation-prerequisites.md)