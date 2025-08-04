#!/usr/bin/env python3

import os
import re
import sys
import glob

# ANSI color codes - disabled to match bash script output
GREEN = ''
RED = ''
YELLOW = ''
RESET = ''

# Track if we found any issues (equivalent to FOUND_ISSUES in bash)
FOUND_ISSUES = False

def load_whitelist():
    """Load the error whitelist from YAML file."""
    whitelist_file = 'error_whitelist.yaml'
    if not os.path.exists(whitelist_file):
        return []
    
    # Simple YAML parser for our specific format
    whitelist = []
    with open(whitelist_file, 'r') as f:
        in_intentional_errors = False
        current_entry = {}
        
        for line in f:
            # Skip comments and empty lines
            if line.strip().startswith('#') or not line.strip():
                continue
                
            # Check if we're in the intentional_errors section
            if line.strip() == 'intentional_errors:':
                in_intentional_errors = True
                continue
                
            if not in_intentional_errors:
                continue
                
            # Parse YAML entries
            if line.strip().startswith('- test:'):
                # Save previous entry if exists
                if current_entry:
                    whitelist.append(current_entry)
                current_entry = {}
                # Extract test name
                match = re.match(r'^\s*-\s*test:\s*"(.+)"', line)
                if match:
                    current_entry['test'] = match.group(1)
            elif line.strip().startswith('error:'):
                match = re.match(r'^\s*error:\s*"(.+)"', line)
                if match:
                    current_entry['error'] = match.group(1)
            elif line.strip().startswith('before:'):
                match = re.match(r'^\s*before:\s*"(.*)"', line)
                if match:
                    current_entry['before'] = match.group(1)
            elif line.strip().startswith('after:'):
                match = re.match(r'^\s*after:\s*"(.*)"', line)
                if match:
                    current_entry['after'] = match.group(1)
            elif line.strip().startswith('comment:'):
                match = re.match(r'^\s*comment:\s*"(.+)"', line)
                if match:
                    current_entry['comment'] = match.group(1)
        
        # Don't forget the last entry
        if current_entry:
            whitelist.append(current_entry)
    
    return whitelist

def is_whitelisted_error(log_file, line_num, error_line, whitelist):
    """Check if an error is whitelisted based on before/error/after attributes."""
    # Read the file to get before and after lines
    try:
        with open(log_file, 'r') as f:
            lines = f.readlines()
    except:
        return False
    
    # Validate line_num is valid
    if line_num < 1 or line_num > len(lines):
        return False
    
    # Get before and after lines (line_num is 1-based)
    before_line = lines[line_num - 2].strip() if line_num > 1 else ""
    after_line = lines[line_num].strip() if line_num < len(lines) else ""
    error_text = error_line.strip()
    
    # Strip timestamps from lines for comparison
    error_text_clean = re.sub(r'^\[.*?\]\s*', '', error_text)
    before_line_clean = re.sub(r'^\[.*?\]\s*', '', before_line)
    after_line_clean = re.sub(r'^\[.*?\]\s*', '', after_line)
    
    # Check against whitelist entries
    for entry in whitelist:
        # Only check before/error/after attributes (ignore test and comment)
        error_pattern = entry.get('error', '')
        before_pattern = entry.get('before', '')
        after_pattern = entry.get('after', '')
        
        # Check if error matches (in cleaned text)
        if error_pattern and error_pattern not in error_text_clean:
            continue
            
        # Check if before matches (empty means ignore)
        if before_pattern and before_pattern not in before_line_clean:
            continue
            
        # Check if after matches (empty means ignore)
        if after_pattern and after_pattern not in after_line_clean:
            continue
            
        # All conditions match - this error is whitelisted
        return True
    
    return False

def check_logs_for_pattern(pattern, description, files_pattern='logs/*.log', 
                          exclude_pattern=None, exclude_text=None, limit=10):
    """Check logs for a specific pattern and report findings."""
    print(f"--- {description} ---")
    found = False
    matches = []
    
    # Get matching files
    if isinstance(files_pattern, list):
        files = []
        for fp in files_pattern:
            files.extend(glob.glob(fp))
    else:
        files = glob.glob(files_pattern)
    
    # Search through log files
    for log_file in files:
        try:
            with open(log_file, 'r') as f:
                for i, line in enumerate(f, 1):
                    if re.search(pattern, line):
                        # Apply exclusions
                        if exclude_pattern and re.search(exclude_pattern, log_file):
                            continue
                        if exclude_text and exclude_text in line:
                            continue
                        matches.append(f"{log_file}:{i}:{line.strip()}")
                        found = True
        except:
            continue
    
    if found:
        print(f"{YELLOW}⚠️  {description.split(' for ')[-1].upper()} FOUND:{RESET}")
        for match in matches[:limit]:
            print(f"    {match}")
        if len(matches) > limit:
            print(f"    ... and {len(matches) - limit} more")
    else:
        print(f"{GREEN}✓ No {description.split(' for ')[-1].lower()} found{RESET}")
    
    print()
    return found

def check_test_failures():
    """Check for test failures with special handling."""
    print("--- Checking for test failures ---")
    found = False
    matches = []
    
    # Only check test logs, not exec logs
    test_logs = ['logs/run-tests.log', 'logs/sanitize-tests.log', 'logs/tsan-tests.log']
    pattern = re.compile(r'(TEST FAILED|Test .* failed|ERROR: Test|FAILED:)')
    
    for log_file in test_logs:
        if os.path.exists(log_file):
            with open(log_file, 'r') as f:
                for i, line in enumerate(f, 1):
                    if pattern.search(line) and 'ERROR: Test error message' not in line:
                        matches.append(f"{log_file}:{i}:{line.strip()}")
                        found = True
    
    if found:
        print(f"{YELLOW}⚠️  TEST FAILURES FOUND:{RESET}")
        for match in matches[:10]:
            print(f"    {match}")
    else:
        print(f"{GREEN}✓ No test failures found{RESET}")
    
    print()
    return found

def check_method_evaluation_failures(whitelist):
    """Check for method evaluation failures with whitelist filtering."""
    global FOUND_ISSUES
    print("--- Checking for method evaluation failures ---")
    found = False
    unintentional_errors = []
    
    pattern = re.compile(r'ERROR: Method evaluation failed')
    
    for log_file in glob.glob('logs/*.log'):
        try:
            with open(log_file, 'r') as f:
                lines = f.readlines()
                for i, line in enumerate(lines):
                    if pattern.search(line):
                        # Use 1-based line number for consistency with grep -n
                        line_num = i + 1
                        # Don't parse the error text - pass the full line
                        if not is_whitelisted_error(log_file, line_num, line, whitelist):
                            unintentional_errors.append(f"{log_file}:{line_num}:{line.strip()}")
                            found = True
        except:
            continue
    
    if found:
        print(f"{YELLOW}⚠️  METHOD EVALUATION FAILURES FOUND:{RESET}")
        for error in unintentional_errors[:10]:
            print(f"    {error}")
        FOUND_ISSUES = True
    else:
        print(f"{GREEN}✓ No unexpected method evaluation failures found{RESET}")
    
    print()
    return found

def check_missing_ast_errors(whitelist):
    """Check for missing AST errors with whitelist filtering."""
    global FOUND_ISSUES
    print("--- Checking for missing AST errors ---")
    found = False
    unintentional_errors = []
    
    pattern = re.compile(r'ERROR: Method has no AST')
    
    for log_file in glob.glob('logs/*.log'):
        try:
            # Special handling for exec logs (expected AST errors)
            if '-exec.log' in log_file:
                continue
                
            with open(log_file, 'r') as f:
                lines = f.readlines()
                for i, line in enumerate(lines):
                    if pattern.search(line):
                        # Use 1-based line number for consistency with grep -n
                        line_num = i + 1
                        # Don't parse the error text - pass the full line
                        if not is_whitelisted_error(log_file, line_num, line, whitelist):
                            unintentional_errors.append(f"{log_file}:{line_num}:{line.strip()}")
                            found = True
        except:
            continue
    
    if found:
        print(f"{YELLOW}⚠️  MISSING AST ERRORS FOUND:{RESET}")
        for error in unintentional_errors[:10]:
            print(f"    {error}")
        FOUND_ISSUES = True
    else:
        print(f"{GREEN}✓ No missing AST errors found{RESET}")
    
    print()
    return found

def check_memory_leaks():
    """Check for memory leaks from custom heap tracking."""
    found = check_logs_for_pattern(
        r'Warning: \d+ memory leaks? detected',
        'Checking for memory leaks',
        limit=20
    )
    
    if found:
        print("Note: These are from AgeRun's custom heap tracking, not AddressSanitizer.")
        print("Check the referenced memory_report_*.log files for details.")
    
    return found

def check_method_loading_warnings():
    """Check for method loading warnings."""
    found = check_logs_for_pattern(
        r'Could not load methods from file',
        'Checking for method loading warnings'
    )
    
    if found:
        print("Note: These warnings indicate the method store file doesn't exist yet.")
        print("This is expected during initial test runs but may indicate issues in production.")
    
    return found

def check_unexpected_test_behaviors():
    """Check for unexpected test behaviors."""
    global FOUND_ISSUES
    found = check_logs_for_pattern(
        r'WARNING: Method creation succeeded with invalid syntax \(expected failure\)',
        'Checking for unexpected test behaviors'
    )
    
    if found:
        print("Note: A test expecting failure actually succeeded - this may indicate a validation bug.")
        FOUND_ISSUES = True
    
    return found

def check_deep_copy_support_errors():
    """Check for deep copy support errors."""
    global FOUND_ISSUES
    found = check_logs_for_pattern(
        r'no deep copy support',
        'Checking for deep copy support errors'
    )
    if found:
        FOUND_ISSUES = True
    return found

def check_compilation_warnings():
    """Check for compilation warnings/errors."""
    return check_logs_for_pattern(
        r'\.(c|h|zig):[0-9]+:[0-9]+: (warning|error):|^(warning|error):',
        'Checking for compilation warnings/errors',
        exclude_pattern=r'logs/analyze-',
        limit=30
    )

def check_linker_warnings():
    """Check for linker warnings/errors."""
    return check_logs_for_pattern(
        r'(ld: warning:|ld: error:|undefined reference|undefined symbol|duplicate symbol|was built for newer.*version.*than being linked|relocation|cannot find -l)',
        'Checking for linker warnings/errors',
        limit=30
    )

def check_static_analysis_issues():
    """Check for static analysis issues."""
    found = False
    matches = []
    
    print("--- Checking for static analysis issues ---")
    
    pattern = re.compile(r'(scan-build: [1-9][0-9]* bug|warning:|error:|ERROR:|WARNING:)')
    
    for log_file in glob.glob('logs/analyze-*.log'):
        if os.path.exists(log_file):
            with open(log_file, 'r') as f:
                for i, line in enumerate(f, 1):
                    if pattern.search(line) and 'No bugs found' not in line:
                        matches.append(f"{log_file}:{i}:{line.strip()}")
                        found = True
    
    if found:
        print(f"{YELLOW}⚠️  STATIC ANALYSIS ISSUES FOUND:{RESET}")
        for match in matches[:10]:
            print(f"    {match}")
    else:
        print(f"{GREEN}✓ No static analysis issues found{RESET}")
    
    print()
    return found

def check_deep_analysis_errors(whitelist):
    """Perform deep analysis for additional error patterns."""
    print("=== Deep Log Analysis ===")
    print()
    
    deep_issues_found = False
    
    # Check for any ERROR: patterns we might have missed
    print("--- Scanning for additional ERROR patterns ---")
    unfiltered_errors = []
    
    error_pattern = re.compile(r'ERROR:|Error:')
    skip_patterns = [
        'ERROR: AddressSanitizer',
        'ERROR: LeakSanitizer', 
        'ERROR: UndefinedBehaviorSanitizer',
        'ERROR: ThreadSanitizer',
        'ERROR: Test error message'
    ]
    
    for log_file in glob.glob('logs/*.log'):
        try:
            with open(log_file, 'r') as f:
                for i, line in enumerate(f, 1):
                    if error_pattern.search(line):
                        # Skip known sanitizer errors
                        if any(skip in line for skip in skip_patterns):
                            continue
                        
                        # Check if whitelisted
                        error_text = re.sub(r'^\[.*?\]\s*', '', line.strip())
                        if not is_whitelisted_error(log_file, i, error_text, whitelist):
                            unfiltered_errors.append(f"{log_file}:{i}:{line.strip()}")
        except:
            continue
    
    if unfiltered_errors:
        print(f"{YELLOW}⚠️  Found {len(unfiltered_errors)} unwhitelisted ERROR messages in logs:{RESET}")
        for error in unfiltered_errors[:10]:
            print(f"    {error}")
        deep_issues_found = True
    else:
        print(f"{GREEN}✓ No additional unwhitelisted ERROR patterns found{RESET}")
    
    print()
    
    # Check for WARNING patterns
    print("--- Scanning for additional WARNING patterns ---")
    warning_count = 0
    warning_matches = []
    
    warning_pattern = re.compile(r'WARNING:|Warning:')
    skip_warning_patterns = [
        'WARNING: .* memory leaks detected',
        'WARNING: ThreadSanitizer',
        'logs/analyze-'
    ]
    
    for log_file in glob.glob('logs/*.log'):
        if 'analyze-' in log_file:
            continue
        try:
            with open(log_file, 'r') as f:
                for i, line in enumerate(f, 1):
                    if warning_pattern.search(line):
                        if not any(skip in line for skip in skip_warning_patterns[:2]):
                            warning_matches.append(f"{log_file}:{i}:{line.strip()}")
                            warning_count += 1
        except:
            continue
    
    if warning_count > 0:
        print(f"{YELLOW}⚠️  Found {warning_count} WARNING messages in logs:{RESET}")
        for match in warning_matches[:10]:
            print(f"    {match}")
        deep_issues_found = True
    else:
        print(f"{GREEN}✓ No additional WARNING patterns found{RESET}")
    
    print()
    
    # Check test output consistency
    print("--- Checking test output consistency ---")
    tests_run = 0
    tests_passed = 0
    
    if os.path.exists('logs/run-tests.log'):
        with open('logs/run-tests.log', 'r') as f:
            content = f.read()
            tests_run = len(re.findall(r'^Running test:', content, re.MULTILINE))
            tests_passed = len(re.findall(r'All .* tests passed', content))
    
    if tests_run > 0 and tests_passed == 0:
        print(f"{YELLOW}⚠️  INCONSISTENCY: {tests_run} tests ran but no 'All tests passed' messages found{RESET}")
        deep_issues_found = True
    else:
        print(f"{GREEN}✓ Test output appears consistent ({tests_run} tests, {tests_passed} pass messages){RESET}")
    
    print()
    
    # Check for suspicious test patterns
    print("--- Checking for suspicious test patterns ---")
    suspicious_found = False
    suspicious_matches = []
    
    pattern = re.compile(r'(FAILED|failed|FAIL|fail|ERROR|error)')
    test_pattern = re.compile(r'(test_|_test|Test)')
    
    for log_file in glob.glob('logs/*.log'):
        try:
            with open(log_file, 'r') as f:
                for i, line in enumerate(f, 1):
                    if pattern.search(line) and test_pattern.search(line):
                        if ('test.*failed.*passed' not in line and 
                            'expected.*fail' not in line and
                            'ERROR: Test error message' not in line):
                            suspicious_matches.append(f"{log_file}:{i}:{line.strip()}")
                            suspicious_found = True
        except:
            continue
    
    if suspicious_found:
        print(f"{YELLOW}⚠️  Found suspicious patterns in test-related output:{RESET}")
        for match in suspicious_matches[:5]:
            print(f"    {match}")
        deep_issues_found = True
    else:
        print(f"{GREEN}✓ No suspicious test patterns found{RESET}")
    
    print()
    
    # Check for failure indicators
    print("--- Checking for failure indicators ---")
    failure_found = False
    failure_matches = []
    
    pattern = re.compile(r'(Could not|Cannot|Unable to|Failed to)')
    
    for log_file in glob.glob('logs/*.log'):
        try:
            with open(log_file, 'r') as f:
                for i, line in enumerate(f, 1):
                    if pattern.search(line):
                        if ('Could not load methods from file' not in line and
                            'expected' not in line):
                            failure_matches.append(f"{log_file}:{i}:{line.strip()}")
                            failure_found = True
        except:
            continue
    
    if failure_found:
        print(f"{YELLOW}⚠️  Found failure indicators:{RESET}")
        for match in failure_matches[:10]:
            print(f"    {match}")
        deep_issues_found = True
    else:
        print(f"{GREEN}✓ No unexpected failure indicators found{RESET}")
    
    print()
    
    return deep_issues_found

def main():
    """Main function to check build logs."""
    global FOUND_ISSUES
    
    print("=== Checking Build Logs for Issues ===")
    print()
    
    # Load whitelist
    whitelist = load_whitelist()
    
    critical_issues = False
    warning_issues = False
    
    # Standard checks
    critical_issues |= check_logs_for_pattern(
        r'Assertion failed', 
        'Checking for assertion failures'
    )
    
    critical_issues |= check_logs_for_pattern(
        r'(Segmentation fault|Abort trap|core dumped)',
        'Checking for crashes'
    )
    
    check_test_failures()
    
    critical_issues |= check_logs_for_pattern(
        r'(ERROR: AddressSanitizer|ERROR: LeakSanitizer|ERROR: UndefinedBehaviorSanitizer)',
        'Checking for memory errors',
        files_pattern='logs/sanitize-*.log'
    )
    
    critical_issues |= check_logs_for_pattern(
        r'WARNING: ThreadSanitizer',
        'Checking for thread safety issues',
        files_pattern='logs/tsan-*.log'
    )
    
    critical_issues |= check_logs_for_pattern(
        r'(runtime error|SIGABRT|SIGSEGV|SIGBUS|SIGILL|SIGFPE)',
        'Checking for runtime errors'
    )
    
    check_logs_for_pattern(
        r'(Invalid read|Invalid write|Conditional jump|Uninitialised value)',
        'Checking for valgrind errors'
    )
    
    check_static_analysis_issues()
    
    warning_issues |= check_compilation_warnings()
    warning_issues |= check_linker_warnings()
    
    # Memory leak check
    critical_issues |= check_memory_leaks()
    
    check_deep_copy_support_errors()
    check_method_loading_warnings()
    check_unexpected_test_behaviors()
    
    # Whitelist-aware checks
    check_method_evaluation_failures(whitelist)
    check_missing_ast_errors(whitelist)
    
    # Check for critical issues in a second pass (like bash script)
    if not critical_issues:
        # Re-check for critical patterns
        for log_file in glob.glob('logs/*.log'):
            try:
                with open(log_file, 'r') as f:
                    content = f.read()
                    if re.search(r'(Assertion failed|Segmentation fault|Abort trap|core dumped|ERROR: AddressSanitizer|ERROR: LeakSanitizer|ERROR: UndefinedBehaviorSanitizer|WARNING: ThreadSanitizer|runtime error|SIGABRT|SIGSEGV)', content):
                        critical_issues = True
                        break
                    if re.search(r'Warning: \d+ memory leaks? detected', content):
                        critical_issues = True
                        break
            except:
                continue
    
    # Check if FOUND_ISSUES was set
    if FOUND_ISSUES:
        critical_issues = True
    
    # Summary
    print("=== Summary ===")
    print(f"Log files are in: logs/")
    print(f"To view a specific log: less logs/<logname>.log")
    print(f"To search logs: grep -r 'pattern' logs/")
    
    # Report results and exit
    if critical_issues:
        print()
        print(f"{RED}⚠️  CRITICAL ISSUES DETECTED - Please review the logs above!{RESET}")
        sys.exit(1)
    elif warning_issues:
        print()
        print(f"{YELLOW}⚠️  WARNINGS DETECTED - Build succeeded but there are compilation/linking warnings to address!{RESET}")
        sys.exit(1)
    else:
        print()
        print(f"{GREEN}✓ All standard checks passed - performing deep analysis...{RESET}")
        print()
        
        # Perform deep analysis
        deep_issues = check_deep_analysis_errors(whitelist)
        
        # Final deep analysis report
        print("=== Deep Analysis Summary ===")
        if deep_issues:
            print(f"{YELLOW}⚠️  ADDITIONAL ISSUES FOUND during deep analysis!{RESET}")
            print("These patterns suggest potential problems that weren't caught by standard checks.")
            print("Please review the specific instances above and determine if they need attention.")
            sys.exit(1)
        else:
            print(f"{GREEN}✓ Deep analysis complete - no additional issues detected{RESET}")
            print("The build logs appear clean.")
            sys.exit(0)

if __name__ == '__main__':
    main()