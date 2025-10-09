# Knowledge Base Consultation Protocol

## Learning
Knowledge base articles must be consulted BEFORE executing related tasks, not as optional references. This protocol ensures consistent application of established patterns and prevents reinventing solutions.

## Importance
- Prevents errors by applying proven patterns
- Ensures consistency across the codebase
- Saves time by avoiding rediscovery of solutions
- Maintains quality standards established in previous sessions

## Example
```c
// Before refactoring ownership patterns
// FIRST: Search for relevant KB articles
// grep -r "ownership\|refactor" AGENTS.md | grep "kb/"
// THEN: Read kb/refactoring-ownership-transitions.md

// Apply the pattern from the KB article
ar_data_t *own_data = ar_data__create_string("example");
// Transfer ownership following KB guidelines
ar_data__set_map_data(mut_memory, "key", own_data);
own_data = NULL; // Ownership transferred
```

## Generalization
1. **Before starting any task**: Search AGENTS.md for relevant KB links
2. **Create checkpoints**: Before major decisions, stop and search for KB articles
3. **Make it visible**: Explicitly state "Reading kb/article.md..." when consulting
4. **Use TodoWrite**: Include "Read kb/relevant-article.md" as first task item

## Implementation
```bash
# Search for relevant KB articles before starting
grep -r "topic\|keyword" AGENTS.md | grep "kb/"

# Use Task tool for comprehensive search
# Task: Search for KB articles about [topic]

# In todo lists, always include KB reading
# First task should be reading relevant KB articles
```

## Related Patterns
- [KB Consultation Before Planning Requirement](kb-consultation-before-planning-requirement.md)
- [Systematic Task Analysis Protocol](systematic-task-analysis-protocol.md)
- [Systematic Guideline Enhancement](systematic-guideline-enhancement.md)
- [User Feedback as QA](user-feedback-as-qa.md)
- [Command KB Consultation Enforcement](command-kb-consultation-enforcement.md) - Strong enforcement mechanisms