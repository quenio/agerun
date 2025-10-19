#!/usr/bin/env python3
"""
Merge local settings into main settings file.
Combines permissions from both files, avoiding duplicates.
"""
import json
import sys
from pathlib import Path


def load_settings(settings_file):
    """Load settings.json file."""
    if not Path(settings_file).exists():
        return None

    with open(settings_file, 'r') as f:
        return json.load(f)


def merge_permissions(main_perms, local_perms):
    """
    Merge permissions from local into main.

    Args:
        main_perms: List of permissions from main settings
        local_perms: List of permissions from local settings

    Returns:
        Merged list with duplicates removed, sorted
    """
    # Use set to remove duplicates, then convert back to sorted list
    merged = sorted(set(main_perms + local_perms))
    return merged


def merge_settings(main_settings, local_settings):
    """
    Merge local settings into main settings.

    Args:
        main_settings: Dict from settings.json
        local_settings: Dict from settings.local.json

    Returns:
        Merged settings dict
    """
    merged = json.loads(json.dumps(main_settings))  # Deep copy

    if not local_settings:
        return merged

    # Merge permissions
    if 'permissions' in local_settings and 'permissions' in merged:
        local_perms = local_settings['permissions'].get('allow', [])
        main_perms = merged['permissions'].get('allow', [])

        merged['permissions']['allow'] = merge_permissions(main_perms, local_perms)

    return merged


def apply_merge(main_file, local_file, output_file):
    """
    Merge local settings into main settings and write result.

    Returns:
        (success: bool, stats: dict)
    """
    # Load files
    main_settings = load_settings(main_file)
    local_settings = load_settings(local_file)

    if not main_settings:
        return False, {'error': f'Main settings file not found: {main_file}'}

    if not local_settings:
        return False, {'error': f'Local settings file not found: {local_file}'}

    # Get counts before merge
    main_perms_count = len(main_settings.get('permissions', {}).get('allow', []))
    local_perms_count = len(local_settings.get('permissions', {}).get('allow', []))

    # Merge settings
    merged_settings = merge_settings(main_settings, local_settings)
    merged_perms_count = len(merged_settings.get('permissions', {}).get('allow', []))

    # Calculate stats
    new_perms = merged_perms_count - main_perms_count

    # Write result
    with open(output_file, 'w') as f:
        json.dump(merged_settings, f, indent=2)
        f.write('\n')

    return True, {
        'main_perms': main_perms_count,
        'local_perms': local_perms_count,
        'merged_perms': merged_perms_count,
        'new_perms': new_perms,
    }


def main():
    """Main entry point."""
    main_file = './.claude/settings.json'
    local_file = './.claude/settings.local.json'
    output_file = './.claude/settings.json'

    success, stats = apply_merge(main_file, local_file, output_file)

    if not success:
        print(f"❌ Merge failed: {stats.get('error')}")
        sys.exit(1)

    print("✅ Settings merged successfully")
    print(f"\nMerge Statistics:")
    print(f"  Main permissions:   {stats['main_perms']}")
    print(f"  Local permissions:  {stats['local_perms']}")
    print(f"  Merged permissions: {stats['merged_perms']}")
    print(f"  New permissions:    {stats['new_perms']}")

    sys.exit(0)


if __name__ == '__main__':
    main()
