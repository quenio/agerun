#!/usr/bin/env python3
"""Update remaining method tests to use fixture's agency"""

import os
import re
import sys

def update_test_file(filepath):
    """Update a test file to use fixture's agency"""
    with open(filepath, 'r') as f:
        content = f.read()
    
    original_content = content
    
    # Pattern to find agent creation calls
    agent_create_pattern = r'int64_t\s+(\w+)\s*=\s*ar_agency__create_agent\s*\('
    
    # Replace with instance version
    def replace_agent_create(match):
        var_name = match.group(1)
        return f'int64_t {var_name} = ar_agency__create_agent(mut_agency, '
    
    content = re.sub(agent_create_pattern, replace_agent_create, content)
    
    # Pattern to find destroy agent calls
    destroy_pattern = r'ar_agency__destroy_agent\s*\('
    content = re.sub(destroy_pattern, 'ar_agency__destroy_agent(mut_agency, ', content)
    
    # Pattern to find send to agent calls
    send_pattern = r'ar_agency__send_to_agent\s*\('
    content = re.sub(send_pattern, 'ar_agency__send_to_agent(mut_agency, ', content)
    
    # Pattern to find get agent memory calls
    memory_pattern = r'ar_agency__get_agent_memory\s*\('
    content = re.sub(memory_pattern, 'ar_agency__get_agent_memory(mut_agency, ', content)
    
    # Add agency retrieval before agent creation if not already present
    # Look for patterns where we create agents
    lines = content.split('\n')
    new_lines = []
    i = 0
    while i < len(lines):
        line = lines[i]
        
        # Check if this line creates an agent
        if 'ar_agency__create_agent' in line and i > 0:
            # Look backwards to see if we already get the agency
            found_agency = False
            for j in range(max(0, i-10), i):
                if 'ar_method_fixture__get_agency' in lines[j]:
                    found_agency = True
                    break
            
            # If we didn't find agency retrieval, add it
            if not found_agency:
                # Find the proper indentation
                indent = ''
                for char in line:
                    if char in ' \t':
                        indent += char
                    else:
                        break
                
                # Add empty line and comment
                new_lines.append('')
                new_lines.append(indent + '// Get the fixture\'s agency')
                new_lines.append(indent + 'ar_agency_t *mut_agency = ar_method_fixture__get_agency(own_fixture);')
                new_lines.append(indent + 'assert(mut_agency != NULL);')
                new_lines.append('')
        
        new_lines.append(line)
        i += 1
    
    content = '\n'.join(new_lines)
    
    # Check if file was actually modified
    if content != original_content:
        with open(filepath, 'w') as f:
            f.write(content)
        return True
    return False

def main():
    # Check we're in the right directory
    if not os.path.exists('methods') or not os.path.exists('modules'):
        print("ERROR: This script must be run from the repository root directory")
        print("Current directory:", os.getcwd())
        sys.exit(1)
    
    # List of test files to update
    test_files = [
        'methods/string_builder_tests.c',
        'methods/method_creator_tests.c',
        'methods/message_router_tests.c',
        'methods/grade_evaluator_tests.c',
        'methods/agent_manager_tests.c'
    ]
    
    updated_count = 0
    
    for test_file in test_files:
        if not os.path.exists(test_file):
            print(f"Warning: {test_file} not found, skipping")
            continue
        
        print(f"Processing {test_file}...")
        if update_test_file(test_file):
            print(f"  ✓ Updated {test_file}")
            updated_count += 1
        else:
            print(f"  - No changes needed for {test_file}")
    
    print(f"\nSummary: Updated {updated_count} test files")

if __name__ == '__main__':
    main()