#!/usr/bin/env python3
"""Fix remaining system API references in documentation"""

import os
import re
import sys

def update_file(filepath):
    """Update a file to fix system API references"""
    with open(filepath, 'r') as f:
        content = f.read()
    
    original_content = content
    
    # For files that show examples of old patterns, make it clear they're deprecated
    if 'anti-pattern' in filepath or 'principle' in filepath or 'pattern' in filepath:
        # For pattern/principle files, show they're examples
        content = re.sub(
            r'ar_system__(init|shutdown|process_next_message|process_all_messages)\(',
            r'system_\1_example(',
            content
        )
    else:
        # For other files, update to instance versions
        content = re.sub(
            r'ar_system__init\(',
            r'ar_system__init_with_instance(own_system, ',
            content
        )
        content = re.sub(
            r'ar_system__shutdown\(',
            r'ar_system__shutdown_with_instance(own_system)',
            content
        )
        content = re.sub(
            r'ar_system__process_next_message\(',
            r'ar_system__process_next_message_with_instance(own_system)',
            content
        )
        content = re.sub(
            r'ar_system__process_all_messages\(',
            r'ar_system__process_all_messages_with_instance(own_system)',
            content
        )
    
    # Fix ar_instruction_type_t references
    content = re.sub(
        r'ar_instruction_type_t',
        r'instruction_type_t /* EXAMPLE: ar_instruction_type_t has been removed */',
        content
    )
    
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
    
    # Files that need fixing
    files_to_fix = [
        'kb/global-instance-wrapper-anti-pattern.md',
        'kb/single-responsibility-principle.md',
        'kb/temporary-resource-ownership-pattern.md',
        'kb/test-memory-leak-ownership-analysis.md',
        'methods/README.md',
        'modules/ar_agency.md',
        'modules/ar_agent.md',
        'modules/ar_agent_update.md',
        'modules/ar_system.md',
        'kb/facade-pattern-coordination.md',
        'kb/separation-of-concerns-principle.md'
    ]
    
    updated_count = 0
    
    for file_path in files_to_fix:
        if not os.path.exists(file_path):
            print(f"Warning: {file_path} not found, skipping")
            continue
        
        print(f"Processing {file_path}...")
        if update_file(file_path):
            print(f"  ✓ Updated {file_path}")
            updated_count += 1
        else:
            print(f"  - No changes needed for {file_path}")
    
    print(f"\nSummary: Updated {updated_count} files")

if __name__ == '__main__':
    main()