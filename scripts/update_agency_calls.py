#!/usr/bin/env python3
"""Update remaining agency calls to use instance versions."""

import re
import sys

def update_file(filepath):
    with open(filepath, 'r') as f:
        content = f.read()
    
    # Update get_agent_memory calls
    pattern1 = r'ar_agency__get_agent_memory\(calc_agent\)'
    replacement1 = r'ar_agency__get_agent_memory(mut_agency, calc_agent)'
    content = re.sub(pattern1, replacement1, content)
    
    # Update get_agent_context calls
    pattern2 = r'ar_agency__get_agent_context\(calc_agent\)'
    replacement2 = r'ar_agency__get_agent_context(mut_agency, calc_agent)'
    content = re.sub(pattern2, replacement2, content)
    
    # Update destroy_agent calls
    pattern3 = r'ar_agency__destroy_agent\(calc_agent\)'
    replacement3 = r'ar_agency__destroy_agent(mut_agency, calc_agent)'
    content = re.sub(pattern3, replacement3, content)
    
    # Write back
    with open(filepath, 'w') as f:
        f.write(content)
    
    print(f"Updated {filepath}")

if __name__ == "__main__":
    update_file("methods/calculator_tests.c")