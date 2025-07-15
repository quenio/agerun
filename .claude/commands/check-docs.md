Run documentation validation to check for broken file references and function names.

```bash
make check-docs
```

This validation ensures:
- All code examples use real AgeRun types ([details](kb/validated-documentation-examples.md))
- File references are valid
- Function names exist in the codebase
- Support for both C and Zig documentation ([details](kb/multi-language-documentation-validation.md))

For fixing common validation errors, see:
- [Documentation Validation Error Patterns](kb/documentation-validation-error-patterns.md)
- [Documentation Validation Enhancement Patterns](kb/documentation-validation-enhancement-patterns.md)