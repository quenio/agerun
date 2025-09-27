#!/usr/bin/env python3
"""
Update documentation to use instance-based agency APIs instead of global ones.

This script updates all .md files that reference removed global agency functions
to use their _with_instance equivalents.

Usage:
    python3 scripts/update_agency_docs.py              # Dry run (default)
    python3 scripts/update_agency_docs.py --apply      # Apply changes
"""

import sys
import re
from pathlib import Path

# Mapping of old global functions to their instance equivalents
FUNCTION_MAPPINGS = {
    'ar_agency__create_agent': 'ar_agency__create_agent',
    'ar_agency__save_agents': 'ar_agency__save_agents',
    'ar_agency__send_to_agent': 'ar_agency__send_to_agent',
    'ar_agency__get_agent_context': 'ar_agency__get_agent_context',
    'ar_agency__get_agent_mutable_memory': 'ar_agency__get_agent_mutable_memory',
    'ar_agency__get_agent_method': 'ar_agency__get_agent_method',
    'ar_agency__get_agent_message': 'ar_agency__get_agent_message',
    'ar_agency__count_agents_using_method': 'ar_agency__count_agents_using_method',
}

def update_file(filepath, dry_run=True):
    """Update references in a single file."""
    content = filepath.read_text()
    original_content = content
    changes_made = []

    for old_func, new_func in FUNCTION_MAPPINGS.items():
        # Find all occurrences
        pattern = re.escape(old_func)
        if pattern in content:
            count = content.count(old_func)
            content = content.replace(old_func, new_func)
            changes_made.append(f"  - {old_func} → {new_func} ({count} occurrences)")

    if changes_made:
        if dry_run:
            print(f"\nWould update: {filepath}")
            for change in changes_made:
                print(change)
        else:
            filepath.write_text(content)
            print(f"\nUpdated: {filepath}")
            for change in changes_made:
                print(change)
        return True
    return False

def main(dry_run=True):
    """Main function."""
    print("=" * 60)
    print("Agency Documentation Update Tool")
    print("=" * 60)

    if dry_run:
        print("MODE: Dry run (use --apply to make changes)")
    else:
        print("MODE: Apply changes")

    # Find all markdown files
    md_files = list(Path(".").glob("**/*.md"))
    # Exclude node_modules, bin, and other generated directories
    md_files = [f for f in md_files if not any(
        part in f.parts for part in ['node_modules', 'bin', '.git']
    )]

    files_updated = 0
    for filepath in sorted(md_files):
        if update_file(filepath, dry_run):
            files_updated += 1

    print("\n" + "=" * 60)
    if dry_run:
        print(f"Would update {files_updated} files.")
        if files_updated > 0:
            print("Run with --apply to make changes.")
    else:
        print(f"Updated {files_updated} files.")
        if files_updated > 0:
            print("Run 'make check-docs' to verify.")
    print("=" * 60)

    return 0

if __name__ == "__main__":
    dry_run = "--apply" not in sys.argv
    sys.exit(main(dry_run))