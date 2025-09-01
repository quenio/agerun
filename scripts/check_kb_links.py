#!/usr/bin/env python3
"""
Check that all KB article cross-references are valid.

This script validates:
1. All markdown links in KB articles point to existing files
2. New KB articles have proper cross-references
3. Related Patterns sections contain valid links
"""

import os
import re
import sys
from pathlib import Path
from typing import List, Tuple, Set

def find_markdown_links(filepath: str) -> List[Tuple[str, str]]:
    """Extract all markdown links from a file."""
    links = []
    with open(filepath, 'r') as f:
        content = f.read()
        # Find all markdown links: [text](file.md)
        pattern = r'\[([^\]]+)\]\(([^)]+\.md)\)'
        for match in re.finditer(pattern, content):
            text = match.group(1)
            link = match.group(2)
            # Skip EXAMPLE tagged lines
            line_start = content.rfind('\n', 0, match.start()) + 1
            line = content[line_start:content.find('\n', match.end())]
            if 'EXAMPLE:' not in line:
                links.append((text, link))
    return links

def validate_kb_links(kb_dir: str = "kb") -> Tuple[int, List[str]]:
    """Validate all links in KB articles."""
    errors = []
    kb_path = Path(kb_dir)
    
    if not kb_path.exists():
        return 1, [f"KB directory not found: {kb_dir}"]
    
    # Get all KB markdown files
    kb_files = list(kb_path.glob("*.md"))
    total_files = len(kb_files)
    total_links = 0
    
    print(f"=== KB Cross-Reference Validation ===")
    print(f"Checking {total_files} KB articles...\n")
    
    for kb_file in kb_files:
        links = find_markdown_links(str(kb_file))
        if not links:
            continue
            
        total_links += len(links)
        file_errors = []
        
        for text, link in links:
            # Resolve the link path
            if link.startswith('../'):
                # Link goes outside kb directory
                target = kb_path.parent / link[3:]
            else:
                # Link within kb directory
                target = kb_path / link
            
            if not target.exists():
                file_errors.append(f"  ✗ Broken link: [{text}]({link})")
                errors.append(f"{kb_file.name}: broken link to {link}")
        
        if file_errors:
            print(f"❌ {kb_file.name}:")
            for error in file_errors:
                print(error)
        else:
            print(f"✓ {kb_file.name}: {len(links)} links valid")
    
    print(f"\n=== Summary ===")
    print(f"Files checked: {total_files}")
    print(f"Total links: {total_links}")
    print(f"Broken links: {len(errors)}")
    
    return len(errors), errors

def check_new_articles_have_references(git_diff: bool = True) -> Tuple[int, List[str]]:
    """Check that new KB articles have cross-references."""
    if not git_diff:
        return 0, []
    
    errors = []
    
    # Get new KB files from git
    import subprocess
    try:
        result = subprocess.run(
            ["git", "diff", "--name-only", "--diff-filter=A", "kb/*.md"],
            capture_output=True, text=True, check=True
        )
        new_files = result.stdout.strip().split('\n') if result.stdout.strip() else []
    except subprocess.CalledProcessError:
        return 0, []  # Not in a git repo or no new files
    
    print(f"\n=== New Article Cross-Reference Check ===")
    
    for filepath in new_files:
        if not filepath or not os.path.exists(filepath):
            continue
            
        with open(filepath, 'r') as f:
            content = f.read()
        
        # Check for Related Patterns section
        if "## Related Patterns" not in content:
            errors.append(f"{os.path.basename(filepath)}: Missing 'Related Patterns' section")
            print(f"⚠️ {os.path.basename(filepath)}: No Related Patterns section")
        else:
            # Count links in Related Patterns
            patterns_start = content.index("## Related Patterns")
            patterns_content = content[patterns_start:]
            links = re.findall(r'\[([^\]]+)\]\(([^)]+\.md)\)', patterns_content)
            
            if len(links) < 2:
                errors.append(f"{os.path.basename(filepath)}: Only {len(links)} cross-references (need 2+)")
                print(f"⚠️ {os.path.basename(filepath)}: Only {len(links)} cross-references")
            else:
                print(f"✓ {os.path.basename(filepath)}: {len(links)} cross-references")
    
    return len(errors), errors

def main():
    """Main entry point."""
    print("=== AgeRun KB Link Validation ===\n")
    
    # Validate all KB links
    link_errors, link_messages = validate_kb_links()
    
    # Check new articles have references
    ref_errors, ref_messages = check_new_articles_have_references()
    
    total_errors = link_errors + ref_errors
    
    if total_errors > 0:
        print(f"\n❌ VALIDATION FAILED: {total_errors} issues found")
        print("\nIssues to fix:")
        for msg in link_messages + ref_messages:
            print(f"  - {msg}")
        return 1
    else:
        print(f"\n✅ VALIDATION PASSED: All KB links and cross-references are valid")
        return 0

if __name__ == "__main__":
    sys.exit(main())