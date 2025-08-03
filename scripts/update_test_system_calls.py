#!/usr/bin/env python3
"""Update test files to use fixture-based system calls."""

import os
import re

def update_file(filepath):
    """Update a single test file."""
    with open(filepath, 'r') as f:
        content = f.read()
    
    original = content
    
    # Check which fixture type is used
    if 'ar_system_fixture' in content:
        # Update system fixture calls
        content = re.sub(r'ar_system__process_next_message\(\)',
                        'ar_system_fixture__process_next_message(own_fixture)', content)
        content = re.sub(r'ar_system__process_all_messages\(\)',
                        'ar_system_fixture__process_all_messages(own_fixture)', content)
        # Also try mut_fixture variant
        content = re.sub(r'ar_system_fixture__process_next_message\(own_fixture\)',
                        'ar_system_fixture__process_next_message(mut_fixture)', 
                        content.replace('(own_fixture)', '(mut_fixture)'))
        content = re.sub(r'ar_system_fixture__process_all_messages\(own_fixture\)',
                        'ar_system_fixture__process_all_messages(mut_fixture)', 
                        content.replace('(own_fixture)', '(mut_fixture)'))
                        
    elif 'ar_method_fixture' in content:
        # Update method fixture calls
        content = re.sub(r'ar_system__process_next_message\(\)',
                        'ar_method_fixture__process_next_message(mut_fixture)', content)
        content = re.sub(r'ar_system__process_all_messages\(\)',
                        'ar_method_fixture__process_all_messages(mut_fixture)', content)
                        
    elif 'ar_instruction_fixture' in content:
        # Instruction fixture already manages system internally
        # These tests shouldn't have direct system calls
        pass
    
    if content != original:
        with open(filepath, 'w') as f:
            f.write(content)
        print(f"Updated {filepath}")

def main():
    """Update all test files."""
    test_dirs = ['modules', 'methods']
    
    for dir_name in test_dirs:
        if not os.path.exists(dir_name):
            continue
            
        for filename in os.listdir(dir_name):
            if filename.endswith('_tests.c'):
                filepath = os.path.join(dir_name, filename)
                update_file(filepath)

if __name__ == '__main__':
    main()