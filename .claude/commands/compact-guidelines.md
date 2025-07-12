---
description: Make CLAUDE.md guidelines more concise by moving details to knowledge base
---

# Compact Guidelines Command

## Objective

Make CLAUDE.md guidelines more concise and actionable by moving detailed explanations, examples, and implementation details to the knowledge base (./kb directory).

## Guidelines for Compacting

### 1. Identify Verbose Sections

Look for sections in CLAUDE.md that contain:
- Long explanations that could be summarized in 1-2 lines
- Detailed examples that illustrate rather than specify
- Implementation details that belong in knowledge base
- Redundant information already covered elsewhere
- Multi-paragraph explanations of concepts

### 2. Space-Conscious Editing (CRITICAL)

- **Target reduction**: Aim for 30-50% reduction in verbose sections
- **Preserve essential information**: Keep all critical rules and requirements
- **One concept per line**: Avoid multi-concept bullet points
- **Use symbols**: → instead of "leads to", & instead of "and" where clear
- **Remove filler words**: "In order to", "It is important that", etc.
- **Combine related points**: Merge similar guidelines into single lines

### 3. Knowledge Base Article Creation

For each section being compacted:

**When details exist in kb/ already:**
- Move details to existing relevant articles
- Add link: `([details](kb/existing-article.md))`

**When no relevant kb/ article exists:**
- Create new article using standard format:
  ```markdown
  # Article Title
  
  ## Learning
  [What the guideline teaches]
  
  ## Importance  
  [Why it matters]
  
  ## Example
  [Specific implementation examples]
  
  ## Generalization
  [How to apply broadly]
  
  ## Implementation
  [Detailed commands/code when applicable]
  
  ## Related Patterns
  [Connected concepts]
  ```

### 4. Preserve Critical Information

**Never remove:**
- Mandatory requirements (MUST, MANDATORY, CRITICAL)
- Specific commands and their exact syntax
- Error prevention rules
- Build/test requirements
- Memory management rules

**Always keep:**
- Action-oriented language ("Use X", "Run Y", "Check Z")
- Specific file paths and naming patterns
- Version-specific information
- Platform-specific notes

### 5. Link Strategy (MANDATORY)

**CRITICAL**: Every compressed guideline MUST include a link to the knowledge base article containing the extracted details.

**Link placement patterns:**
```markdown
- Guideline summary ([details](kb/article.md))
- **Pattern name**: Brief description ([details](kb/article.md))
- Rule with link at end ([see kb/article.md for examples])
```

**When to link (ALWAYS when details are moved):**
- When >3 lines of detail are moved to kb/
- When examples are moved to illustrate concept
- When implementation details are extracted
- When background context is relocated
- When complex explanations are simplified

**Link validation checklist:**
- [ ] Every compressed section has appropriate links
- [ ] Links point to articles with extracted content
- [ ] Link text clearly indicates what details are available
- [ ] No orphaned details (content moved but not linked)

### 6. Section-by-Section Approach

Work through CLAUDE.md systematically:

1. **Quick Start**: Move detailed explanations to kb/make-full-build.md
2. **Memory Management**: Extract debugging examples to existing kb/ articles
3. **TDD**: Move large refactoring patterns to kb/advanced-tdd-patterns.md
4. **Parnas Principles**: Extract clarifications to kb/parnas-compliance.md
5. **Module Development**: Move quality checklists to kb/module-quality.md
6. **Development Practices**: Extract debug tools to kb/debugging-workflow.md
7. **Refactoring Patterns**: Move key patterns to separate kb/ articles

### 7. Quality Checks

After compacting each section:
- **Readability**: Can guidelines be followed without clicking links?
- **Completeness**: Are all essential rules preserved?
- **Actionability**: Do guidelines specify concrete actions?
- **Links**: Do all links point to relevant, complete articles?
- **Link coverage**: Every compression has corresponding knowledge base link
- **No orphaned content**: All moved details are accessible via links

### 8. Commit Strategy

```bash
# First commit: Create/update knowledge base articles
git add kb/
git commit -m "feat: extract detailed guidance to knowledge base

- Created [list new articles]
- Enhanced [list updated articles] 
- Moved implementation details from CLAUDE.md guidelines"

# Second commit: Compact CLAUDE.md with links
git add CLAUDE.md  
git commit -m "docs: compact guidelines with knowledge base links

- Reduced verbose sections by X%
- Preserved all critical requirements
- Added links to detailed kb/ articles
- Improved scanability and actionability"
```

### 9. Validation

Before finalizing:
- Read compacted guidelines end-to-end for flow
- **Verify all links work and point to relevant content** (MANDATORY)
- Check that essential information is still discoverable
- Ensure new developers can follow guidelines effectively
- **Confirm no details are lost** - every moved piece has a link
- Test link accessibility from compressed guidelines

## Expected Outcomes

- **CLAUDE.md**: Concise, scannable, action-oriented guidelines **with links to detailed articles**
- **Knowledge base**: Comprehensive details with examples
- **Improved usability**: Quick reference + deep dive when needed via links
- **Maintained completeness**: No loss of essential information (all accessible via links)
- **Better maintenance**: Details in focused articles vs sprawling guidelines
- **Two-tier system**: Brief guidelines linked to comprehensive knowledge base

## Usage Notes

This command should be used periodically to prevent CLAUDE.md from becoming unwieldy as new learnings are added. The goal is maintaining the two-tier documentation system with optimal balance between conciseness and completeness.