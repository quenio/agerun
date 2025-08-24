Compact the TODO.md file by condensing completed tasks while keeping incomplete tasks untouched.

**Role: Documentation Specialist**

This uses the selective compaction pattern for mixed-state documents ([details](../../kb/selective-compaction-pattern.md)) as part of the broader documentation compacting approach ([details](../../kb/documentation-compacting-pattern.md)).

Follow these steps:
1. Read the TODO.md file
2. For each section, identify completed tasks (marked with [x])
3. For completed top-level tasks with sub-items:
   - Merge key information from sub-items into the top-level description
   - Add completion date if present
   - Remove all sub-items
   - Keep as single-line entry
4. For incomplete tasks (marked with [ ]):
   - KEEP COMPLETELY UNTOUCHED including all sub-items
   - Do not modify or compact these in any way
5. Preserve section headers and overall structure
6. Write the compacted version back to TODO.md
7. Show a summary of changes (completed tasks compacted, incomplete preserved)
8. Commit and push the changes:
   - `git add TODO.md`
   - `git commit -m "docs: compact completed tasks in TODO.md"`
   - `git push`

Guidelines for compaction:
- ONLY compact tasks marked with [x]
- NEVER modify tasks marked with [ ]
- Keep completion dates in format "(Completed YYYY-MM-DD)"
- Merge critical details into single-line descriptions
- Use semicolons to separate merged details
- Remove empty sections only if all tasks are completed
- Preserve all priority indicators and "In Progress" markers