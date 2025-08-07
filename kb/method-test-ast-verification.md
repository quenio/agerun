# Method Test AST Verification

## Learning
Method tests must verify that loaded methods have a valid AST to catch parse errors. Loading a method file successfully doesn't guarantee the method parsed correctly.

## Importance
Without AST verification, tests pass silently while methods contain syntax errors that make them unusable at runtime. This creates false confidence and allows broken code to appear functional.

## Example
```c
// Load the method file
AR_ASSERT(ar_method_fixture__load_method(own_fixture, "bootstrap", 
          "../../methods/bootstrap-1.0.0.method", "1.0.0"),
          "Bootstrap method should load");

// Verify the method parsed successfully (has AST)
ar_agency_t *mut_agency = ar_method_fixture__get_agency(own_fixture);
ar_methodology_t *mut_methodology = ar_agency__get_methodology(mut_agency);
ar_method_t *ref_method = ar_methodology__get_method_with_instance(
    mut_methodology, "bootstrap", "1.0.0");
AR_ASSERT(ref_method != NULL, "Bootstrap method should be found");
AR_ASSERT(ar_method__get_ast(ref_method) != NULL, 
          "Bootstrap method should have valid AST (no parse errors)");
```

## Generalization
All method-loading tests should:
1. Load the method file
2. Retrieve the method from methodology
3. Verify the method has a non-NULL AST
4. Fail immediately if AST is NULL (indicates parse errors)

## Implementation
```c
// Standard pattern for method test AST verification
static void verify_method_parses(ar_methodology_t *mut_methodology, 
                                 const char *name, const char *version) {
    ar_method_t *ref_method = ar_methodology__get_method_with_instance(
        mut_methodology, name, version);
    AR_ASSERT(ref_method != NULL, "Method should be registered");
    AR_ASSERT(ar_method__get_ast(ref_method) != NULL, 
              "Method should have valid AST (no parse errors)");
}
```

## Related Patterns
- [Method Test Template](method-test-template.md)
- [Test First Verification Practice](test-first-verification-practice.md)
- [AgeRun Method Language Nesting Constraint](agerun-method-language-nesting-constraint.md)