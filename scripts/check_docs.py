#!/usr/bin/env python3
"""
Documentation validation script for AgeRun
Checks file references, module names, and function/type consistency
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

def find_markdown_files():
    """Find all markdown files except TODO.md and CHANGELOG.md"""
    markdown_files = []
    for root, dirs, files in os.walk("."):
        # Skip bin directory
        if "/bin/" in root or root.endswith("/bin"):
            continue
        for file in files:
            if file.endswith(".md") and file not in ["TODO.md", "CHANGELOG.md"]:
                markdown_files.append(os.path.join(root, file))
    return sorted(markdown_files)

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
        
        # Find all file references
        matches = file_ref_pattern.findall(content)
        # Extract just the full match from tuple results
        file_refs = set(match[0] if isinstance(match, tuple) else match for match in matches)
        
        for ref in file_refs:
            # Skip URLs and anchor references
            if "://" in ref or "#" in ref:
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
    
    # Extract all function names and types from headers
    all_functions = set()
    all_types = set()
    
    header_files = list(Path("modules").glob("*.h"))
    
    if not header_files:
        print("Function/type check: No header files found ⚠️")
        return 0
    
    # Patterns for extracting definitions
    function_pattern = re.compile(r'ar_[a-zA-Z0-9]+__[a-zA-Z0-9_]+(?=\s*\()')
    typedef_struct_pattern = re.compile(r'typedef\s+struct\s+\w+\s+(\w+);')
    enum_type_pattern = re.compile(r'^\s*}\s*([a-zA-Z0-9_]+)\s*;', re.MULTILINE)
    simple_typedef_pattern = re.compile(r'^typedef\s+\w+\s+(\w+);', re.MULTILINE)
    
    for header in header_files:
        with open(header, 'r', encoding='utf-8') as f:
            content = f.read()
        
        # Extract functions
        functions = function_pattern.findall(content)
        all_functions.update(functions)
        
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
        "void", "const", "unsigned", "signed", "NULL", "PRId64", "PRIu64"
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
        
        # Look for function references in backticks
        func_ref_pattern = re.compile(r'`(ar_[a-zA-Z0-9]+__[a-zA-Z0-9_]+)(?:\(\))?`')
        function_refs = set(func_ref_pattern.findall(content))
        
        # Also look for function references in code (without backticks)
        # This catches functions in code blocks
        # Only check code blocks in module documentation, not in kb articles
        if not doc.startswith('./kb/'):
            code_func_pattern = re.compile(r'\b(ar_[a-zA-Z0-9]+__[a-zA-Z0-9_]+)\s*\(')
            function_refs.update(code_func_pattern.findall(content))
        
        for func_ref in function_refs:
            if func_ref not in all_functions:
                all_refs_valid = False
                broken_function_refs.append(
                    f"  - {doc} references non-existent function '{func_ref}'"
                )
        
        # Look for type references in backticks
        # Match pattern similar to shell script: `word_t` followed by non-letter or end
        # This pattern matches what grep -Eo would match
        type_ref_pattern = re.compile(r'`([a-zA-Z][a-zA-Z0-9_]*_t)(?:[^a-zA-Z]|$)')
        matches = type_ref_pattern.findall(content)
        # Extract type names, mimicking sed behavior
        type_refs = set()
        for match in matches:
            # Remove backtick and anything after the type name
            type_name = match.split('*')[0].strip()
            type_refs.add(type_name)
        
        # Also look for capitalized types
        cap_type_pattern = re.compile(r'`([A-Z][a-zA-Z0-9_]*)`')
        cap_types = set(cap_type_pattern.findall(content))
        type_refs.update(cap_types)
        
        # Also look for type references in code (without backticks)
        # This catches types in code blocks, including with pointers
        # Only check code blocks in module documentation, not in kb articles
        if not doc.startswith('./kb/'):
            code_type_pattern = re.compile(r'\b([a-zA-Z][a-zA-Z0-9_]*_t)\s*\*?\s*\b')
            code_types = code_type_pattern.findall(content)
            for ct in code_types:
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

def main():
    """Main function"""
    check_repo_root()
    
    # Debug mode for CI
    debug_mode = os.environ.get('DEBUG_DOCS_CHECK', '').lower() == 'true'
    
    print("=== AgeRun Documentation Check ===")
    print(f"Starting at {datetime.now().strftime('%a %b %d %H:%M:%S %Z %Y')}")
    print()
    
    overall_status = 0
    
    # Find all markdown files
    doc_files = find_markdown_files()
    
    if not doc_files:
        print("Documentation check: No markdown files found ⚠️")
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
    
    print()
    print("=== Documentation Check Summary ===")
    print(f"Completed at {datetime.now().strftime('%a %b %d %H:%M:%S %Z %Y')}")
    
    sys.exit(overall_status)

if __name__ == "__main__":
    main()