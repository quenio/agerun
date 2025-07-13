# Validated Documentation Examples

## Learning
All AgeRun documentation must use real types and functions from the codebase, with automatic validation to prevent hypothetical examples from creating confusion or becoming outdated.

## Importance
Documentation with real, working code examples provides immediate value to developers, prevents maintenance debt from hypothetical examples, and ensures documentation stays synchronized with the actual codebase.

## Example
```c
// GOOD: Real types and functions that actually exist
ar_data_t* data = ar_data__create_string("hello");
int64_t agent_id = ar_agency__create_agent("calculator", "1.0.0", data);
ar_agency__send_to_agent(agent_id, ar_data__create_string("message"));
ar_data__destroy(data);

// ACCEPTABLE: Hypothetical examples with proper tagging
ar_data_t* result = ar_example__process(data);  // EXAMPLE: Hypothetical function for teaching
custom_type_t* ptr;  // EXAMPLE: Hypothetical type for demonstration

// BAD PRACTICE: Anti-pattern examples with proper tagging
ar_data_t* leaked = ar_data__create_string("oops");  // BAD: Memory leak - not destroyed
```

## Generalization
Apply validated documentation standards to:
- Module documentation (modules/*.md)
- Knowledge base articles (kb/*.md) 
- Method documentation (methods/*.md)
- Architecture documents (MMM.md, etc.)
- Slash command guidance (.claude/commands/*.md)

## Implementation

### Validation Workflow
```bash
# Before committing any .md files
make check-docs

# Fix any validation errors found
# Re-run validation until clean
```

### Preferred Real Types
- `ar_data_t*` - Universal data container
- `ar_agent_t*` - Agent references  
- `ar_method_t*` - Method references
- `ar_expression_ast_t*` - Expression trees
- `ar_instruction_ast_t*` - Instruction trees
- `ar_list_t*` - Lists
- `ar_map_t*` - Maps
- `ar_agent_registry_t*` - Agent management

### Preferred Real Functions
- `ar_data__create_*()`, `ar_data__get_*()`, `ar_data__destroy()`
- `ar_agency__create_agent()`, `ar_agency__send_to_agent()`
- `ar_agent__get_*()` family
- `ar_methodology__*()` family
- `ar_heap__malloc()`, `ar_heap__free()` for memory management

### Discovery Tools
```bash
# Get comprehensive guide with examples
python3 scripts/get_real_types.py --guide

# List all available types
python3 scripts/get_real_types.py --types

# List all available functions  
python3 scripts/get_real_types.py --functions
```

### Tagging Requirements
When hypothetical examples are necessary for teaching:
- `// EXAMPLE:` - Hypothetical code for demonstration
- `// BAD:` - Anti-pattern examples showing what NOT to do
- `// ERROR:` - Code that intentionally shows compile-time errors

## Related Patterns
- [Complete Documentation Principle](complete-documentation-principle.md)
- [Automated Documentation Validation](../scripts/check_docs.py)
- [Type Replacement Guide](../scripts/batch_fix_docs.py)