Analyze session for new learnings and create properly validated kb articles.

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
   - **DO NOT** use vague summaries - provide specific details (see [kb/sub-agent-verification-pattern.md](../../../kb/sub-agent-verification-pattern.md) and [kb/evidence-validation-requirements-pattern.md](../../../kb/evidence-validation-requirements-pattern.md) for examples)

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
Prompt: "Verify Step N: [Step Title] completion for new-learnings command.

Todo Item: [Description of what the step accomplished]
Command File: .opencode/command/ar/new-learnings.md
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

### In-Progress Workflow Detection

Check if a `/new-learnings` workflow is already in progress by reviewing the session todo list. If steps are marked `in_progress` or `pending`, resume from the next incomplete step.

**CRITICAL**: If in-progress workflow exists with pending steps, **DO NOT PERFORM MANUAL KB UPDATES**. The session todo tracking system will enforce all quality requirements and ensure no steps are skipped.

## MANDATORY KB Consultation FIRST

Before starting analysis:
1. Search: `grep "learning\|knowledge\|cross-reference\|search.*result\|target.*compliance" kb/README.md`
2. Must read these articles:
   - session-todo-list-tracking-pattern - Sequential execution discipline for multi-step workflows ([critical](../../../kb/session-todo-list-tracking-pattern.md))
   - new-learnings-cross-reference-requirement - bidirectional linking requirements
   - new-learnings-complete-integration-pattern - complete integration checklist
   - comprehensive-learning-extraction-pattern - multi-pass extraction strategy
   - search-result-completeness-verification - never limit search results without checking totals
   - kb-target-compliance-enforcement - enforce quantitative targets with FAIL gates
   - command-kb-consultation-enforcement - strong KB consultation mechanisms
3. **Check Related Patterns sections** in each article above and read any additional relevant articles found there
4. Apply all KB creation and cross-reference patterns

# New Learnings
## MANDATORY STEP TRACKING SYSTEM

**CRITICAL**: You MUST complete ALL 14 steps in order. NO STEPS CAN BE SKIPPED.

### Initialize Progress Tracking (EXECUTE THIS FIRST)

## Overview of the Process

This command guides you through a comprehensive process to:
1. Identify new learnings from the session
2. Determine whether to create new KB articles or update existing ones
3. Create/update KB articles with real AgeRun code examples
4. Validate all documentation
5. Add proper cross-references between articles
6. Update kb/README.md index
7. Review and update existing Claude commands based on learnings
8. Update AGENTS.md with references
9. Commit and push all changes

**IMPORTANT**: Always consider updating existing KB articles before creating new ones, and ensure all articles are properly cross-referenced to create a web of knowledge.

## STEP SKIP PREVENTION

**NO STEPS CAN BE SKIPPED, NO EXCEPTIONS.**

This is enforced through:

1. **Sequential gate enforcement**:
   - GATE 1 requires steps 1,2,3,4 to be complete
   - GATE 2 requires steps 5,6,7,8,9 to be complete
   - GATE 3 requires step 10 to be complete (quality gate for CHANGELOG.md)
   - FINAL GATE requires ALL 1-12 steps to be complete
   - Cannot proceed to next gate until current gate passes

2. **Quality verification at each gate**:
   - Step 3: Verifies KB files created (blocks if missing)
   - Step 4: Runs `make check-docs` validation (blocks if fails)
   - Step 6: Counts KB articles (blocks if fewer than 3)
   - Step 7: Counts commands (blocks if fewer than 3)
   - Step 10: Verifies CHANGELOG.md updated with session summary
   - Step 12: Checks "READY TO COMMIT" status (blocks if not ready)

3. **Manual enforcement**:
   - Cannot run Step N until Step N-1 is marked complete
   - Session todo list tracks progress and prevents jumping ahead
   - Attempting to skip steps will result in gate failures

**Remember**: The gates exist because skipping steps leads to incomplete integrations. Every step has a purpose for ensuring KB articles are properly integrated throughout the system.

## ANTI-PATTERN: Manual KB Work Outside Session Todo Tracking

❌ **DO NOT DO THIS** (What causes missed steps):

```bash
# ❌ WRONG: Creating KB articles manually without progress tracking
# Just edit files directly and commit without going through /new-learnings workflow

# ❌ WRONG: Updating commands without step tracking
# Modify commands based on "what seems right" instead of following steps

# ❌ WRONG: Skipping quality verification
# Skip GATE 1, GATE 2, and FINAL GATE checks
# Commit without "READY TO COMMIT" verification in Step 12

# ❌ WRONG: Partial execution
# Do steps 1-5 through session todo tracking, then manually update commands
# Skipping the structured KB cross-reference updates in Steps 6-7

# ❌ WRONG: Ignoring existing in-progress workflows
# See session todo list has pending steps
# Decide "I'll just do it manually this time"
# Result: Session todo tracking never reflects actual work done
```

✅ **CORRECT APPROACH**:

```bash
# ✅ ALWAYS use session todo tracking for ALL work
# Check session todo list to see current progress and next required step

# ✅ Follow the sequence strictly
# Complete steps 1,2,3,4 → GATE 1 passes → Continue to 5,6,7,8,9 → GATE 2 passes

# ✅ Let the quality gates enforce completeness
# Cannot proceed unless quality requirements met (Step 6: 3+ KB articles, Step 7: 3+ commands)

# ✅ Verify integration before commit
# Step 12 verifies "READY TO COMMIT" with counts of modified files

# ✅ Always check session todo list if unsure
```

## Minimum Requirements for Thorough Execution

**These are MANDATORY minimums for each execution:**
- **Step 6**: Update at least 3-5 existing KB articles with cross-references
- **Step 7**: Update at least 3-4 existing commands with new KB references  
- **Step 12**: Execute verification script and confirm "READY TO COMMIT" status

**Quality Indicators:**
- More KB articles modified than created (shows good cross-referencing)
- Multiple commands updated (shows good integration)
- Verification script passes all checks before commit


## MANDATORY: Initialize All Todo Items

**CRITICAL**: Before executing ANY steps, add ALL step and verification todo items to the session todo list using `todo_write`:

**Step and Verification Todo Items:**
- Add todo item: "Step 1: Identify New Learnings" - Status: pending
- Add todo item: "Verify Step 1: Identify New Learnings" - Status: pending
- Add todo item: "Step 2: Determine KB Article Strategy" - Status: pending
- Add todo item: "Verify Step 2: Determine KB Article Strategy" - Status: pending
- Add todo item: "Step 3: Knowledge Base Article Creation" - Status: pending
- Add todo item: "Verify Step 3: Knowledge Base Article Creation" - Status: pending
- Add todo item: "Step 4: Validation Before Saving" - Status: pending
- Add todo item: "Verify Step 4: Validation Before Saving" - Status: pending
- Add todo item: "Step 5: Update Knowledge Base Index" - Status: pending
- Add todo item: "Verify Step 5: Update Knowledge Base Index" - Status: pending
- Add todo item: "Step 6: Update Existing KB Articles (3-5 minimum)" - Status: pending
- Add todo item: "Verify Step 6: Update Existing KB Articles (3-5 minimum)" - Status: pending
- Add todo item: "Step 7: Review and Update Commands (3-4 minimum)" - Status: pending
- Add todo item: "Verify Step 7: Review and Update Commands (3-4 minimum)" - Status: pending
- Add todo item: "Step 8: Review Existing Guidelines" - Status: pending
- Add todo item: "Verify Step 8: Review Existing Guidelines" - Status: pending
- Add todo item: "Step 9: Update Guidelines" - Status: pending
- Add todo item: "Verify Step 9: Update Guidelines" - Status: pending
- Add todo item: "Step 10: Update CHANGELOG.md" - Status: pending
- Add todo item: "Verify Step 10: Update CHANGELOG.md" - Status: pending
- Add todo item: "Step 11: Validate No Broken Links" - Status: pending
- Add todo item: "Verify Step 11: Validate No Broken Links" - Status: pending
- Add todo item: "Step 12: Pre-Commit Integration Verification" - Status: pending
- Add todo item: "Verify Step 12: Pre-Commit Integration Verification" - Status: pending
- Add todo item: "Step 14: Automatic Commit and Push" - Status: pending
- Add todo item: "Verify Step 14: Automatic Commit and Push" - Status: pending
- Add todo item: "Verify Complete Workflow: new-learnings" - Status: pending

**Important**: All todo items are initialized as `pending` and will be updated to `in_progress` when their respective step/verification begins, then to `completed` after verification passes.

## Step 1: Identify New Learnings


**CRITICAL: Think deeply and thoroughly about what was learned in this session.**

Before listing learnings, take time to:
1. **Review the entire session chronologically** - Don't just skim recent messages
2. **Look beyond the obvious** - Surface-level fixes often reveal deeper patterns
3. **Re-examine user feedback** - Critical insights often come from corrections
4. **Consider what DIDN'T work** - Failed approaches teach valuable lessons
5. **Ask yourself**: "Is that all I learned? Think harder."

**Deep Analysis Questions**:
- What architectural patterns emerged from seemingly simple fixes?
- What debugging strategies proved most effective?
- What assumptions were challenged by user feedback?
- What systematic approaches could prevent similar issues?
- What subtle patterns appeared across multiple files?
- What meta-patterns emerged about the learning process itself?
- What consistency issues were found between related modules?

Please analyze this session and identify any new learnings, patterns, or insights that emerged. Consider:

1. **Technical discoveries**: New tools, techniques, or patterns discovered
2. **Process improvements**: Better ways of doing things that were identified
3. **Error patterns**: Common mistakes or issues that were encountered
4. **User feedback**: Important corrections or suggestions from the user
5. **Tool usage**: New or better ways of using existing tools
6. **Architectural insights**: Larger patterns revealed through specific work
7. **Debugging revelations**: Approaches that led to breakthrough solutions
8. **Testing strategies**: Patterns for simplifying or improving tests
9. **Module consistency**: Patterns about maintaining consistency across sister modules ([details](../../../kb/module-consistency-verification.md))
10. **Meta-learnings**: Patterns about the learning extraction process itself

**Remember**: Initial analysis often misses 50% of learnings. After your first pass, review again and ask "What else did I learn that I'm not seeing?" ([details](../../../kb/comprehensive-learning-extraction-pattern.md))

For each learning, provide:
- A clear description of what was learned
- Why it's important
- Specific examples from this session (if applicable)
- How it can be generalized for future use

```bash
# Mark Step 1 complete
```

**Expected output:**
```
✓ Step 1 marked as complete: Identify New Learnings

Progress: 1/12 completed
Next pending: Determine KB Article Strategy
```

## Step 2: Determine KB Article Strategy

### First Decision: New Articles vs Update Existing

**IMPORTANT**: Before creating new articles, check if existing KB articles should be updated instead:

1. **Search for related existing articles**:
   ```bash
   grep -r "relevant_keyword" kb/*.md
   ```

2. **Consider updating existing articles when**:
   - The learning extends or clarifies an existing pattern
   - The learning provides a new example of an existing concept
   - The learning adds important context to existing documentation

3. **Create new articles when**:
   - The learning represents a genuinely new pattern
   - The learning is substantial enough to warrant its own article
   - The learning doesn't fit naturally into existing articles

### Second Decision: Cross-References

**CRITICAL**: Plan cross-references between articles ([details](../../../kb/iterative-plan-refinement-pattern.md)):
- New articles should reference related existing articles
- Existing articles should be updated with references to new articles
- Create a bidirectional web of knowledge
- Expect multiple refinement iterations to get methodology right

```bash
# Mark Step 2 complete
```

## Step 3: Knowledge Base Article Creation

**CRITICAL: All code examples MUST use real AgeRun types and functions** ([details](../../../kb/validated-documentation-examples.md))

### Format for Knowledge Base Files
Each learning should be saved as an individual .md file in `./kb/` directory with this structure:

```markdown
# Learning Title

## Learning
[What was discovered]

## Importance
[Why it matters]

## Example
[Code example using REAL AgeRun types/functions - see guidelines below]

## Generalization
[How to apply broadly]

## Implementation
[Commands/code when applicable - using REAL functions]

## Related Patterns
- List only existing kb articles that are genuinely related
- Use actual filenames: [Pattern Name](actual-existing-file.md)
- If no related patterns exist, omit this section entirely
```

### Code Example Guidelines (MANDATORY)

**Use REAL AgeRun types and functions only:**

**✅ PREFERRED Real Types:**
- `ar_data_t*` (most common data structure)
- `ar_agent_t*` (agents)
- `ar_method_t*` (methods)
- `ar_expression_ast_t*` (expressions)
- `ar_instruction_ast_t*` (instructions)
- `ar_list_t*` (lists)
- `ar_map_t*` (maps)
- `ar_agent_registry_t*` (agent management)

**✅ PREFERRED Real Functions:**
- `ar_data__create_*()`, `ar_data__get_*()`, `ar_data__destroy()`
- `ar_agency__create_agent()`, `ar_agency__get_agent_*()` functions
- `ar_agent__get_*()`, `ar_agent__set_*()`
- `ar_methodology__*()` functions
- `ar_heap__malloc()`, `ar_heap__free()` (memory management)
- `ar_expression_evaluator__*()`, `ar_instruction_evaluator__*()`

**⚠️ VERIFY Function Names**: Always check actual function signatures:
```bash
grep -r "function_name" modules/*.h
```
Common pitfalls:
- `ar_data__set_map_value()` doesn't exist → use `ar_data__set_map_string()` etc.  // EXAMPLE: Hypothetical function
- `ar_agency__get_agent()` doesn't exist → use `ar_agency__get_agent_memory()` etc.  // EXAMPLE: Hypothetical function
- `ar_data__release_ownership()` doesn't exist → use `ar_data__drop_ownership()`  // EXAMPLE: Hypothetical function

**✅ EXAMPLE Tag Usage:**
When you need hypothetical examples for teaching:
```c
// Good: Real function with teaching context
ar_data_t* data = ar_data__create_string("hello");  // EXAMPLE: Using real type for demonstration

// Good: Hypothetical function marked appropriately
ar_data_t* result = ar_example__process(data);  // EXAMPLE: Hypothetical function for teaching

// Good: Bad practice example
ar_data_t* leaked = ar_data__create_string("oops");  // BAD: Memory leak - not destroyed
```

**❌ NEVER use hypothetical types without EXAMPLE tags:**
```c
// Wrong: Don't create fake types
custom_type_t* data;  // EXAMPLE: This will fail validation

// Right: Use real types or mark appropriately  
ar_data_t* data;  // Real type - preferred
custom_type_t* data;  // EXAMPLE: Hypothetical type for teaching
```

### Type Replacement Guide

If you're tempted to use hypothetical types, replace with real ones:
- `string_t` → `ar_data_t*` (with string content)  // EXAMPLE: Hypothetical type mapping
- `ar_list_t` → `ar_list_t*` or `ar_data_t*` (with list content)  // EXAMPLE: Hypothetical type mapping  
- `ar_agent_t` → `ar_agent_t*`  // EXAMPLE: Hypothetical type mapping
- `config_t` → `ar_data_t*` (with map content)  // EXAMPLE: Hypothetical type mapping
- `processor_t` → `ar_expression_evaluator_t*` or `ar_instruction_evaluator_t*`  // EXAMPLE: Hypothetical type mapping
- `context_t` → `ar_data_t*`  // EXAMPLE: Hypothetical type mapping

```bash
# Mark Step 3 complete with evidence verification
```

**CRITICAL**: Replace `new-article-filename.md` with the actual filename of the KB article you created. The system will verify:
- File exists and contains real AgeRun types
- Documentation follows KB article format
- All code examples use valid AgeRun functions

## Step 4: Validation Before Saving

**MANDATORY: Test articles before committing**

1. **Create the .md file** with your content
2. **Run validation**:
   ```bash
   make check-docs
   ```
3. **Fix any validation errors** - the script will tell you exactly what's wrong
4. **Re-run validation** until it passes

**Common validation fixes:** ([details](../../../kb/documentation-validation-error-patterns.md))
- Replace hypothetical types with real ones from the list above
- Add `// EXAMPLE:` tags to hypothetical functions
- Use `ar_data_t*` as the universal fallback type
- Reference actual functions from `modules/*.h` files

```bash
# Mark Step 4 complete - verification automated
```

**AUTOMATIC VERIFICATION**: The system will automatically run `make check-docs` and block completion if validation fails.

## GATE 1: ARTICLE CREATION VERIFICATION

```bash
# MANDATORY GATE - Cannot proceed without articles
```

**Expected output when PASSING:**
```
========================================
   GATE: Article Creation
========================================

✅ GATE PASSED: All required steps are complete!

Verified steps:
  ✓ Step 1: Identify New Learnings
  ✓ Step 2: Determine KB Article Strategy
  ✓ Step 3: Knowledge Base Article Creation
  ✓ Step 4: Validation Before Saving

You may proceed to the next section.
```

**Expected output when BLOCKED:**
```
========================================
   GATE: Article Creation
========================================

❌ GATE BLOCKED: Cannot proceed!

The following steps must be completed first:
  ⏳ Step 3: Knowledge Base Article Creation
  ⏳ Step 4: Validation Before Saving

⛔ STOP: Complete the missing steps before continuing.
[Exit code: 1]
```

## Step 5: Update Knowledge Base Index

**MANDATORY: Add new articles to kb/README.md**

1. **Categorize appropriately** in kb/README.md (two-level structure):
   - 1. DEVELOPMENT PROCESS: Workflow, Session Management, Commits, Knowledge Management
   - 2. QUALITY ASSURANCE: Test Design, Implementation, Maintenance, Debugging
   - 3. ARCHITECTURE & DESIGN: Architectural Patterns, Design Principles, Module Design
   - 4. IMPLEMENTATION: Memory Management, Error Handling, Code Quality & Refactoring
   - 5. DOCUMENTATION: Standards, Maintenance, Communication
   - 6. TOOLS & INFRASTRUCTURE: Build/CI, Automation, Platform Specific

2. **Add to correct section** with proper link format:
   ```markdown
   - [Article Title](article-filename.md)
   ```

```bash
# Mark Step 5 complete
```

## Step 6: Update Existing KB Articles with Cross-References (THOROUGH EXECUTION REQUIRED)

**CRITICAL - OFTEN MISSED**: Add cross-references to create a web of knowledge ([details](../../../kb/new-learnings-complete-integration-pattern.md)):

**MINIMUM REQUIREMENT**: Update at least 3-5 existing KB articles with cross-references.

1. **Find related content using helper script**:
   ```bash
   # Search for related KB articles and commands
   ./scripts/find-related-content.sh keyword1 keyword2 keyword3

   # Example for test-related article:
   ./scripts/find-related-content.sh test assertion strength

   # Example for build/CI article:
   ./scripts/find-related-content.sh build verification check-logs CI

   # Example for error/whitelist article:
   ./scripts/find-related-content.sh whitelist intentional error
   ```

2. **Review the output**:
   - Add reference to new article in Related Patterns section of found articles
   - Target articles that would genuinely benefit from the reference

3. **Cross-reference pattern**:
   ```markdown
   ## Related Patterns
   - [Existing Pattern](existing-pattern.md)
   - [New Pattern You Created](new-pattern.md)
   ```

4. **Verification**: Count modified KB articles - should exceed new articles:
   ```bash
   git diff --name-only | grep "kb.*\.md" | wc -l  # Should be > new articles
   ```
   
   **Note**: After architectural changes, verify cross-references are updated ([details](../../../kb/architectural-change-verification-pattern.md)). For systematic updates across all documentation layers, see [kb/architectural-documentation-consistency-pattern.md](../../../kb/architectural-documentation-consistency-pattern.md).

```bash
# Mark Step 6 complete with cross-reference verification
```

**AUTOMATIC VERIFICATION**: The system will verify at least 3 KB articles were modified and block completion if insufficient cross-references were added.

## Step 7: Review and Update Existing Commands (THOROUGH EXECUTION REQUIRED)

**CRITICAL - OFTEN MISSED**: Check if any Claude commands should be updated based on learnings ([details](../../../kb/new-learnings-complete-integration-pattern.md)):

**MINIMUM REQUIREMENT**: Update at least 3-4 relevant commands with new KB references.

1. **Find related commands using helper script**:
   ```bash
   # Search for commands related to your topic
   ./scripts/find-related-content.sh test build check log error whitelist assert

   # Or use specific keywords for your learnings:
   ./scripts/find-related-content.sh [your-keywords-here]
   ```

   The script will show:
   - Related KB articles by keyword
   - Related commands by keyword
   - Priority commands to always review

2. **Priority commands to ALWAYS check**:
   - `commit.md` - Often needs CI/build references
   - `build.md` - Needs build/logs relationship updates
   - `run-tests.md` - Needs test technique references
   - `sanitize-tests.md` - Needs dlsym/exclusion updates
   - `fix-errors-whitelisted.md` - Needs whitelist context

3. **Specific updates to make**:
   - Add references to new KB articles: `([details](../../kb/actual-article-you-created.md))` // EXAMPLE: Replace with real filename
   - Add context about CI requirements where relevant
   - Update test-related commands with new testing techniques
   - Add warnings about check-logs blocking CI where appropriate
   
4. **Update patterns**:
   ```markdown
   # Example: Adding to existing text
   Old: "Run tests to verify"
   New: "Run tests to verify ([details](../../kb/test-assertion-strength-patterns.md))"
   
   # Example: Adding context
   Old: "Check logs for errors"  
   New: "Check logs for errors. Note: check-logs must pass or CI will fail ([details](../../kb/ci-check-logs-requirement.md))"
   ```

5. **Verification**: Count modified commands:
   ```bash
   git diff --name-only | grep ".opencode/command" | wc -l  # Should be >= 3
   ```

```bash
# Mark Step 7 complete with command update verification
```

**AUTOMATIC VERIFICATION**: The system will verify at least 3 command files were modified and block completion if insufficient updates were made.

## Step 8: Review Existing Guidelines

Check AGENTS.md to see if these learnings should be referenced:
- Determine if existing documentation needs links to new kb articles
- Identify appropriate sections where kb articles should be referenced
- Note any gaps that need new content with kb links

```bash
# Mark Step 8 complete
```

## Step 9: Update Guidelines

**CRITICAL**: AGENTS.md updates are MANDATORY for new KB articles ([details](../../../kb/claude-md-reference-requirement.md))

If updates are needed to AGENTS.md:

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

4. **Link to detailed articles**:
   ```markdown
   - Guideline text ([details](kb/filename.md))
   ```
   - Use relative paths only - see [Markdown Link Resolution Patterns](../../../kb/markdown-link-resolution-patterns.md)

5. **Reference integration strategies**:
   - Add `([details](../../kb/actual-article-you-created.md))` links ONLY for articles you actually created in this session // EXAMPLE: Replace with real filename
   - Never add placeholder links or references to non-existent articles
   - Include new kb articles in relevant sections (e.g., Script Development, Documentation Protocol)
   - Maintain two-tier system: brief guidelines with links to comprehensive details

```bash
# Mark Step 9 complete
```

## GATE 2: INTEGRATION VERIFICATION

```bash
# MANDATORY GATE - Check integration completeness
```

## Step 10: Update CHANGELOG.md

**CRITICAL**: Record all session learnings and changes in CHANGELOG.md BEFORE committing.

1. **Add session entry** at the top of CHANGELOG.md:
   ```markdown
   ## [Today's Date] (Session X - New Learnings)

   - **[Session Title]**

     [Brief overview of main learnings]

     **Problem**: [What issue/gap was identified]

     **Solution**: [How learnings address it]

     **KB Articles Created**: [List of new KB files created]

     **KB Articles Updated**: [List of existing KB files updated with cross-references]

     **Commands Updated**: [List of command files updated]

     **AGENTS.md Updates**: [Summary of guideline enhancements]

     **Impact**: [How this improves the project]
   ```

2. **Use multi-line format** ([details](../../../kb/changelog-multi-line-format.md)):
   - Clear title on first line
   - Overview paragraph
   - Labeled sections with bold headers
   - Bullet lists for multiple items
   - Blank lines between sections

3. **Be comprehensive but concise**:
   - Include all KB articles and commands modified
   - Explain the session's purpose and outcomes
   - Document any important patterns discovered
   - Note any quality gates enforced or process improvements

```bash
# Mark Step 11 complete
```

## GATE 3: QUALITY GATES VERIFICATION

```bash
# MANDATORY GATE - Verify CHANGELOG.md
```

**Expected output when PASSING:**
```
========================================
   GATE: Quality Gates
========================================

✅ GATE PASSED: All required steps are complete!

Verified steps:
  ✓ Step 10: Update CHANGELOG.md

You may proceed to the next section.
```

## Step 11: Validate No Broken Links

**CRITICAL**: Before committing, verify all links work:

1. **Check for broken links in kb articles**:
   ```bash
   grep -r "\[.*\](.*\.md)" kb/*.md | grep -v "EXAMPLE:" | while read line; do
     file=$(echo "$line" | cut -d: -f1)
     link=$(echo "$line" | grep -o '([^)]*\.md)' | tr -d '()')
     # Verify each link exists
   done
   ```

2. **Never reference non-existent articles in Related Patterns sections**

```bash
# Mark Step 11 complete
```

## Step 12: Pre-Commit Integration Verification (MANDATORY EXECUTION)

**CRITICAL**: You MUST execute this verification before committing.

```bash
# Run integration verification script
./scripts/verify-new-learnings-integration.sh
```

This script verifies:
1. Count of KB articles modified (minimum: 3)
2. Count of commands modified (minimum: 3)
3. Lists all modified KB articles
4. Lists all modified commands
5. Reports status: READY TO COMMIT or NOT READY

**Status Decision:**
- If BOTH minimums met → ✅ READY TO COMMIT (script exits 0)
- If either below minimum → ⚠️ NOT READY (script exits 1)

**DO NOT PROCEED TO STEP 14 UNLESS THIS SCRIPT SHOWS "READY TO COMMIT"**

```bash
# Mark Step 12 complete with integration verification
```

**AUTOMATIC VERIFICATION**: The system will verify integration completeness automatically and only allow completion if status shows "READY TO COMMIT".

## FINAL GATE: COMMIT READINESS CHECK

```bash
# FINAL MANDATORY GATE - All steps must be complete

# If gate passes, show final status
if [ $? -eq 0 ]; then
    echo ""
    echo "✅ ALL GATES PASSED: Ready for Step 14 (Commit and Push)"
    fi
```

## Step 14: Automatic Commit and Push

**CRITICAL**: Only execute if FINAL GATE shows "ALL GATES PASSED"

**EXECUTE THE FOLLOWING SEQUENCE AUTOMATICALLY:**

1. **Validate first**:
   ```bash
   make check-docs
   ```
   Fix any errors before proceeding.

2. **Stage all knowledge base work**:
   ```bash
   git add kb/ AGENTS.md TODO.md CHANGELOG.md
   # If any commands were updated:
   git add .opencode/command/
   ```

3. **Commit with comprehensive message**:
   ```bash
   git commit -m "$(cat <<'EOF'
   docs: integrate knowledge base articles from session learnings

   - Created comprehensive knowledge base articles documenting new patterns
   - Updated AGENTS.md with references to new articles in appropriate sections
   - Enhanced kb/README.md index with new Development Workflow articles
   - Updated TODO.md and CHANGELOG.md to document completion
   - All documentation validated with make check-docs

   🤖 Generated with [Claude Code](https://claude.ai/code)

   Co-Authored-By: Claude <noreply@anthropic.com>
   EOF
   )"
   ```

4. **Push to remote**:
   ```bash
   git push
   ```

5. **Verify push completed successfully**:
   ```bash
   git status
   ```

```bash
# Mark Step 14 complete
```

## COMMAND COMPLETION VERIFICATION

```bash
# Show final completion report

# Clean up tracking file
echo ""
echo "✅ NEW LEARNINGS COMMAND FULLY COMPLETED!"
```

**Expected final output:**
```
========================================
   PROGRESS STATUS: new-learnings
========================================

Progress: 14/14 steps (100%)

[████████████████████] 100%

Step Details:
  ✓ Step 1: Identify New Learnings
  ✓ Step 2: Determine KB Article Strategy
  ... (all 14 steps shown as ✓)

✅ ALL STEPS COMPLETE!

✓ Tracking file removed: /tmp/new-learnings-progress.txt

✅ NEW LEARNINGS COMMAND FULLY COMPLETED!
```

## Documentation Validation Details

The enhanced documentation system ensures:
- **Real code**: All examples use actual AgeRun types and functions
- **Validation**: Automated checking prevents hypothetical examples
- **Consistency**: Same standards across all documentation
- **Maintenance**: Changes to code automatically flag outdated docs

**Available for reference:**
- Real types and functions: `python3 scripts/get_real_types.py --guide`
- Complete type list: `python3 scripts/get_real_types.py --types`
- Complete function list: `python3 scripts/get_real_types.py --functions`
- Type mappings: See `scripts/batch_fix_docs.py` TYPE_REPLACEMENTS

## Knowledge Base Benefits

The enhanced documentation system provides:
- **AGENTS.md**: Concise, actionable guidelines for quick reference
- **./kb/ files**: Detailed context with VALIDATED examples
- **Real code**: All examples work with actual AgeRun codebase
- **Searchable**: `grep -r "keyword" ./kb/` finds relevant patterns
- **Linked**: Guidelines link to detailed articles when more context needed
- **Validated**: `make check-docs` ensures all references are correct

## Execution Checklist

Use this checklist to ensure thorough execution:

- [ ] **Step 1**: Identified multiple learnings from deep analysis
- [ ] **Step 2**: Searched for existing KB articles to update
- [ ] **Step 3**: Created/updated KB articles with real AgeRun code
- [ ] **Step 4**: Ran `make check-docs` and fixed all validation errors
- [ ] **Step 5**: Added new articles to kb/README.md index
- [ ] **Step 6**: ✅ Updated 3-5+ existing KB articles with cross-references
- [ ] **Step 7**: ✅ Updated 3-4+ commands with new KB references
- [ ] **Step 8**: Reviewed AGENTS.md for update locations
- [ ] **Step 9**: Updated AGENTS.md with new KB references
- [ ] **Step 10**: ✅ Updated CHANGELOG.md with session summary
- [ ] **Step 11**: Validated no broken links exist
- [ ] **Step 12**: ✅ Executed verification script showing "READY TO COMMIT"
- [ ] **Step 14**: Committed and pushed all changes

**Remember**: Steps 6, 7, 10, and 12 are the most commonly under-executed. Ensure these are done thoroughly.

The goal is to create learnings with working code examples that developers can actually use, making future sessions more efficient and error-free.