# Context Parameter Convention

## Learning
Always use the standard 'context' variable when passing context parameters in AgeRun methods and tests, not arbitrary memory structures or other variables. This maintains consistency with the AgeRun language model.

## Importance
Following parameter conventions ensures:
- Consistency across the codebase
- Clarity about parameter purpose
- Alignment with language semantics
- Easier understanding for future developers
- Proper frame variable access

## Example
```c
// BAD: Using memory as context (confusing and non-standard)
const char *args[] = {"\"method\"", "\"1.0.0\"", "memory"};
ar_expression_ast_t *context_ast = ar_expression_ast__create_memory_access(
    "memory", NULL, 0  // Wrong: memory is not context
);

// GOOD: Using the standard context variable
const char *args[] = {"\"method\"", "\"1.0.0\"", "context"};
ar_expression_ast_t *context_ast = ar_expression_ast__create_memory_access(
    "context", NULL, 0  // Correct: context is the standard variable
);
```

In AgeRun methods:
```agerun
# Standard variables available in all methods:
# - memory: agent's persistent storage
# - context: read-only configuration passed at creation
# - message: current message being processed

# Correct usage:
memory.echo_id := spawn("echo", "1.0.0", context)
# NOT: spawn("echo", "1.0.0", memory)
```

## Generalization
Standard AgeRun frame variables:
1. `memory` - Agent's persistent key-value storage
2. `context` - Read-only configuration from agent creation
3. `message` - Current message being processed

When calling functions that need context:
- Always pass `context`, not `memory`
- Context is meant for configuration, memory for state
- Don't create custom context structures in tests

## Implementation
```bash
# Find incorrect context usage
grep -r "spawn.*memory\)" modules/*_tests.c
grep -r "spawn.*memory\)" methods/*.method

# Correct to use context variable
# Update tests to follow convention
```

## Related Patterns
- [Frame Creation Prerequisites](frame-creation-prerequisites.md)