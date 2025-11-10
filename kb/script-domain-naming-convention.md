# Script Domain Naming Convention

## Learning

Utility scripts should include their domain in the filename to clarify purpose, enable discovery, and improve code organization. This is especially important for extracted scripts that may be reused across multiple commands.

## Importance

**Without domain naming:**
- Script purpose is ambiguous (e.g., `validate-structure.sh` - validate what structure?)
- Developers don't know where to find relevant scripts
- Script directories become cluttered and hard to navigate
- Reusability is reduced because scripts are hard to discover
- Script names may conflict across different domains

**With domain naming:**
- Script purpose is immediately clear (e.g., `validate-command-structure.sh`)
- Developers can search and discover related scripts easily
- Naming conventions prevent conflicts
- Scripts are self-documenting through their names
- Easier to maintain and extend utility collections

## Pattern: Domain-Specific Naming

### Naming Structure

Use the pattern: `action-domain-object.sh`

Components:
- **action**: What the script does (`scan`, `validate`, `calculate`, `identify`, `generate`, etc.)
- **domain**: The domain/context being operated on (`command`, `test`, `module`, `kb`, etc.)
- **object**: The specific thing being operated on (optional, can be combined with domain)

### Action Verbs

Standard action prefixes for consistency:

| Prefix | Purpose | Examples |
|--------|---------|----------|
| `scan-` | Find and enumerate things | `scan-commands.sh`, `scan-modules.sh` |
| `validate-` | Check structure/format | `validate-command-structure.sh`, `validate-kb-links.sh` |
| `calculate-` | Compute metrics/scores | `calculate-command-scores.sh`, `calculate-complexity.sh` |
| `identify-` | Analyze and categorize | `identify-command-issues.sh`, `identify-patterns.sh` |
| `generate-` | Create output/reports | `generate-command-report.sh`, `generate-metrics.sh` |
| `verify-` | Check correctness | `verify-links.sh`, `verify-integration.sh` |
| `check-` | General validation | `check-naming-conventions.sh`, `check-syntax.sh` |
| `analyze-` | Deep analysis | `analyze-whitelist.sh`, `analyze-distribution.sh` |
| `detect-` | Find problems/anti-patterns | `detect-code-smells.sh`, `detect-circular-dependencies.sh` |

### Domain Examples

Common domains in AgeRun:
- **command**: Claude Code commands (extracted scripts, validation, reporting)
- **module**: C modules (tests, structure, dependencies)
- **test**: Testing infrastructure (coverage, execution, validation)
- **kb**: Knowledge base (links, structure, cross-references)
- **doc**: Documentation (validation, consistency, completeness)
- **log**: Log analysis and processing
- **build**: Build system and CI

## Example: Check-Commands Refactoring

This learning emerged from extracting shell scripts from command files:

### Before (Generic Names)
```
scripts/scan.sh               # Unclear: scan what?
scripts/validate-structure.sh # Vague: validate what structure?
scripts/calculate-scores.sh   # Ambiguous: what kind of scores?
scripts/identify-issues.sh    # Generic: issues in what?
scripts/generate-report.sh    # Unclear: report about what?
```

### After (Domain-Specific Names)
```
scripts/scan-commands.sh                  # Clear: scan command files
scripts/validate-command-structure.sh     # Specific: validate command doc structure
scripts/calculate-command-scores.sh       # Precise: calculate quality scores for commands
scripts/identify-command-issues.sh        # Clear: identify quality issues in commands
scripts/generate-command-report.sh        # Specific: generate command quality report
```

### Benefits Realized

From the refactoring:
- All scripts' purposes are immediately clear from filenames
- Related scripts cluster together in directory listing
- Developers can grep for `command` to find all command-related utilities
- Scripts are discoverable: "which script analyzes commands?" → grep `command-*.sh`
- No ambiguity about what each script does

## Implementation Pattern

When extracting scripts from commands:

1. **Identify the domain** from the command name
   - `check-naming` → domain is "naming"
   - `analyze-changes` → domain is "changes"
   - `review-logs` → domain is "logs"

2. **Identify the action** the script performs
   - Scanning → `scan-domain.sh`
   - Validating → `validate-domain.sh`
   - Calculating → `calculate-domain.sh`

3. **Create the filename** following the pattern
   - `scripts/scan-commands.sh`
   - `scripts/validate-command-structure.sh`
   - `scripts/calculate-command-scores.sh`

4. **Update the command** to reference the new name
   ```markdown
   ```bash
   ./scripts/scan-commands.sh
   ```
   ```

5. **Document in comments** what the script does
   ```bash
   #!/bin/bash
   # Scan for all command files in .opencode/command/ar
   # Usage: ./scripts/scan-commands.sh
   ```

## Discovering Related Scripts

With domain-specific naming, finding related scripts is straightforward:

```bash
# Find all command-related scripts
ls -1 scripts/\*-command-\*.sh scripts/\*-commands.sh

# Find all validation scripts
ls -1 scripts/validate-\*.sh

# Find all scripts for a specific domain
grep -l "domain_name" scripts/*.sh
```

## Naming Anti-Patterns

❌ **WRONG**: Single-word action names (too generic)
```
scripts/check.sh        # Too broad
scripts/validate.sh     # Unclear what's being validated
scripts/run.sh          # Meaningless
```

❌ **WRONG**: No domain specification
```
scripts/score-calculator.sh    # What kind of scores?
scripts/structure-verifier.sh  # Structure of what?
scripts/issue-finder.sh        # Issues in what domain?
```

❌ **WRONG**: Over-qualified with full context
```
scripts/validate-claude-code-command-documentation-structure.sh  # Too long!
```

✅ **CORRECT**: Clear action + specific domain
```
scripts/calculate-command-scores.sh       # Clear and concise
scripts/validate-command-structure.sh     # Specific domain
scripts/identify-command-issues.sh        # Action + domain
scripts/scan-commands.sh                  # Noun form when enumeration
```

## Python Script Naming Enforcement

**CRITICAL**: Python scripts MUST use underscores per PEP 8, while bash scripts use dashes. This is enforced by `make check-naming`.

**Naming conventions:**
- **Bash scripts**: Use dashes (POSIX convention) - `checkpoint-init.sh`, `add-newline.sh`
- **Python scripts**: Use underscores (PEP 8 convention) - `check_commands.py`, `batch_fix_docs.py`
- **DO NOT MIX**: Never use dashes in Python files or underscores in bash files

**Enforcement:**
```bash
# check-naming validates script naming
make check-naming

# Fails if Python scripts use dashes:
# ✗ Found 4 Python scripts not following underscore naming convention
#     add-step-verification-sections.py (should be: add_step_verification_sections.py)
```

**Example from session:**
```bash
# Fixed 4 Python scripts renamed to use underscores:
# add-step-verification-sections.py → add_step_verification_sections.py
# fix-duplicate-verification.py → fix_duplicate_verification.py
# remove-duplicate-step8-verification.py → remove_duplicate_step8_verification.py
# update-execute-plan-verification.py → update_execute_plan_verification.py
```

## Related Patterns

- [Command File as Orchestrator Pattern](command-orchestrator-pattern.md) - Why domain-specific naming enables discovery in orchestration
- [Command Helper Script Extraction Pattern](command-helper-script-extraction-pattern.md) - When and how to extract scripts (one per block)
- [Function Naming State Change Convention](function-naming-state-change-convention.md) - Naming conventions for functions
- [Single Responsibility Principle](single-responsibility-principle.md) - Keeping scripts focused and discoverable
- [Separation of Concerns Principle](separation-of-concerns-principle.md) - Script organization
