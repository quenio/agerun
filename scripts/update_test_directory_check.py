#!/usr/bin/env python3
"""
Update test files to accept running from bin/ subdirectories
"""

import os
import re
import sys

def update_test_file(filepath):
    """Update a single test file with the new directory check."""
    with open(filepath, 'r') as f:
        content = f.read()
    
    # Pattern to find the old directory check - this matches the if statement
    pattern = r'if \(len < 4 \|\| strcmp\(cwd \+ len - 4, "/bin"\) != 0\) \{'
    
    # Check if this file has the old pattern
    if re.search(pattern, content):
        # Replace with a check that looks for /bin/ in the path
        new_content = re.sub(
            pattern,
            'if (!strstr(cwd, "/bin/") && !strstr(cwd, "/bin")) {',
            content
        )
        
        # Also update the error message to be more flexible
        new_content = re.sub(
            r'fprintf\(stderr, "Please run: cd bin && \./[^"]+"\);',
            'fprintf(stderr, "Please run from a bin directory\\n");',
            new_content
        )
        
        with open(filepath, 'w') as f:
            f.write(new_content)
        
        return True
    
    return False

def update_fixture_file():
    """Update the fixture file separately since it has a different pattern."""
    filepath = 'modules/ar_method_fixture.c'
    with open(filepath, 'r') as f:
        content = f.read()
    
    # Update the fixture's verify_directory function
    pattern = r'if \(len < 4 \|\| strcmp\(cwd \+ len - 4, "/bin"\) != 0\) \{'
    
    if re.search(pattern, content):
        new_content = re.sub(
            pattern,
            'if (!strstr(cwd, "/bin/") && !strstr(cwd, "/bin")) {',
            content
        )
        
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
        if update_test_file(filepath):
            print(f"Updated: {filepath}")
            updated_count += 1
    
    # Update the fixture file
    if update_fixture_file():
        print("Updated: modules/ar_method_fixture.c")
        updated_count += 1
    
    print(f"\nUpdated {updated_count} files")

if __name__ == '__main__':
    main()