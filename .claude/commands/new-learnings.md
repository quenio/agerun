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

## Step 2: Review Existing Guidelines

Check CLAUDE.md to see if these learnings are already documented. If they are:
- Determine if the existing documentation is sufficient
- Identify if updates or clarifications are needed
- Note any gaps in the current documentation

## Step 3: Update Guidelines

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

## Step 4: Commit Guidelines Updates

If CLAUDE.md was updated:

1. Review changes with `git diff CLAUDE.md`
2. Commit with a descriptive message:
   ```
   docs: enhance guidelines with session learnings on [topic]
   
   - [Summary of key updates]
   - [Additional important changes]
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

Remember: The goal is to continuously improve the development guidelines based on real experiences, making future sessions more efficient and error-free.
