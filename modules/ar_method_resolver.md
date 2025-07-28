# ar_method_resolver Module

The `ar_method_resolver` module handles method version resolution logic, separating the concern of finding the appropriate method version from the storage mechanism provided by `ar_method_registry`.

## Purpose

This module implements the version resolution strategy for methods:
- NULL or empty version → latest version
- Full semver (e.g., "1.2.3") → exact match
- Partial version (e.g., "1" or "1.2") → latest matching version

## Key Functions

### Creation and Destruction

```c
ar_method_resolver_t* ar_method_resolver__create(ar_method_registry_t *ref_registry);
```
Creates a new method resolver instance.
- **Parameters**: `ref_registry` - Borrowed reference to the method registry
- **Returns**: New resolver instance or NULL on failure
- **Ownership**: Caller owns the returned resolver

```c
void ar_method_resolver__destroy(ar_method_resolver_t *own_resolver);
```
Destroys a method resolver instance.
- **Parameters**: `own_resolver` - Takes ownership and destroys

### Method Resolution

```c
ar_method_t* ar_method_resolver__resolve_method(ar_method_resolver_t *ref_resolver,
                                                const char *ref_name,
                                                const char *ref_version);
```
Resolves a method by name and version according to the resolution strategy.
- **Parameters**:
  - `ref_resolver` - Borrowed reference to the resolver
  - `ref_name` - Method name (borrowed reference)
  - `ref_version` - Version string or NULL/empty for latest (borrowed reference)
- **Returns**: Borrowed reference to the resolved method or NULL if not found
- **Resolution Strategy**:
  - NULL or empty version → returns latest version
  - Full semver (e.g., "1.2.3") → returns exact match only
  - Partial version (e.g., "1" or "1.2") → returns latest matching version

## Internal Implementation

The resolver uses:
- `ar_method_registry__get_latest_version()` for NULL/empty versions
- `ar_method_registry__get_method_by_exact_match()` for full semver versions
- `ar_method_registry__get_all_methods()` and `ar_semver__find_latest_matching()` for partial versions

## Example Usage

```c
// Create resolver with a registry
ar_method_resolver_t *own_resolver = ar_method_resolver__create(ref_registry);

// Get latest version
ar_method_t *ref_method = ar_method_resolver__resolve_method(own_resolver, "my_method", NULL);

// Get exact version
ref_method = ar_method_resolver__resolve_method(own_resolver, "my_method", "1.2.3");

// Get latest 1.x version
ref_method = ar_method_resolver__resolve_method(own_resolver, "my_method", "1");

// Clean up
ar_method_resolver__destroy(own_resolver);
```

## Design Rationale

This module separates version resolution logic from storage concerns, following the single responsibility principle. The methodology module delegates all version resolution to this module, maintaining clean separation of concerns.