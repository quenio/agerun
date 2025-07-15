# Just-In-Time Knowledge Base Reading

## Learning
Knowledge base articles linked in guidelines should not be read immediately when encountered, but must be read before executing related tasks to ensure detailed knowledge is applied when needed.

## Importance
Reading all kb links immediately creates information overload and reduces efficiency. Reading them just before task execution ensures the detailed knowledge is fresh and immediately applicable.

## Example
```markdown
// In CLAUDE.md:
**Memory Leak Detection**:
- Check test reports: `bin/memory_report_<test_name>.log` ([details](kb/memory-leak-detection-workflow.md))

// When Claude encounters this:
1. DON'T immediately read kb/memory-leak-detection-workflow.md
2. Note the link exists for memory leak detection tasks
3. When user asks "help me debug a memory leak":
   - First step in plan: "Read kb/memory-leak-detection-workflow.md"
   - Then apply the detailed knowledge from the article

// Task planning example:
Task: Debug memory leak in ar_string_tests
1. Read kb/memory-leak-detection-workflow.md for detailed procedure
2. Run `make ar_string_tests`
3. Check `bin/memory_report_ar_string_tests.log`
4. Apply debugging strategy from kb article
```

## Generalization
**Workflow**: See guideline → Note kb link → Read when executing → Apply detailed knowledge

**When to read kb articles**:
- Before starting a task mentioned in the guideline
- When user asks about a topic with a kb link
- As first step in task plans for relevant work
- When encountering errors related to the topic

**When NOT to read kb articles**:
- While initially scanning CLAUDE.md
- When the guideline is clear enough for the current need
- For unrelated tasks

**Task planning pattern**:
```
User: "Help me refactor the agency module"
Claude: I'll help you refactor the agency module. Let me create a plan:
1. Read kb/refactoring-key-patterns.md for detailed patterns
2. Analyze current agency module structure
3. Apply refactoring patterns from kb article
...
```

## Implementation
1. **Scanning phase**: Note all kb links but don't read content
2. **Task identification**: Determine which kb articles relate to requested task
3. **Planning phase**: Include "Read kb/article.md" as first step for relevant articles
4. **Execution phase**: Read article, then apply knowledge immediately
5. **Reference back**: Re-read specific sections if needed during execution

**Search strategy**: When asked about a topic, search CLAUDE.md for kb links first
**Memory efficiency**: Only load detailed knowledge when actively using it

## Related Patterns
- [Documentation Compacting Pattern](documentation-compacting-pattern.md)
- [Cross-Referencing Knowledge Base](cross-referencing-knowledge-base.md)