# Grep OR Syntax Differences

## Learning
Basic grep and extended grep use different syntax for OR operations. Basic grep (default) uses `\|` while extended grep (`-E` flag) uses plain `|`. This distinction is crucial for correct pattern matching.

## Importance
Using the wrong OR syntax will cause grep to fail silently, missing important matches. Understanding the difference prevents frustrating debugging sessions where searches appear to find nothing.

## Example
Both of these commands search for the same patterns but use different syntax:
```bash
# Basic grep - uses backslash-escaped pipe
grep "ar_agency__is_agent_active\|ar_agency__set_agent_active" modules/*.c

# Extended grep - uses plain pipe
grep -E "ar_agency__is_agent_active|ar_agency__set_agent_active" modules/*.c

# With grouping - extended grep is cleaner
grep -E "ar_agency__(is_agent_active|set_agent_active|set_agent_id)" modules/*.c

# Basic grep grouping requires more escaping
grep "ar_agency__\(is_agent_active\|set_agent_active\|set_agent_id\)" modules/*.c
```

Testing the difference:
```bash
# Create test data
echo -e "ar_data__create_string\nar_data__create_integer" > test.txt

# Basic grep with \| works
grep "string\|integer" test.txt
# Output: both lines

# Basic grep with plain | treats it as literal character
grep "string|integer" test.txt
# Output: nothing (looking for literal "string|integer")

# Extended grep with plain | works
grep -E "string|integer" test.txt
# Output: both lines

# Extended grep with \| treats it as literal characters
grep -E "string\|integer" test.txt
# Output: nothing (looking for literal "string\|integer")
```

## Generalization
Choose your grep mode based on pattern complexity:
- **Basic grep**: Simple searches, when you're already using it
- **Extended grep (-E)**: Complex patterns with multiple ORs, grouping
- **Always test**: Echo sample data and verify your pattern works

## Implementation
```bash
# Function to search with proper OR syntax
search_functions() {
    local funcs="$1"
    local files="$2"

    # Convert space-separated list to OR pattern
    # For basic grep
    pattern=$(echo "$funcs" | sed 's/ /\\|/g')
    grep "$pattern" $files

    # For extended grep
    pattern=$(echo "$funcs" | sed 's/ /|/g')
    grep -E "$pattern" $files
}

# Usage
search_functions "create_agent destroy_agent send_to_agent" "modules/ar_agency.c"
```

## Related Patterns
- [Build Log Extraction Patterns](build-log-extraction-patterns.md)
- [Search Replace Precision](search-replace-precision.md)
- [Systematic Whitelist Error Resolution](systematic-whitelist-error-resolution.md)