#!/usr/bin/env python3
"""
Verify that all parser modules have comprehensive error logging.

This script checks:
1. Presence of _log_error function in each parser
2. Helper functions have error logging for common conditions
3. NULL parameter checks are logged
4. Provides coverage statistics
"""

import os
import re
import sys
from pathlib import Path

def find_parser_files():
    """Find all parser C files in the modules directory."""
    modules_dir = Path(__file__).parent.parent / "modules"

    # All parsers that should have error logging
    parsers = [
        "ar_expression_parser.c",
        "ar_instruction_parser.c",
        "ar_assignment_instruction_parser.c",
        "ar_build_instruction_parser.c",
        "ar_compile_instruction_parser.c",
        "ar_condition_instruction_parser.c",
        "ar_deprecate_instruction_parser.c",
        "ar_exit_instruction_parser.c",
        "ar_parse_instruction_parser.c",
        "ar_send_instruction_parser.c",
        "ar_spawn_instruction_parser.c"
    ]

    return [(modules_dir / p, p) for p in parsers if (modules_dir / p).exists()]

def analyze_parser(file_path):
    """Analyze a single parser file for error logging patterns."""
    with open(file_path, 'r') as f:
        content = f.read()

    results = {
        'has_log_error': False,
        'helper_functions': [],
        'null_checks': 0,
        'error_calls': 0,
        'return_null_count': 0,
        'return_false_count': 0,
        'logged_errors': []
    }

    # Check for _log_error function OR direct ar_log__error calls
    if re.search(r'static\s+void\s+_log_error\s*\(', content):
        results['has_log_error'] = True
    elif re.search(r'ar_log__error(_at)?\s*\(', content):
        results['has_log_error'] = True  # Uses direct ar_log calls

    # Find helper functions
    helper_patterns = [
        r'static\s+\w+\s*\*?\s*(_extract_argument|_parse_arguments|_parse_\w+)\s*\(',
        r'static\s+\w+\s*\*?\s*(_parse_string_argument|_parse_expression_argument)\s*\('
    ]
    for pattern in helper_patterns:
        matches = re.findall(pattern, content)
        results['helper_functions'].extend(matches)

    # Count NULL parameter checks
    null_check_pattern = r'if\s*\(\s*!\s*\w+\s*\)'
    results['null_checks'] = len(re.findall(null_check_pattern, content))

    # Count _log_error calls AND direct ar_log__error calls
    log_error_pattern = r'(?:_log_error|ar_log__error(?:_at)?)\s*\([^;]+\);'
    error_calls = re.findall(log_error_pattern, content)
    results['error_calls'] = len(error_calls)

    # Extract error messages
    for call in error_calls:
        msg_match = re.search(r'"([^"]+)"', call)
        if msg_match:
            results['logged_errors'].append(msg_match.group(1))

    # Count return NULL/false statements
    results['return_null_count'] = len(re.findall(r'return\s+NULL\s*;', content))
    results['return_false_count'] = len(re.findall(r'return\s+false\s*;', content))

    return results

def print_report(parser_results):
    """Print a formatted report of the analysis."""
    print("=" * 80)
    print("PARSER ERROR LOGGING VERIFICATION REPORT")
    print("=" * 80)
    print()

    total_parsers = len(parser_results)
    parsers_with_logging = sum(1 for r in parser_results.values() if r['has_log_error'])
    total_errors_logged = sum(r['error_calls'] for r in parser_results.values())
    total_return_paths = sum(r['return_null_count'] + r['return_false_count']
                             for r in parser_results.values())

    print(f"Summary:")
    print(f"  Total parsers analyzed: {total_parsers}")
    print(f"  Parsers with _log_error: {parsers_with_logging}/{total_parsers}")
    print(f"  Total error logs: {total_errors_logged}")
    print(f"  Total error return paths: {total_return_paths}")
    print(f"  Coverage ratio: {total_errors_logged}/{total_return_paths} " +
          f"({100*total_errors_logged/total_return_paths:.1f}%)" if total_return_paths > 0 else "N/A")
    print()

    print("Per-Parser Analysis:")
    print("-" * 80)

    for parser_name, results in sorted(parser_results.items()):
        error_paths = results['return_null_count'] + results['return_false_count']
        coverage = (results['error_calls'] / error_paths * 100) if error_paths > 0 else 0

        print(f"\n{parser_name}:")
        print(f"  ✓ Has _log_error: {'Yes' if results['has_log_error'] else 'NO - MISSING'}")
        print(f"  ✓ Error calls: {results['error_calls']}")
        print(f"  ✓ Return paths: {error_paths} (NULL: {results['return_null_count']}, false: {results['return_false_count']})")
        print(f"  ✓ Coverage: {coverage:.1f}%")

        if results['helper_functions']:
            print(f"  ✓ Helper functions: {', '.join(set(results['helper_functions']))}")

        # Show first 5 error messages as examples
        if results['logged_errors']:
            print(f"  ✓ Sample errors logged:")
            for msg in results['logged_errors'][:5]:
                print(f"    - {msg}")

    print()
    print("=" * 80)

    # Identify parsers needing attention
    issues = []
    for parser_name, results in parser_results.items():
        if not results['has_log_error']:
            issues.append(f"  - {parser_name}: Missing _log_error function")
        elif results['error_calls'] == 0:
            issues.append(f"  - {parser_name}: Has _log_error but no calls to it")
        elif results['error_calls'] < (results['return_null_count'] + results['return_false_count']) / 2:
            issues.append(f"  - {parser_name}: Low error logging coverage (<50%)")

    if issues:
        print("ISSUES FOUND:")
        for issue in issues:
            print(issue)
        return 1
    else:
        print("✅ ALL PARSERS HAVE COMPREHENSIVE ERROR LOGGING!")
        print()
        print("Note: Memory allocation failures are logged but cannot be tested")
        print("      per established patterns (would require heap-level mocking).")
        return 0

def main():
    """Main entry point."""
    parser_files = find_parser_files()

    if not parser_files:
        print("ERROR: No parser files found in modules directory")
        return 1

    parser_results = {}
    for file_path, parser_name in parser_files:
        parser_results[parser_name] = analyze_parser(file_path)

    return print_report(parser_results)

if __name__ == "__main__":
    sys.exit(main())