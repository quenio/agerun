#!/usr/bin/env python3
"""
Remove _with_instance suffix from all functions

This script removes the _with_instance suffix from all function names now that
global APIs have been eliminated, following the API Suffix Cleanup Pattern.
"""

import os
import re
import sys
import argparse
from pathlib import Path
from typing import List, Tuple, Dict, Set

def find_functions_with_suffix(file_path: str) -> List[str]:
    """Find all function declarations with _with_instance suffix in header files."""
    functions = []
    try:
        with open(file_path, 'r') as f:
            content = f.read()

        # Pattern to match function declarations with _with_instance
        pattern = r'^[a-zA-Z_0-9\s\*]+\s+([a-zA-Z_][a-zA-Z0-9_]*_with_instance)\s*\('
        matches = re.findall(pattern, content, re.MULTILINE)
        functions.extend(matches)

    except Exception as e:
        print(f"Warning: Could not read {file_path}: {e}")

    return functions

def get_all_functions_to_rename() -> Dict[str, str]:
    """Get all functions that need renaming from header files."""
    functions_to_rename = {}

    # Scan all header files
    modules_dir = Path("modules")
    for header_file in modules_dir.glob("*.h"):
        functions = find_functions_with_suffix(str(header_file))
        for func in functions:
            # Remove _with_instance suffix
            new_name = func.replace("_with_instance", "")
            functions_to_rename[func] = new_name

    return functions_to_rename

def update_file_content(content: str, functions_to_rename: Dict[str, str], file_path: str) -> Tuple[str, bool]:
    """Update file content by renaming functions."""
    original_content = content
    changes_made = []

    for old_name, new_name in functions_to_rename.items():
        # Use word boundaries to ensure exact matches
        pattern = r'\b' + re.escape(old_name) + r'\b'
        if re.search(pattern, content):
            content = re.sub(pattern, new_name, content)
            changes_made.append(f"{old_name} -> {new_name}")

    if changes_made:
        return content, True
    return content, False

def process_file(file_path: str, functions_to_rename: Dict[str, str], dry_run: bool = True, verbose: bool = False) -> bool:
    """Process a single file."""
    try:
        with open(file_path, 'r') as f:
            content = f.read()

        updated_content, changed = update_file_content(content, functions_to_rename, file_path)

        if changed:
            if verbose:
                print(f"{'Would update' if dry_run else 'Updating'}: {file_path}")
                # Show specific changes
                for old_name, new_name in functions_to_rename.items():
                    if old_name in content:
                        count = len(re.findall(r'\b' + re.escape(old_name) + r'\b', content))
                        if count > 0:
                            print(f"  {old_name} -> {new_name} ({count} occurrences)")
            else:
                print(f"{'Would update' if dry_run else 'Updating'}: {file_path}")

            if not dry_run:
                with open(file_path, 'w') as f:
                    f.write(updated_content)

            return True

        return False

    except Exception as e:
        print(f"Error processing {file_path}: {e}")
        return False

def get_files_to_process() -> List[str]:
    """Get all files that need to be processed."""
    files = []

    # Source files in modules/
    modules_dir = Path("modules")
    files.extend(str(f) for f in modules_dir.glob("*.c"))
    files.extend(str(f) for f in modules_dir.glob("*.h"))
    files.extend(str(f) for f in modules_dir.glob("*.zig"))

    # Test files in modules/
    files.extend(str(f) for f in modules_dir.glob("*_tests.c"))

    # Test files in methods/
    methods_dir = Path("methods")
    if methods_dir.exists():
        files.extend(str(f) for f in methods_dir.glob("*_tests.c"))

    # Documentation files
    files.extend(str(f) for f in modules_dir.glob("*.md"))
    if methods_dir.exists():
        files.extend(str(f) for f in methods_dir.glob("*.md"))

    # Project documentation
    for doc_file in ["README.md", "TODO.md", "AGENTS.md", "CHANGELOG.md", "SPEC.md", "MMM.md"]:
        if Path(doc_file).exists():
            files.append(doc_file)

    # Knowledge base articles
    kb_dir = Path("kb")
    if kb_dir.exists():
        files.extend(str(f) for f in kb_dir.glob("*.md"))

    # Scripts
    scripts_dir = Path("scripts")
    if scripts_dir.exists():
        files.extend(str(f) for f in scripts_dir.glob("*.py"))
        files.extend(str(f) for f in scripts_dir.glob("*.sh"))

    # Commands
    commands_dir = Path(".opencode/command/ar")
    if commands_dir.exists():
        files.extend(str(f) for f in commands_dir.rglob("*.md"))

    return sorted(files)

def main():
    parser = argparse.ArgumentParser(description='Remove _with_instance suffix from all functions')
    parser.add_argument('--apply', action='store_true',
                       help='Actually apply changes (default is dry-run)')
    parser.add_argument('--verbose', '-v', action='store_true',
                       help='Show detailed changes')
    parser.add_argument('--files', nargs='*',
                       help='Specific files to process (default: all relevant files)')

    args = parser.parse_args()

    # Get all functions to rename
    print("Scanning header files for functions with _with_instance suffix...")
    functions_to_rename = get_all_functions_to_rename()

    if not functions_to_rename:
        print("No functions with _with_instance suffix found.")
        return

    print(f"Found {len(functions_to_rename)} functions to rename:")
    for old_name, new_name in sorted(functions_to_rename.items()):
        print(f"  {old_name} -> {new_name}")
    print()

    # Get files to process
    if args.files:
        files_to_process = args.files
    else:
        files_to_process = get_files_to_process()

    print(f"Processing {len(files_to_process)} files...")
    if args.apply:
        print("APPLYING CHANGES")
    else:
        print("DRY-RUN MODE (use --apply to make changes)")
    print()

    # Process files
    files_changed = 0
    for file_path in files_to_process:
        if process_file(file_path, functions_to_rename, dry_run=not args.apply, verbose=args.verbose):
            files_changed += 1

    print()
    if args.apply:
        print(f"Successfully updated {files_changed} files.")
        print("Next steps:")
        print("1. make clean build 2>&1")
        print("2. make check-logs")
        print("3. make check-docs")
    else:
        print(f"Would update {files_changed} files.")
        print("Run with --apply to make changes.")

if __name__ == '__main__':
    main()