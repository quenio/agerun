# Knowledge Base Search Patterns

## Learning

This pattern teaches when and how to make knowledge base searches visible in the conversation, ensuring important patterns are discovered and applied rather than rediscovering solutions.

## Importance

Without visible KB searches, the same mistakes get repeated across sessions. Making searches explicit creates a feedback loop where patterns are discovered, applied, and refined. This dramatically reduces error rates and improves consistency.

## Example

### Wrong Approach (Silent Search)
```
User: "Fix the memory leak in ar_agent"
Assistant: "Let me fix this..." [starts coding immediately]
```

### Right Approach (Visible Search)
```
User: "Fix the memory leak in ar_agent"
Assistant: "Let me search KB for memory leak patterns..."
[runs: grep -r "memory leak" kb/*.md]
"Found 3 relevant articles: kb/memory-debugging-comprehensive-guide.md, 
kb/ownership-naming-conventions.md, kb/test-memory-leak-ownership-analysis.md"
[reads articles]
"According to kb/ownership-naming-conventions.md, I need to check ownership prefixes..."
[then codes with pattern knowledge]
```

## Generalization

### Trigger Words Requiring KB Search

**From User**:
- "Why are you..." → Search for rationale/pattern
- "You should..." → Search for existing guidance
- "Actually..." → Search for corrections/clarifications
- Any correction → IMMEDIATELY search what user said

**From Assistant**:
- "Let me fix..." → Search FIRST for fix patterns
- "The problem is..." → Search for known issues
- "I'll implement..." → Search for implementation patterns

### Search Timing

- **Session start**: Prime with explicit patterns
- **Before Edit/Write**: Search module/feature patterns
- **After test fails**: Search error patterns
- **Mid-implementation doubt**: STOP → search → continue
- **Choosing approach**: Search existing solutions

## Implementation

### Practical Search Commands

```bash
# Search for specific module patterns
grep -r "module_name" kb/*.md

# Search for error patterns
grep -r "error_message" kb/*.md

# Search for implementation patterns
grep -r "implement\|create\|design" kb/*.md | grep "pattern_name"

# Search for test patterns
grep -r "test\|TDD\|assertion" kb/*.md

# Find related articles
ls kb/ | grep -i "topic"
```

### Making Searches Conversational

- "According to kb/pattern.md..."
- "The KB article kb/example.md suggests..."
- "I found in KB that..."
- "Let me check KB for existing patterns..."
- "KB search reveals 3 relevant articles..."

### Search Result Application

1. Show the search command
2. List found articles
3. Read relevant ones
4. Quote specific guidance
5. Apply the pattern
6. Reference the source

## Related Patterns

- [Session Start Priming Pattern](session-start-priming-pattern.md)
- [Evidence-Based Debugging](evidence-based-debugging.md)
- [Pattern Discovery](cross-method-pattern-discovery.md)
- [Struggling Detection Pattern](struggling-detection-pattern.md)