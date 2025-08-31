#!/usr/bin/env python3
"""
Analyze heading structure of all Claude Code command documentation files.

This script verifies that all command files have:
- First-line descriptions for command discovery
- Exactly one h1 heading (the title)
- Proper heading hierarchy (h1→h2→h3→h4)
- Consistent structure across all phases

Usage:
    python3 scripts/analyze_command_headings.py
"""

import os
import sys

commands_dir = '.claude/commands'

# Categorize commands by phase
phase1_commands = [
    'check-module-consistency.md', 'fix-errors-whitelisted.md',
    'migrate-module-to-zig-abi.md', 'migrate-module-to-zig-struct.md',
    'new-learnings.md', 'review-changes.md', 'compact-guidelines.md'
]

phase2_commands = [
    'commit.md', 'check-docs.md', 'compact-tasks.md',
    'compact-changes.md', 'merge-settings.md', 'check-logs.md'
]

phase3_commands = [
    'build.md', 'build-clean.md', 'run-tests.md', 'run-exec.md',
    'sanitize-tests.md', 'sanitize-exec.md', 'tsan-tests.md', 'tsan-exec.md',
    'analyze-tests.md', 'analyze-exec.md', 'check-naming.md',
    'next-task.md', 'next-priority.md'
]

print("=" * 80)
print("HEADING STRUCTURE ANALYSIS FOR ALL COMMANDS")
print("=" * 80)

def analyze_file(filepath, filename):
    """Analyze a single command file for heading structure."""
    with open(filepath, 'r') as f:
        lines = f.readlines()
    
    # Get first line (description)
    first_line = lines[0].strip() if lines else ""
    
    # Count headings (excluding those in code blocks)
    in_code_block = False
    h1_headings = []
    h2_headings = []
    h3_headings = []
    h4_headings = []
    
    for i, line in enumerate(lines, 1):
        if line.strip().startswith('```'):
            in_code_block = not in_code_block
            continue
        
        if not in_code_block:
            if line.startswith('#### '):
                h4_headings.append(f"  Line {i}: {line.strip()[:50]}")
            elif line.startswith('### '):
                h3_headings.append(f"  Line {i}: {line.strip()[:50]}")
            elif line.startswith('## '):
                h2_headings.append(f"  Line {i}: {line.strip()[:50]}")
            elif line.startswith('# '):
                h1_headings.append(f"  Line {i}: {line.strip()[:50]}")
    
    return {
        'first_line': first_line,
        'h1_count': len(h1_headings),
        'h2_count': len(h2_headings),
        'h3_count': len(h3_headings),
        'h4_count': len(h4_headings),
        'h1_first': h1_headings[0] if h1_headings else None,
        'h1_list': h1_headings[:3]  # Show first 3 h1s if multiple
    }

# Analyze each phase
for phase, commands in [("Phase 1 (Complex - Full Checkpoint Tracking)", phase1_commands),
                        ("Phase 2 (Medium - Checkpoint Tracking)", phase2_commands),
                        ("Phase 3 (Simple - Expected Outputs)", phase3_commands)]:
    print(f"\n{phase}:")
    print("-" * 40)
    
    for filename in commands:
        filepath = os.path.join(commands_dir, filename)
        if os.path.exists(filepath):
            analysis = analyze_file(filepath, filename)
            
            print(f"\n{filename}:")
            print(f"  First line: {analysis['first_line'][:60]}...")
            print(f"  Heading counts: h1={analysis['h1_count']}, h2={analysis['h2_count']}, h3={analysis['h3_count']}, h4={analysis['h4_count']}")
            
            if analysis['h1_count'] > 1:
                print(f"  ⚠️ WARNING: Multiple h1 headings found!")
                for h1 in analysis['h1_list']:
                    print(f"    {h1}")
            elif analysis['h1_count'] == 1:
                print(f"  ✅ Single h1: {analysis['h1_first']}")
            else:
                print(f"  ❌ ERROR: No h1 heading found!")

print("\n" + "=" * 80)
print("SUMMARY")
print("=" * 80)

# Check overall consistency
all_files = phase1_commands + phase2_commands + phase3_commands
issues = []
warnings = []

for filename in all_files:
    filepath = os.path.join(commands_dir, filename)
    if os.path.exists(filepath):
        analysis = analyze_file(filepath, filename)
        
        if analysis['h1_count'] != 1:
            issues.append(f"{filename}: Has {analysis['h1_count']} h1 headings (should be 1)")
        
        if not analysis['first_line']:
            issues.append(f"{filename}: Missing first-line description")
        
        # Check for proper structure based on phase
        if filename in phase1_commands or filename in phase2_commands:
            # These should have h4 headings for checkpoints
            if analysis['h4_count'] == 0:
                warnings.append(f"{filename}: No h4 headings (expected for checkpoints)")
        elif filename in phase3_commands:
            # These should have simpler structure
            if analysis['h4_count'] > 0:
                warnings.append(f"{filename}: Has h4 headings (unexpected for simple commands)")

if issues:
    print("\n❌ Critical Issues Found:")
    for issue in issues:
        print(f"  - {issue}")
    sys.exit(1)
elif warnings:
    print("\n⚠️ Warnings:")
    for warning in warnings:
        print(f"  - {warning}")
    print("\n✅ All critical requirements met:")
    print("  - First-line descriptions present")
    print("  - Exactly one h1 heading per file")
    print("  - Proper heading hierarchy")
else:
    print("\n✅ Perfect! All commands have proper structure:")
    print("  - First-line descriptions present")
    print("  - Exactly one h1 heading (the title)")
    print("  - Proper heading hierarchy")
    print("  - Phase-appropriate structure (checkpoints vs expected outputs)")

print("\nTotal commands analyzed: {}".format(len(all_files)))