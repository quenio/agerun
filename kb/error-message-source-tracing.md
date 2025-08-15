# Error Message Source Tracing

## Learning

Error messages in CI/build systems often pass through multiple layers of processing and reformatting. The message you see in the final output may be significantly different from the original error. Tracing errors back through the pipeline reveals their true source.

## Importance

Understanding the error message pipeline:
- Helps locate the actual source of problems
- Reveals where reformatting or summarization occurs
- Shows which tool originally detected the issue
- Enables fixing the right component

## Example

```c
// Error as shown in CI summary:
// "Documentation has invalid function/type references:
//   - kb/file.md references non-existent function 'ar_function'"

// Tracing back through layers:
// 1. CI Summary (build.sh formats for display)
//    ↑
// 2. build.sh (greps for "INVALID REFERENCES FOUND")
//    ↑  
// 3. check_docs.py (outputs "Function/type check: INVALID REFERENCES FOUND")
//    ↑
// 4. Actual check (ar_function not in all_functions set)

// Each layer reformats the message
ar_data_t* own_error = ar_data__create_string("Original error");
// Gets reformatted at each layer
ar_data__destroy(own_error);
```

## Generalization

To trace error messages to their source:
1. Start with the final error message
2. Search for that text in build scripts
3. Find what that script is parsing
4. Locate the tool that generates the original error
5. Examine the actual validation logic
6. Understand each transformation layer

## Implementation

```bash
# Trace error message backwards
ERROR_TEXT="Documentation has invalid"

# Find in build scripts
grep -r "$ERROR_TEXT" scripts/

# Find what it's parsing
grep -B5 -A5 "$ERROR_TEXT" scripts/build.sh

# Find original source
grep "INVALID REFERENCES" scripts/check_docs.py

# Examine actual validation
grep -A20 "def check_function" scripts/check_docs.py
```

## Related Patterns

- [Comprehensive Output Review](comprehensive-output-review.md)
- [Build Verification Before Commit](build-verification-before-commit.md)