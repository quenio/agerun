#!/usr/bin/env python3
"""Fix ar_system__create(NULL) calls to ar_system__create()"""

import os
import sys

def update_file(filepath):
    """Update a file to fix ar_system__create calls"""
    with open(filepath, 'r') as f:
        content = f.read()
    
    original_content = content
    
    # Replace ar_system__create(NULL) with ar_system__create()
    content = content.replace('ar_system__create(NULL)', 'ar_system__create()')
    
    # Write back if changed
    if content != original_content:
        with open(filepath, 'w') as f:
            f.write(content)
        return True
    return False

def main():
    # Check we're in the right directory
    if not os.path.exists('modules') or not os.path.exists('scripts'):
        print("ERROR: This script must be run from the repository root directory")
        print("Current directory:", os.getcwd())
        sys.exit(1)
    
    # Files that need fixing
    files_to_fix = [
        'modules/ar_method_fixture.c',
        'modules/ar_method_tests.c',
        'modules/ar_instruction_evaluator_tests.c',
        'modules/ar_executable_tests.c',
        'modules/ar_methodology_tests.c',
        'modules/ar_system_fixture.c'
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