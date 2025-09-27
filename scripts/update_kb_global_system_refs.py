#!/usr/bin/env python3
"""Update KB articles to remove global system API references"""

import os
import re
import sys

def update_file(filepath):
    """Update a file to remove global system API references"""
    with open(filepath, 'r') as f:
        content = f.read()
    
    original_content = content
    
    # Replace global system calls
    replacements = [
        # Direct replacements
        (r'ar_system__init\("', r'ar_system__init(own_system, "'),
        (r'ar_system__init\(NULL', r'ar_system__init(own_system, NULL'),
        (r'ar_system__shutdown\(\)', r'ar_system__shutdown(own_system)'),
        (r'ar_system__process_next_message\(\)', r'ar_system__process_next_message(own_system)'),
        (r'ar_system__process_all_messages\(\)', r'ar_system__process_all_messages(own_system)'),
        
        # For examples where system is referenced
        (r'while \(ar_system__process_next_message\(\)\)', 
         r'while (ar_system__process_next_message(own_system))'),
    ]
    
    for old_pattern, new_pattern in replacements:
        content = re.sub(old_pattern, new_pattern, content)
    
    # For agent creation patterns, use fixture or instance versions
    content = re.sub(
        r'ar_agency__create_agent\(',
        r'ar_agency__create_agent(mut_agency, ',
        content
    )
    
    # Add note about instance requirement if file was modified
    if content != original_content and 'Note: Examples assume' not in content:
        # Find a good place to add the note (after first code example)
        lines = content.split('\n')
        new_lines = []
        added_note = False
        in_code_block = False
        
        for i, line in enumerate(lines):
            new_lines.append(line)
            
            # Track code blocks
            if line.strip() == '```':
                in_code_block = not in_code_block
            
            # Add note after first code block ends
            if not added_note and not in_code_block and i > 0 and lines[i-1].strip() == '```':
                new_lines.append('')
                new_lines.append('**Note**: Examples assume `own_system`, `mut_agency`, and other instance variables are available. In practice, these would be created via fixtures or passed as parameters.')
                added_note = True
        
        content = '\n'.join(new_lines)
    
    # Write back if changed
    if content != original_content:
        with open(filepath, 'w') as f:
            f.write(content)
        return True
    return False

def main():
    # Check we're in the right directory
    if not os.path.exists('kb') or not os.path.exists('modules'):
        print("ERROR: This script must be run from the repository root directory")
        print("Current directory:", os.getcwd())
        sys.exit(1)
    
    # List of KB files that reference global system APIs
    kb_files = [
        'kb/agent-wake-message-processing.md',
        'kb/design-for-change-principle.md',
        'kb/global-instance-wrapper-anti-pattern.md',
        'kb/instruction-behavior-simplification.md',
        'kb/memory-debugging-comprehensive-guide.md',
        'kb/method-test-template.md'
    ]
    
    updated_count = 0
    
    for kb_file in kb_files:
        if not os.path.exists(kb_file):
            print(f"Warning: {kb_file} not found, skipping")
            continue
        
        print(f"Processing {kb_file}...")
        if update_file(kb_file):
            print(f"  ✓ Updated {kb_file}")
            updated_count += 1
        else:
            print(f"  - No changes needed for {kb_file}")
    
    # Also update CLAUDE.md
    print(f"Processing CLAUDE.md...")
    if update_file('CLAUDE.md'):
        print(f"  ✓ Updated CLAUDE.md")
        updated_count += 1
    
    print(f"\nSummary: Updated {updated_count} files")

if __name__ == '__main__':
    main()