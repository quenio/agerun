#!/usr/bin/env python3
"""
Fix newline issues in test files
"""

import os
import re

def fix_newline_issue(filepath):
    """Fix the newline issue in fprintf statements."""
    with open(filepath, 'r') as f:
        content = f.read()
    
    # Fix the specific pattern where we have a newline in the middle of the string
    pattern = r'fprintf\(stderr, "Please run from a bin directory\n"\);'
    replacement = 'fprintf(stderr, "Please run from a bin directory\\n");'
    
    if pattern in content:
        new_content = content.replace(pattern, replacement)
        
        with open(filepath, 'w') as f:
            f.write(new_content)
        
        return True
    
    return False

def main():
    # Find all test files
    test_files = []
    
    # Module tests
    for filename in os.listdir('modules'):
        if filename.endswith('_tests.c'):
            test_files.append(os.path.join('modules', filename))
    
    # Method tests
    if os.path.exists('methods'):
        for filename in os.listdir('methods'):
            if filename.endswith('_tests.c'):
                test_files.append(os.path.join('methods', filename))
    
    updated_count = 0
    for filepath in test_files:
        if fix_newline_issue(filepath):
            print(f"Fixed: {filepath}")
            updated_count += 1
    
    print(f"\nFixed {updated_count} files")

if __name__ == '__main__':
    main()