# Language Instruction Renaming Pattern

## Learning
A systematic approach to renaming instructions in the AgeRun language requires updating multiple components in a specific order: enums, parsers, evaluators, module files, tests, documentation, and method files.

## Importance
Ensures consistency across the entire codebase and prevents compilation errors from missed references. A single instruction name appears in dozens of places across different file types.

## Example
```c
// Step 1: Update AST enum
typedef enum {
    AR_INSTRUCTION_AST_TYPE__COMPILE,  // Changed from AR_INSTRUCTION_AST_TYPE__METHOD
    // ... other types
} ar_instruction_ast_type_t;

// Step 2: Update parser string comparison
if (strncmp(ref_instruction + pos, "compile", 7) != 0) {  // Changed from "method", 6
    _log_error(mut_parser, "Expected 'compile' function", pos);
    return NULL;
}

// Step 3: Update evaluator type check
if (ar_instruction_ast__get_type(ref_ast) != AR_INSTRUCTION_AST_TYPE__COMPILE) {
    return false;
}

// Step 4: Update test assertions
assert(ar_instruction__get_type(own_parsed) == AR_INSTRUCTION_TYPE__COMPILE);
```

## Generalization
This pattern applies to any instruction renaming:
1. Update enum values (both AST and legacy if applicable)
2. Update parser string comparisons and error messages
3. Update evaluator type checks and comments
4. Rename module files (parser and evaluator)
5. Update all test files and test content
6. Update documentation (.md files)
7. Update method files (.method files)
8. Run `make clean build` to verify

## Implementation
```bash
# Example for renaming 'agent' to 'create'
# 1. Update enums
grep -r "AR_INSTRUCTION_AST_TYPE__AGENT" modules/
grep -r "AR_INSTRUCTION_TYPE__AGENT" modules/

# 2. Find parser string comparisons
grep -r '"agent"' modules/*parser*.c

# 3. Find module files to rename
ls modules/*agent_instruction*

# 4. Find test references
grep -r "agent(" modules/*test*.c

# 5. Find documentation references
grep -r "agent(" *.md modules/*.md

# 6. Verify with build
make clean build
```

## Related Patterns
- [Build Verification Before Commit](build-verification-before-commit.md)
- [API Migration Completion Verification](api-migration-completion-verification.md)