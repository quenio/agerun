#!/bin/bash

# Documentation validation script for AgeRun
# Checks file references, module names, and function/type consistency

echo "=== AgeRun Documentation Check ==="
echo "Starting at $(date)"
echo

# Track overall status
overall_status=0

# Step 1: Check documentation file references
echo "Checking documentation file references..."

# Find all markdown files except TODO.md and CHANGELOG.md
doc_files=$(find . -name "*.md" -not -path "./bin/*" -not -name "TODO.md" -not -name "CHANGELOG.md" 2>/dev/null | sort)

if [ -z "$doc_files" ]; then
    echo "Documentation check: No markdown files found ⚠️"
else
    all_refs_valid=true
    broken_refs=""
    checked_files=0
    
    for doc in $doc_files; do
        checked_files=$((checked_files + 1))
        
        # Extract file references that look like project files (ar_*.* or agerun_*.*)
        # Match any extension to check if the file exists
        file_refs=$(grep -Eo '\b(ar_|agerun_)[a-zA-Z0-9_/-]+\.[a-zA-Z0-9]+\b' "$doc" 2>/dev/null | sort -u)
        
        for ref in $file_refs; do
            # Skip URLs and anchor references
            if [[ "$ref" == *"://"* ]] || [[ "$ref" == *"#"* ]]; then
                continue
            fi
            
            # Check if this is an old agerun_ reference
            if [[ "$ref" == agerun_* ]]; then
                all_refs_valid=false
                broken_refs="$broken_refs\n  - $doc contains outdated reference: $ref (should be ar_*)"
                continue
            fi
            
            # Check if the referenced file exists
            # First try relative to the doc file's directory
            doc_dir=$(dirname "$doc")
            if [ ! -f "$doc_dir/$ref" ] && [ ! -f "$ref" ] && [ ! -f "modules/$ref" ] && [ ! -f "methods/$ref" ]; then
                all_refs_valid=false
                broken_refs="$broken_refs\n  - $doc references missing file: $ref"
            fi
        done
    done
    
    if $all_refs_valid; then
        echo "Documentation check: $checked_files files checked, all references valid ✓"
    else
        echo "Documentation check: BROKEN REFERENCES FOUND ✗"
        echo -e "$broken_refs"
        overall_status=1
    fi
fi

# Step 2: Check module name consistency
echo
echo "Checking module name consistency..."

# First, build a list of valid module names from actual files
valid_modules=""
module_files=$(find modules -name "ar_*.c" -o -name "ar_*.h" | grep -v "_tests" | sort -u)

for module_file in $module_files; do
    module_name=$(basename "$module_file" | sed 's/\.[ch]$//')
    # Add to list if not already there
    if [[ ! " $valid_modules " =~ " $module_name " ]]; then
        valid_modules="$valid_modules $module_name"
    fi
done

if [ -z "$valid_modules" ]; then
    echo "Module name check: No module files found ⚠️"
else
    all_names_valid=true
    name_mismatches=""
    
    # Find all markdown files to check (except TODO.md and CHANGELOG.md)
    doc_files=$(find . -name "*.md" -not -path "./bin/*" -not -name "TODO.md" -not -name "CHANGELOG.md" 2>/dev/null | sort)
    
    for doc in $doc_files; do
        # Look for module name references in backticks (e.g., `module_name`)
        # This catches module references in any context
        module_refs=$(grep -Eo '\`[a-zA-Z_]+[a-zA-Z0-9_]*\`' "$doc" 2>/dev/null | sort -u)
        
        for ref in $module_refs; do
            # Extract the name from backticks
            module_ref=$(echo "$ref" | sed 's/`//g')
            
            # Check if this looks like it could be a module name
            # Module names typically have format: ar_xxx or agerun_xxx
            # Exclude function names (those with __ in them) and type names (ending with _t)
            if [[ "$module_ref" =~ ^[a-zA-Z]+_[a-zA-Z0-9_]+$ ]] && [[ ! "$module_ref" =~ __ ]] && [[ ! "$module_ref" =~ _t$ ]]; then
                # Check if there should be a module file for this reference
                # but the file doesn't exist
                if [ ! -f "modules/${module_ref}.c" ] && [ ! -f "modules/${module_ref}.h" ]; then
                    # Check if this might be a typo or outdated reference
                    # by seeing if a similar valid module exists
                    possible_match=""
                    for valid_module in $valid_modules; do
                        # Check for common patterns like agerun_ vs ar_
                        if [[ "$module_ref" =~ ^agerun_ ]] && [[ "$valid_module" =~ ^ar_ ]]; then
                            suffix="${module_ref#agerun_}"
                            if [ "$valid_module" = "ar_${suffix}" ]; then
                                possible_match="$valid_module"
                                break
                            fi
                        fi
                    done
                    
                    if [ ! -z "$possible_match" ]; then
                        all_names_valid=false
                        name_mismatches="$name_mismatches\n  - $doc references non-existent module '$module_ref' (should be '$possible_match')"
                    else
                        # Check if this really looks like it should be a module
                        # (e.g., has ar_ or agerun_ prefix, or matches module naming patterns)
                        if [[ "$module_ref" =~ ^(ar_|agerun_)[a-zA-Z0-9_]+$ ]]; then
                            all_names_valid=false
                            name_mismatches="$name_mismatches\n  - $doc references non-existent module '$module_ref'"
                        fi
                    fi
                fi
            fi
        done
    done
    
    # Count for summary
    module_count=$(echo "$valid_modules" | wc -w)
    doc_count=$(echo "$doc_files" | wc -l)
    
    if $all_names_valid; then
        echo "Module name check: $doc_count docs checked, all module references exist ✓"
    else
        echo "Module name check: MODULE NAME INCONSISTENCIES FOUND ✗"
        echo -e "$name_mismatches"
        overall_status=1
    fi
fi

# Step 3: Check function and type references in documentation
echo
echo -n "Checking function and type references..."

# Find all .h files to extract function and type definitions
header_files=$(find modules -name "*.h" -type f 2>/dev/null | sort)

if [ -z "$header_files" ]; then
    echo "Function/type check: No header files found ⚠️"
else
    # Extract all function names and type names from headers
    all_functions=""
    all_types=""
    
    for header in $header_files; do
        # Extract function declarations (ar_module__function pattern)
        # Match function declarations that may have return types, asterisks, etc.
        functions=$(grep -E 'ar_[a-zA-Z0-9]+__[a-zA-Z0-9_]+\s*\(' "$header" 2>/dev/null | \
                   sed -E 's/.*[^a-zA-Z0-9_](ar_[a-zA-Z0-9]+__[a-zA-Z0-9_]+)\s*\(.*/\1/' | sort -u)
        all_functions="$all_functions $functions"
        
        # Extract typedef struct names
        # Pattern: typedef struct name_s name_t;
        types=$(grep -E 'typedef\s+struct\s+[a-zA-Z0-9_]+\s+[a-zA-Z0-9_]+;' "$header" 2>/dev/null | \
               awk '{print $NF}' | sed 's/;//' | sort -u)
        all_types="$all_types $types"
        
        # Extract enum types - simpler approach
        # Look for lines with } followed by a type name and semicolon
        enum_types=$(grep -E '^\s*}\s*[a-zA-Z0-9_]+\s*;' "$header" 2>/dev/null | \
                    awk '{gsub(/^.*}[ \t]*/, ""); gsub(/[ \t]*;.*$/, ""); print}' | sort -u)
        all_types="$all_types $enum_types"
        
        # Also extract simple typedefs (typedef old_type new_type;)
        simple_types=$(grep -E '^typedef\s+[a-zA-Z0-9_]+\s+[a-zA-Z0-9_]+;' "$header" 2>/dev/null | \
                      sed -E 's/^typedef\s+[a-zA-Z0-9_]+\s+([a-zA-Z0-9_]+);.*/\1/' | sort -u)
        all_types="$all_types $simple_types"
    done
    
    # Remove duplicates and sort
    all_functions=$(echo $all_functions | tr ' ' '\n' | sort -u | tr '\n' ' ')
    all_types=$(echo $all_types | tr ' ' '\n' | sort -u | tr '\n' ' ')
    
    # Find all documentation files (excluding TODO.md and CHANGELOG.md)
    doc_files=$(find . -name "*.md" -type f ! -path "./bin/*" ! -name "TODO.md" ! -name "CHANGELOG.md" 2>/dev/null | sort)
    
    all_refs_valid=true
    broken_function_refs=""
    broken_type_refs=""
    checked_files=0
    
    for doc in $doc_files; do
        checked_files=$((checked_files + 1))
        
        # First check for incorrect double underscore patterns (ar__module__)
        double_underscore_refs=$(grep -Eo 'ar__[a-zA-Z0-9_]+__[a-zA-Z0-9_]+' "$doc" 2>/dev/null | sort -u)
        for bad_ref in $double_underscore_refs; do
            all_refs_valid=false
            broken_function_refs="$broken_function_refs\n  - $doc contains invalid double underscore pattern '$bad_ref' (should be ar_module__function)"
        done
        
        # Look for function references in backticks (e.g., `ar_module__function()`)
        function_refs=$(grep -Eo '\`ar_[a-zA-Z0-9]+__[a-zA-Z0-9_]+\(\)?\`' "$doc" 2>/dev/null | \
                       sed -E 's/\`(ar_[a-zA-Z0-9]+__[a-zA-Z0-9_]+)\(\)?\`/\1/' | sort -u)
        
        for func_ref in $function_refs; do
            # Check if this function exists in our extracted functions
            if ! echo "$all_functions" | grep -qw "$func_ref"; then
                all_refs_valid=false
                broken_function_refs="$broken_function_refs\n  - $doc references non-existent function '$func_ref'"
            fi
        done
        
        # Look for type references in backticks
        # Focus on patterns that are clearly types:
        # - Ends with _t (common C typedef pattern)
        # - Ensure _t is not followed by a letter (to avoid matching ar_expression_take_ownership)
        # - Also match capitalized types (less common but possible)
        type_refs=$(grep -Eo '\`[a-zA-Z][a-zA-Z0-9_]*_t([^a-zA-Z]|$)' "$doc" 2>/dev/null | \
                   sed -E 's/`([a-zA-Z][a-zA-Z0-9_]*_t).*/\1/' | sort -u)
        # Also look for capitalized types
        cap_types=$(grep -Eo '\`[A-Z][a-zA-Z0-9_]*\`' "$doc" 2>/dev/null | \
                   sed -E 's/`([A-Z][a-zA-Z0-9_]*)`/\1/' | sort -u)
        type_refs="$type_refs $cap_types"
        
        for type_ref in $type_refs; do
            # Skip standard C types and common types
            if [[ "$type_ref" == "int64_t" ]] || [[ "$type_ref" == "uint64_t" ]] || \
               [[ "$type_ref" == "int32_t" ]] || [[ "$type_ref" == "uint32_t" ]] || \
               [[ "$type_ref" == "int16_t" ]] || [[ "$type_ref" == "uint16_t" ]] || \
               [[ "$type_ref" == "int8_t" ]] || [[ "$type_ref" == "uint8_t" ]] || \
               [[ "$type_ref" == "size_t" ]] || [[ "$type_ref" == "ssize_t" ]] || \
               [[ "$type_ref" == "ptrdiff_t" ]] || [[ "$type_ref" == "uintptr_t" ]] || \
               [[ "$type_ref" == "intptr_t" ]] || [[ "$type_ref" == "FILE" ]] || \
               [[ "$type_ref" == "bool" ]] || [[ "$type_ref" == "char" ]] || \
               [[ "$type_ref" == "int" ]] || [[ "$type_ref" == "long" ]] || \
               [[ "$type_ref" == "float" ]] || [[ "$type_ref" == "double" ]] || \
               [[ "$type_ref" == "void" ]] || [[ "$type_ref" == "const" ]] || \
               [[ "$type_ref" == "unsigned" ]] || [[ "$type_ref" == "signed" ]] || \
               [[ "$type_ref" == "NULL" ]] || [[ "$type_ref" == "PRId64" ]] || \
               [[ "$type_ref" == "PRIu64" ]]; then
                continue
            fi
            
            # Skip if it's clearly a function reference (contains __)
            if [[ "$type_ref" == *"__"* ]]; then
                continue
            fi
            
            # Skip if it's a module name (was already checked in previous step)
            # Module names follow pattern ar_<name> without _t suffix
            if [[ "$type_ref" =~ ^ar_[a-zA-Z0-9_]+$ ]] && [[ ! "$type_ref" =~ _t$ ]]; then
                continue
            fi
            
            # Skip if it's just an ownership prefix with _t (these come from variable names like own_tail)
            if [[ "$type_ref" == "own_t" ]] || [[ "$type_ref" == "mut_t" ]] || [[ "$type_ref" == "ref_t" ]]; then
                continue
            fi
            
            # Skip constants and macros (all uppercase, possibly with underscores)
            if [[ "$type_ref" =~ ^[A-Z][A-Z0-9_]*$ ]]; then
                continue
            fi
            
            # For suspicious simple types (word_t pattern), check if they appear as part of longer identifiers
            # This filters out things like output_t from output_template, echo_t from echo_tests.c
            if [[ "$type_ref" =~ ^[a-z]+_t$ ]]; then
                # Check if this appears as part of a longer identifier in the same document
                base_name="${type_ref%_t}"
                if grep -qE "${base_name}_(template|tests|config|data|value|name|id|type)" "$doc" 2>/dev/null; then
                    continue
                fi
            fi
            
            # Check if this type exists in our extracted types
            if ! echo "$all_types" | grep -qw "$type_ref"; then
                all_refs_valid=false
                broken_type_refs="$broken_type_refs\n  - $doc references non-existent type '$type_ref'"
            fi
        done
    done
    
    if $all_refs_valid; then
        echo "Function/type check: $checked_files docs checked, all references valid ✓"
    else
        echo "Function/type check: INVALID REFERENCES FOUND ✗"
        echo "(Note: These may be documentation errors that need fixing)"
        if [ -n "$broken_function_refs" ]; then
            echo -e "$broken_function_refs"
        fi
        if [ -n "$broken_type_refs" ]; then
            echo -e "$broken_type_refs"
        fi
        overall_status=1
    fi
fi

echo
echo "=== Documentation Check Summary ==="
echo "Completed at $(date)"

# Exit with appropriate status
exit $overall_status