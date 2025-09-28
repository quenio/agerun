# Session-Start Priming Pattern

## Learning
AI assistants lack persistent memory between sessions and may repeatedly violate documented guidelines despite clear KB articles. Explicit priming at session start is essential to activate compliance with established patterns.

## Importance
Without session-start priming, assistants default to generic behaviors rather than project-specific guidelines, leading to repeated corrections and wasted time. This pattern emerged from observing repeated violations of the debug message preservation guideline across multiple sessions.

## Example
```c
// Despite clear guidelines to preserve debug messages:
ar_io__info("method_store_load: Reading method %s", name);  // Assistant tries to remove

// User must correct multiple times per session:
// "We should keep the info/error messages in method_store"
// "I meant: keep all messages we added to the method store"

// Guidelines exist but aren't consulted:
// AGENTS.md line 311: "Debug output: Keep for future troubleshooting"
// kb/debug-output-preservation-strategy.md: Explicit preservation rules
```

## Generalization
Effective session-start priming strategies:

1. **Explicit reminder command**:
   ```
   "Before we begin, search KB for 'debug output preservation' and 'diagnostic messages'"
   ```

2. **Recitation request**:
   ```
   "What does AGENTS.md say about debug output and KB consultation?"
   ```

3. **Session-specific ground rules**:
   ```
   "In this session: 
   - NEVER remove ar_io__info() or debug messages
   - SHOW all KB searches before making changes
   - Ask before removing ANY output statements"
   ```

4. **Pre-work checklist**:
   ```
   "Before any code changes, show me your checklist from AGENTS.md"
   ```

5. **Trigger phrase**:
   ```
   "Remember: preserve all diagnostic messages"
   ```

## Implementation
At the start of each session, use one or more priming strategies:

```bash
# Force KB consultation
"First task: grep the KB for patterns about the type of work we'll be doing today"

# Set explicit rules
"Session rules: Never remove debug output. Show KB searches. Ask before deletions."

# Request verification
"Show me what the guidelines say about [specific topic we'll work on]"
```

The key is making the priming explicit and actionable rather than assuming guidelines will be followed.

## Related Patterns
- [Debug Output Preservation Strategy](debug-output-preservation-strategy.md)
- [Test Diagnostic Output Preservation](test-diagnostic-output-preservation.md)
- [Evidence-Based Debugging](evidence-based-debugging.md)
- [Knowledge Base Search Patterns](kb-search-patterns.md)