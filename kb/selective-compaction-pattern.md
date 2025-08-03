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

## Related Patterns
- [Documentation Compacting Pattern](documentation-compacting-pattern.md)
- [Quantitative Documentation Metrics](quantitative-documentation-metrics.md)