# AgeRun Memory Management Model (MMM)

This document describes the memory management model used throughout the AgeRun project, inspired by Mojo's memory management philosophy. It provides a consistent approach to memory ownership, borrowing, and transfer across all modules.

> Inspired by the [Mojo ownership model](https://www.modular.com/blog/deep-dive-into-ownership-in-mojo)

For debugging support and ownership validation, see the [Assert Module Documentation](./modules/agerun_assert.md).

## Core Concepts

AgeRun implements a memory ownership model with three fundamental value categories:

1. **Owned Values (like Mojo RValues):**
   - Have unique ownership semantics
   - Must be explicitly destroyed by the owner
   - Ownership can be transferred but never shared
   - Created by functions that return new objects
   - Use prefix `own_` in variable declarations (e.g., `own_data`)

2. **Mutable References (like Mojo LValues):**
   - Provide read-write access to another object
   - Do not own the underlying memory
   - Must never outlive the owner
   - Marked with parameter names like `target` or documentation about mutation
   - Use prefix `mut_` in variable declarations (e.g., `mut_data`)

3. **Borrowed References (like Mojo BValues):**
   - Provide read-only access to another object
   - Do not own the underlying memory
   - Must never outlive the owner
   - Typically passed as `const` parameters
   - Use prefix `ref_` in variable declarations (e.g., `ref_data`)

## Ownership Transfer Rules

1. **Explicit Ownership Transfer:**
   - Functions with names containing `create`, `make`, or `copy` transfer ownership to the caller
   - The caller is responsible for eventually destroying these objects
   - Example: `ar_data_create_integer()` returns an owned value
   - Comment all return statements that transfer ownership with "// Ownership transferred to caller"

2. **Borrowing without Ownership:**
   - Functions with names like `get`, `find`, `lookup` return borrowed references
   - The caller must not destroy these objects
   - Example: `ar_data_get_map_value()` returns a borrowed reference
   - Always use `const` qualifier for borrowed references in function parameters

3. **Transferring Existing Ownership:**
   - Some functions with names like `set`, `add`, `insert` may take ownership from caller
   - When ownership is transferred, caller must not use or destroy the transferred object
   - Example: `ar_data_set_map_value()` takes ownership of the value
   - Note: Not all `set`/`add`/`insert` functions transfer ownership; check module documentation
   - For example, `ar_map_set()` and `ar_list_add_last()` do NOT take ownership
   - Set transferred pointers to NULL after ownership transfer (e.g., `own_value = NULL;`)
   - Document with a comment like "Don't use after this point" after transfer

4. **Taking Ownership:**
   - Functions with names containing `take` explicitly transfer ownership from another container
   - Example: `ar_expression_take_ownership()` transfers from context to caller

5. **Parameter Assignment Rules:**
   - When passing an owned value to a function that takes ownership:
     - Parameter appears as `own_value` in the function signature
     - Function is responsible for destroying or transferring ownership
     - Function documentation must indicate ownership transfer
   - For non-transferring calls:
     - Use `ref_` prefix for read-only access (always with `const` qualifier)
     - Use `mut_` prefix for mutable access

6. **Ownership Validation with Assertions:**
   - Debug-only validation is provided through the assert module
   - For complex resource management, use `AR_ASSERT_OWNERSHIP()` to validate critical allocations
   - For non-obvious transfers, use `AR_ASSERT_TRANSFERRED()` after setting transferred pointers to NULL
   - For preventing use-after-free, use `AR_ASSERT_NOT_USED_AFTER_FREE()` before potential reuse points
   - See the [Assert Module Documentation](./modules/agerun_assert.md) for detailed guidelines
   - Assertions should be used strategically rather than excessively

## Module-Specific Ownership Guidelines

### Expression Module

```c
// OWNER: Caller owns memory, context, message
// OWNER: Context owns newly created results
expression_context_t* ar_expression_create_context(data_t *mut_memory, const data_t *ref_context, const data_t *ref_message, const char *ref_expr);

// BORROW: Returns reference to existing data or newly created data
// OWNER: Context maintains ownership of result
data_t* ar_expression_evaluate(expression_context_t *mut_ctx);

// TRANSFER: Transfers ownership from context to caller
bool ar_expression_take_ownership(expression_context_t *mut_ctx, data_t *ref_result);
```

**Key Rules:**
- Direct memory access expressions (e.g., `memory.x`) return BORROWED references
- Arithmetic expressions (e.g., `2 + 3`) return OWNED values maintained by context
- Arithmetic expressions with memory access (e.g., `memory.x + 5`) return OWNED values maintained by context
- String expressions (e.g., `"Hello" + " World"`) return OWNED values maintained by context
- String+number concatenation (e.g., `"Price: $" + 42.99`) returns OWNED values maintained by context
- When using results beyond context's lifetime, always call `ar_expression_take_ownership()`

### Data Module

```c
// OWNER: Caller is new owner of returned data
data_t* ar_data_create_integer(int value);
data_t* ar_data_create_double(double value);
data_t* ar_data_create_string(const char *ref_value);
data_t* ar_data_create_map();
data_t* ar_data_create_list();

// BORROW: Returns reference, caller does not own
int ar_data_get_integer(const data_t *ref_data);
double ar_data_get_double(const data_t *ref_data);
const char* ar_data_get_string(const data_t *ref_data);

// TRANSFER: Takes ownership from caller
bool ar_data_set_map_value(data_t *mut_map, const char *ref_key, data_t *own_value);
```

**Key Rules:**
- Creation functions (`ar_data_create_*`) return OWNED values
- Getter functions return BORROWED references
- Setter functions TRANSFER ownership from caller to container
- Once ownership is transferred, caller must not destroy the object
- After transfer, caller should set pointer to NULL: `own_value = NULL;`

### List Module

```c
// OWNER: Caller owns returned list
list_t* ar_list_create();

// BORROW: Caller borrows but doesn't own items
void** ar_list_items(const list_t *ref_list);

// BORROW: Items in list remain owned by their original owners
void* ar_list_get(const list_t *ref_list, size_t index);

// NO-TRANSFER: List doesn't take ownership of item
bool ar_list_add_last(list_t *mut_list, void *ref_item);
```

**Key Rules:**
- The list doesn't own the items it contains
- Adding an item to a list does NOT transfer ownership
- Removing an item from a list does NOT transfer ownership
- When a list is destroyed, items are not automatically destroyed
- Caller maintains responsibility for destroying list items

### Map Module

```c
// OWNER: Caller owns returned map
map_t* ar_map_create();

// NO-TRANSFER: Map does not take ownership of value
bool ar_map_set(map_t *mut_map, const char *ref_key, void *ref_value);

// BORROW: Caller borrows but doesn't own returned value
void* ar_map_get(const map_t *ref_map, const char *ref_key);
```

**Key Rules:**
- The map does not take ownership of keys or values when they are set
- Getting a value from the map returns a BORROWED reference
- When a map is destroyed, keys and values are not destroyed
- The caller is responsible for freeing both keys and values
- Key pointers must remain valid for the lifetime of the map entry
- Prefix naming conventions make ownership responsibilities clear

## Common Patterns

### Factory Functions

```c
// Factory function pattern - returns owned value
data_t* create_something() {
    own_data_t *own_result = ar_data_create_map();
    // configure result...
    return own_result; // Ownership transferred to caller
    // Comment: Ownership transferred to caller
}
```

### Container Management

```c
// Container takes ownership of value
own_data_t *own_value = ar_data_create_integer(42);
ar_data_set_map_value(mut_map, "key", own_value);
// own_value is now owned by map, don't use or free it
own_value = NULL; // Mark as transferred
// Don't use own_value after this point

// Taking ownership from container
ref_data_t *ref_result = ar_expression_evaluate(mut_ctx);
ar_expression_take_ownership(mut_ctx, ref_result);
// ref_result becomes own_result as it is now owned by caller, not context
own_data_t *own_result = ref_result;
```

### Borrowing Pattern

```c
// Borrowing pattern - doesn't take ownership
void process_data(const data_t *ref_data) {
    // Use but don't modify or destroy ref_data
    int value = ar_data_get_integer(ref_data);
    // ...
}
```

### Expression Evaluation Rules

```c
// Direct memory access (returns reference)
ref_data_t *ref_value = ar_expression_evaluate(mut_ctx); // expression: "memory.x"
// No need to destroy ref_value (borrowed reference)

// Arithmetic expressions (returns new owned object)
own_data_t *own_result = ar_expression_evaluate(mut_ctx); // expression: "2 + 3"
ar_expression_take_ownership(mut_ctx, own_result); // Take ownership
// Must destroy own_result when done

// Arithmetic with memory access (returns new owned object)
own_data_t *own_calc = ar_expression_evaluate(mut_ctx); // expression: "memory.x + 5"
ar_expression_take_ownership(mut_ctx, own_calc); // Take ownership
// Must destroy own_calc when done

// String expressions (returns new owned object)
own_data_t *own_str = ar_expression_evaluate(mut_ctx); // expression: "\"Hello\" + \" World\""
ar_expression_take_ownership(mut_ctx, own_str); // Take ownership
// Must destroy own_str when done
```

## Parameter Assignment Rules

The AgeRun project follows specific rules for parameter assignment to ensure memory safety:

1. **For RValues (Owned Values):**
   - When passed as owned parameters, they are owned by the callee
   - Ownership transfer occurs for parameters with `own_` prefix
   - After passing an owned value to a function that takes ownership, the caller must not use or destroy it
   - The original pointer should be set to NULL to indicate ownership transfer

2. **For LValues (Mutable References):**
   - Mutable references provide read-write access
   - Use `mut_` prefix to indicate this parameter may be modified
   - Function can modify the referenced object
   - No ownership transfer occurs

3. **For BValues (Borrowed References):**
   - Provide read-only access to existing objects
   - Always use `const` qualifier with these parameters
   - Use `ref_` prefix to indicate borrowed, immutable status
   - Function cannot modify or destroy the referenced object
   - No ownership transfer occurs

4. **Copy and Move Semantics:**
   - When `ar_data_copy` is used, a new owned object is created
   - For larger data structures, prefer creating handles and transferring ownership rather than copying
   - "Move" semantics are implemented via explicit ownership transfer

## Assignment and Access Restrictions

AgeRun enforces the following restrictions to prevent memory errors:

1. **Ownership Transfer Restrictions:**
   - After transferring ownership of a pointer to a function or container, the original pointer MUST NOT be used
   - Code review should verify that transferred pointers are set to NULL immediately after transfer
   - Example:
   ```c
   own_data_t *own_value = ar_data_create_integer(42);
   ar_data_set_map_value(mut_map, "key", own_value); // ownership transferred
   own_value = NULL; // mark as transferred
   // Using own_value after this point is a programming error
   ```

2. **Access Prohibition After Transfer:**
   - Code that attempts to use a pointer after ownership transfer is considered invalid
   - All transferred ownership variables must be immediately set to NULL
   - Functions that take ownership should document this clearly
   - Example of INCORRECT usage:
   ```c
   own_data_t *own_value = ar_data_create_integer(42);
   ar_data_set_map_value(mut_map, "key", own_value); // ownership transferred
   int value = ar_data_get_integer(own_value); // ERROR: Using after transfer
   ```

3. **Multiple Ownership Prevention:**
   - A single owned object cannot have multiple owners
   - When passing an owned object to another owner, the original owner must relinquish ownership
   - Test code should verify proper ownership transfer by checking that:
     - The value is correctly stored in the new owner
     - The original pointer is set to NULL
     - No memory leaks occur (using valgrind)

4. **Immutable Reference Restrictions:**
   - Functions accepting `const` references (`ref_` prefix) must not modify the referenced object
   - A `const` qualified parameter guarantees the object will not be modified
   - Casting away `const` to modify such objects violates the ownership model
   
5. **Temporary Value Restrictions:**
   - Results of expression evaluations must have their ownership properly managed
   - Temporary values created by expressions must be explicitly owned and destroyed
   - Example:
   ```c
   // CORRECT: Take ownership of expression result
   own_data_t *own_result = ar_expression_evaluate(mut_ctx);
   ar_expression_take_ownership(mut_ctx, own_result);
   // use own_result...
   ar_data_destroy(own_result);

   // INCORRECT: Not taking ownership of temporary
   ref_data_t *ref_result = ar_expression_evaluate(mut_ctx);
   // use ref_result...
   // Memory leak: Expression result not properly handled
   ```

## Consistent Use of Conventions

For clarity and consistency, all ownership prefixes should be used throughout the codebase:

1. **Header Files (.h)**:
   - Function parameters should use ownership prefixes:
   ```c
   bool ar_module_function(module_t *own_object, const data_t *ref_data);
   ```

2. **Implementation Files (.c)**:
   - Local variables should use ownership prefixes:
   ```c
   bool ar_module_function(module_t *own_object, const data_t *ref_data) {
       own_data_t *own_result = ar_data_create_map();
       ref_data_t *ref_value = ar_data_get_map_value(ref_data, "key");
       // ...
   }
   ```

3. **Test Files (_tests.c)**:
   - Test code should maintain the same conventions:
   ```c
   void test_module_function() {
       own_module_t *own_object = ar_module_create();
       own_data_t *own_map = ar_data_create_map();
       // ...
   }
   ```

4. **Documentation Comments**:
   - Documentation must clearly express ownership semantics:
   ```c
   /**
    * Inserts a value into a map with the specified key
    * @param mut_map The map to modify (mutable reference)
    * @param ref_key The key to use (borrowed reference)
    * @param own_value The value to insert (ownership transferred to map)
    * @return True if the insertion succeeded, false otherwise
    * @note Ownership: Takes ownership of the value parameter.
    *       The map will be responsible for destroying own_value.
    *       Caller should set own_value = NULL after this call.
    */
   bool ar_map_insert(map_t *mut_map, const char *ref_key, data_t *own_value);
   ```

5. **Struct Field Names:**
   - Struct field names should follow the same ownership prefix conventions as local variables and parameters:
   ```c
   typedef struct example_s {
       int *own_resource;       // Struct owns this resource and must free it
       data_t *mut_data;        // Field can be modified but struct doesn't own it
       const char *ref_name;    // Read-only reference to data owned elsewhere
   } example_t;
   ```
   - This makes ownership relationships clear at the struct definition level
   - Apply this convention to all struct fields consistently throughout the codebase
   - Use the same naming rules for both opaque and non-opaque types

This consistency makes ownership semantics explicit throughout the entire codebase, reducing the risk of memory management errors and making code reviews more effective.

## Static Analysis Guidelines

While C doesn't offer the same compile-time guarantees as Mojo, we can enforce ownership rules through careful coding, review, and static analysis:

1. **Use Static Analyzers:**
   - Run static analysis tools regularly to detect potential ownership violations
   - Configure analyzers to flag suspicious pointer usage after transfers
   - Check for double-free and use-after-free patterns

2. **Code Review Checklist:**
   - Verify every ownership transfer is followed by setting the source pointer to NULL
   - Check that all owned values are correctly destroyed exactly once
   - Review that functions with ownership transfer are properly documented
   - Confirm proper ownership prefix usage on all variable declarations
   - Ensure consistent const-qualification for borrowed references

3. **Ownership Annotation Validation:**
   - Verify function implementations match their ownership documentation
   - Check that functions documented to take ownership properly handle cleanup
   - Confirm that functions taking borrowed references don't destroy the objects

4. **Test Ownership Semantics:**
   - Write tests specifically targeting ownership semantics
   - Verify memory usage with valgrind or similar tools
   - Test boundary cases like error conditions to ensure proper cleanup

## Debugging Ownership Issues

When debugging memory issues:

1. Track object creation and destruction by adding printf statements
2. Verify that each created object is eventually destroyed exactly once
3. Check parameter documentation to understand ownership semantics
4. Use valgrind to identify leaks and double-free errors
5. When transferring ownership, set the source pointer to NULL:
   ```c
   own_data_t *own_value = ar_data_create_integer(42);
   ar_data_set_map_value(mut_map, "key", own_value);
   own_value = NULL; // Mark as transferred
   ```
6. When a function uses another function that might take ownership, always ensure proper cleanup:
   ```c
   // INCORRECT: Memory leak if ar_data_set_map_data returns false
   own_data_t *own_data = ar_data_create_string("value");
   bool result = ar_data_set_map_data(mut_map, "key", own_data);
   // If result is false, own_data might be leaked

   // CORRECT: Handle cleanup properly
   own_data_t *own_data = ar_data_create_string("value");
   bool result = ar_data_set_map_data(mut_map, "key", own_data);
   if (!result) {
       ar_data_destroy(own_data); // Clean up if ownership transfer failed
   }
   // or even better, modify ar_data_set_map_data to always take ownership
   ```
7. Use consistent prefixes in variable declarations to clearly indicate ownership:
   ```c
   // For owned values (RValues)
   own_data_t *own_data = ar_data_create_integer(42);
   
   // For mutable references (LValues)
   mut_data_t *mut_data = some_mutable_reference;
   
   // For borrowed references (BValues)
   const data_t *ref_data = ar_data_get_map_value(map, "key");
   ```
7. Use assertion macros from the assert module to verify ownership invariants:
   ```c
   own_data_t *own_value = ar_data_create_integer(42);
   AR_ASSERT_OWNERSHIP(own_value); // Verify ownership after creation
   
   ar_data_set_map_value(mut_map, "key", own_value);
   own_value = NULL;
   AR_ASSERT_TRANSFERRED(own_value); // Verify proper transfer marker
   ```

## Development Guidelines for Ownership

All developers working on the AgeRun codebase MUST adhere to the following guidelines:

1. **Zero Tolerance for Ownership Violations:**
   - Any code that violates the ownership rules MUST NOT be committed
   - Ownership violations include:
     - Using pointers after ownership transfer
     - Double freeing memory
     - Memory leaks
     - Missing NULL assignments after ownership transfer
     - Modifying objects through borrowed references
     - Missing ownership documentation

2. **Mandatory Code Review for Ownership:**
   - All pull requests MUST be reviewed with specific attention to ownership
   - Code reviewers MUST verify all ownership transfer points
   - Review checklist MUST include:
     - Proper variable naming with ownership prefixes
     - Setting pointers to NULL after transfer
     - Complete ownership documentation
     - Proper cleanup in all code paths (including error handling)

3. **Testing Requirements:**
   - All modules MUST include tests that verify ownership semantics
   - Memory leak detection tools MUST be run on all tests
   - Tests MUST include error cases to verify proper cleanup

4. **Documentation Requirements:**
   - All functions MUST document their ownership semantics
   - Functions that take ownership MUST explicitly state this
   - Functions that return owned values MUST document the caller's responsibility
   - All ownership transfer points in code MUST be commented

5. **Code Enforcement:**
   - Regular static analysis runs are MANDATORY
   - Valgrind or similar memory checker runs are REQUIRED before release
   - Developers MUST use the assertion macros from the assert module to verify ownership invariants:
     - `AR_ASSERT_OWNERSHIP()` - Verify pointers are non-NULL after creation
     - `AR_ASSERT_TRANSFERRED()` - Verify pointers are NULL after ownership transfer
     - `AR_ASSERT_NOT_USED_AFTER_FREE()` - Verify freed pointers are not accessed
     - These assertions are automatically disabled in release builds for zero overhead

Violating these guidelines is considered a serious issue. Code that doesn't follow these rules must be fixed immediately, as ownership violations lead to memory corruption, crashes, and security vulnerabilities.

## Future Improvements

We are considering adding:
1. Debug-only ownership tracking that logs all ownership transfers
2. Reference counting for complex ownership scenarios where needed
3. Sanitizer options in debug builds to detect ownership violations
4. Custom static analysis rules to verify ownership transfer patterns
5. Automated test generation for ownership boundary cases