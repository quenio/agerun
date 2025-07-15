# Module Quality Checklist

## Learning
Comprehensive checklist for ensuring high-quality module development in AgeRun, covering design, implementation, and documentation aspects.

## Importance
Consistent quality standards across modules ensure maintainability, reduce bugs, and make the codebase easier to understand and extend.

## Example
```c
// Example of quality standards applied to a module:

// ✓ Functions < 50 lines (single responsibility)
ar_data_t* ar_data__list_first(const ar_data_t *ref_data) {
    if (ref_data == NULL || ar_data__get_type(ref_data) != AR_DATA_TYPE_LIST) {
        return NULL;
    }
    // Simple, focused function that does one thing
    return ar_data__list_first(ref_data);
}

// ✓ Parameters ≤ 5 (use structs for more)
typedef struct ar_create_params_s {  // EXAMPLE: Parameter object pattern
    const char *ref_name;
    ar_data_type_t type;
    int initial_capacity;
    bool thread_safe;
} ar_create_params_t;  // EXAMPLE: Hypothetical type

ar_data_t* ar_data__create_with_params(ar_create_params_t *ref_params);  // EXAMPLE: Hypothetical function

// ✓ Named constants > magic numbers
#define MEMORY_PREFIX_LEN 7  // Length of "memory."
#define MAX_METHOD_NAME_LEN 128
#define DEFAULT_LIST_CAPACITY 16

// ✓ Self-documenting code
// BAD: if (x > 7) { ... }
// GOOD: if (strlen(prefix) > MEMORY_PREFIX_LEN) { ... }
```

## Generalization
**Code Quality Checklist**:
✓ Functions < 50 lines (single responsibility)
✓ Parameters ≤ 5 (use structs for more)  
✓ No speculative generality  
✓ Self-documenting (comments = "why" not "what")  
✓ Named constants > magic numbers (e.g., MEMORY_PREFIX_LEN = 7)
✓ Incremental changes with frequent compilation
✓ Verify each change with tests
✓ Remove unused functions immediately
✓ Address all warnings immediately
✓ Think twice before adding global state  
✓ Update modules/README.md for new modules
✓ Documentation with real code: All .md files use actual AgeRun types/functions only
✓ Validate docs: Run `make check-docs` before committing any .md changes

## Implementation
1. **During development**:
   - Check function length as you write
   - Refactor when approaching limits
   - Extract constants immediately when using literals
   - Remove code rather than commenting it out

2. **Before commit**:
   - Run through entire checklist
   - Use automated tools where available:
     ```bash
     make check-all  # Includes various quality checks
     grep -n "^[a-zA-Z_].*{" module.c | awk '{print NR}' # Function starts
     ```

3. **During review**:
   - Focus on violations of checklist items
   - Suggest specific improvements
   - Check for consistency with existing modules

## Related Patterns
- [Code Smell Long Method](code-smell-long-method.md)
- [Code Smell Long Parameter List](code-smell-long-parameter-list.md)
- [Documentation Standards Integration](documentation-standards-integration.md)