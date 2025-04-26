# AgeRun Memory Management Model (MMM)

This document describes the memory management model used throughout the AgeRun project, inspired by Mojo's memory management philosophy. It provides a consistent approach to memory ownership, borrowing, and transfer across all modules.

> Inspired by the [Mojo ownership model](https://www.modular.com/blog/deep-dive-into-ownership-in-mojo)

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

2. **Borrowing without Ownership:**
   - Functions with names like `get`, `find`, `lookup` return borrowed references
   - The caller must not destroy these objects
   - Example: `ar_data_get_map_value()` returns a borrowed reference

3. **Transferring Existing Ownership:**
   - Some functions with names like `set`, `add`, `insert` may take ownership from caller
   - When ownership is transferred, caller must not use or destroy the transferred object
   - Example: `ar_data_set_map_value()` takes ownership of the value
   - Note: Not all `set`/`add`/`insert` functions transfer ownership; check module documentation
   - For example, `ar_map_set()` and `ar_list_add_last()` do NOT take ownership

4. **Taking Ownership:**
   - Functions with names containing `take` explicitly transfer ownership from another container
   - Example: `ar_expression_take_ownership()` transfers from context to caller

## Module-Specific Ownership Guidelines

### Expression Module

```c
// OWNER: Caller owns memory, context, message
// OWNER: Context owns newly created results
expression_context_t* ar_expression_create_context(data_t *memory, data_t *context, data_t *message, const char *expr);

// BORROW: Returns reference to existing data or newly created data
// OWNER: Context maintains ownership of result
data_t* ar_expression_evaluate(expression_context_t *ctx);

// TRANSFER: Transfers ownership from context to caller
bool ar_expression_take_ownership(expression_context_t *ctx, data_t *result);
```

**Key Rules:**
- Direct memory access expressions (e.g., `memory.x`) return BORROWED references
- Arithmetic expressions (e.g., `2 + 3`) return OWNED values maintained by context
- String expressions (e.g., `"Hello" + " World"`) return OWNED values maintained by context
- When using results beyond context's lifetime, always call `ar_expression_take_ownership()`

### Data Module

```c
// OWNER: Caller is new owner of returned data
data_t* ar_data_create_integer(int value);
data_t* ar_data_create_double(double value);
data_t* ar_data_create_string(const char *value);
data_t* ar_data_create_map();
data_t* ar_data_create_list();

// BORROW: Returns reference, caller does not own
int ar_data_get_integer(const data_t *data);
double ar_data_get_double(const data_t *data);
const char* ar_data_get_string(const data_t *data);

// TRANSFER: Takes ownership from caller
bool ar_data_set_map_value(data_t *map, const char *key, data_t *value);
```

**Key Rules:**
- Creation functions (`ar_data_create_*`) return OWNED values
- Getter functions return BORROWED references
- Setter functions TRANSFER ownership from caller to container
- Once ownership is transferred, caller must not destroy the object

### List Module

```c
// OWNER: Caller owns returned list
list_t* ar_list_create();

// BORROW: Caller borrows but doesn't own items
void** ar_list_items(const list_t *list);

// BORROW: Items in list remain owned by their original owners
void* ar_list_get(const list_t *list, size_t index);

// NO-TRANSFER: List doesn't take ownership of item
bool ar_list_add_last(list_t *list, void *item);
```

**Key Rules:**
- The list doesn't own the items it contains
- Adding an item to a list does NOT transfer ownership
- Removing an item from a list does NOT transfer ownership
- When a list is destroyed, items are not automatically destroyed

### Map Module

```c
// OWNER: Caller owns returned map
map_t* ar_map_create();

// NO-TRANSFER: Map does not take ownership of value
bool ar_map_set(map_t *map, const char *key, void *value);

// BORROW: Caller borrows but doesn't own returned value
void* ar_map_get(const map_t *map, const char *key);
```

**Key Rules:**
- The map does not take ownership of keys or values when they are set
- Getting a value from the map returns a BORROWED reference
- When a map is destroyed, keys and values are not destroyed
- The caller is responsible for freeing both keys and values
- Key pointers must remain valid for the lifetime of the map entry

## Common Patterns

### Factory Functions

```c
// Factory function pattern - returns owned value
data_t* create_something() {
    own_data_t *result = ar_data_create_map();
    // configure result...
    return result; // Ownership transferred to caller
}
```

### Container Management

```c
// Container takes ownership of value
own_data_t *own_value = ar_data_create_integer(42);
ar_data_set_map_value(map, "key", own_value);
// own_value is now owned by map, don't use or free it
own_value = NULL; // Mark as transferred

// Taking ownership from container
ref_data_t *ref_result = ar_expression_evaluate(ctx);
ar_expression_take_ownership(ctx, ref_result);
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

## Debugging Ownership Issues

When debugging memory issues:

1. Track object creation and destruction by adding printf statements
2. Verify that each created object is eventually destroyed exactly once
3. Check parameter documentation to understand ownership semantics
4. Use valgrind to identify leaks and double-free errors
5. When transferring ownership, set the source pointer to NULL:
   ```c
   own_data_t *own_value = ar_data_create_integer(42);
   ar_data_set_map_value(map, "key", own_value);
   own_value = NULL; // Mark as transferred
   ```
6. Use consistent prefixes in variable declarations to clearly indicate ownership:
   ```c
   // For owned values (RValues)
   own_data_t *own_data = ar_data_create_integer(42);
   
   // For mutable references (LValues)
   mut_data_t *mut_data = some_mutable_reference;
   
   // For borrowed references (BValues)
   const data_t *ref_data = ar_data_get_map_value(map, "key");
   ```

## Future Improvements

We are considering adding:
1. Debug-only ownership tracking that logs all ownership transfers
2. Helper macros to enforce ownership rules at compile time
3. Reference counting for complex ownership scenarios