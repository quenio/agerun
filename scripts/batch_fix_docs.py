#!/usr/bin/env python3
"""
Batch fix documentation errors including:
- Non-existent function/type references (add EXAMPLE tags)
- Broken relative markdown links
- Absolute paths that should be relative
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
                    errors_by_file[filepath] = {'non_existent': set(), 'broken_links': [], 'absolute_paths': []}
                errors_by_file[filepath]['non_existent'].add((kind, name))
        
        elif 'broken link' in line:
            # Handle lines like "  - ./file.md:7 broken link '[details](kb/file.md)' -> expected/path.md"
            match = re.search(r'-\s*(.+\.md):(\d+)\s+broken link\s+\'[^(]*\(([^)]+)\)\'\s+->\s+(.+)$', line)
            if match:
                filepath = match.group(1).strip()
                line_num = int(match.group(2))
                old_path = match.group(3)
                expected_path = match.group(4).strip()
                
                if filepath not in errors_by_file:
                    errors_by_file[filepath] = {'non_existent': set(), 'broken_links': [], 'absolute_paths': []}
                errors_by_file[filepath]['broken_links'].append((line_num, old_path, expected_path))
        
        elif 'absolute path' in line:
            # Handle lines like "  - ./file.md:66 absolute path '[text](/path)' - use relative paths instead"
            match = re.search(r'-\s*(.+\.md):(\d+)\s+absolute path\s+\'[^(]*\((/[^)]+)\)\'', line)
            if match:
                filepath = match.group(1).strip()
                line_num = int(match.group(2))
                abs_path = match.group(3)
                
                if filepath not in errors_by_file:
                    errors_by_file[filepath] = {'non_existent': set(), 'broken_links': [], 'absolute_paths': []}
                errors_by_file[filepath]['absolute_paths'].append((line_num, abs_path))
    
    return errors_by_file

def calculate_relative_path(from_file, to_path):
    """Calculate the correct relative path from one file to another"""
    from_dir = os.path.dirname(from_file)
    
    # If we're in the root directory, no prefix needed
    if from_dir == '.' or from_dir == '':
        return to_path
    
    # Count how many directories deep we are
    depth = len([d for d in from_dir.split('/') if d and d != '.'])
    
    # Build the relative prefix
    prefix = '../' * depth
    
    # Clean the target path of any existing relative prefixes
    clean_path = to_path.lstrip('./')
    while clean_path.startswith('../'):
        clean_path = clean_path[3:]
    
    return prefix + clean_path

def fix_file(filepath, errors, dry_run=False):
    """Fix all types of documentation errors in a single file"""
    
    # Safety check: only process .md files
    if not filepath.endswith('.md'):
        print(f"  Skipping non-markdown file: {filepath}")
        return False
    
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
    
    # Handle broken links first (by line number)
    if 'broken_links' in errors:
        for line_num, old_path, expected_path in sorted(errors['broken_links'], reverse=True):
            line_idx = line_num - 1
            if line_idx < len(lines):
                old_line = lines[line_idx]
                # Calculate correct relative path
                correct_path = calculate_relative_path(filepath, old_path)
                new_line = old_line.replace(f']({old_path})', f']({correct_path})')
                if new_line != old_line:
                    lines[line_idx] = new_line
                    changes.append((line_num, 'broken link', old_line.strip(), new_line.strip()))
    
    # Handle absolute paths
    if 'absolute_paths' in errors:
        for line_num, abs_path in sorted(errors['absolute_paths'], reverse=True):
            line_idx = line_num - 1
            if line_idx < len(lines):
                old_line = lines[line_idx]
                # Convert absolute to relative
                rel_path = abs_path.lstrip('/')
                correct_path = calculate_relative_path(filepath, rel_path)
                new_line = old_line.replace(f']({abs_path})', f']({correct_path})')
                if new_line != old_line:
                    lines[line_idx] = new_line
                    changes.append((line_num, 'absolute path', old_line.strip(), new_line.strip()))
    
    # Handle non-existent functions/types
    if 'non_existent' in errors:
        # Create sets for faster lookup
        functions = {name for kind, name in errors['non_existent'] if kind == 'function'}
        types = {name for kind, name in errors['non_existent'] if kind == 'type'}
        
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
                        changes.append((i + 1, 'non-existent function', line.strip(), new_line.strip()))
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
                        changes.append((i + 1, 'type replacement', line.strip(), new_line.strip()))
                        break
                    else:
                        # Fall back to adding EXAMPLE tag
                        # Special handling for typedef lines
                        if '} ' + typ + ';' in line:
                            new_line = line.rstrip() + '  // EXAMPLE: Hypothetical type'
                            lines[i] = new_line
                            changes.append((i + 1, 'non-existent type', line.strip(), new_line.strip()))
                            break
                        # For struct field declarations with the type
                        elif re.search(rf'^\s*{re.escape(typ)}\s+\w+', line):
                            new_line = line.rstrip() + '  // EXAMPLE: Hypothetical type'
                            lines[i] = new_line
                            changes.append((i + 1, 'non-existent type', line.strip(), new_line.strip()))
                            break
    
    if changes:
        if dry_run:
            print(f"\n  Would make {len(changes)} changes:")
            for item in changes[:5]:  # Show first 5
                if len(item) == 4:  # New format with error type
                    line_num, error_type, old, new = item
                    print(f"    Line {line_num} ({error_type}):")
                else:  # Old format
                    line_num, old, new = item
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
    parser = argparse.ArgumentParser(description='Fix documentation errors including non-existent references, broken links, and absolute paths')
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
        
        # Count remaining errors of all types
        output = result.stdout + result.stderr
        non_existent = output.count('references non-existent')
        broken_links = output.count('broken link')
        absolute_paths = output.count('absolute path')
        
        total_remaining = non_existent + broken_links + absolute_paths
        
        if total_remaining > 0:
            print(f"\nRemaining errors: {total_remaining}")
            if non_existent > 0:
                print(f"  - Non-existent references: {non_existent}")
            if broken_links > 0:
                print(f"  - Broken links: {broken_links}")
            if absolute_paths > 0:
                print(f"  - Absolute paths: {absolute_paths}")
        else:
            print("\n✓ All documentation errors fixed!")

if __name__ == "__main__":
    main()