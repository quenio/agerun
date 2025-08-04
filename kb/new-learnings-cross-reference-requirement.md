# New Learnings Cross-Reference Requirement

## Learning
The `/new-learnings` command has a MANDATORY requirement to add cross-references between KB articles, but this step is frequently missed, occurring 2-3 times. Cross-referencing must be done immediately when creating or updating KB articles, not as an afterthought.

## Importance
Missing cross-references breaks the knowledge network, making related patterns harder to discover. The recurring nature of this issue suggests it needs explicit attention during the new-learnings workflow to ensure compliance.

## Example
```c
// When creating a KB article about filtering patterns
// File: kb/uniform-filtering-application.md

// BAD: Creating article without planning cross-references
void create_article() {  // EXAMPLE: Metaphorical function
    write_content();
    save_file();
    // Forgot cross-references!
}

// GOOD: Plan cross-references upfront
void create_article_with_references() {  // EXAMPLE: Metaphorical function
    // 1. Identify related articles BEFORE writing
    ar_list_t* own_related = ar_list__create();
    ar_list__add_last(own_related, ar_data__create_string("intentional-test-errors-filtering.md"));
    ar_list__add_last(own_related, ar_data__create_string("single-responsibility-principle.md"));
    
    // 2. Write content with Related Patterns section
    write_content_with_references(own_related);
    
    // 3. Update those related articles to reference this one
    void** items = ar_list__items(own_related);
    size_t count = ar_list__count(own_related);
    for (size_t i = 0; i < count; i++) {
        ar_data_t* ref_filename = (ar_data_t*)items[i];
        update_article_references(ar_data__get_string(ref_filename), "uniform-filtering-application.md");
    }
    
    ar_list__destroy(own_related);
}
```

## Generalization
Best practices to ensure cross-referencing happens:

1. **Before creating articles**: List all related existing articles
2. **While writing**: Add Related Patterns section with those articles
3. **After writing**: Update each related article to reference the new one
4. **Verification**: Use grep to confirm bidirectional links exist
5. **Todo tracking**: Add explicit todo items for cross-referencing

## Implementation
Workflow to prevent missing cross-references:

```bash
# 1. Before creating new article, find related ones
grep -r "filtering\|whitelist\|log" kb/*.md --files-with-matches

# 2. Create article with Related Patterns section
# Include all genuinely related articles found above

# 3. For each new article created, update related ones
# Add reference in their Related Patterns sections

# 4. Verify bidirectional linking
# For new article "new-pattern.md":
grep -l "new-pattern.md" kb/*.md | grep -v "new-pattern.md"
# Should show all articles that reference it

# 5. For updated articles, verify they reference each other
grep -A5 "Related Patterns" kb/article1.md | grep "article2.md"
grep -A5 "Related Patterns" kb/article2.md | grep "article1.md"
```

Common cross-reference patterns:
- Error handling articles link to debugging articles
- Testing articles link to code quality articles  
- Configuration articles link to tool usage articles
- Pattern articles link to principle articles
- Specific implementations link to general patterns

## Related Patterns
- [Cross-Referencing Knowledge Base](cross-referencing-knowledge-base.md)
- [Claude MD Reference Requirement](claude-md-reference-requirement.md)
- [Systematic Guideline Enhancement](systematic-guideline-enhancement.md)
- [Command Continuous Improvement Pattern](command-continuous-improvement-pattern.md)