# Architectural Documentation Consistency Pattern

## Learning
When making architectural changes to documentation patterns (e.g., evidence requirements, verification workflows), all affected files must be updated systematically across all documentation layers: agent specifications, KB articles, command files, and main guides. Systematic updates ensure all documentation reflects the same standards and prevents confusion from inconsistent documentation.

## Importance
Inconsistent documentation creates confusion and reduces effectiveness. When architectural changes affect multiple documentation layers, updating only some layers leaves mixed old/new documentation that misleads future sessions. Systematic updates ensure all documentation reflects the same architectural decisions and standards.

## Example
**Session context**: Architectural change to require concrete evidence in accomplishment reports affected 4 documentation layers.

**Before (inconsistent updates)**:
```bash
# Updated only some layers
1. Updated `.claude/step-verifier.md` (agent specification) ✓
2. Updated `kb/sub-agent-verification-pattern.md` (KB article) ✓
3. Updated 30 command files (command documentation) ✗ (missed)
4. Updated `AGENTS.md` (main guide) ✗ (missed)
```
**Problem**: Commands still reference old pattern, main guide doesn't mention new requirements, causing confusion.

**After (systematic updates)**:
```bash
# Updated all affected layers systematically
1. Updated `.claude/step-verifier.md` (agent specification) ✓
   - Added Step 4: "MANDATORY: Validate Evidence Existence and Validity"
   - Added 8-point evidence validation checklist
   - Added STOP instructions for validation failures

2. Updated `kb/sub-agent-verification-pattern.md` (KB article) ✓
   - Expanded evidence requirements with 6 categories
   - Added GOOD vs BAD examples
   - Added "Evidence Validation by Step-Verifier" section

3. Updated all 30 command files (command documentation) ✓
   - Added mandatory evidence requirements to accomplishment report sections
   - Updated "Report accomplishments" sections with concrete evidence guidance
   - Added references to kb/sub-agent-verification-pattern.md

4. Updated `AGENTS.md` (main guide) ✓
   - Added note in "Command Step Tracking" section
   - Updated "Evidence-based completion" to specify concrete evidence requirements
```
**Result**: All documentation layers reflect same architectural decision, cross-references maintain consistency.

## Generalization
**Documentation layers for architectural changes**:

1. **Agent Specifications** (`.claude/*.md`):
   - Define how agents should behave
   - Include validation requirements and STOP instructions
   - Example: `.claude/step-verifier.md` defines evidence validation requirements

2. **KB Articles** (`kb/*.md`):
   - Document patterns and learnings
   - Provide detailed examples and implementation guidance
   - Example: `kb/sub-agent-verification-pattern.md` documents evidence requirements

3. **Command Files** (`.opencode/command/ar/*.md`):
   - Define command workflows and requirements
   - Reference KB articles for detailed guidance
   - Example: Command files reference `kb/sub-agent-verification-pattern.md`

4. **Main Guides** (`AGENTS.md`, `README.md`):
   - Provide high-level overview and quick reference
   - Link to detailed KB articles
   - Example: `AGENTS.md` references evidence requirements with link to KB article

**Systematic update process**:

1. **Identify all affected layers**: Determine which documentation layers are affected by architectural change
2. **Update in dependency order**: Update agent specs → KB articles → command files → main guides
3. **Maintain cross-references**: Ensure all layers reference same KB articles for consistency
4. **Verify completeness**: Check that all affected files are updated
5. **Validate consistency**: Run validation (e.g., `make check-docs`, `make check-commands`) to ensure consistency

## Implementation
**Systematic update workflow**:

```bash
# 1. Identify architectural change scope
ARCHITECTURAL_CHANGE="evidence requirements in accomplishment reports"

# 2. List all affected documentation layers
echo "=== Agent Specifications ==="
grep -r "$ARCHITECTURAL_CHANGE" .claude/*.md || echo "Need to update"

echo "=== KB Articles ==="
grep -r "$ARCHITECTURAL_CHANGE" kb/*.md || echo "Need to update"

echo "=== Command Files ==="
grep -r "$ARCHITECTURAL_CHANGE" .opencode/command/ar/*.md | wc -l
# Should show all command files updated

echo "=== Main Guides ==="
grep -r "$ARCHITECTURAL_CHANGE" AGENTS.md README.md || echo "Need to update"

# 3. Update in dependency order
# Step 1: Update agent specifications
vim .claude/step-verifier.md

# Step 2: Update KB articles
vim kb/sub-agent-verification-pattern.md

# Step 3: Update command files (use batch script if many files)
python3 scripts/update_accomplishment_reports.py

# Step 4: Update main guides
vim AGENTS.md

# 4. Verify completeness
git diff --stat
# Should show changes across all 4 layers

# 5. Validate consistency
make check-docs
make check-commands
```

**Cross-reference maintenance**:
- All layers reference same KB articles for detailed guidance
- KB articles reference agent specifications for implementation details
- Command files reference KB articles for pattern documentation
- Main guides reference KB articles for comprehensive information

**Example cross-reference chain**:
```
AGENTS.md → kb/sub-agent-verification-pattern.md (evidence requirements)
Command files → kb/sub-agent-verification-pattern.md (evidence requirements)
kb/sub-agent-verification-pattern.md → .claude/step-verifier.md (validation implementation)
```

## Common Consistency Failures
1. **Partial updates**: Updating only some layers, leaving others with old patterns
2. **Missing cross-references**: Not maintaining references between layers
3. **Inconsistent terminology**: Using different terms across layers for same concept
4. **Outdated examples**: Examples in one layer don't match examples in another layer
5. **Missing validation**: Not running validation to ensure consistency

## Prevention Strategy
1. **Systematic updates**: Update all affected layers in dependency order
2. **Cross-reference maintenance**: Ensure all layers reference same KB articles
3. **Validation**: Run validation (check-docs, check-commands) to ensure consistency
4. **Verification**: Verify that all affected files are updated before committing

## Related Patterns
- [Documentation Update Cascade Pattern](documentation-update-cascade-pattern.md) - Dependency order for documentation updates
- [Documentation Implementation Sync](documentation-implementation-sync.md) - Keeping documentation in sync with implementation
- [Architectural Change Verification Pattern](architectural-change-verification-pattern.md) - Systematic verification for architectural changes
- [Systematic Guideline Enhancement](systematic-guideline-enhancement.md) - Systematic updates to guidelines
- [Batch Update Script Pattern](batch-update-script-pattern.md) - Using scripts for systematic file updates

