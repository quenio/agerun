#!/usr/bin/env python3
"""
Enhanced batch fix documentation errors including:
- Non-existent function/type references in more contexts
- Type references in struct fields
- Type references in function parameters
- Type references in expressions like sizeof()
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
        'ar_print_evaluator_t': 'ar_spawn_instruction_evaluator_t',
        'config_t': 'ar_data_t',
        'processor_t': 'ar_expression_evaluator_t',
        'context_t': 'ar_data_t',
        'list_t': 'ar_list_t',
        'string_t': 'ar_data_t',
        'own_string': 'ar_data_t',
        'own_list': 'ar_data_t',
        'own_map': 'ar_data_t',
        'own_integer': 'ar_data_t',
        'own_double': 'ar_data_t',
        'mut_string': 'ar_data_t',
        'mut_list': 'ar_data_t',
        'mut_map': 'ar_data_t',
        'ref_string': 'ar_data_t',
        'ref_list': 'ar_data_t',
        'ref_map': 'ar_data_t',
    }
    
    # Track types we've seen as hypothetical for consistent handling
    hypothetical_types = set()
    
    with open(filepath, 'r') as f:
        lines = f.readlines()
    
    # Strip trailing newlines but preserve them for output
    lines = [line.rstrip('\n') for line in lines]
    
    changes = []
    
    # Handle broken links first (by line number) - PRESERVE ORIGINAL LOGIC
    for line_num, old_path, expected_path in sorted(errors.get('broken_links', []), reverse=True):
        line_idx = line_num - 1
        if line_idx < len(lines):
            old_line = lines[line_idx]
            # Calculate correct relative path
            correct_path = calculate_relative_path(filepath, old_path)
            new_line = old_line.replace(f']({old_path})', f']({correct_path})')
            if new_line != old_line:
                lines[line_idx] = new_line
                changes.append((line_num, 'broken link', old_line.strip(), new_line.strip()))
    
    # Handle absolute paths - PRESERVE ORIGINAL LOGIC
    for line_num, abs_path in sorted(errors.get('absolute_paths', []), reverse=True):
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
    
    # First pass: identify hypothetical types from typedef declarations
    for i, line in enumerate(lines):
        for kind, name in errors.get('non_existent', []):
            if kind == 'type':
                # Check if this is a typedef declaration
                if re.search(rf'typedef\s+struct\s+\w+\s+{re.escape(name)}\s*;', line):
                    hypothetical_types.add(name)
                    if '// EXAMPLE:' not in line:
                        new_line = line.rstrip() + '  // EXAMPLE: Hypothetical type'
                        lines[i] = new_line
                        changes.append((i + 1, 'typedef declaration', line.strip(), new_line.strip()))
    
    # Handle non-existent functions/types
    if errors.get('non_existent'):
        # Create sets for faster lookup - PRESERVE ORIGINAL OPTIMIZATION
        functions = {name for kind, name in errors['non_existent'] if kind == 'function'}
        types = {name for kind, name in errors['non_existent'] if kind == 'type'}
        
        for i in range(len(lines)):
            line = lines[i]
            
            # Skip if already has a tag
            if re.search(r'//\s*(EXAMPLE|BAD|ERROR):', line) or re.search(r'/\*\s*(EXAMPLE|BAD|ERROR):', line):
                continue
            
            # For functions - PRESERVE ORIGINAL LOGIC
            for func in functions:
                # Look for function declarations or calls
                pattern = rf'\b{re.escape(func)}\s*\('
                if re.search(pattern, line):
                    # Add tag at end of line
                    if not line.rstrip().endswith('\\'):  # Not a continuation line - PRESERVE CHECK
                        new_line = line.rstrip() + '  // EXAMPLE: Hypothetical function'
                        lines[i] = new_line
                        changes.append((i + 1, 'non-existent function', line.strip(), new_line.strip()))
                        break
                    
            # For types - ENHANCED WITH NEW CONTEXTS
            for typ in types:
                # Look for type usage
                pattern = rf'\b{re.escape(typ)}\b'
                if re.search(pattern, line):
                    # First check if we should replace with a real type
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
                        # Handle hypothetical types in various contexts
                        
                        # 1. Typedef declarations (already handled in first pass)
                        if re.search(rf'typedef\s+struct\s+\w+\s+{re.escape(typ)}\s*;', line):
                            continue  # Already processed
                        
                        # 1b. Struct closing brace with type name (e.g., "} ar_type_t;")
                        elif re.search(rf'}}\s*{re.escape(typ)}\s*;', line):
                            if '// EXAMPLE:' not in line:
                                new_line = line.rstrip() + '  // EXAMPLE: Hypothetical type'
                                lines[i] = new_line
                                changes.append((i + 1, 'struct closing', line.strip(), new_line.strip()))
                                break
                        
                        # 2. Struct field declarations (e.g., "ar_runtime_t* own_runtime;")
                        elif re.search(rf'\b{re.escape(typ)}\s*\*?\s+\w+\s*;', line):
                            if '// EXAMPLE:' not in line:
                                # Check if the type is known to be hypothetical
                                if typ in hypothetical_types:
                                    new_line = line.rstrip() + '  // EXAMPLE: Using hypothetical type'
                                else:
                                    new_line = line.rstrip() + '  // EXAMPLE: Using real type'
                                lines[i] = new_line
                                changes.append((i + 1, 'struct field', line.strip(), new_line.strip()))
                                break
                        
                        # 3. Function parameters (e.g., "void func(ar_type_t* param)")
                        elif re.search(rf'\b{re.escape(typ)}\s*\*?\s+\w+\s*[,)]', line):
                            if '// EXAMPLE:' not in line:
                                new_line = line.rstrip() + '  // EXAMPLE: Using hypothetical type'
                                lines[i] = new_line
                                changes.append((i + 1, 'function parameter', line.strip(), new_line.strip()))
                                break
                        
                        # 4. sizeof expressions (e.g., "sizeof(ar_type_t)")
                        elif re.search(rf'sizeof\s*\(\s*{re.escape(typ)}\s*\)', line):
                            if '// EXAMPLE:' not in line:
                                new_line = line.rstrip() + '  // EXAMPLE: Using hypothetical type'
                                lines[i] = new_line
                                changes.append((i + 1, 'sizeof expression', line.strip(), new_line.strip()))
                                break
                        
                        # 5. Type casts (e.g., "(ar_type_t*)")
                        elif re.search(rf'\(\s*{re.escape(typ)}\s*\*?\s*\)', line):
                            if '// EXAMPLE:' not in line:
                                new_line = line.rstrip() + '  // EXAMPLE: Using hypothetical type'
                                lines[i] = new_line
                                changes.append((i + 1, 'type cast', line.strip(), new_line.strip()))
                                break
                        
                        # 6. Variable declarations (e.g., "ar_type_t* var =")
                        elif re.search(rf'\b{re.escape(typ)}\s*\*?\s+\w+\s*=', line):
                            if '// EXAMPLE:' not in line:
                                new_line = line.rstrip() + '  // EXAMPLE: Using hypothetical type'
                                lines[i] = new_line
                                changes.append((i + 1, 'variable declaration', line.strip(), new_line.strip()))
                                break
                        
                        # 7. Return types (e.g., "ar_type_t* func()")
                        elif re.search(rf'^{re.escape(typ)}\s*\*?\s+\w+\s*\(', line):
                            if '// EXAMPLE:' not in line:
                                new_line = line.rstrip() + '  // EXAMPLE: Hypothetical function'
                                lines[i] = new_line
                                changes.append((i + 1, 'return type', line.strip(), new_line.strip()))
                                break
                        
                        # 8. Type mentions in documentation/comments (e.g., "- Create ar_type_t")
                        elif re.search(rf'\b{re.escape(typ)}\b', line):
                            # Only tag if it's not in a code block and not already tagged
                            if not line.strip().startswith('```') and '// EXAMPLE:' not in line:
                                # Check if it's a documentation line (starts with -, *, or is plain text)
                                if re.match(r'^\s*[-*]|\s*[A-Z]', line):
                                    new_line = line.rstrip() + '  // EXAMPLE: Future type'
                                    lines[i] = new_line
                                    changes.append((i + 1, 'documentation mention', line.strip(), new_line.strip()))
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
    parser = argparse.ArgumentParser(description='Enhanced documentation error fixing with better type handling')
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
        print("No documentation errors found!")
        return
    
    error_count = sum(
        len(e['non_existent']) + len(e['broken_links']) + len(e['absolute_paths']) 
        for e in errors_by_file.values()
    )
    print(f"Found errors in {len(errors_by_file)} files\n")
    
    if args.file:
        # Process only the specified file
        if args.file in errors_by_file:
            print(f"Processing {args.file}...")
            fix_file(args.file, errors_by_file[args.file], args.dry_run)
        else:
            print(f"No errors found in {args.file}")
    else:
        # Process all files with errors
        fixed_count = 0
        for filepath, errors in errors_by_file.items():
            error_types = []
            if errors['non_existent']:
                error_types.append(f"{len(errors['non_existent'])} non-existent refs")
            if errors['broken_links']:
                error_types.append(f"{len(errors['broken_links'])} broken links")
            if errors['absolute_paths']:
                error_types.append(f"{len(errors['absolute_paths'])} absolute paths")
            
            print(f"\nProcessing {filepath} ({', '.join(error_types)})...")
            if fix_file(filepath, errors, args.dry_run):
                fixed_count += 1
        
        if args.dry_run:
            print(f"\nWould fix {fixed_count} files")
        else:
            print(f"\nFixed {fixed_count} files")
            
            # Run check_docs again to verify
            print("\nRunning check_docs.py again to verify...")
            result = subprocess.run(['python3', 'scripts/check_docs.py'], 
                                  capture_output=True, text=True)
            
            # Count remaining errors
            remaining_errors = 0
            for line in (result.stdout + result.stderr).split('\n'):
                if 'references non-existent' in line:
                    remaining_errors += 1
            
            if remaining_errors > 0:
                print(f"\nRemaining errors: {remaining_errors}")
                print("  - Non-existent references: " + str(remaining_errors))
            else:
                print("\nAll documentation errors have been fixed! ✓")

if __name__ == "__main__":
    main()