---
description: Analyze session for new learnings and update guidelines
---

# New Learnings Analysis and Guidelines Update

## Step 1: Identify New Learnings

Please analyze this session and identify any new learnings, patterns, or insights that emerged. Consider:

1. **Technical discoveries**: New tools, techniques, or patterns discovered
2. **Process improvements**: Better ways of doing things that were identified
3. **Error patterns**: Common mistakes or issues that were encountered
4. **User feedback**: Important corrections or suggestions from the user
5. **Tool usage**: New or better ways of using existing tools

For each learning, provide:
- A clear description of what was learned
- Why it's important
- Specific examples from this session (if applicable)
- How it can be generalized for future use

**Format for Knowledge Base Files**: Each learning should be saved as an individual .md file in `./kb/` directory with this structure:
```markdown
# Learning Title

## Learning
[What was discovered]

## Importance
[Why it matters]

## Example
[Specific instance from this session]

## Generalization
[How to apply broadly]

## Implementation
[Code/commands when applicable]

## Related Patterns
[Connected concepts]
```

## Step 2: Review Existing Guidelines

Check CLAUDE.md to see if these learnings are already documented. If they are:
- Determine if the existing documentation is sufficient
- Identify if updates or clarifications are needed
- Note any gaps in the current documentation

## Step 3: Create Knowledge Base Files

For each significant learning:

1. **Create individual .md files** in `./kb/` directory using the format above
2. **Use kebab-case filenames** (e.g., `api-migration-completion-verification.md`)
3. **Link from CLAUDE.md** when guidelines reference these learnings:
   ```markdown
   - Guideline text ([details](kb/filename.md))
   ```

## Step 4: Update Guidelines

If updates are needed to CLAUDE.md:

1. **Space-conscious writing** (CRITICAL):
   - Keep additions under 50 bytes when possible
   - Prefer updating existing lines over adding new ones
   - Use abbreviations and symbols where clear (e.g., → instead of "leads to")
   - Combine related points into single lines
   - Remove any redundancy with existing content

2. **Make learnings generic**: 
   - Remove session-specific details
   - Use specific examples only as illustrations
   - Focus on patterns that apply broadly

3. **Find appropriate sections**:
   - Locate existing sections that relate to the learnings
   - PREFER: Add to existing bullet points rather than creating new sections
   - If new section needed, keep header concise

4. **Write clear, concise updates**:
   - Use imperative voice for instructions
   - Include examples ONLY if essential (prefer inline mentions)
   - Emphasize critical points with **bold** or MANDATORY labels
   - Avoid verbose explanations - trust reader's understanding
   - One concept per line maximum

5. **Before adding, always check**:
   - Is this already covered elsewhere?
   - Can this be merged with an existing point?
   - Can this replace something less important?
   - Is every word necessary?

6. **Categories to consider**:
   - Memory management patterns
   - Testing and verification procedures
   - Development workflows
   - Tool usage guidelines
   - Error prevention strategies
   - Documentation requirements

## Step 5: Verify Documentation and Commit Updates

**MANDATORY: Run documentation check before committing**:
```bash
make check-docs
```
Fix any broken references found before proceeding with commits.

**Commit knowledge base files first**:
```bash
git add kb/
git commit -m "feat: add knowledge base with [topic] learnings"
```

**Then commit guidelines updates** (if CLAUDE.md was updated):
```bash
git add CLAUDE.md
git commit -m "docs: enhance guidelines with session learnings on [topic]

- [Summary of key updates]  
- [Links to detailed kb/ articles]"
```

## Example Learnings Categories

**Memory Management**:
- Memory report locations
- Leak detection patterns
- Ownership tracking improvements

**Development Process**:
- Plan verification requirements
- Diff verification for code migration
- TDD cycle refinements

**Tool Usage**:
- Better ways to use existing tools
- New tool capabilities discovered
- Automation opportunities

**Error Prevention**:
- Common pitfalls to avoid
- Verification steps to add
- User feedback incorporation

## Knowledge Base Benefits

The two-tier documentation system provides:
- **CLAUDE.md**: Concise, actionable guidelines for quick reference
- **./kb/ files**: Detailed context, examples, and implementation details
- **Searchable**: `grep -r "keyword" ./kb/` finds relevant patterns
- **Linked**: Guidelines link to detailed articles when more context needed

Remember: The goal is to continuously improve the development guidelines based on real experiences, making future sessions more efficient and error-free.
