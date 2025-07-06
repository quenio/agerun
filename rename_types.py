#!/usr/bin/env python3
"""
AgeRun Type Renaming Script

This script safely renames types in the AgeRun codebase to follow the ar_ prefix convention.
It uses whole-word matching to avoid partial replacements and provides detailed reporting.
"""

import os
import re
import sys
import argparse
from pathlib import Path
from typing import List, Tuple, Dict

# Define the type renames
TYPE_RENAMES = {
    # Enum types
    'data_type_t': 'ar_data_type_t',
    'instruction_type_t': 'ar_instruction_type_t',
    'file_result_t': 'ar_file_result_t',
    
    # Core struct types
    'data_t': 'ar_data_t',
    'list_t': 'ar_list_t',
    'map_t': 'ar_map_t',
    
    # Domain types
    'agent_t': 'ar_agent_t',
    'method_t': 'ar_method_t',
    'agent_registry_t': 'ar_agent_registry_t',
    
    # Context types
    'expression_context_t': 'ar_expression_context_t',
    'instruction_context_t': 'ar_instruction_context_t',
    
    # Parser types
    'parsed_instruction_t': 'ar_parsed_instruction_t',
    'instruction_parser_t': 'ar_instruction_parser_t',
    'instruction_evaluator_t': 'ar_instruction_evaluator_t',
    
    # System types
    'interpreter_t': 'ar_interpreter_t',
    'instruction_fixture_t': 'ar_instruction_fixture_t',
    'interpreter_fixture_t': 'ar_interpreter_fixture_t',
    'method_fixture_t': 'ar_method_fixture_t',
    'system_fixture_t': 'ar_system_fixture_t',
}

# File patterns to process
FILE_PATTERNS = ['*.c', '*.h', '*.md', '*.method', '*.zig']

# Directories to search
SEARCH_DIRS = ['modules', 'methods', '.']

# Files to exclude
EXCLUDE_FILES = {'rename_types.py', 'bin', '.git', 'build', '__pycache__'}


class TypeRenamer:
    def __init__(self, dry_run=True, verbose=False):
        self.dry_run = dry_run
        self.verbose = verbose
        self.stats = {}
        self.file_changes = {}
        
    def create_word_boundary_pattern(self, type_name: str) -> re.Pattern:
        """Create a regex pattern that matches whole words only."""
        # Match the type name when it's not part of a larger identifier
        # This handles cases like:
        # - data_t variable
        # - data_t*
        # - data_t**
        # - (data_t)
        # - <data_t>
        # But NOT:
        # - my_data_type
        # - data_type_enum
        return re.compile(r'\b' + re.escape(type_name) + r'\b')
    
    def count_occurrences(self, content: str, pattern: re.Pattern) -> int:
        """Count the number of occurrences of a pattern in content."""
        return len(pattern.findall(content))
    
    def rename_in_content(self, content: str, old_type: str, new_type: str) -> Tuple[str, int]:
        """Rename all occurrences of old_type to new_type in content."""
        pattern = self.create_word_boundary_pattern(old_type)
        new_content = pattern.sub(new_type, content)
        count = self.count_occurrences(content, pattern)
        return new_content, count
    
    def should_process_file(self, file_path: Path) -> bool:
        """Check if a file should be processed."""
        # Skip excluded files and directories
        for part in file_path.parts:
            if part in EXCLUDE_FILES:
                return False
        
        # Check if file matches our patterns
        return any(file_path.match(pattern) for pattern in FILE_PATTERNS)
    
    def process_file(self, file_path: Path, type_mapping: Dict[str, str]) -> bool:
        """Process a single file for type renames."""
        if not self.should_process_file(file_path):
            return False
        
        try:
            with open(file_path, 'r', encoding='utf-8') as f:
                original_content = f.read()
        except Exception as e:
            print(f"Error reading {file_path}: {e}")
            return False
        
        content = original_content
        file_stats = {}
        total_changes = 0
        
        # Apply each rename
        for old_type, new_type in type_mapping.items():
            new_content, count = self.rename_in_content(content, old_type, new_type)
            if count > 0:
                content = new_content
                file_stats[old_type] = count
                total_changes += count
                
                # Update global stats
                if old_type not in self.stats:
                    self.stats[old_type] = 0
                self.stats[old_type] += count
        
        # If changes were made, write the file (or report in dry run)
        if total_changes > 0:
            self.file_changes[str(file_path)] = file_stats
            
            if self.dry_run:
                if self.verbose:
                    print(f"Would modify {file_path}: {total_changes} changes")
                    for old_type, count in file_stats.items():
                        print(f"  {old_type} -> {type_mapping[old_type]}: {count} occurrences")
            else:
                try:
                    with open(file_path, 'w', encoding='utf-8') as f:
                        f.write(content)
                    if self.verbose:
                        print(f"Modified {file_path}: {total_changes} changes")
                except Exception as e:
                    print(f"Error writing {file_path}: {e}")
                    return False
        
        return total_changes > 0
    
    def find_files(self) -> List[Path]:
        """Find all files to process."""
        files = []
        for search_dir in SEARCH_DIRS:
            path = Path(search_dir)
            if path.is_dir():
                for pattern in FILE_PATTERNS:
                    files.extend(path.rglob(pattern))
            elif path.is_file() and self.should_process_file(path):
                files.append(path)
        
        # Remove duplicates and sort
        files = sorted(set(files))
        return files
    
    def run(self, type_mapping: Dict[str, str] = None):
        """Run the renaming process."""
        if type_mapping is None:
            type_mapping = TYPE_RENAMES
        
        print(f"Type Renaming {'(DRY RUN)' if self.dry_run else '(LIVE RUN)'}")
        print(f"Processing {len(type_mapping)} type renames")
        print("-" * 60)
        
        files = self.find_files()
        print(f"Found {len(files)} files to process")
        
        modified_count = 0
        for file_path in files:
            if self.process_file(file_path, type_mapping):
                modified_count += 1
        
        # Print summary
        print("\nSummary:")
        print(f"Files {'would be' if self.dry_run else ''} modified: {modified_count}")
        print("\nChanges by type:")
        for old_type in sorted(self.stats.keys()):
            new_type = type_mapping[old_type]
            count = self.stats[old_type]
            print(f"  {old_type} -> {new_type}: {count} occurrences")
        
        if self.verbose and self.file_changes:
            print("\nDetailed file changes:")
            for file_path in sorted(self.file_changes.keys()):
                print(f"\n{file_path}:")
                for old_type, count in self.file_changes[file_path].items():
                    print(f"  {old_type}: {count} occurrences")


def main():
    parser = argparse.ArgumentParser(description='Rename types in AgeRun codebase')
    parser.add_argument('--live', action='store_true', 
                       help='Actually modify files (default is dry run)')
    parser.add_argument('--verbose', '-v', action='store_true',
                       help='Show detailed output')
    parser.add_argument('--types', nargs='+', 
                       help='Specific types to rename (e.g., data_t list_t)')
    parser.add_argument('--group', choices=['enums', 'core', 'domain', 'context', 'parser', 'system', 'all'],
                       default='all', help='Group of types to rename')
    
    args = parser.parse_args()
    
    # Determine which types to rename
    if args.types:
        # Only rename specified types
        type_mapping = {t: TYPE_RENAMES[t] for t in args.types if t in TYPE_RENAMES}
        if not type_mapping:
            print("Error: No valid types specified")
            return 1
    elif args.group != 'all':
        # Rename a specific group
        groups = {
            'enums': ['data_type_t', 'instruction_type_t', 'file_result_t'],
            'core': ['data_t', 'list_t', 'map_t'],
            'domain': ['agent_t', 'method_t', 'agent_registry_t'],
            'context': ['expression_context_t', 'instruction_context_t'],
            'parser': ['parsed_instruction_t', 'instruction_parser_t', 'instruction_evaluator_t'],
            'system': ['interpreter_t', 'instruction_fixture_t', 'interpreter_fixture_t', 
                      'method_fixture_t', 'system_fixture_t'],
        }
        type_names = groups.get(args.group, [])
        type_mapping = {t: TYPE_RENAMES[t] for t in type_names}
    else:
        # Rename all types
        type_mapping = TYPE_RENAMES
    
    # Create renamer and run
    renamer = TypeRenamer(dry_run=not args.live, verbose=args.verbose)
    renamer.run(type_mapping)
    
    if not args.live:
        print("\nThis was a dry run. Use --live to actually modify files.")
    
    return 0


if __name__ == '__main__':
    sys.exit(main())