---
description: Make CLAUDE.md guidelines more concise by moving details to knowledge base
---

# Compact Guidelines Command

## Objective

Make CLAUDE.md guidelines more concise and actionable by moving detailed explanations, examples, and implementation details to the knowledge base (./kb directory). For the complete compacting technique, see ([details](../../kb/documentation-compacting-pattern.md)).

**CRITICAL PRINCIPLE**: Never create broken links. Every kb article you reference MUST either:
1. Already exist (verify with `ls kb/article-name.md`), OR
2. Be created by you BEFORE adding any links to it

**CRITICAL**: PRESERVE ALL EXISTING KB REFERENCES
When you see KB article links (formatted with parentheses and "details" text pointing to kb/ files) in CLAUDE.md, these links MUST be preserved. They are not redundant - they ARE the detailed documentation. The goal is to make CLAUDE.md more concise while keeping all KB links that provide the depth.

**IMPORTANT**: Before extracting kb articles from CLAUDE.md, read `.claude/commands/new-learnings.md` to understand the proper format and validation requirements for creating kb articles. This ensures extracted content follows established standards.

## Guidelines for Compacting

### 1. Identify Verbose Sections

Look for sections in CLAUDE.md that contain:
- Long explanations that could be summarized in 1-2 lines
- Detailed examples that illustrate rather than specify
- Implementation details that belong in knowledge base
- Redundant information already covered elsewhere
- Multi-paragraph explanations of concepts

### 2. Space-Conscious Editing (CRITICAL) ([details](../../kb/documentation-standards-integration.md))

- **Target reduction**: Aim for 30-50% reduction in verbose sections
- **Preserve essential information**: Keep all critical rules and requirements
- **One concept per line**: Avoid multi-concept bullet points
- **Use symbols**: → instead of "leads to", & instead of "and" where clear
- **Remove filler words**: "In order to", "It is important that", etc.
- **Combine related points**: Merge similar guidelines into single lines

### 3. Knowledge Base Article References (CRITICAL)

**PRESERVE ALL EXISTING KB REFERENCES**: When compacting CLAUDE.md, you MUST:
- Keep all existing KB references intact (the parentheses-style links to kb/ files)
- Never remove KB references - they are the detailed documentation
- If moving content that already has KB links, keep those links with the content
- Only add new KB links when creating new articles

### 4. Knowledge Base Article Creation ([details](../../kb/validated-documentation-examples.md))

For each section being compacted:

**When details exist in kb/ already:**
- Move details to existing relevant articles
- PRESERVE existing links in their exact format
- Add link with actual filename if missing
- Verify the file exists before adding any new links

**When no relevant kb/ article exists:**
- YOU MUST create the new article before adding any links to it
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
  - Only list kb articles that actually exist
  - Verify each link: ls kb/referenced-article.md
  - If no related patterns exist, omit this section
  ```

### 5. Preserve Critical Information

**Never remove:**
- Mandatory requirements (MUST, MANDATORY, CRITICAL)
- Specific commands and their exact syntax
- Error prevention rules
- Build/test requirements
- Memory management rules
- **Existing KB article references** - these ARE the detailed docs

**Always keep:**
- Action-oriented language ("Use X", "Run Y", "Check Z")
- Specific file paths and naming patterns
- Version-specific information
- Platform-specific notes
- All KB article links - they provide essential depth

### 6. Link Strategy (MANDATORY)

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

### 7. Section-by-Section Approach

Work through CLAUDE.md systematically:

**CRITICAL**: Only reference kb articles that actually exist or that you create:

1. **Quick Start**: Check if kb/build-verification-before-commit.md exists before referencing
2. **Memory Management**: Use existing kb articles like kb/memory-debugging-comprehensive-guide.md
3. **TDD**: Reference existing kb/red-green-refactor-cycle.md or create new articles as needed
4. **Parnas Principles**: Use existing principle articles in kb/
5. **Module Development**: Reference existing kb/module-quality-checklist.md if it exists
6. **Development Practices**: Use existing debugging articles in kb/
7. **Refactoring Patterns**: Reference existing kb/refactoring-key-patterns.md

### 8. Quality Checks

After compacting each section:
- **Readability**: Can guidelines be followed without clicking links?
- **Completeness**: Are all essential rules preserved?
- **Actionability**: Do guidelines specify concrete actions?
- **Links**: Do all links point to relevant, complete articles?
- **Link coverage**: Every compression has corresponding knowledge base link
- **No orphaned content**: All moved details are accessible via links

### 9. Create Required KB Articles First

**MANDATORY**: Before adding any link to a kb article:

1. **Check if the article exists**:
   ```bash
   ls kb/article-name.md
   ```

2. **If it doesn't exist, CREATE IT**:
   - Extract the relevant content from CLAUDE.md
   - Create the kb article with proper format
   - Validate with `make check-docs`
   - Only then add the link in CLAUDE.md

3. **Never leave placeholder links**

### 10. Commit Strategy

**CRITICAL**: Commit kb articles BEFORE updating CLAUDE.md:

```bash
# First commit: Create ALL required knowledge base articles
git add kb/
git commit -m "feat: create knowledge base articles for guideline extraction

- Created [list all new articles]
- Enhanced [list updated articles] 
- Prepared detailed content for CLAUDE.md compaction"

# Run validation to ensure no broken links
make check-docs

# Second commit: Compact CLAUDE.md with verified links
git add CLAUDE.md  
git commit -m "docs: compact guidelines with knowledge base links

- Reduced verbose sections by X%
- Preserved all critical requirements
- Added links to detailed kb/ articles (all verified to exist)
- Improved scanability and actionability"
```

### 11. Validation

Before finalizing:
- Read compacted guidelines end-to-end for flow
- **Verify all links work and point to relevant content** (MANDATORY)
- Check that essential information is still discoverable
- Ensure new developers can follow guidelines effectively
- **Confirm no details are lost** - every moved piece has a link
- Test link accessibility from compressed guidelines
- Run `make check-docs` to validate all documentation ([details](../../kb/documentation-validation-enhancement-patterns.md))

## Expected Outcomes

- **CLAUDE.md**: Concise, scannable, action-oriented guidelines **with links to detailed articles**
- **Knowledge base**: Comprehensive details with examples
- **Improved usability**: Quick reference + deep dive when needed via links
- **Maintained completeness**: No loss of essential information (all accessible via links)
- **Better maintenance**: Details in focused articles vs sprawling guidelines
- **Two-tier system**: Brief guidelines linked to comprehensive knowledge base

## Usage Notes

This command should be used periodically to prevent CLAUDE.md from becoming unwieldy as new learnings are added. The goal is maintaining the two-tier documentation system with optimal balance between conciseness and completeness.