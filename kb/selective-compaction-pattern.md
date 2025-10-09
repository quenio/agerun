# Selective Compaction Pattern

## Learning
A compaction strategy where only certain parts of a document or data structure are compacted based on their state, purpose, or other criteria, while other parts remain completely untouched. This preserves the utility of mixed-state documents.

## Importance
Many documents serve dual purposes - as historical records AND as active working documents. Uniform compaction approaches that process all content equally can damage their utility. Selective compaction maintains document functionality while still achieving size reduction.

## Example
```markdown
// TODO.md example - only completed tasks are compacted:
### Before compaction:
- [x] Fixed memory leaks in agent module
- [x] Updated documentation for new API
- [x] Added comprehensive test coverage
- [ ] Implement new feature X
  - [ ] Design interface
  - [ ] Write implementation
  - [ ] Add tests

### After selective compaction:
- [x] Fixed memory leaks in agent module; updated documentation for new API; added comprehensive test coverage
- [ ] Implement new feature X
  - [ ] Design interface
  - [ ] Write implementation
  - [ ] Add tests
```

```c
// Code example using AgeRun data structures:
#include <string.h>
#include "ar_list.h"
#include "ar_data.h"

void compact_task_list(ar_list_t* mut_tasks) {
    void** items = ar_list__items(mut_tasks);
    size_t count = ar_list__count(mut_tasks);
    
    for (size_t i = 0; i < count; i++) {
        ar_data_t* ref_task = (ar_data_t*)items[i];
        
        // Check if task is complete by looking for "status" field
        const char* status = ar_data__get_map_string(ref_task, "status");
        bool is_complete = (status && strcmp(status, "complete") == 0);
        
        if (is_complete) {
            // Compact completed tasks
            _merge_subtasks_into_description(ref_task);  // EXAMPLE: Helper function
        } else {
            // Preserve incomplete tasks exactly as they are
            // Do not modify structure, formatting, or sub-items
        }
    }
}
```

## Generalization
**When to use selective compaction**:
- Documents with mixed states (complete/incomplete, active/archived)
- Data structures containing both historical and current information
- Configuration files with active and deprecated sections
- Log files where recent entries need full detail
- Any content where preservation criteria varies by item

**Selection criteria examples**:
- State markers: [x] vs [ ], DONE vs TODO
- Date ranges: compact items older than X days
- Priority levels: compact low-priority items more aggressively
- Usage frequency: preserve frequently accessed items
- Ownership: compact only user-owned content

**Benefits**:
- Maintains document utility for ongoing work
- Achieves meaningful size reduction
- Preserves critical context where needed
- Allows gradual archival strategies

## Implementation
1. **Define selection criteria**: Establish clear rules for what gets compacted
2. **Categorize content**: Separate items into compact vs preserve groups
3. **Apply different strategies**: Use appropriate compaction for each group
4. **Preserve markers**: Maintain state indicators for future processing
5. **Document the approach**: Make selection criteria visible to users

**Example criteria patterns**:
```bash
# Compact by state
if [[ "$status" == "complete" ]]; then compact; else preserve; fi

# Compact by age  
if [[ $(days_old "$item") -gt 30 ]]; then compact; else preserve; fi

# Compact by size
if [[ $(line_count "$section") -gt 20 ]]; then compact; else preserve; fi
```

**Key principles**:
- Never modify preserved content - not even whitespace
- Make selection criteria explicit and discoverable
- Consider reversibility - can compaction be undone if needed?
- Test with real documents to verify utility is maintained

## Relationship to Manual Semantic Analysis

**Selective compaction** is about *what* to compact (choosing which items based on state/criteria).

**Manual semantic analysis** is about *how* to compact (grouping related entries across dates, rewriting coherently).

These are complementary, especially for mixed-state documents:

### For Mixed-State Documents (e.g., TODO.md)

**Use BOTH selective compaction + manual semantic analysis**:

1. **Selective compaction determines scope**:
   - Categorize: completed [x] vs. incomplete [ ] tasks
   - Selection rule: Only completed tasks can be compacted
   - Preservation rule: ALL incomplete tasks remain 100% untouched

2. **Manual semantic analysis applied to selected items**:
   - For completed tasks: Merge sub-items, combine related efforts, preserve metrics
   - For incomplete tasks: No analysis needed - they're preserved entirely
   - Result: Meaningful reduction (10-20%) while maintaining all active work

**Example workflow**:
```bash
# Step 1: Selective compaction (what)
Categorize: 150 completed tasks, 75 incomplete tasks
Decision: Compact the 150 completed, preserve the 75 incomplete

# Step 2: Manual semantic analysis (how)
For each of the 150 completed tasks:
  - Merge verbose sub-items into concise parent description
  - Combine related completed tasks with semicolons
  - Preserve completion dates and metrics

# Step 3: Verification
Verify: All 75 incomplete tasks still present and unchanged
Result: 15% reduction achieved while preserving all active work
```

### For Pure Historical Records (e.g., CHANGELOG.md)

**Use manual semantic analysis only** (selective compaction not needed):
- All content is historical and can be compacted
- No preservation constraints
- Apply semantic grouping across dates freely
- Result: Higher reduction (40-50%+) through intelligent synthesis

### For Reference Documentation (e.g., AGENTS.md)

**Use KB extraction** (different strategy entirely):
- Extract verbose content to KB articles
- Compress main doc to essential rules + links
- Result: 30-50% reduction through delegation

See [Documentation Compacting Pattern](documentation-compacting-pattern.md) for the full decision tree of when to use each approach.

## Related Patterns
- [Documentation Compacting Pattern](documentation-compacting-pattern.md) - When to use automation vs. manual
- [Quantitative Documentation Metrics](quantitative-documentation-metrics.md) - What to preserve during compaction