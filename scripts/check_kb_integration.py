#!/usr/bin/env python3
"""
Verify KB article integration is thorough.

This script checks:
1. Minimum number of KB articles modified with cross-references
2. Minimum number of commands updated with KB references
3. CLAUDE.md updated with new KB references
4. Overall integration quality score
"""

import os
import re
import sys
import subprocess
from pathlib import Path
from typing import Dict, List, Tuple

# Minimum requirements for thorough integration
MIN_KB_MODIFIED = 3
MIN_COMMANDS_MODIFIED = 3
MIN_CLAUDE_REFS = 1

def run_git_command(cmd: List[str]) -> str:
    """Run a git command and return output."""
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, check=True)
        return result.stdout.strip()
    except subprocess.CalledProcessError:
        return ""

def get_modified_files() -> Dict[str, List[str]]:
    """Get all modified files categorized by type."""
    files = {
        'kb': [],
        'commands': [],
        'claude_md': False,
        'all': []
    }
    
    # Get all modified files
    output = run_git_command(["git", "diff", "--name-only"])
    if not output:
        return files
    
    for filepath in output.split('\n'):
        if not filepath:
            continue
            
        files['all'].append(filepath)
        
        if filepath.startswith('kb/') and filepath.endswith('.md'):
            files['kb'].append(filepath)
        elif filepath.startswith('.claude/commands/') and filepath.endswith('.md'):
            files['commands'].append(filepath)
        elif filepath == 'CLAUDE.md':
            files['claude_md'] = True
    
    return files

def check_kb_cross_references(kb_files: List[str]) -> Tuple[int, List[str]]:
    """Check that modified KB files have new cross-references added."""
    issues = []
    valid_count = 0
    
    for filepath in kb_files:
        if not os.path.exists(filepath):
            continue
            
        # Check git diff for added cross-references
        diff = run_git_command(["git", "diff", filepath])
        
        # Look for added lines with KB links
        added_refs = re.findall(r'^\+.*\[([^\]]+)\]\(([^)]+\.md)\)', diff, re.MULTILINE)
        
        if added_refs:
            valid_count += 1
            print(f"  ✓ {os.path.basename(filepath)}: {len(added_refs)} cross-references added")
        else:
            issues.append(f"{os.path.basename(filepath)}: No cross-references added")
            print(f"  ⚠️ {os.path.basename(filepath)}: No cross-references added")
    
    return valid_count, issues

def check_command_kb_references(cmd_files: List[str]) -> Tuple[int, List[str]]:
    """Check that modified commands have KB references added."""
    issues = []
    valid_count = 0
    
    for filepath in cmd_files:
        if not os.path.exists(filepath):
            continue
            
        # Check git diff for added KB references
        diff = run_git_command(["git", "diff", filepath])
        
        # Look for added lines with KB links (details pattern)
        added_refs = re.findall(r'^\+.*\(\[details\]\(.*kb/[^)]+\.md\)\)', diff, re.MULTILINE)
        
        if added_refs:
            valid_count += 1
            print(f"  ✓ {os.path.basename(filepath)}: {len(added_refs)} KB references added")
        else:
            issues.append(f"{os.path.basename(filepath)}: No KB references added")
            print(f"  ⚠️ {os.path.basename(filepath)}: No KB references added")
    
    return valid_count, issues

def check_claude_md_updates(updated: bool) -> Tuple[bool, List[str]]:
    """Check that CLAUDE.md has been updated with new KB references."""
    if not updated:
        return False, ["CLAUDE.md not updated"]
    
    # Check diff for new KB references
    diff = run_git_command(["git", "diff", "CLAUDE.md"])
    
    # Look for added KB references
    added_refs = re.findall(r'^\+.*\(\[details\]\(kb/[^)]+\.md\)\)', diff, re.MULTILINE)
    
    if added_refs:
        print(f"  ✓ CLAUDE.md: {len(added_refs)} KB references added")
        return True, []
    else:
        print(f"  ⚠️ CLAUDE.md: Updated but no KB references added")
        return False, ["CLAUDE.md updated but no KB references added"]

def calculate_integration_score(kb_count: int, cmd_count: int, claude_updated: bool) -> int:
    """Calculate integration quality score (0-100)."""
    score = 0
    
    # KB cross-references (40 points)
    kb_points = min(40, (kb_count / MIN_KB_MODIFIED) * 40)
    score += kb_points
    
    # Command updates (40 points)  
    cmd_points = min(40, (cmd_count / MIN_COMMANDS_MODIFIED) * 40)
    score += cmd_points
    
    # CLAUDE.md update (20 points)
    if claude_updated:
        score += 20
    
    return int(score)

def main():
    """Main entry point."""
    print("=== KB Integration Verification ===\n")
    
    # Get modified files
    files = get_modified_files()
    
    if not files['all']:
        print("No modified files found. Run after making changes.")
        return 0
    
    print(f"Modified files summary:")
    print(f"  KB articles: {len(files['kb'])}")
    print(f"  Commands: {len(files['commands'])}")
    print(f"  CLAUDE.md: {'Yes' if files['claude_md'] else 'No'}\n")
    
    issues = []
    
    # Check KB cross-references
    print("=== KB Cross-References ===")
    if files['kb']:
        kb_valid, kb_issues = check_kb_cross_references(files['kb'])
        issues.extend(kb_issues)
    else:
        kb_valid = 0
        print("  No KB articles modified")
    
    # Check command updates
    print("\n=== Command Updates ===")
    if files['commands']:
        cmd_valid, cmd_issues = check_command_kb_references(files['commands'])
        issues.extend(cmd_issues)
    else:
        cmd_valid = 0
        print("  No commands modified")
    
    # Check CLAUDE.md
    print("\n=== CLAUDE.md Update ===")
    claude_valid, claude_issues = check_claude_md_updates(files['claude_md'])
    issues.extend(claude_issues)
    
    # Calculate score
    score = calculate_integration_score(kb_valid, cmd_valid, claude_valid)
    
    # Final assessment
    print(f"\n=== Integration Status ===")
    print(f"Quality Score: {score}/100")
    print(f"\nMetrics:")
    print(f"  KB articles with cross-refs: {kb_valid}/{MIN_KB_MODIFIED} minimum")
    print(f"  Commands with KB refs: {cmd_valid}/{MIN_COMMANDS_MODIFIED} minimum")
    print(f"  CLAUDE.md updated: {'✓' if claude_valid else '✗'}")
    
    if kb_valid >= MIN_KB_MODIFIED and cmd_valid >= MIN_COMMANDS_MODIFIED:
        print(f"\n✅ READY TO COMMIT: Thorough integration completed (Score: {score}/100)")
        return 0
    else:
        print(f"\n⚠️ NOT READY: Need more integration (Score: {score}/100)")
        print(f"\nRequired actions:")
        if kb_valid < MIN_KB_MODIFIED:
            print(f"  - Add cross-references to {MIN_KB_MODIFIED - kb_valid} more KB articles")
        if cmd_valid < MIN_COMMANDS_MODIFIED:
            print(f"  - Update {MIN_COMMANDS_MODIFIED - cmd_valid} more commands with KB references")
        if not claude_valid:
            print(f"  - Add KB references to CLAUDE.md")
        return 1

if __name__ == "__main__":
    sys.exit(main())