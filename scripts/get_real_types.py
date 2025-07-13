#!/usr/bin/env python3
"""
Helper script to get real types and functions from AgeRun codebase.
Used by new-learnings command to ensure kb articles use real code.
"""

import os
import re
import subprocess
from pathlib import Path

def check_repo_root():
    """Ensure script is run from repository root"""
    if not all(Path(p).exists() for p in ["Makefile", "modules", "methods"]):
        print("ERROR: This script must be run from the AgeRun repository root directory.")
        return False
    return True

def get_real_types():
    """Get all real ar_*_t types from the codebase"""
    types = set()
    
    for header_file in Path("modules").glob("*.h"):
        try:
            with open(header_file, 'r') as f:
                content = f.read()
                
            # Find typedef declarations for ar_*_t types
            type_matches = re.findall(r'typedef\s+.*?\s+(ar_\w*_t);', content)
            types.update(type_matches)
            
            # Find enum types
            enum_matches = re.findall(r'}\s+(ar_\w*_t);', content)
            types.update(enum_matches)
            
        except Exception as e:
            print(f"Warning: Could not read {header_file}: {e}")
    
    return sorted(types)

def get_real_functions():
    """Get all real ar_*__* functions from the codebase"""
    functions = set()
    
    for header_file in Path("modules").glob("*.h"):
        try:
            with open(header_file, 'r') as f:
                content = f.read()
                
            # Find function declarations
            func_matches = re.findall(r'\b(ar_\w+__\w+)\s*\(', content)
            functions.update(func_matches)
            
        except Exception as e:
            print(f"Warning: Could not read {header_file}: {e}")
    
    return sorted(functions)

def get_common_patterns():
    """Get common usage patterns"""
    return {
        "creation": [
            "ar_data__create_string()",
            "ar_data__create_integer()",
            "ar_data__create_map()",
            "ar_data__create_list()",
            "ar_agency__create_agent()",
        ],
        "destruction": [
            "ar_data__destroy()",
            "ar_agency__destroy_agent()",
        ],
        "access": [
            "ar_data__get_string()",
            "ar_data__get_integer()",
            "ar_data__get_type()",
            "ar_agency__get_agent_memory()",
        ],
        "modification": [
            "ar_data__set_map_value()",
            "ar_data__list_add()", 
            "ar_agency__send_to_agent()",
        ]
    }

def print_usage_guide():
    """Print guidance for using real types and functions"""
    print("# AgeRun Real Types and Functions Guide")
    print()
    
    print("## Most Common Types")
    types = get_real_types()
    common_types = [t for t in types if t in [
        'ar_data_t', 'ar_agent_t', 'ar_method_t', 'ar_expression_ast_t', 
        'ar_instruction_ast_t', 'ar_list_t', 'ar_map_t', 'ar_agent_registry_t'
    ]]
    
    for type_name in common_types:
        print(f"- `{type_name}*`")
    
    print(f"\n## All Available Types ({len(types)} total)")
    for type_name in types[:20]:  # Show first 20
        print(f"- `{type_name}*`")
    if len(types) > 20:
        print(f"- ... and {len(types) - 20} more")
    
    print()
    
    print("## Common Function Patterns")
    patterns = get_common_patterns()
    for category, funcs in patterns.items():
        print(f"\n### {category.title()}")
        for func in funcs:
            print(f"- `{func}`")
    
    functions = get_real_functions()
    print(f"\n## All Available Functions ({len(functions)} total)")
    for func_name in functions[:20]:  # Show first 20
        print(f"- `{func_name}()`")
    if len(functions) > 20:
        print(f"- ... and {len(functions) - 20} more")
    
    print()
    print("## Usage in Knowledge Base Articles")
    print()
    print("**Preferred approach - use real types:**")
    print("```c")
    print("ar_data_t* data = ar_data__create_string(\"example\");")
    print("ar_agency__send_to_agent(123, data);")
    print("ar_data__destroy(data);")
    print("```")
    print()
    print("**When teaching with hypothetical examples:**")
    print("```c") 
    print("ar_data_t* result = ar_example__process(data);  // EXAMPLE: Hypothetical function")
    print("fake_type_t* ptr;  // EXAMPLE: Hypothetical type for teaching")
    print("```")
    print()
    print("**For complete lists, run:**")
    print("```bash")
    print("python3 scripts/get_real_types.py --types")
    print("python3 scripts/get_real_types.py --functions") 
    print("```")

def main():
    if not check_repo_root():
        return 1
    
    import argparse
    parser = argparse.ArgumentParser(description='Get real types and functions from AgeRun codebase')
    parser.add_argument('--types', action='store_true', help='List all real types')
    parser.add_argument('--functions', action='store_true', help='List all real functions')
    parser.add_argument('--guide', action='store_true', help='Print usage guide (default)')
    
    args = parser.parse_args()
    
    if args.types:
        types = get_real_types()
        print("# Real AgeRun Types")
        for type_name in types:
            print(f"{type_name}")
    elif args.functions:
        functions = get_real_functions()
        print("# Real AgeRun Functions")
        for func_name in functions:
            print(f"{func_name}")
    else:
        print_usage_guide()
    
    return 0

if __name__ == "__main__":
    exit(main())