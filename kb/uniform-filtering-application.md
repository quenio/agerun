# Uniform Filtering Application

## Learning
Filtering rules must be applied consistently across all code paths to avoid confusion. When implementing filtering like whitelists, audit every location that processes the filtered data and ensure they all use the same filtering logic.

## Importance
Partial implementation of filtering creates unexpected behavior where the same data appears filtered in some views but not others. This inconsistency confuses users and makes the system appear broken even when working correctly.

## Example
```c
// Problem: Multiple analysis functions, inconsistent filtering
typedef struct {
    ar_list_t* whitelist;
    ar_list_t* errors;
} analysis_context_t;  // EXAMPLE: Context structure for demonstration

// BAD: Main check applies whitelist
void check_errors(analysis_context_t* ctx) {  // EXAMPLE: Function using example type
    ar_list_t* ref_errors = ctx->errors;
    ar_list_t* ref_whitelist = ctx->whitelist;
    
    void** items = ar_list__items(ref_errors);
    size_t count = ar_list__count(ref_errors);
    
    for (size_t i = 0; i < count; i++) {
        ar_data_t* ref_error = (ar_data_t*)items[i];
        if (!is_whitelisted(ref_error, ref_whitelist)) {
            report_error(ref_error);
        }
    }
}

// BAD: Deep analysis doesn't apply whitelist
void deep_analysis(analysis_context_t* ctx) {  // EXAMPLE: Function using example type
    ar_list_t* ref_errors = ctx->errors;
    
    // Shows all errors, including whitelisted ones!
    void** items = ar_list__items(ref_errors);
    size_t count = ar_list__count(ref_errors);
    
    for (size_t i = 0; i < count; i++) {
        ar_data_t* ref_error = (ar_data_t*)items[i];
        analyze_pattern(ref_error);  // No whitelist check!
    }
}

// GOOD: Create filtered list once, use everywhere
ar_list_t* get_unwhitelisted_errors(analysis_context_t* ctx) {  // EXAMPLE: Function using example type
    ar_list_t* own_filtered = ar_list__create();
    ar_list_t* ref_errors = ctx->errors;
    ar_list_t* ref_whitelist = ctx->whitelist;
    
    void** items = ar_list__items(ref_errors);
    size_t count = ar_list__count(ref_errors);
    
    for (size_t i = 0; i < count; i++) {
        ar_data_t* ref_error = (ar_data_t*)items[i];
        if (!is_whitelisted(ref_error, ref_whitelist)) {
            ar_list__add_last(own_filtered, ar_data__claim_or_copy(ref_error));
        }
    }
    
    return own_filtered;  // Ownership transferred to caller
}

// Now all analysis uses same filtered data
void unified_analysis(analysis_context_t* ctx) {  // EXAMPLE: Function using example type
    ar_list_t* own_errors = get_unwhitelisted_errors(ctx);
    
    // Main check
    check_errors_from_list(own_errors);
    
    // Deep analysis  
    deep_analysis_from_list(own_errors);
    
    // Pattern analysis
    pattern_analysis_from_list(own_errors);
    
    ar_list__destroy(own_errors);
}
```

## Generalization
Principles for uniform filtering application:

1. **Single source of truth**: Filter data once, use filtered version everywhere
2. **Audit all code paths**: Find every location that processes the data
3. **Consistent interfaces**: All analysis functions should accept same filtered data
4. **Clear naming**: Distinguish filtered vs unfiltered data in variable names
5. **Document filtering**: Make it obvious when data has been filtered

## Implementation
Strategies for ensuring uniform filtering:

1. **Centralize filtering logic**: One function that all paths use
   ```c
   bool should_include_error(ar_data_t* ref_error, ar_data_t* ref_config) {
       // All filtering logic in one place
       return !is_whitelisted(ref_error, ref_config) &&
              !is_below_threshold(ref_error, ref_config) &&
              !is_duplicate(ref_error, ref_config);
   }
   ```

2. **Use filtered collections**: Create filtered lists/maps upfront
3. **Validate completeness**: Search codebase for data access patterns
   ```bash
   # Find all functions that might process errors
   grep -r "error\|warning" --include="*.c" | grep -E "for.*count|while.*next"
   ```

4. **Test all views**: Verify filtering appears in all outputs
5. **Refactor incrementally**: Update one path at a time, test each

Common locations to check:
- Summary statistics
- Detailed reports  
- Deep analysis sections
- Export functions
- API endpoints
- Log outputs

## Related Patterns
- [Intentional Test Errors Filtering](intentional-test-errors-filtering.md)
- [Single Responsibility Principle](single-responsibility-principle.md)
- [Facade Pattern Coordination](facade-pattern-coordination.md)