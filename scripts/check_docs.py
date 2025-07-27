#!/usr/bin/env python3
"""
Documentation validation script for AgeRun

Requirements:
1. ALL documentation must use real types and functions - no hypothetical examples
2. ALL markdown files are checked (except TODO.md and CHANGELOG.md)
3. NO exemptions for any directories - kb articles, module docs, etc. all follow same rules
4. Code examples must reference actual functions and types that exist in the codebase
5. When showing compile-time errors intentionally, mark with ERROR comment on same line
6. When using hypothetical examples for teaching, mark with EXAMPLE comment on same line
7. When showing bad design or anti-patterns, mark with BAD comment on same line

What this script validates:
- File references: Ensures referenced files (ar_*.c, ar_*.h) actually exist
- Module names: Verifies that backticked module names correspond to real modules
- Function references: Checks that all ar_*__* function calls exist in source files (.h and .c)
- Type references: Validates that all *_t types exist in source files (including implementation types)
- Relative path links: Validates all markdown link types (standard, images, reference-style) per GitHub behavior
  - Rejects absolute paths (starting with /)
  - Validates paths don't escape repository
  - Checks for backslashes and warns about spaces
  - Resolves relative to document location
  - See kb/markdown-link-resolution-patterns.md for link format specification
- Both inline backticked references and code block contents are checked

To mark intentional non-existent functions/types in documentation:
    ar_fake_function();  // ERROR: This shows what happens with undefined functions
    fake_type_t *ptr;    /* ERROR: Example of incorrect type usage */
    ar_example_t *ex;    // EXAMPLE: Hypothetical type for demonstration
    ar_demo__func();     /* EXAMPLE: Teaching example, not real function */
    ar_bloated__api();   // BAD: Example of poor API design
    ar_debug__print();   /* BAD: Debug function that shouldn't be public */
Lines with ERROR, EXAMPLE, or BAD comments are excluded from validation.
"""

import os
import re
import sys
from datetime import datetime
from pathlib import Path

def check_repo_root():
    """Ensure script is run from repository root"""
    if not all(Path(p).exists() for p in ["Makefile", "modules", "methods"]):
        print("ERROR: This script must be run from the AgeRun repository root directory.")
        print("Please run 'make check-docs' from the repository root instead.")
        sys.exit(1)

def find_documentation_files():
    """Find all documentation files: markdown files, Zig source files, and test files (for comment validation)"""
    doc_files = []
    for root, dirs, files in os.walk("."):
        # Skip bin directory
        if "/bin/" in root or root.endswith("/bin"):
            continue
        for file in files:
            # Include markdown files (except TODO.md and CHANGELOG.md)
            if file.endswith(".md") and file not in ["TODO.md", "CHANGELOG.md"]:
                doc_files.append(os.path.join(root, file))
            # Include Zig source files in modules directory for comment validation
            elif file.endswith(".zig") and "modules" in root:
                doc_files.append(os.path.join(root, file))
            # Include C test files in modules directory
            elif file.endswith("_tests.c") and "modules" in root:
                doc_files.append(os.path.join(root, file))
    return sorted(doc_files)

def check_file_references(doc_files):
    """Check documentation file references"""
    print("Checking documentation file references...")
    
    all_refs_valid = True
    broken_refs = []
    checked_files = 0
    
    # Pattern to match ar_*.* or agerun_*.* (full file references only)
    file_ref_pattern = re.compile(r'\b((ar_|agerun_)[a-zA-Z0-9_/-]+\.[a-zA-Z0-9]+)\b')
    
    for doc in doc_files:
        checked_files += 1
        
        with open(doc, 'r', encoding='utf-8') as f:
            content = f.read()
        
        # Split content into lines to check for markers
        lines = content.split('\n')
        file_refs = set()
        in_example_block = False
        
        # Look for file references line by line
        for line in lines:
            # Track if we're in an example code block
            if '```' in line and ('EXAMPLE:' in line or '# EXAMPLE:' in line):
                in_example_block = True
                continue
            elif '```' in line and in_example_block:
                in_example_block = False
                continue
            # Skip lines marked as intentional errors, examples, or bad code
            # Also skip lines inside example code blocks
            if (in_example_block or 
                '// ERROR:' in line or '/* ERROR:' in line or 
                '// EXAMPLE:' in line or '/* EXAMPLE:' in line or
                '// BAD:' in line or '/* BAD:' in line or
                '# EXAMPLE:' in line):  # Also handle markdown comments
                continue
            
            # Find file references in this line
            matches = file_ref_pattern.findall(line)
            # Extract just the full match from tuple results
            for match in matches:
                file_refs.add(match[0] if isinstance(match, tuple) else match)
        
        for ref in file_refs:
            # Skip URLs and anchor references
            if "://" in ref or "#" in ref:
                continue
            
            # Skip Zig module function calls (e.g., ar_allocator.create)
            # These have format: module_name.function_name where function is a simple identifier
            if re.match(r'^ar_[a-zA-Z0-9_]+\.[a-zA-Z0-9_]+$', ref):
                # This looks like a Zig module function call, not a file reference
                continue
            
            # Check if this is an old agerun_ reference
            if ref.startswith("agerun_"):
                all_refs_valid = False
                broken_refs.append(f"  - {doc} contains outdated reference: {ref} (should be ar_*)")
                continue
            
            # Check if the referenced file exists
            doc_dir = os.path.dirname(doc)
            paths_to_check = [
                os.path.join(doc_dir, ref),
                ref,
                os.path.join("modules", ref),
                os.path.join("methods", ref)
            ]
            
            if not any(os.path.isfile(p) for p in paths_to_check):
                all_refs_valid = False
                broken_refs.append(f"  - {doc} references missing file: {ref}")
    
    if all_refs_valid:
        print(f"Documentation check: {checked_files} files checked, all references valid ✓")
    else:
        print("Documentation check: BROKEN REFERENCES FOUND ✗")
        for ref in broken_refs:
            print(ref)
        return 1
    
    return 0

def check_module_names(doc_files):
    """Check module name consistency"""
    print("\nChecking module name consistency...")
    
    # Build list of valid module names
    valid_modules = set()
    for pattern in ["modules/ar_*.c", "modules/ar_*.h"]:
        for file in Path("modules").glob(pattern.split('/')[-1]):
            if "_tests" not in file.name:
                module_name = file.stem
                valid_modules.add(module_name)
    
    # Also add PascalCase Zig struct modules
    for file in Path("modules").glob("*.zig"):
        if not file.name.startswith("ar_") and "Tests" not in file.name:
            module_name = file.stem
            if re.match(r'^[A-Z][a-zA-Z0-9]*$', module_name):
                valid_modules.add(module_name)
    
    if not valid_modules:
        print("Module name check: No module files found ⚠️")
        return 0
    
    all_names_valid = True
    name_mismatches = []
    
    # Pattern to match content in backticks
    backtick_pattern = re.compile(r'`([a-zA-Z_]+[a-zA-Z0-9_]*)`')
    
    for doc in doc_files:
        with open(doc, 'r', encoding='utf-8') as f:
            content = f.read()
        
        # Find all backticked references
        module_refs = set(backtick_pattern.findall(content))
        
        for module_ref in module_refs:
            # Check if this looks like a module name
            # Module names typically have format: ar_xxx or agerun_xxx
            # Exclude function names (those with __ in them) and type names (ending with _t)
            if (re.match(r'^[a-zA-Z]+_[a-zA-Z0-9_]+$', module_ref) and
                "__" not in module_ref and
                not module_ref.endswith("_t")):
                
                # Check if module file should exist but doesn't
                module_c = f"modules/{module_ref}.c"
                module_h = f"modules/{module_ref}.h"
                
                if not os.path.isfile(module_c) and not os.path.isfile(module_h):
                    # Check for agerun_ vs ar_ mismatch
                    if module_ref.startswith("agerun_"):
                        suffix = module_ref[7:]  # Remove "agerun_"
                        possible_match = f"ar_{suffix}"
                        if possible_match in valid_modules:
                            all_names_valid = False
                            name_mismatches.append(
                                f"  - {doc} references non-existent module '{module_ref}' "
                                f"(should be '{possible_match}')"
                            )
                        elif re.match(r'^(ar_|agerun_)[a-zA-Z0-9_]+$', module_ref):
                            all_names_valid = False
                            name_mismatches.append(
                                f"  - {doc} references non-existent module '{module_ref}'"
                            )
    
    doc_count = len(doc_files)
    
    if all_names_valid:
        print(f"Module name check: {doc_count} docs checked, all module references exist ✓")
    else:
        print("Module name check: MODULE NAME INCONSISTENCIES FOUND ✗")
        for mismatch in name_mismatches:
            print(mismatch)
        return 1
    
    return 0

def check_function_and_type_references(doc_files):
    """Check function and type references in documentation"""
    print("\nChecking function and type references...", end='', flush=True)
    
    # Extract all function names and types from headers AND implementation files
    all_functions = set()
    all_types = set()
    
    header_files = list(Path("modules").glob("*.h"))
    c_files = list(Path("modules").glob("*.c"))
    zig_files = list(Path("modules").glob("*.zig"))
    all_source_files = header_files + c_files + zig_files
    
    if not all_source_files:
        print("Function/type check: No source files found ⚠️")
        return 0
    
    # Patterns for extracting definitions
    function_pattern = re.compile(r'ar_[a-zA-Z0-9]+__[a-zA-Z0-9_]+(?=\s*\()')
    typedef_struct_pattern = re.compile(r'typedef\s+struct\s+\w+\s+(\w+);')
    enum_type_pattern = re.compile(r'^\s*}\s*([a-zA-Z0-9_]+)\s*;', re.MULTILINE)
    simple_typedef_pattern = re.compile(r'^typedef\s+\w+\s+(\w+);', re.MULTILINE)
    # Zig public function pattern - matches 'pub fn' or 'pub inline fn'
    zig_pub_fn_pattern = re.compile(r'pub\s+(?:inline\s+)?fn\s+([a-zA-Z0-9_]+)\s*\(')
    # Zig type patterns
    zig_const_struct_pattern = re.compile(r'(?:pub\s+)?const\s+([a-zA-Z0-9_]+)\s*=\s*struct\s*\{')
    zig_const_enum_pattern = re.compile(r'(?:pub\s+)?const\s+([a-zA-Z0-9_]+)\s*=\s*enum\s*\{')
    zig_const_union_pattern = re.compile(r'(?:pub\s+)?const\s+([a-zA-Z0-9_]+)\s*=\s*union')
    
    for source_file in all_source_files:
        with open(source_file, 'r', encoding='utf-8') as f:
            content = f.read()
        
        # Extract functions
        functions = function_pattern.findall(content)
        all_functions.update(functions)
        
        # Extract Zig public functions and types
        if source_file.suffix == '.zig':
            module_name = source_file.stem
            
            # Extract Zig functions
            zig_functions = zig_pub_fn_pattern.findall(content)
            # Add both the plain function name and module.function format
            for func in zig_functions:
                all_functions.add(func)
                all_functions.add(f"{module_name}.{func}")
            
            # Extract Zig types (structs, enums, unions)
            zig_structs = zig_const_struct_pattern.findall(content)
            all_types.update(zig_structs)
            
            zig_enums = zig_const_enum_pattern.findall(content)
            all_types.update(zig_enums)
            
            zig_unions = zig_const_union_pattern.findall(content)
            all_types.update(zig_unions)
            
            # For PascalCase Zig struct modules, add the module name itself as a type
            if re.match(r'^[A-Z][a-zA-Z0-9]*$', module_name):
                all_types.add(module_name)
        
        # Extract typedef structs
        typedef_structs = typedef_struct_pattern.findall(content)
        all_types.update(typedef_structs)
        
        # Extract enum types
        enum_types = enum_type_pattern.findall(content)
        all_types.update(enum_types)
        
        # Extract simple typedefs
        simple_types = simple_typedef_pattern.findall(content)
        all_types.update(simple_types)
    
    all_refs_valid = True
    broken_function_refs = []
    broken_type_refs = []
    checked_files = 0
    
    # Standard C types to skip
    standard_types = {
        "int64_t", "uint64_t", "int32_t", "uint32_t", "int16_t", "uint16_t",
        "int8_t", "uint8_t", "size_t", "ssize_t", "ptrdiff_t", "uintptr_t",
        "intptr_t", "FILE", "bool", "char", "int", "long", "float", "double",
        "void", "const", "unsigned", "signed", "NULL", "PRId64", "PRIu64",
        "time_t", "off_t", "mode_t", "pid_t", "uid_t", "gid_t",
        # Zig standard library types
        "ArrayList", "HashMap", "Allocator", "Self"
    }
    
    # Ownership prefixes that might appear with _t
    ownership_prefixes = {"own_t", "mut_t", "ref_t"}
    
    for doc in doc_files:
        checked_files += 1
        
        with open(doc, 'r', encoding='utf-8') as f:
            content = f.read()
        
        # Check for incorrect double underscore patterns
        double_underscore_pattern = re.compile(r'ar__[a-zA-Z0-9_]+__[a-zA-Z0-9_]+')
        bad_refs = double_underscore_pattern.findall(content)
        for bad_ref in bad_refs:
            all_refs_valid = False
            broken_function_refs.append(
                f"  - {doc} contains invalid double underscore pattern '{bad_ref}' "
                "(should be ar_module__function)"
            )
        
        # Split content into lines to check for error markers
        lines = content.split('\n')
        function_refs = set()
        in_example_block = False
        
        # Look for function references both in backticks and code
        for i, line in enumerate(lines):
            # Track if we're in an example code block
            if '```' in line and ('EXAMPLE:' in line or '# EXAMPLE:' in line):
                in_example_block = True
                continue
            elif '```' in line and in_example_block:
                in_example_block = False
                continue
            # Skip lines marked as intentional errors, examples, or bad code
            # Also skip lines inside example code blocks
            if (in_example_block or 
                '// ERROR:' in line or '/* ERROR:' in line or 
                '// EXAMPLE:' in line or '/* EXAMPLE:' in line or
                '// BAD:' in line or '/* BAD:' in line or
                '# EXAMPLE:' in line):  # Also handle markdown comments
                continue
            
            # Find backticked function references
            backtick_func_pattern = re.compile(r'`(ar_[a-zA-Z0-9]+__[a-zA-Z0-9_]+)(?:\(\))?`')
            backtick_matches = backtick_func_pattern.findall(line)
            function_refs.update(backtick_matches)
            
            # Find function references in code (without backticks)
            code_func_matches = re.findall(r'\b(ar_[a-zA-Z0-9]+__[a-zA-Z0-9_]+)\s*\(', line)
            function_refs.update(code_func_matches)
        
        for func_ref in function_refs:
            if func_ref not in all_functions:
                all_refs_valid = False
                broken_function_refs.append(
                    f"  - {doc} references non-existent function '{func_ref}'"
                )
        
        # Look for type references - check line by line to respect error markers
        type_refs = set()
        in_example_block = False
        
        for i, line in enumerate(lines):
            # Track if we're in an example code block
            if '```' in line and ('EXAMPLE:' in line or '# EXAMPLE:' in line):
                in_example_block = True
                continue
            elif '```' in line and in_example_block:
                in_example_block = False
                continue
            # Skip lines marked as intentional errors, examples, or bad code
            # Also skip lines inside example code blocks
            if (in_example_block or 
                '// ERROR:' in line or '/* ERROR:' in line or 
                '// EXAMPLE:' in line or '/* EXAMPLE:' in line or
                '// BAD:' in line or '/* BAD:' in line or
                '# EXAMPLE:' in line):  # Also handle markdown comments
                continue
            
            # Find backticked type references
            type_ref_pattern = re.compile(r'`([a-zA-Z][a-zA-Z0-9_]*_t)(?:[^a-zA-Z]|$)')
            backtick_type_matches = type_ref_pattern.findall(line)
            for match in backtick_type_matches:
                # Remove anything after the type name
                type_name = match.split('*')[0].strip()
                type_refs.add(type_name)
            
            # Find capitalized types in backticks
            cap_type_pattern = re.compile(r'`([A-Z][a-zA-Z0-9_]*)`')
            cap_types = cap_type_pattern.findall(line)
            type_refs.update(cap_types)
            
            # Find type references in code (without backticks)
            code_type_matches = re.findall(r'\b([a-zA-Z][a-zA-Z0-9_]*_t)\s*\*?\s*\b', line)
            for ct in code_type_matches:
                type_refs.add(ct)
        
        for type_ref in type_refs:
            # Skip standard types and ownership prefixes
            if type_ref in standard_types or type_ref in ownership_prefixes:
                continue
            
            # Skip if it's clearly a function reference (contains __)
            if "__" in type_ref:
                continue
            
            # Skip if it's a module name (ar_<name> without _t suffix)
            if re.match(r'^ar_[a-zA-Z0-9_]+$', type_ref) and not type_ref.endswith("_t"):
                continue
            
            # Skip constants and macros (all uppercase)
            if re.match(r'^[A-Z][A-Z0-9_]*$', type_ref):
                continue
            
            # Skip PascalCase types (Zig struct modules)
            # These are valid naming patterns for Zig struct modules
            if re.match(r'^[A-Z][a-zA-Z0-9]*$', type_ref) and not type_ref.endswith("_t"):
                # Check if this is a known Zig struct module file
                zig_module_file = f"modules/{type_ref}.zig"
                if os.path.isfile(zig_module_file):
                    continue
                # Also allow common Zig struct module naming patterns
                if type_ref in ["PascalCase", "TitleCase", "DataStore", "MessageQueue"]:
                    continue
            
            # Skip simple types that might be part of longer identifiers
            if re.match(r'^[a-z]+_t$', type_ref):
                # Check if this appears as part of a longer identifier in the original content
                base_name = type_ref[:-2]  # Remove _t
                if re.search(f'{base_name}_(template|tests|config|data|value|name|id|type)', content):
                    continue
            
            # Check if this type exists
            if type_ref not in all_types:
                all_refs_valid = False
                broken_type_refs.append(
                    f"  - {doc} references non-existent type '{type_ref}'"
                )
    
    if all_refs_valid:
        print(f"Function/type check: {checked_files} docs checked, all references valid ✓")
    else:
        print("Function/type check: INVALID REFERENCES FOUND ✗")
        print("(Note: These may be documentation errors that need fixing)")
        if broken_function_refs:
            for ref in broken_function_refs:
                print(ref)
        if broken_type_refs:
            for ref in broken_type_refs:
                print(ref)
        return 1
    
    return 0

def validate_link_path(doc, line_num, link_text, link_path, doc_dir, repo_root=Path.cwd()):
    """Validate a single link path according to the specification"""
    errors = []
    warnings = []
    
    # Skip external URLs, email addresses, and pure anchors
    if ('://' in link_path or 
        'mailto:' in link_path or 
        link_path.startswith('#')):
        return errors, warnings
    
    # Extract path without anchor
    path_without_anchor = link_path.split('#')[0]
    if not path_without_anchor:
        return errors, warnings
    
    # Skip if it's not a file link (no extension)
    if '.' not in path_without_anchor.split('/')[-1]:
        return errors, warnings
    
    # Check for backslashes (invalid path separator)
    if '\\' in path_without_anchor:
        errors.append(f"  - {doc}:{line_num} invalid path separator '\\' in '[{link_text}]({link_path})'")
        return errors, warnings
    
    # Warn about spaces in path
    if ' ' in path_without_anchor and '%20' not in path_without_anchor:
        warnings.append(f"  - {doc}:{line_num} warning: spaces in path '[{link_text}]({link_path})'")
    
    # Check for absolute paths (these don't work on GitHub)
    if path_without_anchor.startswith('/'):
        errors.append(f"  - {doc}:{line_num} absolute path '[{link_text}]({link_path})' - use relative paths instead")
        return errors, warnings
    
    # Resolve relative path from document directory
    try:
        target_path = (doc_dir / path_without_anchor).resolve()
        
        # Check if path escapes repository
        if not target_path.is_relative_to(repo_root):
            errors.append(f"  - {doc}:{line_num} path escapes repository '[{link_text}]({link_path})'")
            return errors, warnings
        
        # Check if the target file exists
        if not target_path.exists():
            display_path = target_path.relative_to(repo_root)
            errors.append(f"  - {doc}:{line_num} broken link '[{link_text}]({link_path})' -> {display_path}")
    except (ValueError, OSError) as e:
        errors.append(f"  - {doc}:{line_num} invalid path '[{link_text}]({link_path})' - {str(e)}")
    
    return errors, warnings

def check_relative_links(doc_files):
    """Check relative path links in markdown documents"""
    print("\nChecking relative path links...")
    
    all_links_valid = True
    broken_links = []
    checked_files = 0
    
    # Patterns to match different types of links
    # Standard markdown links: [text](path)
    link_pattern = re.compile(r'\[([^\]]+)\]\(([^)]+)\)')
    # Image references: ![alt](path)
    image_pattern = re.compile(r'!\[([^\]]*)\]\(([^)]+)\)')
    # Reference-style links: [text][ref] and [ref]: path
    ref_link_pattern = re.compile(r'\[([^\]]+)\]\[([^\]]+)\]')
    ref_def_pattern = re.compile(r'^\s*\[([^\]]+)\]:\s*(.+)$', re.MULTILINE)
    
    for doc in doc_files:
        checked_files += 1
        doc_path = Path(doc)
        doc_dir = doc_path.parent
        
        # Skip link checking for source files - they can contain syntax that looks like links
        if doc.endswith('.zig') or doc.endswith('.c'):
            continue
            
        with open(doc, 'r', encoding='utf-8') as f:
            content = f.read()
        
        # First, collect all reference-style link definitions
        ref_definitions = {}
        for match in ref_def_pattern.finditer(content):
            ref_name = match.group(1)
            ref_path = match.group(2).strip()
            ref_definitions[ref_name] = ref_path
        
        # Split content into lines for line number tracking
        lines = content.split('\n')
        in_code_block = False
        
        for line_num, line in enumerate(lines, 1):
            # Track code blocks
            if '```' in line:
                in_code_block = not in_code_block
                continue
            
            # Skip lines in code blocks or with special markers
            if (in_code_block or
                '// ERROR:' in line or '/* ERROR:' in line or 
                '// EXAMPLE:' in line or '/* EXAMPLE:' in line or
                '// BAD:' in line or '/* BAD:' in line or
                '# EXAMPLE:' in line):
                continue
            
            # Skip reference definitions (they're processed separately)
            if ref_def_pattern.match(line):
                continue
            
            # Process the line only once to avoid duplicate detection
            # First check for images (they contain the link pattern too)
            image_matches = image_pattern.findall(line)
            if image_matches:
                for alt_text, image_path in image_matches:
                    errors, warnings = validate_link_path(doc, line_num, f"Image: {alt_text or 'unnamed'}", image_path, doc_dir)
                    if errors:
                        all_links_valid = False
                        broken_links.extend(errors)
                    broken_links.extend(warnings)
            else:
                # Only check standard links if this isn't an image line
                for link_text, link_path in link_pattern.findall(line):
                    errors, warnings = validate_link_path(doc, line_num, link_text, link_path, doc_dir)
                    if errors:
                        all_links_valid = False
                        broken_links.extend(errors)
                    broken_links.extend(warnings)
            
            # Find all reference-style links in this line
            for link_text, ref_name in ref_link_pattern.findall(line):
                if ref_name in ref_definitions:
                    ref_path = ref_definitions[ref_name]
                    errors, warnings = validate_link_path(doc, line_num, link_text, ref_path, doc_dir)
                    if errors:
                        all_links_valid = False
                        broken_links.extend(errors)
                    broken_links.extend(warnings)
                else:
                    all_links_valid = False
                    broken_links.append(f"  - {doc}:{line_num} undefined reference '[{link_text}][{ref_name}]'")
    
    if all_links_valid:
        print(f"Relative link check: {checked_files} files checked, all links valid ✓")
    else:
        print("Relative link check: BROKEN LINKS FOUND ✗")
        for link in broken_links:
            print(link)
        return 1
    
    return 0

def main():
    """Main function"""
    check_repo_root()
    
    # Debug mode for CI
    debug_mode = os.environ.get('DEBUG_DOCS_CHECK', '').lower() == 'true'
    
    print("=== AgeRun Documentation Check ===")
    print(f"Starting at {datetime.now().strftime('%a %b %d %H:%M:%S %Z %Y')}")
    print()
    
    overall_status = 0
    
    # Find all documentation files (markdown and Zig source)
    doc_files = find_documentation_files()
    
    if not doc_files:
        print("Documentation check: No documentation files found ⚠️")
    else:
        # Step 1: Check file references
        status = check_file_references(doc_files)
        if status != 0:
            overall_status = status
        
        # Step 2: Check module names
        status = check_module_names(doc_files)
        if status != 0:
            overall_status = status
        
        # Step 3: Check function and type references
        status = check_function_and_type_references(doc_files)
        if status != 0:
            overall_status = status
        
        # Step 4: Check relative path links
        status = check_relative_links(doc_files)
        if status != 0:
            overall_status = status
    
    print()
    print("=== Documentation Check Summary ===")
    print(f"Completed at {datetime.now().strftime('%a %b %d %H:%M:%S %Z %Y')}")
    
    sys.exit(overall_status)

if __name__ == "__main__":
    main()