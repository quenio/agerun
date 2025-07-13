#!/usr/bin/env python3
"""
Automatically add EXAMPLE/BAD/ERROR tags to hypothetical functions and types in documentation.
This script helps fix documentation validation errors for code examples.
"""

import os
import re
import sys
from pathlib import Path

def check_repo_root():
    """Ensure script is run from repository root"""
    if not all(Path(p).exists() for p in ["Makefile", "modules", "methods"]):
        print("ERROR: This script must be run from the AgeRun repository root directory.")
        sys.exit(1)

def find_errors_for_file(filepath):
    """Run check_docs.py and extract errors for a specific file"""
    import subprocess
    result = subprocess.run(['python3', 'scripts/check_docs.py'], 
                          capture_output=True, text=True, stderr=subprocess.STDOUT)
    
    errors = []
    for line in result.stdout.split('\n'):
        if filepath in line and 'references non-existent' in line:
            # Extract the function/type name
            match = re.search(r"references non-existent (function|type) '([^']+)'", line)
            if match:
                errors.append((match.group(1), match.group(2)))
    
    return errors

def add_tags_to_file(filepath, errors):
    """Add EXAMPLE tags to functions and types that are flagged as non-existent"""
    if not errors:
        return False
    
    with open(filepath, 'r') as f:
        lines = f.readlines()
    
    modified = False
    functions = {name for kind, name in errors if kind == 'function'}
    types = {name for kind, name in errors if kind == 'type'}
    
    for i, line in enumerate(lines):
        # Skip if line already has a comment tag
        if any(tag in line for tag in ['// EXAMPLE:', '// BAD:', '// ERROR:', '/* EXAMPLE:', '/* BAD:', '/* ERROR:']):
            continue
        
        # Check for function references
        for func in functions:
            # Match function calls or declarations
            if re.search(rf'\b{re.escape(func)}\s*\(', line):
                # Determine context - is this in a "bad" example or "good" example?
                # Look backwards for section headers
                is_bad_example = False
                for j in range(max(0, i-20), i):
                    if '### Bad Example' in lines[j] or '// BAD:' in lines[j]:
                        is_bad_example = True
                        break
                    elif '### Good Example' in lines[j] or '// GOOD:' in lines[j]:
                        is_bad_example = False
                        break
                
                # Add appropriate comment
                if line.strip().endswith(';'):
                    lines[i] = line.rstrip() + '  // EXAMPLE: Hypothetical function\n'
                elif line.strip().endswith('{'):
                    lines[i] = line.rstrip() + '  // EXAMPLE: Hypothetical function\n'
                elif line.strip().endswith(')'):
                    lines[i] = line.rstrip() + '  // EXAMPLE: Hypothetical function\n'
                else:
                    # Don't modify if we can't determine where to add the comment
                    continue
                
                modified = True
                break
        
        # Check for type references
        for typ in types:
            # Match type usage (declarations, casts, sizeof, etc)
            if re.search(rf'\b{re.escape(typ)}\b', line):
                # Skip if it's already part of a typedef that will be tagged
                if 'typedef' in line and line.strip().endswith(';'):
                    # Tag the typedef line
                    lines[i] = line.rstrip() + '  // EXAMPLE: Hypothetical type\n'
                    modified = True
                    break
                elif f'} {typ};' in line:
                    # Tag the closing of a struct/enum typedef
                    lines[i] = line.rstrip() + '  // EXAMPLE: Hypothetical type\n'
                    modified = True
                    break
    
    if modified:
        with open(filepath, 'w') as f:
            f.writelines(lines)
    
    return modified

def main():
    check_repo_root()
    
    if len(sys.argv) < 2:
        print("Usage: python3 scripts/fix_doc_examples.py <markdown-file>")
        print("Example: python3 scripts/fix_doc_examples.py kb/code-smell-feature-envy.md")
        sys.exit(1)
    
    filepath = sys.argv[1]
    if not os.path.exists(filepath):
        print(f"Error: File {filepath} does not exist")
        sys.exit(1)
    
    print(f"Analyzing {filepath}...")
    errors = find_errors_for_file(filepath)
    
    if not errors:
        print(f"No errors found for {filepath}")
        return
    
    print(f"Found {len(errors)} errors:")
    for kind, name in errors[:10]:  # Show first 10
        print(f"  - {kind}: {name}")
    if len(errors) > 10:
        print(f"  ... and {len(errors) - 10} more")
    
    print(f"\nAdding tags to {filepath}...")
    if add_tags_to_file(filepath, errors):
        print("File updated successfully!")
        
        # Check if errors are fixed
        print("\nRechecking...")
        new_errors = find_errors_for_file(filepath)
        if new_errors:
            print(f"Still {len(new_errors)} errors remaining - may need manual fixes")
        else:
            print("All errors fixed!")
    else:
        print("No changes made - manual intervention may be needed")

if __name__ == "__main__":
    main()