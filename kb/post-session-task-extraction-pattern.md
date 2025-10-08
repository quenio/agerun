# Post-Session Task Extraction Pattern

## Learning
Sessions often reveal improvement opportunities that aren't part of the immediate work but are discovered during implementation. These insights should be captured as prioritized TODO items before the session ends, otherwise they're lost. The pattern is to end sessions by asking "What improvements did this work reveal?" and documenting them with context, priority, and success criteria.

## Importance
Without systematic extraction:
- **Lost insights**: Discoveries from real implementation experience disappear
- **Repeated discoveries**: Future sessions rediscover same issues
- **Missing context**: Tasks added months later lack implementation details
- **Priority confusion**: No distinction between critical and nice-to-have

Capturing tasks immediately preserves the context and reasoning that makes them actionable.

## Example
```c
// During session: Implementing agent persistence integration

// Discovery 1: YAML header bug
// While debugging test crashes, discovered that ar_yaml_writer adds header
// but ar_yaml_reader expects it without validation
// Insight: Should validate header explicitly

// Discovery 2: Test isolation issue
// After adding save functionality, tests started failing due to shared files
// Insight: Need systematic cleanup pattern for persistence tests

// Discovery 3: Corruption scenarios
// Basic corruption handled, but many edge cases not tested
// Insight: Need comprehensive corruption scenario tests

// END OF SESSION: Capture these as TODO items

// BAD: Vague tasks without context
ar_data_t* own_tasks = ar_list__create();
ar_list__add_last(own_tasks, ar_data__create_string("Fix YAML validation"));
ar_list__add_last(own_tasks, ar_data__create_string("Improve tests"));
// Future you: "What YAML validation? Which tests?"

// GOOD: Detailed tasks with context and priority
// EXAMPLE: Conceptual structure for organizing task information
// In practice, use ar_data_t* with maps for structured task data

void* create_yaml_validation_task(void) {  // EXAMPLE: Conceptual function
    // Conceptual task fields:
    // - priority: "Medium"
    // - title: "Add validation for missing YAML header"
    // - context: Why discovered
    // - rationale: Why important
    // - tasks: ar_list_t* of sub-tasks
    // - success: ar_list_t* of criteria

    const char* priority = "Medium";
    const char* title = "Add validation for missing YAML header in ar_yaml_reader";

    const char* context =
        "Discovered during executable integration - missing YAML "
        "header caused silent parsing failures. Tests crashed because "
        "manual YAML lacked '# AgeRun YAML File' header that reader expected.";

    const char* rationale =
        "Currently reader silently skips first line. Should detect "
        "if header is missing and provide clear error message. Would "
        "have caught the test file bug immediately.";

    ar_list_t* own_tasks = ar_list__create();
    ar_list__add_last(own_tasks,
        ar_data__create_string("Update ar_yaml_reader to validate header line"));
    ar_list__add_last(own_tasks,
        ar_data__create_string("Add error: 'Invalid YAML format: Expected header'"));
    ar_list__add_last(own_tasks,
        ar_data__create_string("Add test for missing header scenario"));

    ar_list_t* own_success = ar_list__create();
    ar_list__add_last(own_success,
        ar_data__create_string("Clear error when YAML file missing header"));
    ar_list__add_last(own_success,
        ar_data__create_string("Test coverage for missing header scenario"));

    // Cleanup
    ar_list__destroy(own_tasks);
    ar_list__destroy(own_success);

    return NULL;  // Placeholder for teaching purposes
}
```

## Generalization
**Task extraction workflow**:

1. **During session**: Keep running list of discoveries
   - Note what surprised you
   - Record workarounds or compromises
   - Mark edge cases not yet handled
   - Document patterns that could be improved

2. **End of session**: Review discoveries systematically
   ```bash
   # Ask these questions:
   - What validation is missing?
   - What edge cases aren't tested?
   - What patterns could be more systematic?
   - What documentation would have helped?
   - What cleanup or refactoring is needed?
   ```

3. **Categorize by priority**:
   - **High**: Blocks future work or causes bugs
   - **Medium**: Improves quality or prevents issues
   - **Low**: Nice-to-have improvements

4. **Document with context**:
   - **Context**: What you were doing when you discovered it
   - **Rationale**: Why it matters (with specifics)
   - **Tasks**: Concrete sub-tasks to complete it
   - **Success criteria**: How to know it's done
   - **File references**: Specific locations affected

## Implementation
```bash
# End-of-session checklist for task extraction

echo "=== Post-Session Task Extraction ==="

# 1. Review what was implemented
git diff --stat main

# 2. List discoveries made during implementation
echo "What surprised me:"
echo "What I worked around:"
echo "What edge cases I skipped:"
echo "What could be more systematic:"

# 3. For each discovery, create TODO entry

# Template for TODO.md entry:
cat >> TODO.md <<'EOF'
## [Priority] - [Title] (From YYYY-MM-DD Session)

### [Subsection Name]
**Context**: [What you were doing when discovered]

- [ ] **[Task Title]**
  - [Detailed description]
  - [Rationale - why important]
  - [File references with line numbers]
  - [Related existing priority items]

**Success Criteria**:
- [Criterion 1]
- [Criterion 2]
- [Criterion 3]
EOF

# 4. Commit TODO updates
git add TODO.md
git commit -m "docs: Add tasks from [feature] integration session

Added [N] tasks discovered during implementation:
- [Brief list of task titles]

These tasks address [category of improvements] revealed by [the work done]."
```

**Example TODO.md structure**:
```markdown
## Medium Priority - YAML Module Improvements (From 2025-10-08 Session)

### YAML Header Validation
**Context**: Discovered during executable integration - missing YAML header caused silent parsing failures.

- [ ] **Add validation for missing YAML header in ar_yaml_reader**
  - Currently silently skips first line assuming it's a header
  - Should detect if header is missing and provide clear error message
  - Would have caught the test file bug immediately
  - Error message: "Invalid YAML format: Expected '# AgeRun YAML File' header"
  - File: modules/ar_yaml_reader.c (around line 97-99)
  - Related to existing Priority 1 item about replacing direct file I/O

**Success Criteria**:
- Clear error when YAML file missing header
- Test coverage for missing header scenario
- Updated ar_yaml_reader.md documentation
```

## Related Patterns
- [Comprehensive Learning Extraction Pattern](comprehensive-learning-extraction-pattern.md)
- [New Learnings Complete Integration Pattern](new-learnings-complete-integration-pattern.md)
