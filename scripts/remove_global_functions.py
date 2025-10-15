#!/usr/bin/env python3
"""
Remove global API functions from ar_agency.c that delegate to instance-based versions.

This script is part of the global API removal refactoring (Stage 2b).
It identifies and removes function implementations that are simple delegations
to their _with_instance counterparts.

Usage:
    python3 scripts/remove_global_functions.py              # Dry run (default)
    python3 scripts/remove_global_functions.py --apply      # Apply changes
"""

import sys
import re
from pathlib import Path

# Global functions to remove (these delegate to _with_instance versions)
FUNCTIONS_TO_REMOVE = [
    '_get_global_instance',  # Static helper no longer needed
    'ar_agency__reset',
    'ar_agency__count_agents',
    'ar_agency__save_agents',
    'ar_agency__load_agents',
    'ar_agency__update_agent_methods',
    'ar_agency__count_agents_using_method',
    'ar_agency__get_first_agent',
    'ar_agency__get_next_agent',
    'ar_agency__agent_has_messages',
    'ar_agency__get_agent_message',
    'ar_agency__create_agent',
    'ar_agency__destroy_agent',
    'ar_agency__send_to_agent',
    'ar_agency__agent_exists',
    'ar_agency__get_agent_memory',
    'ar_agency__get_agent_context',
    'ar_agency__is_agent_active',
    'ar_agency__get_agent_method',
    'ar_agency__get_agent_method_info',
    'ar_agency__get_agent_mutable_memory',
    'ar_agency__update_agent_method',
    'ar_agency__set_agent_active',
    'ar_agency__count_active_agents',
    'ar_agency__set_agent_id',
    'ar_agency__get_registry',
]

def find_function_ranges(lines):
    """Find the line ranges for each function to remove."""
    ranges_to_remove = []

    for func_name in FUNCTIONS_TO_REMOVE:
        # Find function definition - match various return type patterns
        # Pattern accounts for return types like: void, int, bool, int64_t, const ar_data_t*, ar_data_t*, etc.
        pattern = rf'^[a-zA-Z_0-9]+(\s+\*|\s*\*|\s+)?{re.escape(func_name)}\s*\([^)]*\)\s*{{\s*$'

        for i, line in enumerate(lines):
            if re.search(rf'{re.escape(func_name)}\s*\([^)]*\)\s*{{\s*$', line):
                # Found function start, now find its end
                brace_count = 1
                end_idx = i + 1

                while end_idx < len(lines) and brace_count > 0:
                    if '{' in lines[end_idx]:
                        brace_count += lines[end_idx].count('{')
                    if '}' in lines[end_idx]:
                        brace_count -= lines[end_idx].count('}')
                    end_idx += 1

                if brace_count == 0:
                    # Found complete function
                    ranges_to_remove.append((i, end_idx, func_name))
                    print(f"  Found {func_name} at lines {i+1}-{end_idx}")
                break

    return ranges_to_remove

def remove_functions(filepath, dry_run=True):
    """Remove global functions from the file."""
    if not filepath.exists():
        print(f"Error: {filepath} does not exist")
        return False

    lines = filepath.read_text().splitlines(keepends=True)
    original_line_count = len(lines)

    # Find all function ranges
    ranges_to_remove = find_function_ranges(lines)

    if not ranges_to_remove:
        print("No functions found to remove")
        return False

    # Sort ranges in reverse order to delete from end to beginning
    ranges_to_remove.sort(reverse=True)

    if dry_run:
        print(f"\nDry run mode - would remove {len(ranges_to_remove)} functions:")
        for start, end, func_name in ranges_to_remove:
            print(f"  - {func_name} (lines {start+1}-{end})")

        # Show line count impact
        total_lines_to_remove = sum(end - start for start, end, _ in ranges_to_remove)
        print(f"\nWould remove {total_lines_to_remove} lines from {filepath.name}")
        print(f"File would shrink from {original_line_count} to {original_line_count - total_lines_to_remove} lines")
    else:
        # Actually remove the functions
        for start, end, func_name in ranges_to_remove:
            del lines[start:end]
            print(f"  Removed {func_name}")

        # Write back
        filepath.write_text(''.join(lines))

        new_line_count = len(lines)
        print(f"\nRemoved {len(ranges_to_remove)} functions")
        print(f"File shrunk from {original_line_count} to {new_line_count} lines")

    return True

def main(dry_run=True):
    """Main function."""
    agency_file = Path("modules/ar_agency.c")

    print("=" * 60)
    print("Global Function Removal Tool")
    print("=" * 60)

    if dry_run:
        print("MODE: Dry run (use --apply to make changes)")
    else:
        print("MODE: Apply changes")

    print(f"\nProcessing: {agency_file}")
    print("-" * 40)

    success = remove_functions(agency_file, dry_run)

    if dry_run and success:
        print("\n" + "=" * 60)
        print("Dry run complete. Run with --apply to make changes.")
        print("=" * 60)
    elif success:
        print("\n" + "=" * 60)
        print("Changes applied successfully!")
        print("Next step: Run 'make clean run-tests' to verify")
        print("=" * 60)

    return 0 if success else 1

if __name__ == "__main__":
    dry_run = "--apply" not in sys.argv
    sys.exit(main(dry_run))