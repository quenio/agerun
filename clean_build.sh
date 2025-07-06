#!/bin/bash

# Clean Build Script for AgeRun
# Performs all build steps with minimal output

echo "=== AgeRun Clean Build ==="
echo "Starting at $(date)"
echo

# Function to run command and report status
run_step() {
    local step_name="$1"
    local cmd="$2"
    
    printf "%-30s" "$step_name..."
    
    # Run command and capture output
    if output=$($cmd 2>&1); then
        echo "✓"
        return 0
    else
        echo "✗"
        echo "Error output:"
        echo "$output" | tail -20
        return 1
    fi
}

# Step 1: Clean and build
echo -n "Clean and build...            "
if make clean >/dev/null 2>&1 && make >/dev/null 2>&1; then
    echo "✓"
else
    echo "✗"
    make clean 2>&1 | tail -5
    make 2>&1 | tail -5
fi

# Step 2: Static analysis
echo -n "Static analysis (lib)...      "
output=$(make clean analyze 2>&1)
exit_code=$?
if [ $exit_code -eq 0 ]; then
    echo "✓"
else
    echo "✗"
    # Check if it's due to static analysis findings
    if echo "$output" | grep -q "Static analysis FAILED:"; then
        echo "Static analysis found issues:"
        # Extract file-specific errors (e.g., "✗ 1 bugs found in modules/ar_interpreter.c")
        echo "$output" | grep "✗.*bugs found in" | sed 's/^/  /'
        # Extract warnings with file:line:column format
        echo "$output" | grep -E "^modules/[a-zA-Z0-9_/.-]+\.[ch]:[0-9]+:[0-9]+: warning:" | head -10 | sed 's/^/  /'
        # Show total summary
        echo "$output" | grep "Static analysis FAILED:" | sed 's/^/  /'
    else
        echo "Build/analysis error:"
        echo "$output" | tail -20
    fi
fi

echo -n "Static analysis (tests)...    "
output=$(make clean analyze-tests 2>&1)
exit_code=$?
if [ $exit_code -eq 0 ]; then
    echo "✓"
else
    echo "✗"
    # Check if it's due to static analysis findings
    if echo "$output" | grep -q "Static analysis FAILED:"; then
        echo "Static analysis found issues:"
        # Extract file-specific errors (e.g., "✗ 2 bugs found in methods/message_router_tests.c")
        echo "$output" | grep "✗.*bugs found in" | sed 's/^/  /'
        # Extract warnings with file:line:column format
        echo "$output" | grep -E "^(modules|methods)/[a-zA-Z0-9_/.-]+\.[ch]:[0-9]+:[0-9]+: warning:" | head -10 | sed 's/^/  /'
        # Show total summary
        echo "$output" | grep "Static analysis FAILED:" | sed 's/^/  /'
    else
        echo "Build/analysis error:"
        echo "$output" | tail -20
    fi
fi

# Step 3: Build executable
run_step "Build executable" "make clean executable"

# Step 4: Run tests
echo
echo "Running tests..."
output=$(make clean test 2>&1)
test_exit_code=$?

# Check if tests failed to compile
if [ $test_exit_code -ne 0 ]; then
    # Check for compilation errors
    compile_errors=$(echo "$output" | grep -c "error:")
    if [ $compile_errors -gt 0 ]; then
        echo "Tests: COMPILATION FAILED ✗"
        echo
        echo "Compilation errors:"
        echo "$output" | grep -E "error:|^modules/.*_tests\.c:" | head -20
        exit 1
    fi
fi

# Count passed and failed tests by counting test executables that ran
total_tests=$(echo "$output" | grep -c "^Running test: bin/")
passed=$(echo "$output" | grep -c "All .* tests passed")
errors=$(echo "$output" | grep -c "ERROR: Test .* failed")
aborts=$(echo "$output" | grep -c "Abort trap")
failed=$((errors + aborts))

# If no tests ran but make succeeded, something is wrong
if [ $test_exit_code -eq 0 ] && [ $total_tests -eq 0 ]; then
    echo "Tests: WARNING - No tests were run ⚠️"
    echo "Check if test executables are being built properly"
elif [ $failed -eq 0 ] && [ $test_exit_code -eq 0 ]; then
    echo "Tests: $total_tests run, all passed ✓"
else
    echo "Tests: $total_tests run, $passed passed, $failed FAILED ✗"
    echo
    echo "Failed tests:"
    echo "$output" | grep "ERROR: Test .* failed" | sed 's/ERROR: Test /  - /' | sed 's/ failed.*//'
    echo "$output" | grep -B1 "Abort trap" | grep "^Running test:" | sed 's/Running test: bin\//  - /' | sed 's/ (aborted)$//' | sort -u
    echo
    echo "Error details (last 10):"
    echo "$output" | grep -B2 "ERROR: Test .* failed" | grep -E "(Assertion failed|Abort trap)" | head -5
    echo "$output" | grep -E "(FAIL:|failed)" | grep -v "EXPECTED FAIL" | tail -10
    exit 1
fi

# Check for memory leaks in all memory report logs
echo
echo "Checking for memory leaks..."
memory_reports=$(find bin -name "memory_report*.log" 2>/dev/null | sort)

if [ -z "$memory_reports" ]; then
    echo "Memory: No memory reports found"
else
    all_clean=true
    leaky_tests=""
    
    for report in $memory_reports; do
        if [ -f "$report" ]; then
            # Check if there are actual memory leaks (not just intentional test leaks)
            actual_leaks=$(grep -E "^Actual memory leaks: ([0-9]+)" "$report" 2>/dev/null | awk '{print $4}')
            
            # If we can't find the "Actual memory leaks" line, fall back to old behavior
            if [ -z "$actual_leaks" ]; then
                if ! grep -q "No memory leaks detected" "$report" 2>/dev/null; then
                    all_clean=false
                    test_name=$(basename "$report" .log | sed 's/memory_report_//')
                    leaky_tests="$leaky_tests $test_name"
                fi
            # If we found it and it's greater than 0, we have real leaks
            elif [ "$actual_leaks" -gt 0 ]; then
                all_clean=false
                test_name=$(basename "$report" .log | sed 's/memory_report_//')
                leaky_tests="$leaky_tests $test_name"
            fi
        fi
    done
    
    if $all_clean; then
        echo "Memory: No leaks detected ✓"
    else
        echo "Memory: LEAKS DETECTED in:$leaky_tests"
        echo "Check memory_report_*.log files in bin/"
        exit 1
    fi
fi

# Step 5: Run executable
echo
echo "Running executable..."
output=$(make clean run 2>&1)
exec_exit_code=$?
if [ $exec_exit_code -eq 0 ]; then
    echo "Executable: ✓"
    # Wait a moment for report to be written
    sleep 1
    # Check if executable memory report was created and check for leaks
    if [ -f "bin/memory_report_agerun.log" ]; then
        # Check for memory leaks in executable
        actual_leaks=$(grep -E "^Actual memory leaks: ([0-9]+)" "bin/memory_report_agerun.log" 2>/dev/null | awk '{print $4}')
        
        # If we can't find the "Actual memory leaks" line, fall back to old behavior
        if [ -z "$actual_leaks" ]; then
            if grep -q "No memory leaks detected" "bin/memory_report_agerun.log" 2>/dev/null; then
                echo "Executable memory: No leaks detected ✓"
            else
                echo "Executable memory: LEAKS DETECTED ✗"
                echo "Check bin/memory_report_agerun.log for details"
            fi
        # If we found it and it's greater than 0, we have real leaks
        elif [ "$actual_leaks" -gt 0 ]; then
            echo "Executable memory: LEAKS DETECTED ($actual_leaks leaks) ✗"
            echo "Check bin/memory_report_agerun.log for details"
        else
            echo "Executable memory: No leaks detected ✓"
        fi
    else
        echo "Warning: Executable memory report not found"
    fi
    # Clean up persistence files created by executable
    rm -f bin/*.agerun
else
    echo "Executable: ✗"
    echo "Error output:"
    echo "$output" | tail -10
fi

# Step 6: Sanitize tests (ASan + UBSan)
echo
echo "Running sanitizer tests (ASan + UBSan)..."
output=$(make clean test-sanitize 2>&1)
sanitize_exit_code=$?

# Count sanitizer test runs
sanitize_total=$(echo "$output" | grep -c "^Running test: bin/")
sanitize_passed=$(echo "$output" | grep -c "All .* tests passed")

# Check if sanitizer tests failed to build
build_failed=$(echo "$output" | grep -c "Undefined symbols for architecture\|ld: symbol.* not found")

# Check for AddressSanitizer errors
asan_errors=$(echo "$output" | grep -c "ERROR: AddressSanitizer:")
heap_use_after_free=$(echo "$output" | grep -c "heap-use-after-free")
stack_buffer_overflow=$(echo "$output" | grep -c "stack-buffer-overflow")
heap_buffer_overflow=$(echo "$output" | grep -c "heap-buffer-overflow")
memory_leaks=$(echo "$output" | grep -c "ERROR: LeakSanitizer:")

# Check for UndefinedBehaviorSanitizer errors
ubsan_errors=$(echo "$output" | grep -c "runtime error:")
integer_overflow=$(echo "$output" | grep -c "signed integer overflow")
array_bounds=$(echo "$output" | grep -c "index.*out of bounds")
null_pointer=$(echo "$output" | grep -c "null pointer")

if [ $build_failed -gt 0 ]; then
    echo "Sanitizer: Build failed (missing sanitizer runtime) ⚠️"
    echo "Note: This is a known issue on some macOS systems"
elif [ $sanitize_exit_code -eq 0 ] && [ $asan_errors -eq 0 ] && [ $ubsan_errors -eq 0 ]; then
    echo "Sanitizer: $sanitize_total tests run, all passed ✓"
else
    echo "Sanitizer: $sanitize_total tests run, ERRORS DETECTED ✗"
    echo
    echo "Sanitizer Report:"
    
    if [ $asan_errors -gt 0 ]; then
        echo "  - AddressSanitizer errors: $asan_errors"
        if [ $heap_use_after_free -gt 0 ]; then
            echo "    • Heap use-after-free: $heap_use_after_free"
        fi
        if [ $stack_buffer_overflow -gt 0 ]; then
            echo "    • Stack buffer overflow: $stack_buffer_overflow"
        fi
        if [ $heap_buffer_overflow -gt 0 ]; then
            echo "    • Heap buffer overflow: $heap_buffer_overflow"
        fi
        if [ $memory_leaks -gt 0 ]; then
            echo "    • Memory leaks detected: $memory_leaks"
        fi
    fi
    
    if [ $ubsan_errors -gt 0 ]; then
        echo "  - UndefinedBehavior errors: $ubsan_errors"
        if [ $integer_overflow -gt 0 ]; then
            echo "    • Integer overflow: $integer_overflow"
        fi
        if [ $array_bounds -gt 0 ]; then
            echo "    • Array bounds violation: $array_bounds"
        fi
        if [ $null_pointer -gt 0 ]; then
            echo "    • Null pointer dereference: $null_pointer"
        fi
    fi
    
    # Show first error details
    echo
    if [ $asan_errors -gt 0 ]; then
        echo "First AddressSanitizer error:"
        echo "$output" | grep -A10 "ERROR: AddressSanitizer:" | head -15
    elif [ $ubsan_errors -gt 0 ]; then
        echo "First UndefinedBehavior error:"
        echo "$output" | grep -A2 "runtime error:" | head -5
    fi
fi

# Step 7: Run executable with ASan + UBSan
echo
echo "Running executable with sanitizers (ASan + UBSan)..."
output=$(make clean run-sanitize 2>&1)
exec_san_exit_code=$?

# Check for sanitizer errors in executable
exec_asan_errors=$(echo "$output" | grep -c "ERROR: AddressSanitizer:")
exec_ubsan_errors=$(echo "$output" | grep -c "runtime error:")

if [ $exec_san_exit_code -eq 0 ] && [ $exec_asan_errors -eq 0 ] && [ $exec_ubsan_errors -eq 0 ]; then
    echo "Sanitized Executable: ✓"
else
    echo "Sanitized Executable: ERRORS DETECTED ✗"
    if [ $exec_asan_errors -gt 0 ]; then
        echo "  - AddressSanitizer errors: $exec_asan_errors"
    fi
    if [ $exec_ubsan_errors -gt 0 ]; then
        echo "  - UndefinedBehavior errors: $exec_ubsan_errors"
    fi
    echo
    echo "First error:"
    echo "$output" | grep -E "(ERROR: AddressSanitizer:|runtime error:)" -A5 | head -10
fi

# Step 8: ThreadSanitizer tests
echo
echo "Running ThreadSanitizer tests..."
output=$(make clean test-tsan 2>&1)
tsan_exit_code=$?

# Count TSan test runs
tsan_total=$(echo "$output" | grep -c "^Running test: bin/")
tsan_passed=$(echo "$output" | grep -c "All .* tests passed")

# Check for ThreadSanitizer errors
tsan_errors=$(echo "$output" | grep -c "WARNING: ThreadSanitizer:")
data_races=$(echo "$output" | grep -c "data race")

if [ $tsan_exit_code -eq 0 ] && [ $tsan_errors -eq 0 ]; then
    echo "ThreadSanitizer: $tsan_total tests run, all passed ✓"
else
    echo "ThreadSanitizer: $tsan_total tests run, ERRORS DETECTED ✗"
    echo
    echo "ThreadSanitizer Report:"
    echo "  - Data races detected: $data_races"
    
    # Show first race details
    if [ $tsan_errors -gt 0 ]; then
        echo
        echo "First data race:"
        echo "$output" | grep -A15 "WARNING: ThreadSanitizer:" | head -20
    fi
fi

# Step 9: ThreadSanitizer executable
echo
echo "Running executable with ThreadSanitizer..."
output=$(make clean run-tsan 2>&1)
exec_tsan_exit_code=$?

# Check for TSan errors in executable
exec_tsan_errors=$(echo "$output" | grep -c "WARNING: ThreadSanitizer:")
exec_data_races=$(echo "$output" | grep -c "data race")

if [ $exec_tsan_exit_code -eq 0 ] && [ $exec_tsan_errors -eq 0 ]; then
    echo "TSan Executable: ✓"
else
    echo "TSan Executable: ERRORS DETECTED ✗"
    echo "  - Data races detected: $exec_data_races"
    if [ $exec_tsan_errors -gt 0 ]; then
        echo
        echo "First data race:"
        echo "$output" | grep -A10 "WARNING: ThreadSanitizer:" | head -15
    fi
fi

# Step 10: Check documentation file references
echo
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
    fi
fi

# Step 11: Check module name consistency in documentation
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
    fi
fi

# Step 12: Check function and type references in documentation
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
    fi
fi

echo
echo "=== Build Summary ==="
echo "Completed at $(date)"