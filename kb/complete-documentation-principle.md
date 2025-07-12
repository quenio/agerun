# Complete Documentation Principle

## Overview

The Complete Documentation principle requires that every module be fully documented with clear explanations of purpose, interfaces, behavior, and usage examples. Documentation must be comprehensive, accurate, and maintainable.

## Core Concept

**Definition**: Every public interface element must be documented with purpose, parameters, return values, ownership semantics, and usage examples. Documentation must be kept current with code changes.

**Purpose**: Enables effective use of modules, reduces learning curve, prevents misuse, and serves as a contract between module provider and clients.

## AgeRun Documentation Standards

### Header Documentation

**Function Documentation**:
```c
/**
 * Creates a new string data object from the given text.
 * 
 * @param text The source text to copy (must not be NULL)
 * @return New string data object, or NULL if allocation fails
 * @note Caller owns the returned object and must call ar_data__destroy()
 * @note The input text is copied, so the original can be safely modified
 */
ar_data_t* ar_data__create_string(const char* text);

/**
 * Retrieves the string value from a string data object.
 * 
 * @param data The data object (must be non-NULL and type STRING)
 * @return Borrowed reference to internal string, valid until data is destroyed
 * @note The returned string must not be modified or freed by caller
 * @note Returns NULL if data is NULL or not a string type
 */
const char* ar_data__get_string(ar_data_t* data);
```

**Type Documentation**:
```c
/**
 * Opaque type representing a data value of various types.
 * 
 * Supports strings, integers, lists, and maps with unified interface.
 * All instances must be created through ar_data__create_* functions
 * and destroyed with ar_data__destroy().
 */
typedef struct ar_data_s ar_data_t;

/**
 * Enumeration of supported data types.
 * 
 * Used to identify the type of data stored in ar_data_t objects.
 * Type checking should be done before attempting type-specific operations.
 */
typedef enum {
    AR_DATA_TYPE_STRING,   /**< Text string value */
    AR_DATA_TYPE_INTEGER,  /**< 64-bit signed integer */
    AR_DATA_TYPE_LIST,     /**< Ordered collection of data objects */
    AR_DATA_TYPE_MAP       /**< Key-value mapping with string keys */
} ar_data_type_t;
```

### Module Documentation Files

**Required .md File for Each Module**:
```markdown
# ar_data Module

## Purpose
Provides unified interface for handling different data types (strings, integers, lists, maps) in the AgeRun system.

## Key Concepts
- **Unified Type System**: All data values use same ar_data_t interface
- **Ownership Model**: Callers own returned objects, must destroy them
- **Type Safety**: Runtime type checking prevents invalid operations
- **Memory Management**: Automatic cleanup of nested structures

## Usage Examples

### Creating and Using String Data
```c
ar_data_t* text = ar_data__create_string("Hello, World!");
const char* value = ar_data__get_string(text);
printf("Text: %s\n", value);
ar_data__destroy(text);
```

### Working with Lists
```c
ar_data_t* list = ar_data__create_list();
ar_data_t* item = ar_data__create_string("item1");
ar_data__list_add(list, item);
size_t count = ar_data__list_get_count(list);
ar_data__destroy(list);  // Also destroys contained items
```

## Ownership Rules
- **create_* functions**: Caller owns returned object
- **get_* functions**: Return borrowed references
- **list_add/map_set**: Transfer ownership of added items to container
- **destroy**: Recursively destroys all owned objects

## Error Handling
- Functions return NULL for allocation failures
- Type mismatches return NULL/false
- Invalid parameters are handled gracefully
- Use ar_data__get_type() to verify type before operations

## Dependencies
- ar_heap: Memory allocation and tracking
- ar_list: Dynamic list implementation  
- ar_map: Hash map implementation

## Related Modules
- ar_expression: Uses ar_data for expression evaluation
- ar_instruction: Uses ar_data for instruction parameters
- ar_agent: Uses ar_data for agent memory and messages
```

### Ownership Documentation

**Critical Ownership Information**:
```c
/**
 * Removes and returns the first item from the list.
 * 
 * @param list The list to modify (must be non-NULL and type LIST)
 * @return The removed item, or NULL if list is empty
 * @note Ownership of returned item transfers to caller
 * @note Caller must call ar_data__destroy() on returned item
 * @note List size is reduced by one
 */
ar_data_t* ar_data__list_remove_first(ar_data_t* list);

/**
 * Gets the item at the specified index without removing it.
 * 
 * @param list The list to query (must be non-NULL and type LIST)
 * @param index Zero-based index (must be < list size)
 * @return Borrowed reference to item, or NULL if index invalid
 * @note Returned reference is valid until list is modified or destroyed
 * @note Caller must NOT destroy the returned item
 */
ar_data_t* ar_data__list_get_at(ar_data_t* list, size_t index);
```

## Documentation Maintenance

### Keeping Documentation Current

**Pre-Commit Verification**:
```bash
# Check for undocumented functions
make check-docs

# Verify documentation references
grep -r "ar_data__old_function" modules/*.md  # Should return nothing after refactoring
```

**Interface Change Protocol**:
1. Update function documentation first
2. Update module .md file examples
3. Update related module documentation
4. Verify all cross-references are correct
5. Test documentation examples

### Common Documentation Errors

**Incorrect Ownership Information**:
```c
// BAD: Wrong ownership claim
/**
 * @return String value that caller must free
 */
const char* ar_data__get_string(ar_data_t* data);  // Actually returns borrowed reference!

// GOOD: Correct ownership documentation  
/**
 * @return Borrowed reference to string, valid until data is destroyed
 * @note Caller must NOT free the returned string
 */
const char* ar_data__get_string(ar_data_t* data);
```

**Missing Error Conditions**:
```c
// BAD: Incomplete error documentation
/**
 * @return The data item at index
 */
ar_data_t* ar_data__list_get_at(ar_data_t* list, size_t index);

// GOOD: Complete error documentation
/**
 * @return The data item at index, or NULL if index >= size or list is NULL
 * @note Returns NULL for invalid parameters without error messages
 */
ar_data_t* ar_data__list_get_at(ar_data_t* list, size_t index);
```

**Outdated Examples**:
```c
// BAD: Example uses old API
/**
 * Example:
 * ar_data_t* data = ar_data__create_string_with_length("test", 4);  // OLD API
 */

// GOOD: Current API usage
/**
 * Example:
 * ar_data_t* data = ar_data__create_string("test");  // Current API
 */
```

## Documentation Verification

### Automated Checks

**Document Validation Script**:
```bash
# scripts/check_docs.sh verifies:
# - All public functions have documentation
# - All parameters documented with @param
# - All return values documented with @return
# - Ownership notes present for pointer returns
# - Examples compile and run correctly
```

### Manual Review Checklist

- [ ] Every public function has complete documentation
- [ ] Parameter types and constraints clearly specified
- [ ] Return value meaning and ownership documented
- [ ] Error conditions and edge cases covered
- [ ] Usage examples provided and tested
- [ ] Ownership semantics clearly explained
- [ ] Dependencies and relationships documented
- [ ] Module .md file exists and is current

## Benefits

### Reduced Learning Curve
- New developers understand interfaces quickly
- Clear examples accelerate adoption
- Ownership rules prevent memory leaks

### Fewer Integration Errors
- Parameter constraints prevent invalid usage
- Return value documentation prevents misuse
- Error handling guidance improves robustness

### Easier Maintenance
- Interface contracts are explicit
- Changes require documentation updates
- Cross-references help track dependencies

## Integration with Build System

### Documentation Validation

**Makefile Integration**:
```makefile
check-docs:
    @echo "Validating module documentation..."
    @scripts/check_docs.sh
    
build: check-docs run-tests
    @echo "Build complete with documentation validation"
```

**CI/CD Integration**:
- Documentation validation runs on every commit
- Examples are compiled and tested automatically
- Cross-reference checking prevents broken links

## Related Principles

- **Information Hiding**: Documents what is hidden and why ([details](information-hiding-principle.md))
- **Minimal Interfaces**: Reduces documentation burden ([details](minimal-interfaces-principle.md))
- **Single Responsibility**: Clarifies module purpose ([details](single-responsibility-principle.md))

## Examples

**Good Complete Documentation**:
```c
/**
 * Parses a semantic version string into structured representation.
 * 
 * Supports standard semantic versioning format: MAJOR.MINOR.PATCH
 * with optional pre-release and build metadata.
 * 
 * @param version_string The version string to parse (e.g., "1.2.3-alpha+build")
 * @return New semver object, or NULL if parsing fails or allocation fails
 * @note Caller owns returned object and must call ar_semver__destroy()
 * @note Input string is not modified and can be freed after this call
 * @note Returns NULL for invalid format (missing components, non-numeric parts)
 * 
 * Example:
 * ar_semver_t* ver = ar_semver__parse("2.1.0");
 * if (ver) {
 *     // Use version object
 *     ar_semver__destroy(ver);
 * }
 */
ar_semver_t* ar_semver__parse(const char* version_string);
```

**Poor Incomplete Documentation**:
```c
// BAD: Minimal, unclear documentation
/**
 * Parse version.
 */
ar_semver_t* ar_semver__parse(const char* version_string);
```