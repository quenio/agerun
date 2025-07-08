#!/usr/bin/env python3
"""
Fix unused len variable in test files
"""

import os
import re

def fix_unused_len(filepath):
    """Remove unused len variable from test files."""
    with open(filepath, 'r') as f:
        content = f.read()
    
    # Pattern to find and remove the len variable declaration and usage
    pattern = r'(\s*)size_t len = strlen\(cwd\);\n'
    
    if re.search(pattern, content):
        new_content = re.sub(pattern, '', content)
        
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
        if fix_unused_len(filepath):
            print(f"Fixed: {filepath}")
            updated_count += 1
    
    print(f"\nFixed {updated_count} files")

if __name__ == '__main__':
    main()