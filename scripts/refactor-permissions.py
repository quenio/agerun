#!/usr/bin/env python3
"""
Refactor Claude Code permissions to use generic wildcard patterns.
Supports dry-run mode by default.
"""
import json
import sys
from pathlib import Path
from collections import defaultdict


def load_settings(settings_file):
    """Load settings.json file."""
    with open(settings_file, 'r') as f:
        return json.load(f)


def analyze_permissions(allow_list):
    """Analyze permissions and suggest generic patterns."""
    # Group permissions by tool
    bash_commands = defaultdict(list)
    read_commands = []
    webfetch_commands = []
    websearch = False

    for perm in allow_list:
        if perm.startswith('Bash('):
            cmd = perm[5:-1]  # Remove 'Bash(' and ')'
            bash_commands['__all__'].append(cmd)

            # Extract base command
            if '::' in cmd:
                base = cmd.split('::')[0]
                bash_commands[base].append(cmd)
            elif ' ' in cmd:
                base = cmd.split()[0]
                bash_commands[base].append(cmd)
            elif ':' in cmd:
                base = cmd.split(':')[0]
                bash_commands[base].append(cmd)
            else:
                bash_commands[cmd].append(cmd)

        elif perm.startswith('Read('):
            read_commands.append(perm)
        elif perm.startswith('WebFetch('):
            webfetch_commands.append(perm)
        elif perm == 'WebSearch':
            websearch = True

    return bash_commands, read_commands, webfetch_commands, websearch


def suggest_generic_permissions(bash_commands, read_commands, webfetch_commands, websearch):
    """Suggest generic permission patterns."""
    generic = []

    # Commands that should NOT be wildcarded (they're shell keywords or control flow)
    skip_wildcards = {
        'do', 'done', 'if', 'then', 'else', 'fi', 'for', 'while',
        'bash', 'source'
    }

    # Commands that are safe to use wildcards
    script_commands = {
        './scripts/', './bin/', './', '/private/tmp/'
    }

    # Process Bash commands
    for base_cmd in sorted(bash_commands.keys()):
        if base_cmd == '__all__':
            continue

        cmds = bash_commands[base_cmd]

        # Skip shell keywords - never use wildcards
        if base_cmd in skip_wildcards:
            for cmd in cmds:
                generic.append(f'Bash({cmd})')
            continue

        # If there's only one variation, add as-is
        if len(cmds) == 1:
            generic.append(f'Bash({cmds[0]})')
            continue

        # Multiple variations - use wildcard for safe commands
        is_script = any(base_cmd.startswith(prefix) for prefix in script_commands)
        is_tool = not is_script and ' ' not in base_cmd

        if is_script or (is_tool and base_cmd not in skip_wildcards):
            # Use wildcard for script paths or simple tools
            generic.append(f'Bash({base_cmd}:*)')
        else:
            # Keep individual commands for everything else
            for cmd in sorted(cmds):
                generic.append(f'Bash({cmd})')

    # Add Read permissions
    generic.extend(sorted(read_commands))

    # Add WebFetch permissions
    generic.extend(sorted(webfetch_commands))

    # Add WebSearch
    if websearch:
        generic.append('WebSearch')

    return sorted(set(generic))


def count_reductions(original, generic):
    """Calculate reduction in permission entries."""
    original_count = len(original)
    generic_count = len(generic)
    reduction = original_count - generic_count
    reduction_pct = (reduction / original_count * 100) if original_count > 0 else 0

    return {
        'original_count': original_count,
        'generic_count': generic_count,
        'reduction': reduction,
        'reduction_pct': reduction_pct
    }


def print_analysis(bash_commands, read_commands, webfetch_commands, websearch):
    """Print analysis of current permissions."""
    print("=== Current Permissions Analysis ===\n")

    # Bash commands by type
    print("Bash Commands by Type:")
    for tool in sorted(bash_commands.keys()):
        if tool == '__all__':
            continue
        cmds = bash_commands[tool]
        if len(cmds) > 1:
            print(f"  {tool}: {len(cmds)} variations (candidate for {tool}:*)")
        else:
            print(f"  {tool}: 1 permission")

    print(f"\nRead Permissions: {len(read_commands)}")
    print(f"WebFetch Permissions: {len(webfetch_commands)}")
    print(f"WebSearch: {websearch}\n")


def print_preview(original, generic, reductions):
    """Print preview of changes."""
    print("=== Permission Refactoring Preview ===\n")

    print(f"Original count: {reductions['original_count']}")
    print(f"Generic count: {reductions['generic_count']}")
    print(f"Reduction: {reductions['reduction']} entries ({reductions['reduction_pct']:.1f}%)\n")

    # Show top-level categories that will be consolidated
    print("=== Consolidations ===")
    consolidations = defaultdict(list)
    for perm in original:
        if perm.startswith('Bash('):
            cmd = perm[5:-1]
            if ':' in cmd:
                base = cmd.split(':')[0]
                if f'Bash({base}:*)' in generic:
                    consolidations[f'Bash({base}:*)'].append(perm)

    if consolidations:
        for pattern, items in sorted(consolidations.items()):
            if len(items) > 1:
                print(f"\n{pattern} consolidates {len(items)} permissions:")
                for item in items[:3]:
                    print(f"  - {item}")
                if len(items) > 3:
                    print(f"  ... and {len(items) - 3} more")

    print("\n=== Refactored Permissions ===")
    print("(First 20 entries shown)")
    for perm in generic[:20]:
        print(f"  {perm}")
    if len(generic) > 20:
        print(f"  ... and {len(generic) - 20} more")


def apply_refactoring(settings_file, generic_permissions):
    """Apply the refactoring to settings.json."""
    with open(settings_file, 'r') as f:
        settings = json.load(f)

    settings['permissions']['allow'] = generic_permissions

    with open(settings_file, 'w') as f:
        json.dump(settings, f, indent=2)
        f.write('\n')

    print("✅ Settings file refactored")


def main():
    """Main entry point."""
    dry_run = '--apply' not in sys.argv
    settings_file = './.claude/settings.json'

    # Load settings
    settings = load_settings(settings_file)
    original_permissions = settings['permissions']['allow']

    # Analyze
    bash_commands, read_commands, webfetch_commands, websearch = analyze_permissions(
        original_permissions
    )

    # Print analysis
    print_analysis(bash_commands, read_commands, webfetch_commands, websearch)

    # Suggest generic permissions
    generic_permissions = suggest_generic_permissions(
        bash_commands, read_commands, webfetch_commands, websearch
    )

    # Calculate reductions
    reductions = count_reductions(original_permissions, generic_permissions)

    # Print preview
    print_preview(original_permissions, generic_permissions, reductions)

    if dry_run:
        print("\n" + "="*50)
        print("DRY RUN: No changes made")
        print("Run with --apply to make changes")
        print("="*50)
    else:
        print("\n" + "="*50)
        apply_refactoring(settings_file, generic_permissions)
        print("✅ Refactoring complete!")
        print("="*50)


if __name__ == '__main__':
    main()
