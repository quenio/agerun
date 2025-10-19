# Command Helper Script Extraction Pattern

## Learning

When commands contain multi-line embedded bash logic for verification, analysis, or automation, extract it into standalone helper scripts in the `scripts/` directory. Commands should reference these scripts rather than embedding the logic inline.

## Importance

**Without Helper Script Extraction:**
- Embedded bash code is hard to test in isolation
- Logic cannot be reused across commands
- Commands become bloated and hard to maintain
- Debugging requires running entire command workflow
- No standalone documentation for verification logic

**With Helper Script Extraction:**
- Scripts are testable, reusable, and maintainable
- Commands remain focused on workflow orchestration
- Verification logic can be run independently
- Easier to debug and enhance over time
- Self-documenting through usage headers

## Pattern: Extract Complex Logic to Scripts

### When to Extract

Extract embedded bash logic to helper scripts when:

1. **Multi-line complexity** (10+ lines of bash logic)
2. **Reusable verification** (could be useful in multiple commands)
3. **Complex analysis** (parsing, pattern matching, graph building)
4. **Automation opportunities** (can be tested standalone)
5. **Non-trivial algorithms** (dependency checking, metric calculation)

### When NOT to Extract

Keep bash inline when:

1. **Simple orchestration** (single-line commands: `git diff`, `make build`)
2. **Checkpoint tracking** (workflow state management)
3. **User prompts** (manual verification steps)
4. **Context-specific** (uses command-specific variables/state)

### Extraction Discipline: One Script Per Shell Block

**CRITICAL**: Each embedded shell block should be extracted into its own focused script:

- ❌ **WRONG**: Combining multiple embedded blocks into one script
  ```bash
  # scripts/command-processor.sh (BAD: mixing multiple responsibilities)
  scan_files()   { ... }  # Scanning files
  validate_structure() { ... }  # Validating structure
  calculate_scores() { ... }  # Computing scores
  ```

- ✅ **CORRECT**: Extract each shell block to separate, focused scripts
  ```bash
  # scripts/scan-commands.sh       - Only scanning
  # scripts/validate-command-structure.sh - Only validation
  # scripts/calculate-command-scores.sh   - Only calculation
  ```

**Benefits of focused extraction:**
1. Each script does ONE thing (Single Responsibility Principle)
2. Scripts are independently testable
3. Scripts can be reused in different contexts
4. Updates to one function don't affect others
5. Clear naming communicates exact purpose
6. Easier to maintain and debug

**Real example from check-commands refactoring**:
- 5 embedded blocks → 5 separate scripts, each with one responsibility
- Each script name describes exactly what it does
- No monolithic script trying to do everything

### Helper Script Structure

Every helper script should follow this pattern:

```bash
#!/bin/bash
# Brief description of what the script does
# Usage: ./scripts/script-name.sh [args...]
#
# Purpose:
# - What this script checks/verifies/analyzes
# - What problems it detects
#
# Returns:
# - Exit 0 if check passes / no issues found
# - Exit 1 if violations / issues detected
# - Outputs results with file:line references

set -e  # Fail fast on errors

# Argument parsing with defaults
ARG1=${1:-default}

# Validation
if [ $# -lt 1 ]; then
    echo "Usage: $0 <arg1> [arg2]"
    echo ""
    echo "Example: $0 modules test"
    exit 1
fi

ISSUES_FOUND=0

echo "Script Name"
echo "========================================"
echo ""

# Step 1: First check
echo "1. Check Description"
echo "   Details..."
echo ""

# ... verification logic ...

if [ "$VIOLATIONS" -eq 0 ]; then
    echo "   ✅ Check passed"
else
    echo "   ❌ Found $VIOLATIONS issue(s)"
    ISSUES_FOUND=$((ISSUES_FOUND + VIOLATIONS))
fi
echo ""

# Step 2: Additional checks
# ... more steps ...

# Summary
echo "========================================"
echo "Summary:"
echo "  Check 1:     $VIOLATIONS1"
echo "  Check 2:     $VIOLATIONS2"
echo "  Total:       $ISSUES_FOUND"
echo ""

if [ "$ISSUES_FOUND" -eq 0 ]; then
    echo "✅ All checks passed"
    exit 0
else
    echo "❌ Found $ISSUES_FOUND issue(s)"
    echo ""
    echo "Recommendations:"
    echo "  • Actionable fix suggestion 1"
    echo "  • Actionable fix suggestion 2"
    echo ""
    echo "Related: kb/related-pattern.md"
    exit 1
fi
```

### Script Naming Conventions

Use descriptive verb-domain patterns to improve discoverability and reusability:

**Pattern**: `<action>-<domain>-<object>.sh` (domain is key for script discovery)

**Action verbs**:
- `detect-*.sh` - Finding issues (detect-code-smells.sh, detect-circular-dependencies.sh)
- `verify-*.sh` - Validating correctness (verify-memory-management.sh, verify-test-coverage.sh)
- `check-*.sh` - General checking (check-naming-conventions.sh, check-file-hygiene.sh)
- `analyze-*.sh` - Analysis tasks (analyze-whitelist.sh, analyze-changelog-patterns.sh)
- `validate-*.sh` - Schema/structure validation (validate-kb-links.sh, validate-tdd-plan.sh)
- `scan-*.sh` - Enumeration/discovery (scan-commands.sh, scan-modules.sh)
- `calculate-*.sh` - Metric computation (calculate-command-scores.sh)
- `identify-*.sh` - Categorization (identify-command-issues.sh)
- `generate-*.sh` - Output creation (generate-command-report.sh)

**Domain inclusion** ([details](script-domain-naming-convention.md)):
- Always include domain for discoverability: `validate-command-structure.sh` ✅ not `validate-structure.sh` ❌
- Domain enables grep-based script discovery: `grep command scripts/*.sh` finds all command-related scripts
- Real example from check-commands refactoring:
  - ❌ WRONG: `step-1-scan.sh`, `step-2-validate.sh` (ordinal prefix, no domain)
  - ✅ CORRECT: `scan-commands.sh`, `validate-command-structure.sh` (domain, no ordinal prefix)

**Anti-patterns to avoid**:
- ❌ Using step-N prefixes: `step-1-script.sh`, `step-2-script.sh` (prevents reuse, implies ordering)
- ❌ Generic names without domain: `validate-structure.sh` (too vague - structure of what?)
- ❌ Over-qualified names: `validate-claude-code-command-documentation-structure.sh` (too long)

## Example: Code Smell Detection

### Before (Embedded in Command)

````markdown
#### Step 2: Code Smells Detection

```bash
# Check for long methods
for file in modules/*.c; do
    awk '/^[a-zA-Z_][a-zA-Z0-9_]*.*\(.*\).*\{/ {
        func_start = NR
        brace_count = 1
        next
    }
    /\{/ { brace_count++ }
    /\}/ {
        brace_count--
        if (brace_count == 0 && func_start > 0) {
            length = NR - func_start
            if (length > 50) {
                print FILENAME ":" func_start " - Long method: " length " lines"
            }
            func_start = 0
        }
    }' "$file"
done

# Check for large modules
for file in modules/*.c; do
    lines=$(wc -l < "$file")
    if [ "$lines" -gt 850 ]; then
        echo "❌ $file - $lines lines (limit: 850)"
    fi
done

# ... more embedded checks ...

make checkpoint-update CMD=review-changes STEP=2
```
````

**Problems:**
- 30+ lines of embedded bash
- Can't test independently
- Can't reuse in other commands
- Hard to enhance or debug
```

### After (Extracted to Helper Script)

**Command file:**
```markdown
#### Step 2: Code Smells Detection

Scans for known issues and anti-patterns:
- Long methods (>50 lines)
- Large modules (>850 lines)
- Excessive parameters (>5)
- Code duplication

**Helper script available:**
```bash
./scripts/detect-code-smells.sh [path]
# Returns: Exit 1 if code smells found, 0 if clean
```

```bash
make checkpoint-update CMD=review-changes STEP=2
```
\`\`\`

**Benefits:**
- Command is concise and readable
- Script is testable: `./scripts/detect-code-smells.sh modules`
- Logic is documented in the script
- Can be reused in other commands
- Easy to enhance independently
```

## Real-World Examples

From the AgeRun project, we extracted 23+ helper scripts:

### Check-Commands Command (5 scripts) - Session 2025-10-18
- `scan-commands.sh` - Scans .opencode/command/ar directory for all command files
- `validate-command-structure.sh` - Runs structure validation via check_commands.py
- `calculate-command-scores.sh` - Extracts average scores and runs quality gates
- `identify-command-issues.sh` - Analyzes command distribution by quality tier
- `generate-command-report.sh` - Generates final quality report with status

This extraction from check-commands.md reduced the command documentation by 92 lines while maintaining full functionality. Each script has a single responsibility following the [Script Domain Naming Convention](script-domain-naming-convention.md).

### Check-Docs Command - Anti-Pattern Correction - Session 2025-10-18

**Initial extraction (ANTI-PATTERN)**:
- Created `run-check-docs.sh` as monolithic wrapper (180+ lines)
- Combined all 5 steps into single script
- check-docs.md just called run-check-docs.sh
- Orchestration logic was hidden
- Violated extraction discipline

**Corrected extraction (CORRECT PATTERN)**:
- Created 5 focused scripts:
  - `validate-docs.sh` - Run validation only
  - `preview-doc-fixes.sh` - Preview fixes only
  - `apply-doc-fixes.sh` - Apply fixes only
  - `verify-docs.sh` - Verify fixes only
  - `commit-docs.sh` - Commit changes only
- Deleted run-check-docs.sh entirely
- check-docs.md shows all steps directly with orchestration logic
- Each script does ONE thing
- Command file is the orchestrator (not wrapper script) - see [Command File as Orchestrator Pattern](command-orchestrator-pattern.md)

**Key lesson**: Never combine multiple shell blocks into one monolithic wrapper. Each block = one script. Command file = orchestrator.

### Check-Naming Command (2 scripts) - Session 2025-10-18

**Extraction applied in Session 2f**:
- `run-naming-check.sh` - Single responsibility: execute check and capture violations
- `check-naming-conditional-flow.sh` - Single responsibility: implement conditional flow (skip analysis if no violations)

**Application of extraction discipline**:
- Each 10+ line embedded bash block extracted to its own script
- NOT combined into one wrapper - maintained single responsibilities
- check-naming.md simplified to show orchestration logic directly
- Checkpoint workflow tested end-to-end with conditional step skipping
- Verified proper script naming using action-verb patterns: `run-*` for execution, conditional logic name for logic

**Pattern verification**: Demonstrates that extraction discipline prevents anti-patterns. When user feedback indicated embedded logic violations, extraction fixed them by creating focused scripts, not monolithic wrappers.

### Review-Changes Command (6 scripts)
- `detect-code-smells.sh` - Finds long methods, large modules, excessive params
- `verify-memory-management.sh` - Checks ownership prefixes, heap tracking
- `check-naming-conventions.sh` - Validates ar_module__function pattern
- `verify-test-coverage.sh` - Verifies BDD structure, AR_ASSERT usage
- `detect-circular-dependencies.sh` - Builds dependency graph, detects cycles
- `check-file-hygiene.sh` - Finds backup, temp, debug files

### New-Learnings Command (1 script) - Session 2025-10-18

**Extraction applied in Session 2f**:
- `check-new-learnings-checkpoint.sh` - Single responsibility: check for existing checkpoint and show status/initialization options

**Pattern demonstration**: Shows how even small checkpoint detection logic (10+ lines) should be extracted. The script detects if checkpoint tracking exists and provides appropriate guidance, making the command file cleaner while maintaining all orchestration visibility.

### Other Commands (12+ scripts)
- `categorize-log-errors.sh` - From check-logs.md
- `verify-new-learnings-integration.sh` - From new-learnings.md
- `validate-kb-links.sh` - From compact-guidelines.md
- `analyze-whitelist.sh` - From fix-errors-whitelisted.md
- `check-module-dependencies.sh` - From migrate-module-to-zig-struct.md
- Plus 7+ more...

### Command Updates

Update command files to reference the helper scripts:

```markdown
**Helper script available:**
```bash
./scripts/script-name.sh [args]
# Auto-detects context from git if no args provided
# Returns: Exit 1 if violations found, 0 if clean
```
\`\`\`

This provides:
- Clear usage example
- Expected behavior description
- Exit code semantics
```

## Implementation Checklist

When extracting embedded logic:

- [ ] Identify multi-line bash blocks (10+ lines) - **Each block gets its own script**
- [ ] **DO NOT combine multiple blocks** into one script - each has one responsibility
- [ ] Determine if logic is reusable/testable
- [ ] Create script in `scripts/` with descriptive domain-specific name
- [ ] Add usage header with examples
- [ ] Implement defensive programming (set -e, validation)
- [ ] Provide clear output with emojis (✅, ❌, ⚠️)
- [ ] Use exit 0 for success, exit 1 for failures
- [ ] Make script executable (chmod +x)
- [ ] Test script independently
- [ ] Update command file to reference extracted script (replace only the shell block it came from)
- [ ] Update related KB articles if needed

## Script Quality Standards

All helper scripts must follow:

1. **Exit Code Discipline**
   - Exit 0: Success, no issues found
   - Exit 1: Violations or errors detected
   - Never exit with other codes

2. **Defensive Programming**
   - Use `set -e` to fail fast
   - Validate all arguments
   - Provide usage help
   - Handle missing files gracefully

3. **User-Friendly Output**
   - Clear section headers
   - Progress indicators
   - Summary at end
   - Emoji status markers
   - Actionable recommendations

4. **Shell-Sourceable Results**
   - Output KEY=value pairs when appropriate
   - Allows automation: `source /tmp/results.txt`
   - Example: `ERROR_COUNT=5`

5. **Documentation**
   - Usage header with examples
   - Clear purpose statement
   - Parameter descriptions
   - Exit code documentation

## Testing Helper Scripts

Test scripts independently before integrating:

```bash
# Test on sample input
./scripts/detect-code-smells.sh modules/

# Test with different arguments
./scripts/verify-memory-management.sh modules/ar_assert.c

# Test error handling
./scripts/check-naming-conventions.sh nonexistent-file

# Verify exit codes
./scripts/check-file-hygiene.sh . && echo "PASSED" || echo "FAILED"
```

## Metrics

Track extraction progress:

```markdown
## Statistics

- Total commands processed: 10/10 (100%)
- Scripts extracted: 18
- Total LoC in scripts: 2,500+
- Commands improved: 10
- Reusability factor: 2.1x (some scripts used by multiple commands)
```

## Benefits Realized

From real extraction work:

1. **Testability**: All scripts tested independently
2. **Reusability**: Scripts used across multiple commands
3. **Maintainability**: 1,235 lines extracted from review-changes.md alone
4. **Debuggability**: Issues found and fixed in isolation
5. **Documentation**: Self-documenting through usage headers
6. **Quality**: Consistent patterns across all scripts

## Anti-Pattern: Over-Extraction

❌ **WRONG** - Extracting simple orchestration:

```bash
# Don't extract this - it's simple workflow
./scripts/run-git-diff.sh  # Just calls "git diff"
./scripts/checkpoint-update.sh  # Just calls "make checkpoint-update"
```

✅ **CORRECT** - Keep simple commands inline:

```markdown
```bash
git diff
make checkpoint-update CMD=review-changes STEP=1
```
\`\`\`
```

## Related Patterns

- [Command File as Orchestrator Pattern](command-orchestrator-pattern.md) - Why command files are orchestrators, not wrapper scripts
- [Command Documentation Excellence Gate](command-documentation-excellence-gate.md) - Quality standards for commands
- [Checkpoint Implementation Guide](checkpoint-implementation-guide.md) - Structuring commands with checkpoints
- [Command Thoroughness Requirements Pattern](command-thoroughness-requirements-pattern.md) - Thoroughness standards
- [Script Domain Naming Convention](script-domain-naming-convention.md) - Naming conventions for extracted scripts
- [Single Responsibility Principle](single-responsibility-principle.md) - Keeping scripts focused
- [Concise Script Output Principle](concise-script-output-principle.md) - Script output guidelines
- [Build System Exit Code Verification](build-system-exit-code-verification.md) - Exit code patterns
- [Command Orchestrator and Checkpoint Separation](command-orchestrator-checkpoint-separation.md) - How to separate checkpoint concerns from scripts
- [Helper Script Reusability Pattern](helper-script-reusability-pattern.md) - Decoupling scripts for cross-workflow use
