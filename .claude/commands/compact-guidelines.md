Make CLAUDE.md guidelines more concise by moving details to knowledge base.

## Checkpoint Tracking

This command uses checkpoint tracking to ensure systematic documentation compacting with link verification. The process has 15 checkpoints across 6 phases.

### Initialize Tracking
```bash
# Start the compacting process
make checkpoint-init CMD=compact-guidelines STEPS='"Analyze CLAUDE.md" "Identify Verbose Sections" "List Extraction Targets" "Check Existing KB Articles" "Plan New Articles" "Create KB Article 1" "Create KB Article 2" "Create KB Article 3" "Update CLAUDE.md References" "Add Cross-References" "Update kb/README.md" "Run Documentation Validation" "Verify Link Coverage" "Review Changes" "Create Commit"'
```

**Expected output:**
```
========================================
   CHECKPOINT TRACKING INITIALIZED
========================================

Command: compact-guidelines
Tracking file: /tmp/compact-guidelines_progress.txt
Total steps: 15

Steps to complete:
  1. Analyze CLAUDE.md
  2. Identify Verbose Sections
  3. List Extraction Targets
  4. Check Existing KB Articles
  5. Plan New Articles
  6. Create KB Article 1
  7. Create KB Article 2
  8. Create KB Article 3
  9. Update CLAUDE.md References
  10. Add Cross-References
  11. Update kb/README.md
  12. Run Documentation Validation
  13. Verify Link Coverage
  14. Review Changes
  15. Create Commit

Goal: Compact CLAUDE.md by 30-50%
Minimum: Create 2+ KB articles
```

### Check Progress
```bash
make checkpoint-status CMD=compact-guidelines
```

**Expected output (example at 53% completion):**
```
========================================
   CHECKPOINT STATUS: compact-guidelines
========================================

Progress: 8/15 steps (53%)

[██████████░░░░░░░░] 53%

Current Phase: Creating KB Articles
Articles Created: 2/3
Line Reduction: 127 lines (32%)
Links Verified: ✓

Next Action:
  → Step 9: Update CLAUDE.md References
```

## Minimum Requirements

**MANDATORY for successful completion:**
- [ ] Create at least 2 new KB articles
- [ ] Achieve 30-50% line reduction in verbose sections
- [ ] Zero broken links after completion
- [ ] All moved content accessible via KB links
- [ ] Pass `make check-docs` validation

## Objective

Make CLAUDE.md guidelines more concise and actionable by moving detailed explanations, examples, and implementation details to the knowledge base (./kb directory). For the complete compacting technique including document-specific strategies, see ([details](../../kb/documentation-compacting-pattern.md)).

**CRITICAL PRINCIPLE**: Never create broken links. Every kb article you reference MUST either:
1. Already exist (verify with `ls kb/article-name.md`), OR
2. Be created by you BEFORE adding any links to it

**CRITICAL**: PRESERVE ALL EXISTING KB REFERENCES
When you see KB article links (formatted with parentheses and "details" text pointing to kb/ files) in CLAUDE.md, these links MUST be preserved. They are not redundant - they ARE the detailed documentation. The goal is to make CLAUDE.md more concise while keeping all KB links that provide the depth.

**IMPORTANT**: Before extracting kb articles from CLAUDE.md, read `.claude/commands/new-learnings.md` to understand the proper format and validation requirements for creating kb articles. This ensures extracted content follows established standards.

## Guidelines for Compacting

## Phase 1: Analysis (Steps 1-3)

**[CHECKPOINT START - PHASE 1]**

### Step 1: Identify Verbose Sections

**Checkpoint 1: Analyze CLAUDE.md**

```bash
# Calculate current size
BEFORE_LINES=$(wc -l < CLAUDE.md)
echo "Current CLAUDE.md size: $BEFORE_LINES lines"
echo "BEFORE_LINES=$BEFORE_LINES" > /tmp/compact-guidelines-metrics.txt

make checkpoint-update CMD=compact-guidelines STEP=1
```

**Checkpoint 2: Identify Verbose Sections**

Look for sections in CLAUDE.md that contain:
- Long explanations that could be summarized in 1-2 lines
- Detailed examples that illustrate rather than specify
- Implementation details that belong in knowledge base
- Redundant information already covered elsewhere
- Multi-paragraph explanations of concepts

```bash
# Document verbose sections found
echo "Verbose sections identified:"
echo "1. [Section name] - [lines that can be reduced]"
echo "2. [Section name] - [lines that can be reduced]"
echo "3. [Section name] - [lines that can be reduced]"

make checkpoint-update CMD=compact-guidelines STEP=2
```

**Checkpoint 3: List Extraction Targets**

```bash
# List specific content to extract
echo "Content to extract to KB:"
echo "- [Topic 1]: [estimated lines]"
echo "- [Topic 2]: [estimated lines]"
echo "- [Topic 3]: [estimated lines]"

# Calculate potential reduction
ESTIMATED_REDUCTION=[number]
echo "Estimated reduction: $ESTIMATED_REDUCTION lines"

make checkpoint-update CMD=compact-guidelines STEP=3
```

**[ANALYSIS GATE]**
```bash
# MANDATORY: Complete analysis before proceeding
make checkpoint-gate CMD=compact-guidelines GATE="Analysis" REQUIRED="1,2,3"
```

**Expected gate output:**
```
========================================
   GATE: Analysis
========================================

✅ GATE PASSED: Analysis complete!

Verified steps:
  ✓ Step 1: Analyze CLAUDE.md
  ✓ Step 2: Identify Verbose Sections
  ✓ Step 3: List Extraction Targets

You may proceed to planning phase.
```

## Phase 2: Planning (Steps 4-5)

**[CHECKPOINT START - PHASE 2]**

**Checkpoint 4: Check Existing KB Articles**

```bash
# Check which KB articles already exist
echo "Checking existing KB articles..."
for topic in "topic1" "topic2" "topic3"; do
  if [ -f "kb/${topic}.md" ]; then
    echo "✓ Exists: kb/${topic}.md"
  else
    echo "✗ Need to create: kb/${topic}.md"
  fi
done

make checkpoint-update CMD=compact-guidelines STEP=4
```

**Checkpoint 5: Plan New Articles**

```bash
# Plan which new articles to create
echo "KB articles to create:"
echo "1. kb/[article1].md - [description]"
echo "2. kb/[article2].md - [description]"
echo "3. kb/[article3].md - [description]"

NEW_ARTICLES_PLANNED=3
echo "NEW_ARTICLES_PLANNED=$NEW_ARTICLES_PLANNED" >> /tmp/compact-guidelines-metrics.txt

make checkpoint-update CMD=compact-guidelines STEP=5
```

**[PLANNING GATE]**
```bash
# MANDATORY: Ensure no duplicate articles
make checkpoint-gate CMD=compact-guidelines GATE="Planning" REQUIRED="4,5"
```

**Expected gate output:**
```
========================================
   GATE: Planning
========================================

✅ GATE PASSED: Planning complete!

Verified steps:
  ✓ Step 4: Check Existing KB Articles
  ✓ Step 5: Plan New Articles

No duplicate articles will be created.
You may proceed to article creation.
```

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

## Phase 3: Creation (Steps 6-8)

**[CHECKPOINT START - PHASE 3]**

### 4. Knowledge Base Article Creation ([details](../../kb/validated-documentation-examples.md))

**Checkpoint 6: Create KB Article 1**

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

```bash
# After creating first KB article
if [ -f "kb/[article1].md" ]; then
  echo "✓ Created: kb/[article1].md"
  make checkpoint-update CMD=compact-guidelines STEP=6
else
  echo "❌ Failed to create article 1"
  exit 1
fi
```

**Checkpoint 7: Create KB Article 2**

```bash
# After creating second KB article
if [ -f "kb/[article2].md" ]; then
  echo "✓ Created: kb/[article2].md"
  make checkpoint-update CMD=compact-guidelines STEP=7
else
  echo "❌ Failed to create article 2"
  exit 1
fi
```

**Checkpoint 8: Create KB Article 3 (Optional)**

```bash
# After creating third KB article (if needed)
if [ -f "kb/[article3].md" ]; then
  echo "✓ Created: kb/[article3].md"
else
  echo "ℹ️ Third article not needed"
fi
make checkpoint-update CMD=compact-guidelines STEP=8
```

**[CRITICAL CREATION GATE]**
```bash
# ⚠️ CRITICAL: All KB articles must exist before updating references!
make checkpoint-gate CMD=compact-guidelines GATE="Creation" REQUIRED="6,7,8"
```

**Expected gate output:**
```
========================================
   GATE: Creation
========================================

⚠️ CRITICAL: All KB articles must exist before updating references!

Articles created:
  ✓ kb/article1.md
  ✓ kb/article2.md
  ✓ kb/article3.md (optional)

✅ GATE PASSED: Safe to update CLAUDE.md references

Proceed to integration phase.
```

## Phase 4: Integration (Steps 9-11)

**[CHECKPOINT START - PHASE 4]**

**Checkpoint 9: Update CLAUDE.md References**

```bash
# Update CLAUDE.md with KB links
echo "Updating CLAUDE.md with KB references..."
# Edit CLAUDE.md to add links to new KB articles

# Verify no broken links
BROKEN=0
for link in $(grep -o 'kb/[^)]*\.md' CLAUDE.md); do
  if [ ! -f "$link" ]; then
    echo "❌ BROKEN LINK: $link"
    BROKEN=$((BROKEN + 1))
  fi
done

if [ $BROKEN -eq 0 ]; then
  echo "✅ All KB links valid"
  make checkpoint-update CMD=compact-guidelines STEP=9
else
  echo "❌ Fix broken links before proceeding"
  exit 1
fi
```

**Checkpoint 10: Add Cross-References**

```bash
# Update existing KB articles with cross-references
echo "Adding cross-references to existing KB articles..."
echo "Updated articles:"
echo "- kb/existing1.md: Added reference to new article"
echo "- kb/existing2.md: Added reference to new article"

make checkpoint-update CMD=compact-guidelines STEP=10
```

**Checkpoint 11: Update kb/README.md**

```bash
# Update the KB index
echo "Updating kb/README.md index..."
# Add new articles to appropriate sections

make checkpoint-update CMD=compact-guidelines STEP=11
```

**[INTEGRATION GATE]**
```bash
# MANDATORY: Ensure all references updated
make checkpoint-gate CMD=compact-guidelines GATE="Integration" REQUIRED="9,10,11"
```

**Expected gate output:**
```
========================================
   GATE: Integration
========================================

✅ GATE PASSED: Integration complete!

Verified steps:
  ✓ Step 9: Update CLAUDE.md References
  ✓ Step 10: Add Cross-References
  ✓ Step 11: Update kb/README.md

Ready for validation.
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

**Bidirectional cross-referencing (MANDATORY):**
- For each new KB article created, find related existing articles
- Update existing articles' "Related Patterns" sections to reference new articles
- Creates a web of knowledge where patterns are discoverable from any entry point

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

## Phase 5: Validation (Steps 12-14)

**[CHECKPOINT START - PHASE 5]**

**Checkpoint 12: Run Documentation Validation**

```bash
# Run documentation validation
if make check-docs; then
  echo "✅ Documentation validation passed"
  make checkpoint-update CMD=compact-guidelines STEP=12
else
  echo "❌ Documentation validation failed - fix errors"
  exit 1
fi
```

**Checkpoint 13: Verify Link Coverage**

```bash
# Verify all compacted sections have KB links
echo "Verifying link coverage..."

# Check that every major compaction has a corresponding link
COMPACTED_SECTIONS=3  # Number of sections you compacted
KB_LINKS=$(grep -c '\[details\](.*kb/.*\.md)' CLAUDE.md)

if [ $KB_LINKS -ge $COMPACTED_SECTIONS ]; then
  echo "✅ Link coverage adequate: $KB_LINKS links for $COMPACTED_SECTIONS compactions"
  make checkpoint-update CMD=compact-guidelines STEP=13
else
  echo "⚠️ Insufficient links: $KB_LINKS links for $COMPACTED_SECTIONS compactions"
fi
```

**Checkpoint 14: Review Changes**

```bash
# Calculate final metrics
source /tmp/compact-guidelines-metrics.txt
AFTER_LINES=$(wc -l < CLAUDE.md)
REDUCTION=$((BEFORE_LINES - AFTER_LINES))
PERCENT=$((REDUCTION * 100 / BEFORE_LINES))

echo "Final metrics:"
echo "- Line reduction: $REDUCTION lines ($PERCENT%)"
echo "- KB articles created: $(git status --porcelain | grep '^A.*kb/.*\.md' | wc -l)"
echo "- Cross-references added: [count]"

# Verify minimum requirements
NEW_ARTICLES=$(git status --porcelain | grep '^A.*kb/.*\.md' | wc -l)
if [ $NEW_ARTICLES -lt 2 ]; then
  echo "⚠️ Only created $NEW_ARTICLES KB articles (minimum: 2)"
  exit 1
fi

make checkpoint-update CMD=compact-guidelines STEP=14
```

**[VALIDATION GATE]**
```bash
# MANDATORY: Ensure no broken links before commit
make checkpoint-gate CMD=compact-guidelines GATE="Validation" REQUIRED="12,13,14"
```

**Expected gate output:**
```
========================================
   GATE: Validation
========================================

✅ GATE PASSED: Validation complete!

Verified steps:
  ✓ Step 12: Run Documentation Validation
  ✓ Step 13: Verify Link Coverage
  ✓ Step 14: Review Changes

Validation Summary:
- Documentation check: ✓ Passed
- Broken links: 0
- Line reduction: 145 lines (35%)
- Articles created: 3
- Cross-references added: 7

Ready to commit!
```

## Phase 6: Commit (Step 15)

**[CHECKPOINT START - PHASE 6]**

### 10. Commit Strategy

**Checkpoint 15: Create Commit**

**CRITICAL**: Use SINGLE commit for all related documentation changes:

```bash
# 1. Create and validate ALL kb articles first
make check-docs  # Ensure no validation errors

# 2. Stage all related changes together
git add kb/ CLAUDE.md TODO.md CHANGELOG.md

# 3. Single comprehensive commit
git commit -m "docs: compact guidelines with comprehensive knowledge base articles

- Reduced CLAUDE.md from X to Y lines (Z% reduction)
- Created N new KB articles extracting verbose content:
  - article1.md: Brief description
  - article2.md: Brief description
- Preserved all existing KB references in CLAUDE.md
- Added cross-references between new and existing KB articles
- Updated kb/README.md index with new entries
- Validated all documentation with make check-docs

Impact: CLAUDE.md is now more scannable and actionable while detailed
information remains accessible through KB links.

🤖 Generated with [Claude Code](https://claude.ai/code)

Co-Authored-By: Claude <noreply@anthropic.com>"
```

**Rationale**: Single commit keeps logically related changes atomic and makes history cleaner

```bash
# After creating commit
make checkpoint-update CMD=compact-guidelines STEP=15
```

**[CHECKPOINT COMPLETE]**
```bash
# Show final summary
make checkpoint-status CMD=compact-guidelines
```

**Expected completion output:**
```
========================================
   CHECKPOINT STATUS: compact-guidelines
========================================

Progress: 15/15 steps (100%)

[████████████████████] 100%

✅ ALL CHECKPOINTS COMPLETE!

Summary:
- Line reduction: [N] lines ([%]%)
- KB articles created: [N] (minimum: 2) ✓
- Broken links: 0 ✓
- Documentation: ✓ Valid
- Commit: ✓ Created

CLAUDE.md successfully compacted!
```

```bash
# Clean up tracking
make checkpoint-cleanup CMD=compact-guidelines
rm -f /tmp/compact-guidelines-metrics.txt
```

## Troubleshooting

### If Creation Gate fails:
```bash
# The Creation Gate is CRITICAL - it prevents broken links
# If it fails, you MUST create the KB articles first:
1. Create any missing KB articles
2. Verify they exist: ls kb/article-name.md
3. Only then proceed to update CLAUDE.md references
```

### If check-docs fails:
```bash
# Common issues:
1. Broken KB links - verify all referenced files exist
2. Invalid markdown - check for syntax errors
3. Missing real AgeRun types - use actual types from codebase
```

### If link verification fails:
```bash
# Check for broken links
for link in $(grep -o 'kb/[^)]*\.md' CLAUDE.md); do
  [ ! -f "$link" ] && echo "Missing: $link"
done
```

### To resume an interrupted session:
```bash
# Check where you left off
make checkpoint-status CMD=compact-guidelines

# Continue from the next pending step
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

## Related Documentation

### Checkpoint Patterns
- [Multi-Step Checkpoint Tracking Pattern](../../kb/multi-step-checkpoint-tracking-pattern.md)
- [Gate Enforcement Exit Codes Pattern](../../kb/gate-enforcement-exit-codes-pattern.md)
- [Command Thoroughness Requirements Pattern](../../kb/command-thoroughness-requirements-pattern.md)

### Documentation Patterns
- [Documentation Compacting Pattern](../../kb/documentation-compacting-pattern.md)
- [Documentation Standards Integration](../../kb/documentation-standards-integration.md)
- [Validated Documentation Examples](../../kb/validated-documentation-examples.md)
- [Documentation Validation Enhancement Patterns](../../kb/documentation-validation-enhancement-patterns.md)

## Usage Notes

This command should be used periodically to prevent CLAUDE.md from becoming unwieldy as new learnings are added. The goal is maintaining the two-tier documentation system with optimal balance between conciseness and completeness.

**Remember**: The Creation Gate is critical - it ensures all KB articles exist BEFORE any references are added to CLAUDE.md, preventing broken links.