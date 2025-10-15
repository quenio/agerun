# Language Instruction Renaming Pattern

## Learning
Systematic approach for renaming AgeRun language instructions across the entire codebase, maintaining consistency between grammar definition, implementation, tests, and documentation.

## Importance
Language-level changes affect multiple layers of the system. Missing updates in any layer can cause compilation failures, test failures, or inconsistency between specification and implementation. A systematic approach ensures completeness.

## Example
```c
// Stage 1: Update enum definition
typedef enum {
    AR_INSTRUCTION_AST_TYPE__SPAWN,  // Changed from CREATE
    AR_INSTRUCTION_AST_TYPE__EXIT,   // Changed from DESTROY
    // ...
} ar_instruction_ast_type_t;

// Stage 2: Update parser string matching
if (strncmp(ref_instruction + pos, "spawn", 5) != 0) {  // Changed from "create", 6
    _log_error(mut_parser, "Expected 'spawn' function", pos);
    return NULL;
}
pos += 5;  // Changed from 6

// Stage 3: Update facade dispatch
if (func_len == 5 && strncmp(func_name, "spawn", 5) == 0) {  // Changed from "create", 6
    ar_instruction_ast_t *own_ast = ar_spawn_instruction_parser__parse(
        mut_parser->own_spawn_parser,  // Changed from create_parser
        ref_instruction, ref_assignment
    );
}
```

## Generalization
For any instruction rename, update these components in order:

1. **Enum definitions** - Update `ar_instruction_ast_type_t` values
2. **Parser modules** - Update string matching and character counts
3. **Evaluator modules** - Update function names and type references  
4. **Facade dispatch** - Update main parser/evaluator routing logic
5. **Test files** - Update enum references and expected values
6. **Method files** - Update `.method` files using the instructions
7. **BNF grammar** - Update SPEC.md formal grammar definitions
8. **Documentation** - Update all .md files with new terminology

## Implementation
```bash
# 1. Rename files (preserves git history)  
git mv modules/ar_old_instruction_*.c modules/ar_new_instruction_*.c

# 2. Update content systematically per component
# Use careful search-and-replace with verification:
grep -r "OLD_INSTRUCTION_TYPE" modules/  # EXAMPLE: Search pattern
# Fix each occurrence individually to avoid unintended changes

# 3. Test after each major component
make run-tests  # Catch compilation errors early

# 4. Final validation
make clean build  # Comprehensive verification
make check-docs   # Ensure documentation consistency
```

## Related Patterns
- [Systematic Guideline Enhancement](systematic-guideline-enhancement.md)
- [Facade Pattern Coordination](facade-pattern-coordination.md)
- [API Migration Completion Verification](api-migration-completion-verification.md)