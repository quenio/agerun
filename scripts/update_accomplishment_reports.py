#!/usr/bin/env python3
"""
Update all command files to include detailed evidence requirements in accomplishment report sections.
"""

import re
import sys
from pathlib import Path

# Files already updated manually
UPDATED_FILES = {
    '.opencode/command/ar/commit.md',
    '.opencode/command/ar/execute-plan.md'
}

# Pattern to find accomplishment report sections
ACCOMPLISHMENT_REPORT_PATTERN = r'Accomplishment Report:\s*\n\[Report what was accomplished with evidence:.*?\]"'

# Replacement text with detailed evidence requirements
REPLACEMENT = '''Accomplishment Report:
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

See [kb/sub-agent-verification-pattern.md](../../../kb/sub-agent-verification-pattern.md) for complete evidence requirements and examples.]"'''

# Pattern to find "Report accomplishments with evidence" sections
REPORT_SECTION_PATTERN = r'(1\. \*\*Report accomplishments with evidence\*\*\s*\n\s*- Describe what was accomplished.*?\n\s*- \*\*DO NOT\*\* tell step-verifier what to verify - report what was done)'

# Replacement for report section
REPORT_REPLACEMENT = '''1. **Report accomplishments with concrete evidence**
   - Describe what was accomplished (files created/modified, commands executed, outputs produced)
   - Provide **concrete evidence**: actual file paths with line numbers, full command outputs, git diff output, test results with specific test names, grep/search output proving claims
   - **DO NOT** tell step-verifier what to verify - report what was done with evidence
   - **DO NOT** use vague summaries - provide specific details (see [kb/sub-agent-verification-pattern.md](../../../kb/sub-agent-verification-pattern.md) for examples)'''

# Pattern to find CRITICAL section
CRITICAL_PATTERN = r'\*\*CRITICAL\*\*:\s*\n- Report accomplishments with evidence, NOT instructions\s*\n- The step-verifier independently verifies.*?\n- If step-verifier reports.*?'

# Replacement for CRITICAL section
CRITICAL_REPLACEMENT = '''**CRITICAL**: 
- Report accomplishments with **concrete evidence** (file paths, line numbers, command outputs, git diff, test results), NOT instructions or vague summaries
- The step-verifier independently verifies by reading command files, checking files, git status/diff, etc.
- If step-verifier reports "⚠️ STOP EXECUTION", you MUST fix issues before proceeding
- If accomplishment report lacks concrete evidence, step-verifier will STOP execution and require evidence'''

def update_file(file_path):
    """Update a single command file."""
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            content = f.read()
        
        original_content = content
        
        # Update accomplishment report section
        content = re.sub(
            ACCOMPLISHMENT_REPORT_PATTERN,
            REPLACEMENT,
            content,
            flags=re.DOTALL
        )
        
        # Update report section
        content = re.sub(
            REPORT_SECTION_PATTERN,
            REPORT_REPLACEMENT,
            content,
            flags=re.DOTALL
        )
        
        # Update CRITICAL section (only if it matches the old pattern)
        if 'Report accomplishments with evidence, NOT instructions' in content:
            content = re.sub(
                CRITICAL_PATTERN,
                CRITICAL_REPLACEMENT,
                content,
                flags=re.DOTALL
            )
        
        if content != original_content:
            with open(file_path, 'w', encoding='utf-8') as f:
                f.write(content)
            return True
        return False
    except Exception as e:
        print(f"Error updating {file_path}: {e}", file=sys.stderr)
        return False

def main():
    """Update all command files."""
    command_dir = Path('.opencode/command/ar')
    
    if not command_dir.exists():
        print(f"Error: {command_dir} does not exist", file=sys.stderr)
        sys.exit(1)
    
    updated_count = 0
    skipped_count = 0
    
    for file_path in command_dir.glob('*.md'):
        if str(file_path) in UPDATED_FILES:
            skipped_count += 1
            continue
        
        if update_file(file_path):
            print(f"Updated: {file_path}")
            updated_count += 1
        else:
            print(f"No changes needed: {file_path}")
    
    print(f"\nSummary: {updated_count} files updated, {skipped_count} files skipped (already updated)")
    
    if updated_count > 0:
        sys.exit(0)
    else:
        sys.exit(1)

if __name__ == '__main__':
    main()

