Compact the TODO.md file by keeping only top-level items under each section while preserving important information.

Follow these steps:
1. Read the TODO.md file
2. For each section, identify top-level tasks (not sub-items)
3. For completed top-level tasks with important sub-items:
   - Merge key information into the top-level description
   - Add completion date if present
   - Remove the sub-items
4. For incomplete top-level tasks:
   - Keep the main task description
   - Remove detailed sub-items but note if there are many pending subtasks
5. Preserve section headers and overall structure
6. Write the compacted version back to TODO.md
7. Show a summary of changes (lines before/after, sections processed)
8. Commit and push the changes:
   - `git add TODO.md`
   - `git commit -m "docs: compact TODO.md to show only top-level tasks"`
   - `git push`

Guidelines for compaction:
- Keep completion dates in format "(Completed YYYY-MM-DD)"
- Merge critical details into single-line descriptions
- Use semicolons to separate merged details
- Preserve [x] and [ ] checkbox states
- Remove empty sections
- Keep "In Progress" and priority indicators