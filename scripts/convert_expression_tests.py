#!/usr/bin/env python3
"""
Convert expression evaluator tests to use fixture pattern.
This script helps automate the conversion of tests from direct creation
to using the instruction evaluator fixture.
"""

import re
import sys
import os
from pathlib import Path

# Check we're in the right directory
if not os.path.exists("./modules") or not os.path.exists("./Makefile"):
    print("ERROR: This script must be run from the repository root directory")
    print("Try: python3 scripts/convert_expression_tests.py")
    sys.exit(1)

def identify_test_pattern(test_content):
    """Identify which pattern a test is using."""
    if "ar_instruction_evaluator_fixture__create" in test_content:
        return "fixture"
    elif "ar_log__create()" in test_content and "ar_data__create_map()" in test_content:
        return "direct"
    else:
        return "unknown"

def convert_test_to_fixture(test_name, test_content):
    """Convert a single test function to use the fixture pattern."""
    
    # Skip if already using fixture
    if identify_test_pattern(test_content) == "fixture":
        return test_content, False
    
    # Extract the function signature
    match = re.search(r'static void (test_\w+)\(void\)', test_content)
    if not match:
        return test_content, False
    
    func_name = match.group(1)
    
    # Replace log creation pattern
    test_content = re.sub(
        r'// Given a log instance\s*\n\s*ar_log_t \*log = ar_log__create\(\);\s*\n\s*assert\(log != NULL\);',
        f'// Given a test fixture\n    ar_instruction_evaluator_fixture_t *own_fixture = ar_instruction_evaluator_fixture__create("{func_name}");\n    assert(own_fixture != NULL);',
        test_content
    )
    
    # Replace memory and evaluator creation
    test_content = re.sub(
        r'\s*// Given a memory map and evaluator\s*\n\s*ar_data_t \*memory = ar_data__create_map\(\);\s*\n\s*ar_expression_evaluator_t \*evaluator = ar_expression_evaluator__create\(log\);\s*\n\s*assert\(evaluator != NULL\);',
        '\n    ar_expression_evaluator_t *ref_evaluator = ar_instruction_evaluator_fixture__get_expression_evaluator(own_fixture);',
        test_content
    )
    
    # Replace standalone evaluator creation
    test_content = re.sub(
        r'ar_expression_evaluator_t \*evaluator = ar_expression_evaluator__create\(log\);\s*\n\s*assert\(evaluator != NULL\);',
        'ar_expression_evaluator_t *ref_evaluator = ar_instruction_evaluator_fixture__get_expression_evaluator(own_fixture);',
        test_content
    )
    
    # Replace evaluator references
    test_content = re.sub(r'\bevaluator\b', 'ref_evaluator', test_content)
    
    # Replace memory references where appropriate
    test_content = re.sub(r'\bmemory\b(?!\.)', 'mut_memory', test_content)
    
    # Add frame creation for tests that need it
    if "ar_expression_evaluator__evaluate(" in test_content and "ref_frame" not in test_content:
        # Find where to insert frame creation
        insert_pos = test_content.find("ar_expression_evaluator_t *ref_evaluator")
        if insert_pos != -1:
            # Find the end of the line
            line_end = test_content.find('\n', insert_pos)
            if line_end != -1:
                frame_creation = "\n    ar_frame_t *ref_frame = ar_instruction_evaluator_fixture__create_frame(own_fixture);"
                test_content = test_content[:line_end] + frame_creation + test_content[line_end:]
    
    # Replace cleanup section
    test_content = re.sub(
        r'// Clean up\s*\n(?:\s*ar_expression_ast__destroy\(\w+\);\s*\n)*\s*ar_expression_evaluator__destroy\(ref_evaluator\);\s*\n\s*ar_data__destroy\(memory\);\s*\n\s*ar_log__destroy\(log\);',
        '// Clean up\n    ar_expression_ast__destroy(ast);\n    ar_instruction_evaluator_fixture__destroy(own_fixture);',
        test_content
    )
    
    # Handle variable renames
    test_content = re.sub(r'\bart\b', 'own_ast', test_content)
    test_content = re.sub(r'\bresult\b', 'own_result', test_content)
    
    # Fix specific patterns
    test_content = re.sub(
        r'ar_data__destroy\(own_result\);\s*\n\s*ar_expression_ast__destroy\(own_ast\);\s*\n\s*ar_expression_evaluator__destroy\(ref_evaluator\);\s*\n\s*ar_data__destroy\(mut_memory\);\s*\n\s*ar_log__destroy\(log\);',
        'ar_data__destroy(own_result);\n    ar_expression_ast__destroy(own_ast);\n    ar_instruction_evaluator_fixture__destroy(own_fixture);',
        test_content
    )
    
    return test_content, True

def process_file(filepath, dry_run=True):
    """Process the test file and convert tests to use fixtures."""
    
    with open(filepath, 'r') as f:
        content = f.read()
    
    # Find all test functions
    test_pattern = re.compile(r'(static void test_\w+\(void\)\s*\{[^}]+\})', re.MULTILINE | re.DOTALL)
    tests = test_pattern.findall(content)
    
    converted_count = 0
    skipped_count = 0
    
    for test in tests:
        # Extract test name
        name_match = re.search(r'static void (test_\w+)\(void\)', test)
        if not name_match:
            continue
        
        test_name = name_match.group(1)
        pattern = identify_test_pattern(test)
        
        if pattern == "fixture":
            print(f"  SKIP: {test_name} - already using fixture")
            skipped_count += 1
        elif pattern == "direct":
            converted_test, was_converted = convert_test_to_fixture(test_name, test)
            if was_converted:
                print(f"  CONVERT: {test_name}")
                content = content.replace(test, converted_test)
                converted_count += 1
            else:
                print(f"  ERROR: {test_name} - conversion failed")
        else:
            print(f"  UNKNOWN: {test_name} - pattern not recognized")
    
    if not dry_run and converted_count > 0:
        with open(filepath, 'w') as f:
            f.write(content)
        print(f"\nWrote {converted_count} converted tests to {filepath}")
    else:
        print(f"\nDRY RUN: Would convert {converted_count} tests, skip {skipped_count} tests")
        if converted_count > 0:
            print("\nTo apply changes, run with --apply flag")
    
    return converted_count, skipped_count

def main():
    """Main entry point."""
    
    dry_run = "--apply" not in sys.argv
    
    if dry_run:
        print("Running in DRY RUN mode - no files will be modified")
        print("To apply changes, run: python3 scripts/convert_expression_tests.py --apply\n")
    else:
        print("Running in APPLY mode - files will be modified\n")
    
    filepath = Path("modules/ar_expression_evaluator_tests.c")
    
    if not filepath.exists():
        print(f"ERROR: {filepath} not found")
        sys.exit(1)
    
    print(f"Processing {filepath}...")
    converted, skipped = process_file(filepath, dry_run)
    
    print(f"\nSummary: {converted} tests to convert, {skipped} already using fixture")

if __name__ == "__main__":
    main()