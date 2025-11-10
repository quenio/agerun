Make AGENTS.md guidelines more concise by moving details to knowledge base.

## STEP VERIFICATION ENFORCEMENT

**MANDATORY**: After completing each step, you MUST verify step completion using the **step-verifier sub-agent** before proceeding to the next step ([details](../../../kb/sub-agent-verification-pattern.md)).

### About the step-verifier Sub-Agent

The **step-verifier** is a specialized sub-agent that independently verifies step completion:

- **Reads command files** to understand step requirements
- **Checks files, git status/diff, test results, build outputs** to verify accomplishments
- **Compares accomplishments against requirements** systematically
- **Reports verification results with evidence** (what was verified, what's missing)
- **Provides STOP instructions** when failures are detected (blocks execution until fixed)
- **Read-only agent**: Never modifies files, commits changes, or makes autonomous decisions

**CRITICAL**: The step-verifier independently verifies your claims. You report accomplishments with evidence; the step-verifier verifies by reading files and checking outputs.

### Step Verification Process

After completing each step, you MUST:

1. **Report accomplishments with concrete evidence**
   - Describe what was accomplished (files created/modified, commands executed, outputs produced)
   - Provide **concrete evidence**: actual file paths with line numbers, full command outputs, git diff output, test results with specific test names, grep/search output proving claims
   - **DO NOT** tell step-verifier what to verify - report what was done with evidence
   - **DO NOT** use vague summaries - provide specific details (see [kb/sub-agent-verification-pattern.md](../../../kb/sub-agent-verification-pattern.md) for examples)

2. **Invoke step-verifier sub-agent**
   - Use `mcp_sub-agents_run_agent` tool with:
     - Agent: `"step-verifier"`
     - Prompt: See format below
     - The step-verifier will independently verify your claims

3. **Handle Verification Results**
  
   **If verification PASSES** (report shows "✅ STEP VERIFIED" or "All requirements met"):
     - Proceed to next step
     -   
   **If verification FAILS** (report shows "⚠️ STOP EXECUTION" or missing elements):
     - **STOP execution immediately** - do not proceed to next step
     - Fix all reported issues from verification report
     - Re-invoke step-verifier with updated evidence after fixes
     - Only proceed after verification report shows "✅ STEP VERIFIED"
  
   **If sub-agent CANNOT be executed** (MCP unavailable or tool error):
     - STOP execution immediately
     - Inform user: "⚠️ Step verification sub-agent unavailable. Please manually verify Step N completion before proceeding."
     - Wait for explicit user confirmation before proceeding

### How to Invoke step-verifier

Use the `mcp_sub-agents_run_agent` tool:

```
Agent: "step-verifier"
Prompt: "Verify Step N: [Step Title] completion for compact-guidelines command.

Todo Item: [Description of what the step accomplished]
Command File: .opencode/command/ar/compact-guidelines.md
Step: Step N: [Step Title]

Accomplishment Report:
[Report what was accomplished with CONCRETE EVIDENCE. The step-verifier will independently verify these claims by reading files, checking git status, etc.

**MANDATORY Evidence Requirements:**
- **File Changes**: Include actual file paths, line numbers, and git diff output showing exact changes
- **Command Execution**: Include full command output, exit codes, test results with specific test names
- **Documentation Updates**: Include file paths, section names, actual content snippets, git diff output
- **Git Status**: Include actual `git status` and `git diff` output showing what changed
- **Verification Output**: Include actual grep/search command output proving claims
- **Build/Test Results**: Include full output showing compilation, test execution, memory leak reports

**Examples:**
✅ GOOD: "Updated `.opencode/command/ar/execute-plan.md` line 2356: `git diff` shows lines changed from `### If progress tracking` to `### If step tracking`. Verification: `grep -i 'checkpoint' file.md` returned no matches (exit code 1)"
❌ BAD: "Updated execute-plan.md to remove checkpoint references"

See [kb/sub-agent-verification-pattern.md](../../../kb/sub-agent-verification-pattern.md) for complete evidence requirements and examples.]"
```

**CRITICAL**: 
- Report accomplishments with **concrete evidence** (file paths, line numbers, command outputs, git diff, test results), NOT instructions or vague summaries
- The step-verifier independently verifies by reading command files, checking files, git status/diff, etc.
- If step-verifier reports "⚠️ STOP EXECUTION", you MUST fix issues before proceeding
- If accomplishment report lacks concrete evidence, step-verifier will STOP execution and require evidence "⚠️ STOP EXECUTION", you MUST fix issues before proceeding

## MANDATORY: Initialize All Todo Items

**CRITICAL**: Before executing ANY steps, add ALL step and verification todo items to the session todo list using `todo_write`:

**Step and Verification Todo Items:**
- Add todo item: "Step 1: Analyze AGENTS.md" - Status: pending
- Add todo item: "Verify Step 1: Analyze AGENTS.md" - Status: pending
- Add todo item: "Step 2: Identify Verbose Sections" - Status: pending
- Add todo item: "Verify Step 2: Identify Verbose Sections" - Status: pending
- Add todo item: "Step 3: List Extraction Targets" - Status: pending
- Add todo item: "Verify Step 3: List Extraction Targets" - Status: pending
- Add todo item: "Step 4: Check Existing KB Articles" - Status: pending
- Add todo item: "Verify Step 4: Check Existing KB Articles" - Status: pending
- Add todo item: "Step 5: Plan New Articles" - Status: pending
- Add todo item: "Verify Step 5: Plan New Articles" - Status: pending
- Add todo item: "Step 6: Create KB Article 1" - Status: pending
- Add todo item: "Verify Step 6: Create KB Article 1" - Status: pending
- Add todo item: "Step 7: Create KB Article 2" - Status: pending
- Add todo item: "Verify Step 7: Create KB Article 2" - Status: pending
- Add todo item: "Step 8: Create KB Article 3" - Status: pending
- Add todo item: "Verify Step 8: Create KB Article 3" - Status: pending
- Add todo item: "Step 9: Update AGENTS.md References" - Status: pending
- Add todo item: "Verify Step 9: Update AGENTS.md References" - Status: pending
- Add todo item: "Step 10: Add Cross-References" - Status: pending
- Add todo item: "Verify Step 10: Add Cross-References" - Status: pending
- Add todo item: "Step 11: Update kb/README.md" - Status: pending
- Add todo item: "Verify Step 11: Update kb/README.md" - Status: pending
- Add todo item: "Step 12: Run Documentation Validation" - Status: pending
- Add todo item: "Verify Step 12: Run Documentation Validation" - Status: pending
- Add todo item: "Step 13: Verify Link Coverage" - Status: pending
- Add todo item: "Verify Step 13: Verify Link Coverage" - Status: pending
- Add todo item: "Step 14: Review Changes" - Status: pending
- Add todo item: "Verify Step 14: Review Changes" - Status: pending
- Add todo item: "Step 15: Create Commit" - Status: pending
- Add todo item: "Verify Step 15: Create Commit" - Status: pending
- Add todo item: "Verify Complete Workflow: compact-guidelines" - Status: pending

**Important**: All todo items are initialized as `pending` and will be updated to `in_progress` when their respective step/verification begins, then to `completed` after verification passes.

### In-Progress Workflow Detection

If a `/compact-guidelines` workflow is already in progress:

### First-Time Initialization Check


## MANDATORY KB Consultation

Before compacting:
1. Search: `grep "compact\|documentation\|target.*compliance\|search.*result" kb/README.md`
2. Must read:
   - documentation-compacting-pattern - for compaction targets (30-50% for reference docs)
   - selective-compaction-pattern - for preservation rules
   - kb-target-compliance-enforcement - **MANDATORY**: How to enforce KB targets with FAIL gates
   - search-result-completeness-verification - Never limit search results without checking totals
3. **Check Related Patterns sections** in each article above and read any additional relevant articles found there
4. In your response, quote these specific items from the KB:
   - The target reduction percentage for reference documentation (from documentation-compacting-pattern)
   - The enforcement pattern from kb-target-compliance-enforcement (FAIL not warn when target missed)
5. Apply compaction best practices
6. **ENFORCE targets with gates**: Use exit 1 to FAIL if target not met, not just warnings
7. **Command structure**: This command demonstrates the orchestrator pattern where documentation shows all steps directly ([details](../../../kb/command-orchestrator-pattern.md))

# Compact Guidelines
This command uses session todo tracking to ensure systematic documentation compacting with link verification. The process has 15 steps across 6 phases.

### Initialize Tracking
```bash
# Start the compacting process
```

**Expected output:**
```
========================================
   SESSION TODO TRACKING INITIALIZED
========================================

Command: compact-guidelines
Tracking file: /tmp/compact-guidelines-progress.txt
Total steps: 15

Steps to complete:
  1. Analyze AGENTS.md
  2. Identify Verbose Sections
  3. List Extraction Targets
  4. Check Existing KB Articles
  5. Plan New Articles
  6. Create KB Article 1
  7. Create KB Article 2
  8. Create KB Article 3
  9. Update AGENTS.md References
  10. Add Cross-References
  11. Update kb/README.md
  12. Run Documentation Validation
  13. Verify Link Coverage
  14. Review Changes
  15. Create Commit

Goal: Compact AGENTS.md by 30-50%
Minimum: Create 2+ KB articles
```

### Check Progress
**Expected output (example at 53% completion):**
```
📈 command: X/Y steps (Z%)
   [████░░░░░░░░░░░░░░░░] Z%
→ Next: ```

## Minimum Requirements

**MANDATORY for successful completion:**
- [ ] Create at least 2 new KB articles
- [ ] Achieve 30-50% line reduction in verbose sections
- [ ] Zero broken links after completion
- [ ] All moved content accessible via KB links
- [ ] Pass `make check-docs` validation

### Objective

Make AGENTS.md guidelines more concise and actionable by moving detailed explanations, examples, and implementation details to the knowledge base (./kb directory). For the complete compacting technique including document-specific strategies, see ([details](../../../kb/documentation-compacting-pattern.md)).

**CRITICAL PRINCIPLE**: Never create broken links. Every kb article you reference MUST either:
1. Already exist (verify with `ls kb/article-name.md`), OR
2. Be created by you BEFORE adding any links to it

**CRITICAL**: PRESERVE ALL EXISTING KB REFERENCES
When you see KB article links (formatted with parentheses and "details" text pointing to kb/ files) in AGENTS.md, these links MUST be preserved. They are not redundant - they ARE the detailed documentation. The goal is to make AGENTS.md more concise while keeping all KB links that provide the depth.

**IMPORTANT**: Before extracting kb articles from AGENTS.md, read `.opencode/command/ar/new-learnings.md` to understand the proper format and validation requirements for creating kb articles. This ensures extracted content follows established standards.

### Guidelines for Compacting

### Stage 1: Analysis (Steps 1-3)

#### Step 1: Identify Verbose Sections

#### Step 1: Analyze AGENTS.md

```bash
# Calculate current size
BEFORE_LINES=$(wc -l < AGENTS.md)
echo "Current AGENTS.md size: $BEFORE_LINES lines"
echo "BEFORE_LINES=$BEFORE_LINES" > /tmp/compact-guidelines-metrics.txt

```

#### Step 2: Identify Verbose Sections

Look for sections in AGENTS.md that contain:
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

```

#### Step 3: List Extraction Targets

```bash
# List specific content to extract
echo "Content to extract to KB:"
echo "- [Topic 1]: [estimated lines]"
echo "- [Topic 2]: [estimated lines]"
echo "- [Topic 3]: [estimated lines]"

# Calculate potential reduction
ESTIMATED_REDUCTION=[number]
echo "Estimated reduction: $ESTIMATED_REDUCTION lines"

```

#### [ANALYSIS GATE]
```bash
# MANDATORY: Complete analysis before proceeding
```

**Expected gate output:**
```
✅ GATE 'Analysis' - PASSED
   Verified: Steps 1,2,3
```

### Stage 2: Planning (Steps 4-5)

#### Step 4: Check Existing KB Articles

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

```

#### Step 5: Plan New Articles

```bash
# Plan which new articles to create
echo "KB articles to create:"
echo "1. kb/[article1].md - [description]"
echo "2. kb/[article2].md - [description]"
echo "3. kb/[article3].md - [description]"

NEW_ARTICLES_PLANNED=3
echo "NEW_ARTICLES_PLANNED=$NEW_ARTICLES_PLANNED" >> /tmp/compact-guidelines-metrics.txt

```

#### [PLANNING GATE]
```bash
# MANDATORY: Ensure no duplicate articles
```

**Expected gate output:**
```
✅ GATE 'Planning' - PASSED
   Verified: Steps 4,5
```

#### 2. Space-Conscious Editing (CRITICAL) ([details](../../../kb/documentation-standards-integration.md))

- **Target reduction**: Aim for 30-50% reduction in verbose sections
- **Preserve essential information**: Keep all critical rules and requirements
- **One concept per line**: Avoid multi-concept bullet points
- **Use symbols**: → instead of "leads to", & instead of "and" where clear
- **Remove filler words**: "In order to", "It is important that", etc.
- **Combine related points**: Merge similar guidelines into single lines

#### 3. Knowledge Base Article References (CRITICAL)

**PRESERVE ALL EXISTING KB REFERENCES**: When compacting AGENTS.md, you MUST:
- Keep all existing KB references intact (the parentheses-style links to kb/ files)
- Never remove KB references - they are the detailed documentation
- If moving content that already has KB links, keep those links with the content
- Only add new KB links when creating new articles

### Stage 3: Creation (Steps 6-8)

#### 4. Knowledge Base Article Creation ([details](../../../kb/validated-documentation-examples.md))

#### Step 6: Create KB Article 1

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
  else
  echo "❌ Failed to create article 1"
  exit 1
fi
```

#### Step 7: Create KB Article 2

```bash
# After creating second KB article
if [ -f "kb/[article2].md" ]; then
  echo "✓ Created: kb/[article2].md"
  else
  echo "❌ Failed to create article 2"
  exit 1
fi
```

#### Step 8: Create KB Article 3 (Optional)

```bash
# After creating third KB article (if needed)
if [ -f "kb/[article3].md" ]; then
  echo "✓ Created: kb/[article3].md"
else
  echo "ℹ️ Third article not needed"
fi
```

#### [CRITICAL CREATION GATE]
```bash
# ⚠️ CRITICAL: All KB articles must exist before updating references!
```

**Expected gate output:**
```
✅ GATE 'Creation' - PASSED
   Verified: Steps 6,7,8
```

### Stage 4: Integration (Steps 9-11)

#### Step 9: Update AGENTS.md References

```bash
# Update AGENTS.md with KB links
echo "Updating AGENTS.md with KB references..."
# Edit AGENTS.md to add links to new KB articles

# Verify no broken links using helper script
if ./scripts/validate-kb-links.sh AGENTS.md; then
  echo "✅ All KB links valid"
  else
  echo "❌ Fix broken links before proceeding"
  exit 1
fi
```

#### Step 10: Add Cross-References

```bash
# Update existing KB articles with cross-references
echo "Adding cross-references to existing KB articles..."
echo "Updated articles:"
echo "- kb/existing1.md: Added reference to new article"
echo "- kb/existing2.md: Added reference to new article"

```

#### Step 11: Update kb/README.md

```bash
# Update the KB index
echo "Updating kb/README.md index..."
# Add new articles to appropriate sections

```

#### [INTEGRATION GATE]
```bash
# MANDATORY: Ensure all references updated
```

**Expected gate output:**
```
✅ GATE 'Integration' - PASSED
   Verified: Steps 9,10,11
```

#### 5. Preserve Critical Information

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

#### 6. Link Strategy (MANDATORY)

**CRITICAL**: Every compressed guideline MUST include a link to the knowledge base article containing the extracted details.

**Link placement patterns:**
```markdown
- Guideline summary ([details](../../../kb/command-helper-script-extraction-pattern.md))
- **Pattern name**: Brief description ([details](../../../kb/script-domain-naming-convention.md))
- Rule with link at end ([see ../../../kb/documentation-compacting-pattern.md for examples])
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

#### 7. Section-by-Section Approach

Work through AGENTS.md systematically:

**CRITICAL**: Only reference kb articles that actually exist or that you create:

1. **Quick Start**: Check if kb/build-verification-before-commit.md exists before referencing
2. **Memory Management**: Use existing kb articles like kb/memory-debugging-comprehensive-guide.md
3. **TDD**: Reference existing kb/red-green-refactor-cycle.md or create new articles as needed
4. **Parnas Principles**: Use existing principle articles in kb/
5. **Module Development**: Reference existing kb/module-quality-checklist.md if it exists
6. **Development Practices**: Use existing debugging articles in kb/
7. **Refactoring Patterns**: Reference existing kb/refactoring-key-patterns.md

#### 8. Quality Checks

After compacting each section:
- **Readability**: Can guidelines be followed without clicking links?
- **Completeness**: Are all essential rules preserved?
- **Actionability**: Do guidelines specify concrete actions?
- **Links**: Do all links point to relevant, complete articles?
- **Link coverage**: Every compression has corresponding knowledge base link
- **No orphaned content**: All moved details are accessible via links

#### 9. Create Required KB Articles First

**MANDATORY**: Before adding any link to a kb article:

1. **Check if the article exists**:
   ```bash
   ls kb/article-name.md
   ```

2. **If it doesn't exist, CREATE IT**:
   - Extract the relevant content from AGENTS.md
   - Create the kb article with proper format
   - Validate with `make check-docs`
   - Only then add the link in AGENTS.md

3. **Never leave placeholder links**

### Stage 5: Validation (Steps 12-14)

#### Step 12: Run Documentation Validation

```bash
# Run documentation validation
if make check-docs; then
  echo "✅ Documentation validation passed"
  else
  echo "❌ Documentation validation failed - fix errors"
  exit 1
fi
```

#### Step 13: Verify Link Coverage

```bash
# Verify all compacted sections have KB links
echo "Verifying link coverage..."

# Check that every major compaction has a corresponding link
COMPACTED_SECTIONS=3  # Number of sections you compacted
KB_LINKS=$(grep -cE '\[details\]\(.*kb/.*\.md\)' AGENTS.md 2>/dev/null || echo "0")

if [ $KB_LINKS -ge $COMPACTED_SECTIONS ]; then
  echo "✅ Link coverage adequate: $KB_LINKS links for $COMPACTED_SECTIONS compactions"
  else
  echo "⚠️ Insufficient links: $KB_LINKS links for $COMPACTED_SECTIONS compactions"
fi
```

#### Step 14: Review Changes

```bash
# Calculate final metrics
source /tmp/compact-guidelines-metrics.txt
AFTER_LINES=$(wc -l < AGENTS.md)
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

```

#### [VALIDATION GATE]
```bash
# MANDATORY: Ensure no broken links before commit
```

**Expected gate output:**
```
✅ GATE 'Validation' - PASSED
   Verified: Steps 12,13,14
```

### Stage 6: Commit (Step 15)

#### 10. Commit Strategy

#### Step 15: Create Commit

**CRITICAL**: Use SINGLE commit for all related documentation changes:

```bash
# 1. Create and validate ALL kb articles first
make check-docs  # Ensure no validation errors

# 2. Stage all related changes together
git add kb/ AGENTS.md TODO.md CHANGELOG.md

# 3. Single comprehensive commit
git commit -m "docs: compact guidelines with comprehensive knowledge base articles

- Reduced AGENTS.md from X to Y lines (Z% reduction)
- Created N new KB articles extracting verbose content:
  - article1.md: Brief description
  - article2.md: Brief description
- Preserved all existing KB references in AGENTS.md
- Added cross-references between new and existing KB articles
- Updated kb/README.md index with new entries
- Validated all documentation with make check-docs

Impact: AGENTS.md is now more scannable and actionable while detailed
information remains accessible through KB links.

🤖 Generated with [Claude Code](https://claude.ai/code)

Co-Authored-By: Claude <noreply@anthropic.com>"
```

**Rationale**: Single commit keeps logically related changes atomic and makes history cleaner

```bash
# After creating commit
```

**Expected completion output:**
```
========================================
   WORKFLOW COMPLETION SUMMARY
========================================

📈 compact-guidelines: X/Y steps (Z%)
   [████████████████████] 100%

```
rm -f /tmp/compact-guidelines-metrics.txt
```

## Troubleshooting

### If Creation Gate fails:
```bash
# The Creation Gate is CRITICAL - it prevents broken links
# If it fails, you MUST create the KB articles first:
1. Create any missing KB articles
2. Verify they exist: ls kb/article-name.md
3. Only then proceed to update AGENTS.md references
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
for link in $(grep -o 'kb/[^)]*\.md' AGENTS.md); do
  [ ! -f "$link" ] && echo "Missing: $link"
done
```

### To resume an interrupted session:
```bash
# Check where you left off

# Continue from the next pending step
```

#### 11. Validation

Before finalizing:
- Read compacted guidelines end-to-end for flow
- **Verify all links work and point to relevant content** (MANDATORY)
- Check that essential information is still discoverable
- Ensure new developers can follow guidelines effectively
- **Confirm no details are lost** - every moved piece has a link
- Test link accessibility from compressed guidelines
- Run `make check-docs` to validate all documentation ([details](../../../kb/documentation-validation-enhancement-patterns.md))

## Expected Outcomes

- **AGENTS.md**: Concise, scannable, action-oriented guidelines **with links to detailed articles**
- **Knowledge base**: Comprehensive details with examples
- **Improved usability**: Quick reference + deep dive when needed via links
- **Maintained completeness**: No loss of essential information (all accessible via links)
- **Better maintenance**: Details in focused articles vs sprawling guidelines
- **Two-tier system**: Brief guidelines linked to comprehensive knowledge base

## Related Documentation

### Tracking Patterns
- [Session Todo List Tracking Pattern](../../../kb/session-todo-list-tracking-pattern.md)
- [Gate Enforcement Exit Codes Pattern](../../../kb/gate-enforcement-exit-codes-pattern.md)
- [Command Thoroughness Requirements Pattern](../../../kb/command-thoroughness-requirements-pattern.md)

### Documentation Patterns
- [Documentation Compacting Pattern](../../../kb/documentation-compacting-pattern.md)
- [Documentation Standards Integration](../../../kb/documentation-standards-integration.md)
- [Validated Documentation Examples](../../../kb/validated-documentation-examples.md)
- [Documentation Validation Enhancement Patterns](../../../kb/documentation-validation-enhancement-patterns.md)

## Usage Notes

This command should be used periodically to prevent AGENTS.md from becoming unwieldy as new learnings are added. The goal is maintaining the two-tier documentation system with optimal balance between conciseness and completeness.

**Remember**: The Creation Gate is critical - it ensures all KB articles exist BEFORE any references are added to AGENTS.md, preventing broken links.