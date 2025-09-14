#!/usr/bin/env python3
"""Fix global function references in KB articles."""

import os
import re
import sys

# Mapping of old global functions to new instance-based functions
REPLACEMENTS = {
    # Methodology functions
    r'ar_methodology__get_method\b': 'ar_methodology__get_method_with_instance',
    r'ar_methodology__create_method\b': 'ar_methodology__create_method_with_instance',
    r'ar_methodology__register_method\b': 'ar_methodology__register_method_with_instance',
    r'ar_methodology__unregister_method\b': 'ar_methodology__unregister_method_with_instance',
    r'ar_methodology__save_methods\b': 'ar_methodology__save_methods_with_instance',
    r'ar_methodology__load_methods\b': 'ar_methodology__load_methods_with_instance',
    r'ar_methodology__cleanup\b': 'ar_methodology__cleanup_with_instance',

    # Agency functions
    r'ar_agency__create_agent\b': 'ar_agency__create_agent_with_instance',
    r'ar_agency__destroy_agent\b': 'ar_agency__destroy_agent_with_instance',
    r'ar_agency__send_to_agent\b': 'ar_agency__send_to_agent_with_instance',
    r'ar_agency__get_agent\b': 'ar_agency__get_agent_with_instance',
    r'ar_agency__reset\b': 'ar_agency__reset_with_instance',
}

def fix_file(filepath):
    """Fix global function references in a file."""
    with open(filepath, 'r') as f:
        content = f.read()

    original = content
    for old, new in REPLACEMENTS.items():
        content = re.sub(old, new, content)

    if content != original:
        with open(filepath, 'w') as f:
            f.write(content)
        return True
    return False

def main():
    """Main function."""
    # Process KB articles
    kb_dir = 'kb'
    fixed_count = 0

    for filename in os.listdir(kb_dir):
        if filename.endswith('.md'):
            filepath = os.path.join(kb_dir, filename)
            if fix_file(filepath):
                print(f"Fixed: {filepath}")
                fixed_count += 1

    # Also fix README.md
    if fix_file('README.md'):
        print(f"Fixed: README.md")
        fixed_count += 1

    print(f"\nTotal files fixed: {fixed_count}")
    return 0

if __name__ == '__main__':
    sys.exit(main())