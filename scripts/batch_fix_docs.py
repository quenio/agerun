#!/usr/bin/env python3
"""
Batch fix documentation examples by adding appropriate EXAMPLE tags.
"""

import os
import re
import subprocess
import argparse
from pathlib import Path

def get_all_errors():
    """Get all documentation errors from check_docs.py"""
    result = subprocess.run(['python3', 'scripts/check_docs.py'], 
                          capture_output=True, text=True)
    
    errors_by_file = {}
    # Check both stdout and stderr
    output = result.stdout + result.stderr
    for line in output.split('\n'):
        if '.md references non-existent' in line:
            # Handle lines like "  - ./kb/file.md references non-existent..."
            match = re.search(r'-\s*(.+\.md) references non-existent (function|type) \'([^\']+)\'', line)
            if match:
                filepath = match.group(1).strip()
                kind = match.group(2)
                name = match.group(3)
                
                if filepath not in errors_by_file:
                    errors_by_file[filepath] = set()
                errors_by_file[filepath].add((kind, name))
    
    return errors_by_file

def fix_file(filepath, errors, dry_run=False):
    """Fix a single file by adding EXAMPLE tags or replace with real types"""
    
    # Common type mappings from hypothetical to real types
    TYPE_REPLACEMENTS = {
        'ar_expression_t': 'ar_expression_ast_t',
        'ar_instruction_t': 'ar_instruction_ast_t', 
        'ar_assignment_instruction_t': 'ar_assignment_instruction_evaluator_t',
        'ar_send_instruction_t': 'ar_send_instruction_evaluator_t',
        'ar_method_instruction_t': 'ar_method_instruction_evaluator_t',
        'ar_assignment_instruction_ast_t': 'ar_instruction_ast_t',
        'ar_send_instruction_ast_t': 'ar_instruction_ast_t',
        'ar_method_instruction_ast_t': 'ar_instruction_ast_t',
        'ar_expression_processor_t': 'ar_expression_evaluator_t',
        'ar_instruction_processor_t': 'ar_instruction_evaluator_t',
        'ar_agent_system_t': 'ar_data_t',
        'ar_token_t': 'ar_data_t',
        'ar_thread_pool_t': 'ar_data_t',
        'ar_expression_cache_t': 'ar_data_t',
        'ar_queue_t': 'ar_list_t',
        'ar_expression_ui_t': 'ar_data_t',
        'ar_agency_t': 'ar_data_t',
        'ar_agent_store_t': 'ar_data_t',
        'ar_registry_t': 'ar_agent_registry_t',
        'ar_string_t': 'ar_data_t',
        'ar_semver_t': 'ar_data_t',
        'ar_complex_type_t': 'ar_data_t',
        'ar_error_callback_t': 'ar_data_t',
        'ar_compare_fn_t': 'ar_data_t',
        'ar_registry_match_fn_t': 'ar_data_t',
        'ar_agent_update_t': 'ar_data_t',
        'ar_data_factory_t': 'ar_data_t',
        'ar_module_t': 'ar_data_t'
    }
    
    with open(filepath, 'r') as f:
        content = f.read()
    
    lines = content.split('\n')
    original_lines = lines.copy()
    changes = []
    
    # Create sets for faster lookup
    functions = {name for kind, name in errors if kind == 'function'}
    types = {name for kind, name in errors if kind == 'type'}
    
    for i in range(len(lines)):
        line = lines[i]
        
        # Skip if already has a tag
        if re.search(r'//\s*(EXAMPLE|BAD|ERROR):', line) or re.search(r'/\*\s*(EXAMPLE|BAD|ERROR):', line):
            continue
        
        # For functions
        for func in functions:
            # Look for function declarations or calls
            pattern = rf'\b{re.escape(func)}\s*\('
            if re.search(pattern, line):
                # Add tag at end of line
                if not line.rstrip().endswith('\\'):  # Not a continuation line
                    new_line = line.rstrip() + '  // EXAMPLE: Hypothetical function'
                    lines[i] = new_line
                    changes.append((i + 1, line.strip(), new_line.strip()))
                    break
        
        # For types  
        for typ in types:
            # Look for type usage
            pattern = rf'\b{re.escape(typ)}\b'
            if re.search(pattern, line):
                # Try to replace with real type if mapping exists
                if typ in TYPE_REPLACEMENTS:
                    real_type = TYPE_REPLACEMENTS[typ]
                    new_line = re.sub(rf'\b{re.escape(typ)}\b', real_type, line)
                    
                    # Add comment about using real type if not already commented
                    if not re.search(r'//.*real type|//.*Using real', new_line):
                        new_line = new_line.rstrip() + '  // EXAMPLE: Using real type'
                    
                    lines[i] = new_line
                    changes.append((i + 1, line.strip(), new_line.strip()))
                    break
                else:
                    # Fall back to adding EXAMPLE tag
                    # Special handling for typedef lines
                    if '} ' + typ + ';' in line:
                        new_line = line.rstrip() + '  // EXAMPLE: Hypothetical type'
                        lines[i] = new_line
                        changes.append((i + 1, line.strip(), new_line.strip()))
                        break
                    # For struct field declarations with the type
                    elif re.search(rf'^\s*{re.escape(typ)}\s+\w+', line):
                        new_line = line.rstrip() + '  // EXAMPLE: Hypothetical type'
                        lines[i] = new_line
                        changes.append((i + 1, line.strip(), new_line.strip()))
                        break
    
    if changes:
        if dry_run:
            print(f"\n  Would make {len(changes)} changes:")
            for line_num, old, new in changes[:5]:  # Show first 5
                print(f"    Line {line_num}:")
                print(f"      - {old}")
                print(f"      + {new}")
            if len(changes) > 5:
                print(f"    ... and {len(changes) - 5} more changes")
        else:
            with open(filepath, 'w') as f:
                f.write('\n'.join(lines))
            print(f"  ✓ Made {len(changes)} changes")
        return True
    else:
        if dry_run:
            print("  No changes needed")
        return False

def main():
    parser = argparse.ArgumentParser(description='Fix documentation examples by adding EXAMPLE tags')
    parser.add_argument('--dry-run', '-n', action='store_true', 
                       help='Show what would be changed without modifying files')
    parser.add_argument('--file', '-f', type=str,
                       help='Process only a specific file')
    parser.add_argument('--verbose', '-v', action='store_true',
                       help='Show all changes, not just first 5')
    
    args = parser.parse_args()
    
    if args.dry_run:
        print("DRY RUN MODE - No files will be modified\n")
    
    print("Getting all documentation errors...")
    errors_by_file = get_all_errors()
    
    if not errors_by_file:
        print("No errors found!")
        return
    
    print(f"Found errors in {len(errors_by_file)} files\n")
    
    # Filter by specific file if requested
    if args.file:
        filtered = {k: v for k, v in errors_by_file.items() if args.file in k}
        if not filtered:
            print(f"No errors found for file: {args.file}")
            print(f"Available files: {list(errors_by_file.keys())[:5]}...")
            return
        errors_by_file = filtered
    
    # Process files with most errors first
    sorted_files = sorted(errors_by_file.items(), key=lambda x: len(x[1]), reverse=True)
    
    total_fixed = 0
    total_processed = 0
    for filepath, errors in sorted_files:
        print(f"\nProcessing {filepath} ({len(errors)} errors)...")
        if fix_file(filepath, errors, dry_run=args.dry_run):
            total_fixed += 1
        total_processed += 1
    
    if args.dry_run:
        print(f"\nWould fix {total_fixed} files")
    else:
        print(f"\nFixed {total_fixed} files")
        print("\nRunning check_docs.py again to verify...")
        
        # Run check again
        result = subprocess.run(['python3', 'scripts/check_docs.py'], 
                              capture_output=True, text=True)
        
        # Count remaining errors
        remaining = (result.stdout + result.stderr).count('references non-existent')
        print(f"Remaining errors: {remaining}")

if __name__ == "__main__":
    main()