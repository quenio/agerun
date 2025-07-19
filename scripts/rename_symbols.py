#!/usr/bin/env python3
"""
AgeRun Symbol Renaming Script

This script safely renames symbols (types, enums, structs) in the AgeRun codebase to follow 
naming conventions. It uses whole-word matching to avoid partial replacements and provides 
detailed reporting.
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
    
    # Rename instruction_evaluator_fixture to evaluator_fixture
    'instruction_evaluator_fixture_t': 'ar_evaluator_fixture_t',
}

# Define struct tag renames (typedef struct foo_s bar_t; -> typedef struct ar_foo_s bar_t;)
STRUCT_TAG_RENAMES = {
    # Core struct tags
    'data_s': 'ar_data_s',
    'list_s': 'ar_list_s', 
    'map_s': 'ar_map_s',
    'list_node_s': 'ar_list_node_s',
    
    # Domain struct tags
    'agent_s': 'ar_agent_s',
    'method_s': 'ar_method_s',
    'agent_registry_s': 'ar_agent_registry_s',
    
    # Context struct tags
    'expression_context_s': 'ar_expression_context_s',
    'instruction_context_s': 'ar_instruction_context_s',
    
    # Parser struct tags  
    'parsed_instruction_s': 'ar_parsed_instruction_s',
    'instruction_parser_s': 'ar_instruction_parser_s',
    'instruction_evaluator_s': 'ar_instruction_evaluator_s',
    'expression_parser_s': 'ar_expression_parser_s',
    'expression_evaluator_s': 'ar_expression_evaluator_s',
    
    # AST struct tags
    'expression_ast_s': 'ar_expression_ast_s',
    'instruction_ast_s': 'ar_instruction_ast_s',
    
    # System struct tags
    'interpreter_s': 'ar_interpreter_s',
    'instruction_fixture_s': 'ar_instruction_fixture_s',
    'interpreter_fixture_s': 'ar_interpreter_fixture_s',
    'method_fixture_s': 'ar_method_fixture_s',
    'system_fixture_s': 'ar_system_fixture_s',
    
    # Rename instruction_evaluator_fixture to evaluator_fixture
    'instruction_evaluator_fixture_s': 'ar_evaluator_fixture_s',
}

# Define enum value renames
ENUM_VALUE_RENAMES = {
    # ar_data_type_t enum values
    'DATA_INTEGER': 'AR_DATA_TYPE__INTEGER',
    'DATA_DOUBLE': 'AR_DATA_TYPE__DOUBLE',
    'DATA_STRING': 'AR_DATA_TYPE__STRING',
    'DATA_LIST': 'AR_DATA_TYPE__LIST',
    'DATA_MAP': 'AR_DATA_TYPE__MAP',
    
    # ar_event_type_t enum values
    'AR_EVENT_ERROR': 'AR_EVENT_TYPE__ERROR',
    'AR_EVENT_WARNING': 'AR_EVENT_TYPE__WARNING',
    'AR_EVENT_INFO': 'AR_EVENT_TYPE__INFO',
    
    # ar_expression_ast_type_t enum values
    'AR_EXPR__LITERAL_INT': 'AR_EXPRESSION_AST_TYPE__LITERAL_INT',
    'AR_EXPR__LITERAL_DOUBLE': 'AR_EXPRESSION_AST_TYPE__LITERAL_DOUBLE',
    'AR_EXPR__LITERAL_STRING': 'AR_EXPRESSION_AST_TYPE__LITERAL_STRING',
    'AR_EXPR__MEMORY_ACCESS': 'AR_EXPRESSION_AST_TYPE__MEMORY_ACCESS',
    'AR_EXPR__BINARY_OP': 'AR_EXPRESSION_AST_TYPE__BINARY_OP',
    
    # ar_binary_operator_t enum values
    'AR_OP__ADD': 'AR_BINARY_OPERATOR__ADD',
    'AR_OP__SUBTRACT': 'AR_BINARY_OPERATOR__SUBTRACT',
    'AR_OP__MULTIPLY': 'AR_BINARY_OPERATOR__MULTIPLY',
    'AR_OP__DIVIDE': 'AR_BINARY_OPERATOR__DIVIDE',
    'AR_OP__EQUAL': 'AR_BINARY_OPERATOR__EQUAL',
    'AR_OP__NOT_EQUAL': 'AR_BINARY_OPERATOR__NOT_EQUAL',
    'AR_OP__LESS': 'AR_BINARY_OPERATOR__LESS',
    'AR_OP__LESS_EQ': 'AR_BINARY_OPERATOR__LESS_EQ',
    'AR_OP__GREATER': 'AR_BINARY_OPERATOR__GREATER',
    'AR_OP__GREATER_EQ': 'AR_BINARY_OPERATOR__GREATER_EQ',
    
    # ar_instruction_type_t enum values
    'INST_ASSIGNMENT': 'AR_INSTRUCTION_TYPE__ASSIGNMENT',
    'INST_SEND': 'AR_INSTRUCTION_TYPE__SEND',
    'INST_IF': 'AR_INSTRUCTION_TYPE__IF',
    'INST_METHOD': 'AR_INSTRUCTION_TYPE__METHOD',
    'INST_AGENT': 'AR_INSTRUCTION_TYPE__AGENT',
    'INST_DESTROY': 'AR_INSTRUCTION_TYPE__DESTROY',
    'INST_PARSE': 'AR_INSTRUCTION_TYPE__PARSE',
    
    # ar_instruction_ast_type_t enum values
    'AR_INST__ASSIGNMENT': 'AR_INSTRUCTION_AST_TYPE__ASSIGNMENT',
    'AR_INST__SEND': 'AR_INSTRUCTION_AST_TYPE__SEND',
    'AR_INST__IF': 'AR_INSTRUCTION_AST_TYPE__IF',
    'AR_INST__METHOD': 'AR_INSTRUCTION_AST_TYPE__METHOD',
    'AR_INST__AGENT': 'AR_INSTRUCTION_AST_TYPE__AGENT',
    'AR_INST__DESTROY_AGENT': 'AR_INSTRUCTION_AST_TYPE__DESTROY_AGENT',
    'AR_INST__DESTROY_METHOD': 'AR_INSTRUCTION_AST_TYPE__DESTROY_METHOD',
    'AR_INST__PARSE': 'AR_INSTRUCTION_AST_TYPE__PARSE',
    'AR_INST__BUILD': 'AR_INSTRUCTION_AST_TYPE__BUILD',
    
    # ar_file_result_t enum values
    'FILE_SUCCESS': 'AR_FILE_RESULT__SUCCESS',
    'FILE_ERROR_OPEN': 'AR_FILE_RESULT__ERROR_OPEN',
    'FILE_ERROR_READ': 'AR_FILE_RESULT__ERROR_READ',
    'FILE_ERROR_WRITE': 'AR_FILE_RESULT__ERROR_WRITE',
    'FILE_ERROR_PERMISSIONS': 'AR_FILE_RESULT__ERROR_PERMISSIONS',
    'FILE_ERROR_NOT_FOUND': 'AR_FILE_RESULT__ERROR_NOT_FOUND',
    'FILE_ERROR_CORRUPT': 'AR_FILE_RESULT__ERROR_CORRUPT',
    'FILE_ERROR_ALREADY_EXISTS': 'AR_FILE_RESULT__ERROR_ALREADY_EXISTS',
    'FILE_ERROR_UNKNOWN': 'AR_FILE_RESULT__ERROR_UNKNOWN',
}

# Zig struct type renames (const FooBar = struct -> const ar_foo_bar_t = struct)
ZIG_STRUCT_RENAMES = {
    'MemoryRecord': 'ar_memory_record_t',
}

# Module renames (handles module name prefix in functions, types, etc.)
MODULE_RENAMES = {
    # Rename instruction_evaluator_fixture module to evaluator_fixture
    'ar_instruction_evaluator_fixture': 'ar_evaluator_fixture',
}

# Function renames (handles specific function name changes)
FUNCTION_RENAMES = {
    # Rename ownership functions for clarity
    'ar_data__transfer_ownership': 'ar_data__drop_ownership',
}

# File patterns to process
FILE_PATTERNS = ['*.c', '*.h', '*.md', '*.method', '*.zig']

# Directories to search
SEARCH_DIRS = ['modules', 'methods', '.']

# Files to exclude
EXCLUDE_FILES = {'rename_symbols.py', 'bin', '.git', 'build', '__pycache__'}


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
    
    def rename_module_in_content(self, content: str, old_module: str, new_module: str) -> Tuple[str, int]:
        """Rename all occurrences of a module name (includes functions, types, etc.)."""
        # For modules, we want to replace the prefix anywhere it appears
        # This will catch ar_instruction_evaluator_fixture_t, ar_instruction_evaluator_fixture__create, etc.
        pattern = re.compile(re.escape(old_module))
        new_content = pattern.sub(new_module, content)
        count = len(pattern.findall(content))
        return new_content, count
    
    def rename_function_in_content(self, content: str, old_func: str, new_func: str) -> Tuple[str, int]:
        """Rename all occurrences of a function name."""
        # Use word boundary to ensure we only match the exact function name
        pattern = self.create_word_boundary_pattern(old_func)
        new_content = pattern.sub(new_func, content)
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
    
    def process_file(self, file_path: Path, type_mapping: Dict[str, str], module_mapping: Dict[str, str] = None, function_mapping: Dict[str, str] = None) -> bool:
        """Process a single file for type, module, and function renames."""
        if not self.should_process_file(file_path):
            return False
        
        if module_mapping is None:
            module_mapping = {}
        if function_mapping is None:
            function_mapping = {}
        
        try:
            with open(file_path, 'r', encoding='utf-8') as f:
                original_content = f.read()
        except Exception as e:
            print(f"Error reading {file_path}: {e}")
            return False
        
        content = original_content
        file_stats = {}
        total_changes = 0
        
        # Apply module renames first (they are more specific)
        for old_module, new_module in module_mapping.items():
            new_content, count = self.rename_module_in_content(content, old_module, new_module)
            if count > 0:
                content = new_content
                file_stats[old_module] = count
                total_changes += count
                
                # Update global stats
                if old_module not in self.stats:
                    self.stats[old_module] = 0
                self.stats[old_module] += count
        
        # Apply function renames (more specific than type renames)
        for old_func, new_func in function_mapping.items():
            new_content, count = self.rename_function_in_content(content, old_func, new_func)
            if count > 0:
                content = new_content
                file_stats[old_func] = count
                total_changes += count
                
                # Update global stats
                if old_func not in self.stats:
                    self.stats[old_func] = 0
                self.stats[old_func] += count
        
        # Apply type renames
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
                    for old_name, count in file_stats.items():
                        # Determine the new name based on which mapping contains it
                        if old_name in type_mapping:
                            new_name = type_mapping[old_name]
                        elif old_name in module_mapping:
                            new_name = module_mapping[old_name]
                        elif old_name in function_mapping:
                            new_name = function_mapping[old_name]
                        else:
                            new_name = "???"  # Should not happen
                        print(f"  {old_name} -> {new_name}: {count} occurrences")
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
    
    def run(self, type_mapping: Dict[str, str] = None, struct_tag_mapping: Dict[str, str] = None, 
            enum_value_mapping: Dict[str, str] = None, module_mapping: Dict[str, str] = None,
            function_mapping: Dict[str, str] = None):
        """Run the renaming process."""
        if type_mapping is None:
            type_mapping = {}
        if struct_tag_mapping is None:
            struct_tag_mapping = {}
        if enum_value_mapping is None:
            enum_value_mapping = {}
        if function_mapping is None:
            function_mapping = {}
        if module_mapping is None:
            module_mapping = {}
        
        # Combine all mappings
        all_mappings = {**type_mapping, **struct_tag_mapping, **enum_value_mapping, **module_mapping, **function_mapping}
        
        print(f"Type Renaming {'(DRY RUN)' if self.dry_run else '(LIVE RUN)'}")
        print(f"Processing {len(type_mapping)} type renames + {len(struct_tag_mapping)} struct tag renames + " +
              f"{len(enum_value_mapping)} enum value renames + {len(module_mapping)} module renames + " +
              f"{len(function_mapping)} function renames")
        print("-" * 60)
        
        files = self.find_files()
        print(f"Found {len(files)} files to process")
        
        modified_count = 0
        for file_path in files:
            # Pass type/struct/enum mappings and module mappings separately
            type_and_other_mappings = {**type_mapping, **struct_tag_mapping, **enum_value_mapping}
            if self.process_file(file_path, type_and_other_mappings, module_mapping, function_mapping):
                modified_count += 1
        
        # Print summary
        print("\nSummary:")
        print(f"Files {'would be' if self.dry_run else ''} modified: {modified_count}")
        print("\nChanges by type:")
        for old_type in sorted(self.stats.keys()):
            new_type = all_mappings[old_type]
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
    parser.add_argument('--group', choices=['enums', 'core', 'domain', 'context', 'parser', 'system', 'struct-tags', 
                                           'enum-values', 'data-enums', 'event-enums', 'expr-enums', 'op-enums', 
                                           'inst-enums', 'inst-ast-enums', 'file-enums', 'zig-structs', 'modules', 'all'],
                       default='all', help='Group of types to rename')
    parser.add_argument('--include-struct-tags', action='store_true',
                       help='Include struct tag renames (enabled by default for --group=all)')
    parser.add_argument('--function', nargs='+', 
                       help='Specific functions to rename (e.g., ar_data__transfer_ownership:ar_data__drop_ownership)')
    
    args = parser.parse_args()
    
    # Determine which types to rename
    type_mapping = {}
    struct_tag_mapping = {}
    enum_value_mapping = {}
    
    if args.types:
        # Only rename specified types
        type_mapping = {t: TYPE_RENAMES[t] for t in args.types if t in TYPE_RENAMES}
        if not type_mapping:
            print("Error: No valid types specified")
            return 1
        # No struct tags or enum values when specific types are requested
    elif args.group == 'struct-tags':
        # Only rename struct tags
        struct_tag_mapping = STRUCT_TAG_RENAMES
    elif args.group == 'enum-values':
        # Only rename enum values
        enum_value_mapping = ENUM_VALUE_RENAMES
    elif args.group == 'data-enums':
        # Only rename data type enum values
        data_enum_keys = ['DATA_INTEGER', 'DATA_DOUBLE', 'DATA_STRING', 'DATA_LIST', 'DATA_MAP']
        enum_value_mapping = {k: ENUM_VALUE_RENAMES[k] for k in data_enum_keys}
    elif args.group == 'event-enums':
        # Only rename event type enum values
        event_enum_keys = ['AR_EVENT_ERROR', 'AR_EVENT_WARNING', 'AR_EVENT_INFO']
        enum_value_mapping = {k: ENUM_VALUE_RENAMES[k] for k in event_enum_keys}
    elif args.group == 'expr-enums':
        # Only rename expression AST type enum values
        expr_enum_keys = ['AR_EXPR__LITERAL_INT', 'AR_EXPR__LITERAL_DOUBLE', 'AR_EXPR__LITERAL_STRING',
                         'AR_EXPR__MEMORY_ACCESS', 'AR_EXPR__BINARY_OP']
        enum_value_mapping = {k: ENUM_VALUE_RENAMES[k] for k in expr_enum_keys}
    elif args.group == 'op-enums':
        # Only rename binary operator enum values
        op_enum_keys = ['AR_OP__ADD', 'AR_OP__SUBTRACT', 'AR_OP__MULTIPLY', 'AR_OP__DIVIDE',
                       'AR_OP__EQUAL', 'AR_OP__NOT_EQUAL', 'AR_OP__LESS', 'AR_OP__LESS_EQ',
                       'AR_OP__GREATER', 'AR_OP__GREATER_EQ']
        enum_value_mapping = {k: ENUM_VALUE_RENAMES[k] for k in op_enum_keys}
    elif args.group == 'inst-enums':
        # Only rename instruction type enum values
        inst_enum_keys = ['INST_ASSIGNMENT', 'INST_SEND', 'INST_IF', 'INST_METHOD', 
                         'INST_AGENT', 'INST_DESTROY', 'INST_PARSE']
        enum_value_mapping = {k: ENUM_VALUE_RENAMES[k] for k in inst_enum_keys}
    elif args.group == 'inst-ast-enums':
        # Only rename instruction AST type enum values
        inst_ast_enum_keys = ['AR_INST__ASSIGNMENT', 'AR_INST__SEND', 'AR_INST__IF', 'AR_INST__METHOD',
                             'AR_INST__AGENT', 'AR_INST__DESTROY_AGENT', 'AR_INST__DESTROY_METHOD',
                             'AR_INST__PARSE', 'AR_INST__BUILD']
        enum_value_mapping = {k: ENUM_VALUE_RENAMES[k] for k in inst_ast_enum_keys}
    elif args.group == 'file-enums':
        # Only rename file result enum values
        file_enum_keys = ['FILE_SUCCESS', 'FILE_ERROR_OPEN', 'FILE_ERROR_READ', 'FILE_ERROR_WRITE',
                         'FILE_ERROR_PERMISSIONS', 'FILE_ERROR_NOT_FOUND', 'FILE_ERROR_CORRUPT',
                         'FILE_ERROR_ALREADY_EXISTS', 'FILE_ERROR_UNKNOWN']
        enum_value_mapping = {k: ENUM_VALUE_RENAMES[k] for k in file_enum_keys}
    elif args.group == 'zig-structs':
        # Only rename Zig struct types
        type_mapping = ZIG_STRUCT_RENAMES
    elif args.group == 'modules':
        # Handle module renames - pass to run method separately
        pass  # Will be handled specially in run method
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
        
        # Include struct tags if requested
        if args.include_struct_tags:
            struct_tag_groups = {
                'core': ['data_s', 'list_s', 'map_s'],
                'domain': ['agent_s', 'method_s', 'agent_registry_s'],
                'context': ['expression_context_s', 'instruction_context_s'],
                'parser': ['parsed_instruction_s', 'instruction_parser_s', 'instruction_evaluator_s'],
                'system': ['interpreter_s', 'instruction_fixture_s', 'interpreter_fixture_s', 
                          'method_fixture_s', 'system_fixture_s'],
            }
            struct_tag_names = struct_tag_groups.get(args.group, [])
            struct_tag_mapping = {t: STRUCT_TAG_RENAMES[t] for t in struct_tag_names}
    else:
        # Rename all types, struct tags, enum values, and Zig structs
        type_mapping = {**TYPE_RENAMES, **ZIG_STRUCT_RENAMES}
        struct_tag_mapping = STRUCT_TAG_RENAMES
        enum_value_mapping = ENUM_VALUE_RENAMES
    
    # Create renamer and run
    renamer = TypeRenamer(dry_run=not args.live, verbose=args.verbose)
    
    # Handle module renames specially
    module_mapping = {}
    if args.group == 'modules':
        module_mapping = MODULE_RENAMES
    elif args.group == 'all':
        module_mapping = MODULE_RENAMES
    
    # Handle function renames
    function_mapping = {}
    if args.function:
        # Parse function rename arguments
        for func_rename in args.function:
            if ':' not in func_rename:
                print(f"Error: Function rename must be in format 'old_name:new_name', got '{func_rename}'")
                return 1
            old_func, new_func = func_rename.split(':', 1)
            function_mapping[old_func] = new_func
    elif args.group == 'all':
        function_mapping = FUNCTION_RENAMES
    
    renamer.run(type_mapping, struct_tag_mapping, enum_value_mapping, module_mapping, function_mapping)
    
    if not args.live:
        print("\nThis was a dry run. Use --live to actually modify files.")
    
    return 0


if __name__ == '__main__':
    sys.exit(main())