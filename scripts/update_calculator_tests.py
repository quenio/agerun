#!/usr/bin/env python3
"""Update calculator tests to use fixture's agency."""

import re
import sys

def update_file(filepath):
    with open(filepath, 'r') as f:
        content = f.read()
    
    # Pattern to find the agent creation
    pattern = r'(\s*)// Create calculator agent with initial memory\n(\s*)int64_t calc_agent = ar_agency__create_agent\("calculator", "1\.0\.0", own_initial_memory\);'
    
    replacement = r'\1// Get the fixture\'s agency\n\1ar_agency_t *mut_agency = ar_method_fixture__get_agency(own_fixture);\n\1assert(mut_agency != NULL);\n\1\n\1// Create calculator agent with initial memory using the fixture\'s agency\n\2int64_t calc_agent = ar_agency__create_agent_with_instance(mut_agency, "calculator", "1.0.0", own_initial_memory);'
    
    # Replace all occurrences
    new_content = re.sub(pattern, replacement, content)
    
    # Update send_to_agent calls
    pattern2 = r'ar_agency__send_to_agent\(calc_agent,'
    replacement2 = r'ar_agency__send_to_agent_with_instance(mut_agency, calc_agent,'
    new_content = re.sub(pattern2, replacement2, new_content)
    
    # Write back
    with open(filepath, 'w') as f:
        f.write(new_content)
    
    print(f"Updated {filepath}")

if __name__ == "__main__":
    update_file("methods/calculator_tests.c")