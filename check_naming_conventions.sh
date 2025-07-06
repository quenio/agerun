#!/bin/bash

# Naming Convention Checker for AgeRun
# Verifies that all source files follow the established naming conventions

echo "=== AgeRun Naming Convention Check ==="
echo "Starting at $(date)"
echo

# Track overall status
overall_status=0
total_issues=0

# Color codes for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Helper function to print errors
print_error() {
    echo -e "${RED}✗ $1${NC}"
    ((total_issues++))
    overall_status=1
}

# Helper function to print warnings
print_warning() {
    echo -e "${YELLOW}⚠ $1${NC}"
    ((total_issues++))
}

# Helper function to print success
print_success() {
    echo -e "${GREEN}✓ $1${NC}"
}

# Step 1: Check module file naming conventions
echo "Checking module file naming conventions..."
module_files=$(find modules -name "*.c" -o -name "*.h" -o -name "*.zig" | grep -v "_tests" | sort)
bad_module_files=0

for file in $module_files; do
    basename=$(basename "$file")
    # Module files should start with ar_ (not agerun_)
    if [[ ! "$basename" =~ ^ar_ ]]; then
        print_error "Module file '$file' doesn't follow ar_<module> naming convention"
        ((bad_module_files++))
    fi
done

if [ $bad_module_files -eq 0 ]; then
    print_success "All module files follow ar_<module> naming convention"
fi

# Step 2: Check test file naming conventions
echo
echo "Checking test file naming conventions..."
test_files=$(find modules methods -name "*_tests.c" | sort)
bad_test_files=0

for file in $test_files; do
    basename=$(basename "$file")
    # Test files should be <module>_tests.c
    if [[ ! "$basename" =~ ^[a-zA-Z0-9_]+_tests\.c$ ]]; then
        print_error "Test file '$file' doesn't follow <module>_tests.c convention"
        ((bad_test_files++))
    fi
done

if [ $bad_test_files -eq 0 ]; then
    print_success "All test files follow <module>_tests.c naming convention"
fi

# Step 3: Check function naming conventions
echo
echo "Checking function naming conventions..."

# Public module functions should be ar_<module>__<function>
echo -n "  Public module functions... "
# Check C files
bad_c_funcs=$(grep -h "^[a-zA-Z_].*(" modules/*.c 2>/dev/null | \
    grep -v "^static" | \
    grep -v "^//" | \
    grep -v "^\*" | \
    grep -v "^int main" | \
    grep -v "^void main" | \
    grep -E "^[a-zA-Z_][a-zA-Z0-9_]*\s*\(" | \
    grep -v -E "^ar_[a-zA-Z0-9_]+__[a-zA-Z0-9_]+\s*\(" | \
    grep -v -E "^_[a-zA-Z0-9_]+\s*\(" | \
    wc -l)

# Check Zig files - export fn should follow same convention
# Extract just the function name part before the parenthesis
bad_zig_funcs=0
zig_exports=$(grep -h "^export fn" modules/*.zig 2>/dev/null)
while IFS= read -r line; do
    if [ ! -z "$line" ]; then
        # Extract function name
        func_name=$(echo "$line" | sed -E 's/^export fn ([a-zA-Z0-9_]+).*/\1/')
        # Check if it follows convention
        if ! echo "$func_name" | grep -qE "^ar_[a-zA-Z0-9_]+__[a-zA-Z0-9_]+$"; then
            ((bad_zig_funcs++))
        fi
    fi
done <<< "$zig_exports"

bad_public_funcs=$((bad_c_funcs + bad_zig_funcs))

if [ $bad_public_funcs -gt 0 ]; then
    print_error "Found $bad_public_funcs public functions not following ar_<module>__<function> convention"
    # Show first few examples
    echo "    Examples:"
    # Show C examples
    c_examples=$(grep -h "^[a-zA-Z_].*(" modules/*.c 2>/dev/null | \
        grep -v "^static" | \
        grep -v "^//" | \
        grep -v "^\*" | \
        grep -v "^int main" | \
        grep -v "^void main" | \
        grep -E "^[a-zA-Z_][a-zA-Z0-9_]*\s*\(" | \
        grep -v -E "^ar_[a-zA-Z0-9_]+__[a-zA-Z0-9_]+\s*\(" | \
        grep -v -E "^_[a-zA-Z0-9_]+\s*\(" | \
        head -3 | sed 's/^/      /')
    if [ ! -z "$c_examples" ]; then
        echo "$c_examples"
    fi
    # Show Zig examples
    zig_examples=""
    while IFS= read -r line; do
        if [ ! -z "$line" ]; then
            func_name=$(echo "$line" | sed -E 's/^export fn ([a-zA-Z0-9_]+).*/\1/')
            if ! echo "$func_name" | grep -qE "^ar_[a-zA-Z0-9_]+__[a-zA-Z0-9_]+$"; then
                zig_examples="$zig_examples      $line\n"
            fi
        fi
    done <<< "$zig_exports"
    if [ ! -z "$zig_examples" ]; then
        echo -e "$zig_examples" | head -2
    fi
else
    print_success "All public module functions follow ar_<module>__<function> convention"
fi

# Static functions should be _<function> (excluding test files)
echo -n "  Static functions... "
# Exclude test files from this check (both .c and .zig)
non_test_files=$(find modules -name "*.c" -o -name "*.zig" | grep -v "_tests")
bad_static_funcs=0
for file in $non_test_files; do
    if [[ "$file" == *.c ]]; then
        count=$(grep "^static.*(" "$file" 2>/dev/null | \
            grep -v "^static inline" | \
            grep -E "^static\s+[a-zA-Z_][a-zA-Z0-9_]*\s+[a-zA-Z_][a-zA-Z0-9_]*\s*\(" | \
            grep -v -E "^static\s+[a-zA-Z_][a-zA-Z0-9_]*\s+_[a-zA-Z0-9_]+\s*\(" | \
            wc -l)
        bad_static_funcs=$((bad_static_funcs + count))
    elif [[ "$file" == *.zig ]]; then
        # Zig uses fn for private functions, they should also use _
        count=$(grep "^fn [a-zA-Z]" "$file" 2>/dev/null | \
            grep -v "^fn _" | \
            wc -l)
        bad_static_funcs=$((bad_static_funcs + count))
    fi
done

if [ $bad_static_funcs -gt 0 ]; then
    print_error "Found $bad_static_funcs static functions not following _<function> convention"
    # Show first few examples
    echo "    Examples:"
    for file in $non_test_files; do
        if [[ "$file" == *.c ]]; then
            grep "^static.*(" "$file" 2>/dev/null | \
                grep -v "^static inline" | \
                grep -E "^static\s+[a-zA-Z_][a-zA-Z0-9_]*\s+[a-zA-Z_][a-zA-Z0-9_]*\s*\(" | \
                grep -v -E "^static\s+[a-zA-Z_][a-zA-Z0-9_]*\s+_[a-zA-Z0-9_]+\s*\(" | \
                sed "s|^|      $file: |"
        elif [[ "$file" == *.zig ]]; then
            grep "^fn [a-zA-Z]" "$file" 2>/dev/null | \
                grep -v "^fn _" | \
                sed "s|^|      $file: |"
        fi
    done | head -5
else
    print_success "All static functions follow _<function> convention"
fi

# Test functions should be test_<module>__<test_name>
echo -n "  Test functions... "
bad_test_funcs=$(grep -h "^void test_" modules/*_tests.c methods/*_tests.c 2>/dev/null | \
    grep -v -E "^void test_[a-zA-Z0-9]+__[a-zA-Z0-9_]+\s*\(" | \
    wc -l)

if [ $bad_test_funcs -gt 0 ]; then
    print_error "Found $bad_test_funcs test functions not following test_<module>__<test_name> convention"
    # Show first few examples
    echo "    Examples:"
    grep -h "^void test_" modules/*_tests.c methods/*_tests.c 2>/dev/null | \
        grep -v -E "^void test_[a-zA-Z0-9]+__[a-zA-Z0-9_]+\s*\(" | \
        head -5 | sed 's/^/      /'
else
    print_success "All test functions follow test_<module>__<test_name> convention"
fi

# Step 4: Check type naming conventions
echo
echo "Checking type naming conventions..."

# Check enum values should follow AR_<ENUM_TYPE>__<VALUE> pattern
echo -n "  Enum values... "
bad_enum_values=0
enum_value_issues=""

# Check C header files
for file in modules/*.h; do
    # First find typedef enum blocks and their type names
    # Use a simpler approach with sed and grep
    
    # Extract enum type definitions
    enum_types=$(grep -E "} ar_[a-zA-Z0-9_]+_t;" "$file" 2>/dev/null | sed -E 's/.*} (ar_[a-zA-Z0-9_]+_t);.*/\1/')
    
    for enum_type in $enum_types; do
        if [ ! -z "$enum_type" ]; then
            # Find the enum block for this type
            # Extract from "typedef enum {" to "} $enum_type;"
            enum_block=$(awk -v type="$enum_type" '
                /typedef enum/ { capture=1; block="" }
                capture { block = block "\n" $0 }
                capture && $0 ~ "} " type ";" { print block; capture=0 }
            ' "$file" 2>/dev/null)
            
            # Extract enum values from the block
            enum_values=$(echo "$enum_block" | grep -E '^\s*[A-Z][A-Z0-9_]+' | sed -E 's/^\s*([A-Z][A-Z0-9_]+)[,\s]*.*/\1/')
            
            # Convert type name to expected prefix: ar_data_type_t -> AR_DATA_TYPE
            expected_prefix=$(echo "$enum_type" | sed -E 's/^ar_//; s/_t$//' | tr '[:lower:]' '[:upper:]')
            
            # Check each enum value
            for value in $enum_values; do
                if [ ! -z "$value" ]; then
                    # Check if value follows the pattern AR_<TYPE>__<VALUE>
                    if ! echo "$value" | grep -qE "^AR_${expected_prefix}__"; then
                        enum_value_issues="$enum_value_issues\n    $file: $value (in $enum_type, should start with AR_${expected_prefix}__)"
                        ((bad_enum_values++))
                    fi
                fi
            done
        fi
    done
done

# Check Zig files for enum definitions
for file in modules/*.zig; do
    if [ -f "$file" ]; then
        # Extract Zig enum type definitions (pub const name = enum)
        zig_enums=$(grep -E "pub const ar_[a-zA-Z0-9_]+_t = enum" "$file" 2>/dev/null | sed -E 's/pub const (ar_[a-zA-Z0-9_]+_t).*/\1/')
        
        for enum_type in $zig_enums; do
            if [ ! -z "$enum_type" ]; then
                # Find the enum block for this type in Zig
                # Extract from "pub const $enum_type = enum" to "};"
                enum_block=$(awk -v type="$enum_type" '
                    $0 ~ "pub const " type " = enum" { capture=1; block="" }
                    capture { block = block "\n" $0 }
                    capture && /^};/ { print block; capture=0 }
                ' "$file" 2>/dev/null)
                
                # Extract enum values from the Zig enum block
                # Zig format: VALUE_NAME = number,
                enum_values=$(echo "$enum_block" | grep -E '^[[:space:]]*[A-Z][A-Z0-9_]+[[:space:]]*=' | sed -E 's/^[[:space:]]*([A-Z][A-Z0-9_]+)[[:space:]]*=.*/\1/')
                
                # Convert type name to expected prefix: ar_file_result_t -> AR_FILE_RESULT
                expected_prefix=$(echo "$enum_type" | sed -E 's/^ar_//; s/_t$//' | tr '[:lower:]' '[:upper:]')
                
                # Check each enum value
                for value in $enum_values; do
                    if [ ! -z "$value" ]; then
                        # Check if value follows the pattern AR_<TYPE>__<VALUE>
                        if ! echo "$value" | grep -qE "^AR_${expected_prefix}__"; then
                            enum_value_issues="${enum_value_issues}\n    $file: $value (in $enum_type, should start with AR_${expected_prefix}__)"
                            ((bad_enum_values++))
                        fi
                    fi
                done
            fi
        done
    fi
done

if [ $bad_enum_values -eq 0 ]; then
    print_success "All enum values follow AR_<ENUM_TYPE>__<VALUE> convention"
else
    print_error "Found enum values not following AR_<ENUM_TYPE>__<VALUE> convention"
    echo -e "    Issues:$enum_value_issues" | head -40
fi

# Typedef structs should be ar_<type>_t
echo -n "  Typedef structs... "
bad_typedefs=0
typedef_issues=""

# Check for typedef struct patterns
for file in modules/*.h; do
    # Look for typedef struct patterns that don't follow ar_*_t convention
    bad_types=$(grep -E "typedef\s+struct\s+[a-zA-Z0-9_]+\s+[a-zA-Z0-9_]+;" "$file" 2>/dev/null | \
        grep -v -E "typedef\s+struct\s+[a-zA-Z0-9_]+\s+ar_[a-zA-Z0-9_]+_t;" | \
        sed "s|^|$file: |")
    
    if [ ! -z "$bad_types" ]; then
        typedef_issues="$typedef_issues\n$bad_types"
        ((bad_typedefs++))
    fi
done

if [ $bad_typedefs -eq 0 ]; then
    print_success "All typedef structs follow ar_<type>_t convention"
else
    print_error "Found typedef structs not following ar_<type>_t convention"
    echo -e "    Issues:$typedef_issues" | head -10
fi

# Enum types should be ar_<type>_t
echo -n "  Enum types... "
bad_enums=0
enum_issues=""

for file in modules/*.h; do
    # Look for enum definitions ending with } <type>;
    bad_enum_types=$(grep -E "^\s*}\s*[a-zA-Z0-9_]+\s*;" "$file" 2>/dev/null | \
        grep -v -E "^\s*}\s*ar_[a-zA-Z0-9_]+_t\s*;" | \
        grep -v -E "^\s*}\s*[a-zA-Z0-9_]+_s\s*;" | \
        sed "s|^|$file: |")
    
    if [ ! -z "$bad_enum_types" ]; then
        enum_issues="$enum_issues\n$bad_enum_types"
        ((bad_enums++))
    fi
done

# Check Zig files for enum type definitions
for file in modules/*.zig; do
    if [ -f "$file" ]; then
        # Look for Zig enum definitions that don't follow ar_<type>_t pattern
        bad_zig_enum_types=$(grep -E "pub const [a-zA-Z0-9_]+ = enum" "$file" 2>/dev/null | \
            grep -v -E "pub const ar_[a-zA-Z0-9_]+_t = enum" | \
            sed "s|^|$file: |")
        
        if [ ! -z "$bad_zig_enum_types" ]; then
            enum_issues="$enum_issues\n$bad_zig_enum_types"
            ((bad_enums++))
        fi
    fi
done

if [ $bad_enums -eq 0 ]; then
    print_success "All enum types follow ar_<type>_t convention"
else
    print_error "Found enum types not following ar_<type>_t convention"
    echo -e "    Issues:$enum_issues" | head -10
fi

# Step 5: Check macro naming conventions
echo
echo "Checking macro naming conventions..."

# Heap macros should be AR__HEAP__<OPERATION>
echo -n "  Heap macros... "
bad_heap_macros=$(grep -h "#define AR" modules/ar_heap.h 2>/dev/null | \
    grep -v -E "#define AR__HEAP__[A-Z_]+" | \
    wc -l)

if [ $bad_heap_macros -gt 0 ]; then
    print_error "Found heap macros not following AR__HEAP__<OPERATION> convention"
else
    print_success "All heap macros follow AR__HEAP__<OPERATION> convention"
fi

# Assert macros should be AR_ASSERT or AR_ASSERT_<TYPE>
echo -n "  Assert macros... "
bad_assert_macros=$(grep -h "#define AR_ASSERT" modules/ar_assert.h 2>/dev/null | \
    grep -v -E "^#define AR_ASSERT(_[A-Z_]+)?\(" | \
    wc -l)

if [ $bad_assert_macros -gt 0 ]; then
    print_error "Found assert macros not following AR_ASSERT_<TYPE> convention"
else
    print_success "All assert macros follow AR_ASSERT_<TYPE> convention"
fi

# Step 6: Check for old naming patterns
echo
echo "Checking for outdated naming patterns..."

# Check for agerun_ prefix (should be ar_)
echo -n "  Old agerun_ prefix... "
old_agerun=$(grep -r "agerun_" modules/*.c modules/*.h methods/*.c 2>/dev/null | \
    grep -v "agerun\.h" | \
    grep -v "agerun\.c" | \
    grep -v "memory_report_agerun" | \
    grep -v "\"agerun\"" | \
    grep -v "agerun\." | \
    wc -l)

if [ $old_agerun -gt 0 ]; then
    print_warning "Found $old_agerun instances of old 'agerun_' prefix (should be 'ar_')"
    # Show first few examples
    echo "    Examples:"
    grep -r "agerun_" modules/*.c modules/*.h methods/*.c 2>/dev/null | \
        grep -v "agerun\.h" | \
        grep -v "agerun\.c" | \
        grep -v "memory_report_agerun" | \
        grep -v "\"agerun\"" | \
        grep -v "agerun\." | \
        head -5 | sed 's/^/      /'
else
    print_success "No outdated 'agerun_' prefixes found"
fi

# Check for double underscore at start (ar__) which should be single (ar_)
echo -n "  Double underscore prefix... "
double_underscore=$(grep -r "ar__[a-z]" modules/*.c modules/*.h methods/*.c 2>/dev/null | \
    grep -v "AR__HEAP__" | \
    wc -l)

if [ $double_underscore -gt 0 ]; then
    print_warning "Found $double_underscore instances of 'ar__' (should be 'ar_')"
    # Show first few examples
    echo "    Examples:"
    grep -r "ar__[a-z]" modules/*.c modules/*.h methods/*.c 2>/dev/null | \
        grep -v "AR__HEAP__" | \
        head -5 | sed 's/^/      /'
else
    print_success "No incorrect double underscore prefixes found"
fi

# Step 7: Check struct naming conventions
echo
echo "Checking struct naming conventions..."

# Struct names should be ar_<name>_s
echo -n "  Struct definitions... "
bad_structs=0
struct_issues=""

for file in modules/*.c modules/*.h; do
    # Look for struct definitions
    bad_struct_names=$(grep -E "^struct\s+[a-zA-Z0-9_]+\s*{" "$file" 2>/dev/null | \
        grep -v -E "^struct\s+ar_[a-zA-Z0-9_]+_s\s*{" | \
        sed "s|^|$file: |")
    
    if [ ! -z "$bad_struct_names" ]; then
        struct_issues="$struct_issues\n$bad_struct_names"
        ((bad_structs++))
    fi
done

if [ $bad_structs -eq 0 ]; then
    print_success "All struct definitions follow ar_<name>_s convention"
else
    print_error "Found struct definitions not following ar_<name>_s convention"
    echo -e "    Issues:$struct_issues" | head -10
fi

# Step 8: Check Zig-specific naming conventions
echo
echo "Checking Zig-specific naming conventions..."

# Check Zig struct types
echo -n "  Zig struct types... "
bad_zig_structs=0
zig_struct_issues=""

for file in modules/*.zig; do
    if [ -f "$file" ]; then
        # Look for struct definitions that don't follow naming convention
        # Pattern: const StructName = struct {
        bad_structs=$(grep -E "^const [A-Z][a-zA-Z0-9_]* = struct" "$file" 2>/dev/null | \
            grep -v -E "^const ar_[a-zA-Z0-9_]+_t = struct" | \
            sed "s|^|$file: |")
        
        if [ ! -z "$bad_structs" ]; then
            zig_struct_issues="$zig_struct_issues\n$bad_structs"
            ((bad_zig_structs++))
        fi
    fi
done

if [ $bad_zig_structs -eq 0 ]; then
    print_success "All Zig struct types follow naming convention"
else
    print_error "Found Zig struct types not following ar_<type>_t convention"
    echo -e "    Issues:$zig_struct_issues" | head -10
fi

# Check Zig global variables
echo -n "  Zig global variables... "
bad_zig_globals=0
zig_global_issues=""

for file in modules/*.zig; do
    if [ -f "$file" ]; then
        # Look for global variables (var declarations at module level)
        # They should use g_ prefix
        bad_globals=$(grep -E "^(pub )?var [a-zA-Z]" "$file" 2>/dev/null | \
            grep -v -E "^(pub )?var g_" | \
            sed "s|^|$file: |")
        
        if [ ! -z "$bad_globals" ]; then
            zig_global_issues="$zig_global_issues\n$bad_globals"
            ((bad_zig_globals++))
        fi
    fi
done

if [ $bad_zig_globals -eq 0 ]; then
    print_success "All Zig global variables follow g_ prefix convention"
else
    print_error "Found Zig global variables not following g_ prefix convention"
    echo -e "    Issues:$zig_global_issues" | head -10
fi

# Step 9: Summary
echo
echo "=== Naming Convention Check Summary ==="
echo "Completed at $(date)"
echo

if [ $total_issues -eq 0 ]; then
    echo -e "${GREEN}All naming conventions are being followed correctly! ✓${NC}"
else
    echo -e "${RED}Found $total_issues naming convention issues that need attention.${NC}"
    echo
    echo "Naming Convention Guidelines:"
    echo "  - Module files: ar_<module>.{c,h,zig}"
    echo "  - Public functions: ar_<module>__<function>"
    echo "  - Static/private functions: _<function>"
    echo "  - Test functions: test_<module>__<test_name>"
    echo "  - Types: ar_<type>_t"
    echo "  - Structs: ar_<name>_s (C), ar_<name>_t (Zig)"
    echo "  - Enum values: AR_<ENUM_TYPE>__<VALUE>"
    echo "  - Global variables: g_<name>"
    echo "  - Heap macros: AR__HEAP__<OPERATION>"
    echo "  - Assert macros: AR_ASSERT_<TYPE>"
fi

# Exit with appropriate status
exit $overall_status