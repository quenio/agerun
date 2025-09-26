# Documentation Error Type Classification

## Learning
Documentation validation errors fall into distinct categories that require different fixing strategies. Understanding these categories enables choosing the appropriate intervention level and tool.

## Importance
Applying the wrong fixing strategy wastes time and can introduce new errors. Automatable errors should use scripts, while judgment-based errors need human intervention.

## Example
```c
// AUTOMATABLE ERROR: Non-existent function reference
ar_data_t* result = ar_nonexistent_function(data);  // EXAMPLE: This function doesn't exist

// JUDGMENT-BASED ERROR: Broken link in markdown
[Missing Documentation](../kb/nonexistent-file.md)  // EXAMPLE: File doesn't exist
```

## Generalization

### Automatable Error Types
- **Function/Type References**: Non-existent functions, malformed type names
- **Syntax Errors**: Malformed markdown, incorrect code formatting
- **Pattern Mismatches**: Wrong prefixes, missing ownership annotations

**Tools**: `python3 scripts/batch_fix_docs.py`, automated replacement scripts

### Judgment-Based Error Types
- **Content Decisions**: Whether to create missing files or remove references
- **Broken Links**: Links to intentionally removed or never-created content
- **Structural Issues**: Section organization, cross-reference appropriateness

**Tools**: Manual editing, human review

## Implementation
```bash
# 1. Run validation to identify all errors
make check-docs 2>&1 | tee /tmp/doc-errors.txt

# 2. Classify error types
grep "function.*not found" /tmp/doc-errors.txt  # Automatable
grep "broken link" /tmp/doc-errors.txt         # Often judgment-based

# 3. Apply appropriate fixing strategy
python3 scripts/batch_fix_docs.py --dry-run    # For automatable errors
# Manual review and editing for judgment-based errors
```

## Related Patterns
- [Batch Documentation Fix Enhancement](batch-documentation-fix-enhancement.md)
- [Script Enhancement Over One Off](script-enhancement-over-one-off.md)
- [Documentation Validation Enhancement Patterns](documentation-validation-enhancement-patterns.md)