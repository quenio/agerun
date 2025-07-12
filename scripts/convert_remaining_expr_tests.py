#!/usr/bin/env python3
"""
Convert remaining expression evaluator tests to use fixture pattern.
"""

import re
import sys
import os

# Check we're in the right directory
if not os.path.exists("./modules") or not os.path.exists("./Makefile"):
    print("ERROR: This script must be run from the repository root directory")
    print("Try: python3 scripts/convert_remaining_expr_tests.py")
    sys.exit(1)

# Tests that need conversion
tests_to_convert = [
    "test_evaluate_literal_string_wrong_type",
    "test_evaluate_literal_string_empty", 
    "test_evaluate_memory_access_wrong_type",
    "test_evaluate_memory_access_nested",
    "test_evaluate_memory_access_missing",
    "test_evaluate_binary_op_multiply_doubles",
    "test_evaluate_binary_op_concatenate_strings",
    "test_evaluate_binary_op_wrong_type"
]

def convert_test(content, test_name):
    """Convert a single test to use fixture pattern."""
    
    # Find the test function
    pattern = rf'static void {test_name}\(void\) \{{[^{{]*?printf\(".*?\\n"\);'
    match = re.search(pattern, content, re.DOTALL)
    if not match:
        print(f"  ERROR: Could not find start of {test_name}")
        return content
    
    start_pos = match.start()
    
    # Find the end of the function (closing brace at same level)
    brace_count = 0
    in_function = False
    end_pos = start_pos
    
    for i, char in enumerate(content[start_pos:], start_pos):
        if char == '{':
            brace_count += 1
            in_function = True
        elif char == '}':
            brace_count -= 1
            if in_function and brace_count == 0:
                end_pos = i + 1
                break
    
    if end_pos == start_pos:
        print(f"  ERROR: Could not find end of {test_name}")
        return content
    
    old_test = content[start_pos:end_pos]
    
    # Basic replacements
    new_test = old_test
    
    # Replace log/memory/evaluator creation
    new_test = re.sub(
        r'// Given a log instance\s*\n\s*ar_log_t \*log = ar_log__create\(\);\s*\n\s*assert\(log != NULL\);\s*\n\s*\n\s*// Given a memory map and evaluator\s*\n\s*ar_data_t \*memory = ar_data__create_map\(\);\s*\n\s*ar_expression_evaluator_t \*evaluator = ar_expression_evaluator__create\(log, memory, NULL\);\s*\n\s*assert\(evaluator != NULL\);',
        f'// Given a test fixture\n    ar_instruction_evaluator_fixture_t *own_fixture = ar_instruction_evaluator_fixture__create("{test_name}");\n    assert(own_fixture != NULL);\n    \n    ar_expression_evaluator_t *ref_evaluator = ar_instruction_evaluator_fixture__get_expression_evaluator(own_fixture);',
        new_test
    )
    
    # Alternative pattern for empty memory
    new_test = re.sub(
        r'// Given a log instance\s*\n\s*ar_log_t \*log = ar_log__create\(\);\s*\n\s*assert\(log != NULL\);\s*\n\s*\n\s*// Given an empty memory map and evaluator\s*\n\s*ar_data_t \*memory = ar_data__create_map\(\);\s*\n\s*ar_expression_evaluator_t \*evaluator = ar_expression_evaluator__create\(log, memory, NULL\);\s*\n\s*assert\(evaluator != NULL\);',
        f'// Given a test fixture\n    ar_instruction_evaluator_fixture_t *own_fixture = ar_instruction_evaluator_fixture__create("{test_name}");\n    assert(own_fixture != NULL);\n    \n    ar_expression_evaluator_t *ref_evaluator = ar_instruction_evaluator_fixture__get_expression_evaluator(own_fixture);',
        new_test
    )
    
    # Handle memory map with nested structure
    if "nested structure" in new_test:
        new_test = re.sub(
            r'// Given a memory map with nested structure\s*\n\s*ar_data_t \*memory = ar_data__create_map\(\);',
            '// Get memory from fixture\n    ar_data_t *mut_memory = ar_instruction_evaluator_fixture__get_memory(own_fixture);',
            new_test
        )
        new_test = re.sub(
            r'\n\s*ar_expression_evaluator_t \*evaluator = ar_expression_evaluator__create\(log, memory, NULL\);\s*\n\s*assert\(evaluator != NULL\);',
            '',
            new_test
        )
    
    # Replace evaluator references
    new_test = re.sub(r'\bevaluator\b', 'ref_evaluator', new_test)
    
    # Replace ast with own_ast
    new_test = re.sub(r'\bast\b', 'own_ast', new_test)
    
    # Replace result with own_result
    new_test = re.sub(r'\bresult\b', 'own_result', new_test)
    
    # Fix memory references
    new_test = re.sub(r'\bmemory\b(?!\.)', 'mut_memory', new_test)
    
    # Replace cleanup
    new_test = re.sub(
        r'// Clean up.*?ar_log__destroy\(log\);',
        '// Clean up\n    ar_expression_ast__destroy(own_ast);\n    ar_instruction_evaluator_fixture__destroy(own_fixture);',
        new_test,
        flags=re.DOTALL
    )
    
    # Handle special cases
    if "ar_data__destroy(own_result)" in old_test:
        new_test = re.sub(
            r'// Clean up\s*\n\s*ar_expression_ast__destroy\(own_ast\);\s*\n\s*ar_instruction_evaluator_fixture__destroy\(own_fixture\);',
            '// Clean up (MUST destroy own_result - it\'s owned)\n    ar_data__destroy(own_result);\n    ar_expression_ast__destroy(own_ast);\n    ar_instruction_evaluator_fixture__destroy(own_fixture);',
            new_test
        )
    
    # Handle nested memory access test special case
    if test_name == "test_evaluate_memory_access_nested":
        # Need to add fixture get
        new_test = re.sub(
            r'// Given a test fixture\s*\n\s*ar_instruction_evaluator_fixture_t \*own_fixture = ar_instruction_evaluator_fixture__create\("test_evaluate_memory_access_nested"\);\s*\n\s*assert\(own_fixture != NULL\);\s*\n\s*\n\s*ar_expression_evaluator_t \*ref_evaluator = ar_instruction_evaluator_fixture__get_expression_evaluator\(own_fixture\);',
            f'// Given a test fixture\n    ar_instruction_evaluator_fixture_t *own_fixture = ar_instruction_evaluator_fixture__create("{test_name}");\n    assert(own_fixture != NULL);',
            new_test
        )
        # Add evaluator get after memory setup
        new_test = re.sub(
            r'(ar_data__set_map_data\(mut_memory, "user", user\);)',
            r'\1\n    \n    ar_expression_evaluator_t *ref_evaluator = ar_instruction_evaluator_fixture__get_expression_evaluator(own_fixture);',
            new_test
        )
    
    # Handle missing key test
    if test_name == "test_evaluate_memory_access_missing":
        new_test = re.sub(
            r'// Get memory from fixture\s*\n\s*ar_data_t \*mut_memory = ar_instruction_evaluator_fixture__get_memory\(own_fixture\);',
            '// Given an empty memory map and evaluator\n    ar_data_t *mut_memory = ar_data__create_map();\n    ar_expression_evaluator_t *ref_evaluator = ar_instruction_evaluator_fixture__get_expression_evaluator(own_fixture);',
            new_test
        )
    
    # Replace old-style evaluate calls with new frame-based ones if needed
    if "_wrong_type" not in test_name and "ar_expression_evaluator__evaluate_" in new_test:
        # These tests need frames
        if "ref_frame" not in new_test:
            # Add frame creation after evaluator
            new_test = re.sub(
                r'(ar_expression_evaluator_t \*ref_evaluator = ar_instruction_evaluator_fixture__get_expression_evaluator\(own_fixture\);)',
                r'\1\n    ar_frame_t *ref_frame = ar_instruction_evaluator_fixture__create_frame(own_fixture);',
                new_test
            )
        # Update evaluate calls
        new_test = re.sub(
            r'ar_expression_evaluator__evaluate_binary_op\(ref_evaluator, own_ast\)',
            'ar_expression_evaluator__evaluate(ref_evaluator, ref_frame, own_ast)',
            new_test
        )
    
    return content[:start_pos] + new_test + content[end_pos:]

def main():
    """Main entry point."""
    
    dry_run = "--apply" not in sys.argv
    
    if dry_run:
        print("Running in DRY RUN mode")
    else:
        print("Running in APPLY mode")
    
    filepath = "modules/ar_expression_evaluator_tests.c"
    
    with open(filepath, 'r') as f:
        content = f.read()
    
    original_content = content
    
    for test_name in tests_to_convert:
        print(f"\nConverting {test_name}...")
        content = convert_test(content, test_name)
    
    if not dry_run and content != original_content:
        with open(filepath, 'w') as f:
            f.write(content)
        print(f"\nWrote changes to {filepath}")
    else:
        print(f"\nDRY RUN: Would modify {filepath}")

if __name__ == "__main__":
    main()