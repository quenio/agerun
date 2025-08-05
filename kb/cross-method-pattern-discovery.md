# Cross-Method Pattern Discovery

## Learning
When facing implementation challenges, examining how existing methods solve similar problems provides proven patterns and prevents reinventing solutions. The codebase itself is a repository of working patterns.

## Importance
This approach saves development time, ensures consistency across the codebase, and leverages battle-tested solutions. It's particularly valuable when working with language constraints or uncommon scenarios.

## Example
When fixing the echo method's wake message handling, examining method_creator revealed the solution:

```c
// First, search for methods that might handle similar issues
grep -l "__wake__" methods/*.method

// Found method_creator-1.0.0.method handled it elegantly:
memory.is_wake := if(message = "__wake__", 1, 0)
memory.is_sleep := if(message = "__sleep__", 1, 0)
memory.is_special := memory.is_wake + memory.is_sleep
memory.method_name := if(memory.is_special > 0, "bogus", message.method_name)

// Applied same pattern to echo method:
memory.is_wake := if(message = "__wake__", 1, 0)
memory.is_sleep := if(message = "__sleep__", 1, 0)
memory.is_special := memory.is_wake + memory.is_sleep
memory.sender := if(memory.is_special > 0, 0, message.sender)
memory.content := if(memory.is_special > 0, message, message.content)
```

## Generalization
Pattern discovery workflow:

1. **Identify the problem domain** (e.g., "handling special messages")
2. **Search existing code** for similar scenarios:
   ```bash
   grep -r "pattern" methods/
   grep -r "pattern" modules/
   ```
3. **Examine multiple implementations** to find the most elegant
4. **Adapt the pattern** to your specific needs
5. **Document the pattern** for future reuse

## Implementation
Effective search strategies:

1. **Search by problem keyword**:
   ```bash
   grep -r "__wake__\|__sleep__" methods/
   ```

2. **Search by error message**:
   ```bash
   grep "Cannot access field" log_whitelist.yaml | cut -d'"' -f4 | sort -u
   ```

3. **Search by function usage**:
   ```bash
   grep -r "if.*message.*=" methods/  # Find conditional message checks
   ```

4. **Examine test files** for usage patterns:
   ```bash
   grep -r "test.*wake" methods/*_tests.c
   ```

5. **Look for comments** explaining workarounds:
   ```bash
   grep -r "workaround\|hack\|special case" methods/
   ```

Benefits of pattern discovery:
- Consistency across codebase
- Proven solutions to edge cases
- Understanding of design decisions
- Faster problem resolution

## Related Patterns
- [Wake Message Field Access Pattern](wake-message-field-access-pattern.md)
- [Code Smell - Duplicate Code](code-smell-duplicate-code.md)
- [Refactoring Key Patterns](refactoring-key-patterns.md)