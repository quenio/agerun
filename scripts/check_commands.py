#!/usr/bin/env python3
"""
Validate that all Claude Code commands follow the comprehensive structure
exemplified by new-learnings.md.

Expected structure for ALL commands:
1. First-line description
2. Single h1 title
3. Checkpoint/step tracking system
4. Clear checkpoint markers ([CHECKPOINT START], [CHECKPOINT END])
5. Bash commands to mark progress
6. Critical thinking prompts and deep questions
7. Expected outputs
8. Minimum requirements section
9. Troubleshooting section

Usage:
    python3 scripts/validate_comprehensive_structure.py [--verbose] [--fix]
"""

import os
import sys
import re

commands_dir = '.claude/commands'
verbose = '--verbose' in sys.argv
fix_mode = '--fix' in sys.argv

# The ideal structure based on new-learnings.md
COMPREHENSIVE_STRUCTURE = {
    'checkpoint_markers': {
        'start': r'\[CHECKPOINT START',
        'end': r'\[CHECKPOINT END',
        'gate': r'\[.*GATE\]',
        'complete': r'\[CHECKPOINT COMPLETE\]'
    },
    'required_sections': {
        'tracking_system': [
            'Checkpoint Tracking',
            'MANDATORY STEP TRACKING SYSTEM',
            'Step Tracking',
            'Process Overview'
        ],
        'initialization': [
            'Initialize Tracking',
            'Initialize Progress Tracking',
            'Start Tracking'
        ],
        'progress_check': [
            'Check Progress',
            'Check Status',
            'Progress Status'
        ],
        'minimum_requirements': [
            'Minimum Requirements',
            'MANDATORY Requirements',
            'Requirements'
        ],
        'expected_output': [
            'Expected output',
            'Expected Output',
            'Example Output'
        ]
    },
    'bash_commands': {
        'init': r'make checkpoint-init',
        'update': r'make checkpoint-update',
        'status': r'make checkpoint-status',
        'gate': r'make checkpoint-gate',
        'cleanup': r'make checkpoint-cleanup'
    },
    'quality_indicators': {
        'critical_prompts': r'CRITICAL:|MANDATORY:|IMPORTANT:',
        'deep_questions': r'\?.*\n.*\?',  # Multiple questions
        'numbered_steps': r'\d+\.\s+\*\*.*\*\*:',  # Numbered bold steps
        'verification_steps': r'verify|check|ensure|confirm',
        'examples': r'Example:|For example:|e\.g\.',
        'details_links': r'\[details\]\(.*\.md\)'
    }
}

def analyze_file(filepath):
    """Analyze a command file for comprehensive structure."""
    with open(filepath, 'r') as f:
        content = f.read()
        lines = content.split('\n')
    
    analysis = {
        'first_line': lines[0] if lines else '',
        'has_h1': bool(re.search(r'^# ', content, re.MULTILINE)),
        'checkpoint_markers': {},
        'sections_found': {},
        'bash_commands': {},
        'quality_scores': {},
        'missing_elements': []
    }
    
    # Check checkpoint markers
    for marker_type, pattern in COMPREHENSIVE_STRUCTURE['checkpoint_markers'].items():
        count = len(re.findall(pattern, content))
        analysis['checkpoint_markers'][marker_type] = count
        if count == 0:
            analysis['missing_elements'].append(f"No {marker_type} markers")
    
    # Check required sections (flexible matching)
    for section_type, alternatives in COMPREHENSIVE_STRUCTURE['required_sections'].items():
        found = False
        for alt in alternatives:
            if alt in content:
                analysis['sections_found'][section_type] = alt
                found = True
                break
        if not found:
            analysis['missing_elements'].append(f"Missing {section_type} section")
    
    # Check bash commands
    for cmd_type, pattern in COMPREHENSIVE_STRUCTURE['bash_commands'].items():
        count = len(re.findall(pattern, content))
        analysis['bash_commands'][cmd_type] = count
        if count == 0 and cmd_type in ['init', 'update', 'status']:
            analysis['missing_elements'].append(f"No {cmd_type} bash commands")
    
    # Quality scoring
    for indicator, pattern in COMPREHENSIVE_STRUCTURE['quality_indicators'].items():
        count = len(re.findall(pattern, content, re.IGNORECASE))
        analysis['quality_scores'][indicator] = count
    
    # Overall score
    analysis['structure_score'] = calculate_structure_score(analysis)
    
    return analysis

def calculate_structure_score(analysis):
    """Calculate a structure completeness score (0-100)."""
    score = 0
    max_score = 0
    
    # Checkpoint markers (30 points)
    max_score += 30
    if analysis['checkpoint_markers']['start'] > 0:
        score += 15
    if analysis['checkpoint_markers']['end'] > 0:
        score += 15
    
    # Required sections (30 points)
    max_score += 30
    sections_present = len(analysis['sections_found'])
    sections_expected = len(COMPREHENSIVE_STRUCTURE['required_sections'])
    score += (sections_present / sections_expected) * 30
    
    # Bash commands (20 points)
    max_score += 20
    commands_present = sum(1 for v in analysis['bash_commands'].values() if v > 0)
    commands_expected = 3  # init, update, status are essential
    score += min((commands_present / commands_expected) * 20, 20)
    
    # Quality indicators (20 points)
    max_score += 20
    quality_count = sum(1 for v in analysis['quality_scores'].values() if v > 0)
    quality_expected = 4  # At least 4 quality indicators
    score += min((quality_count / quality_expected) * 20, 20)
    
    return int((score / max_score) * 100)

def get_all_commands():
    """Get all command files."""
    commands = []

    # Check direct files in commands_dir
    if os.path.exists(commands_dir):
        for filename in os.listdir(commands_dir):
            if filename.endswith('.md'):
                commands.append(filename)

    # Check ar/ subdirectory (new namespace structure)
    ar_dir = os.path.join(commands_dir, 'ar')
    if os.path.exists(ar_dir):
        for filename in os.listdir(ar_dir):
            if filename.endswith('.md'):
                commands.append(os.path.join('ar', filename))

    return sorted(commands)

def categorize_score(score):
    """Categorize score with emoji."""
    if score >= 90:
        return "🌟 Excellent"
    elif score >= 70:
        return "✅ Good"
    elif score >= 50:
        return "⚠️ Needs Work"
    else:
        return "❌ Poor"

# Main execution
print("=" * 80)
print("COMPREHENSIVE STRUCTURE VALIDATION")
print("Based on new-learnings.md gold standard")
print("=" * 80)

all_commands = get_all_commands()
results = {}
needs_fixing = []

for filename in all_commands:
    filepath = os.path.join(commands_dir, filename)
    analysis = analyze_file(filepath)
    results[filename] = analysis
    
    if analysis['structure_score'] < 90:
        needs_fixing.append((filename, analysis))

# Sort by score for reporting
sorted_results = sorted(results.items(), key=lambda x: x[1]['structure_score'], reverse=True)

print("\n📊 STRUCTURE SCORES")
print("-" * 40)

for filename, analysis in sorted_results:
    score = analysis['structure_score']
    category = categorize_score(score)
    print(f"{category} {filename:40} {score:3}%")
    
    if verbose and analysis['missing_elements']:
        for missing in analysis['missing_elements'][:3]:
            print(f"     - {missing}")

print("\n" + "=" * 80)
print("SUMMARY")
print("=" * 80)

# Statistics
scores = [a['structure_score'] for a in results.values()]
avg_score = sum(scores) / len(scores) if scores else 0
excellent = sum(1 for s in scores if s >= 90)
good = sum(1 for s in scores if 70 <= s < 90)
needs_work = sum(1 for s in scores if 50 <= s < 70)
poor = sum(1 for s in scores if s < 50)

print(f"\nTotal Commands: {len(all_commands)}")
print(f"Average Score: {avg_score:.1f}%")
print(f"\nDistribution:")
print(f"  🌟 Excellent (90-100%): {excellent} commands")
print(f"  ✅ Good (70-89%): {good} commands")
print(f"  ⚠️ Needs Work (50-69%): {needs_work} commands")
print(f"  ❌ Poor (0-49%): {poor} commands")

if needs_fixing:
    print(f"\n🔧 {len(needs_fixing)} commands need structure improvements")
    print("\nTop issues to address:")
    
    # Collect all missing elements
    issue_counts = {}
    for _, analysis in needs_fixing:
        for missing in analysis['missing_elements']:
            issue_counts[missing] = issue_counts.get(missing, 0) + 1
    
    # Sort by frequency
    sorted_issues = sorted(issue_counts.items(), key=lambda x: x[1], reverse=True)
    for issue, count in sorted_issues[:5]:
        print(f"  - {issue}: affects {count} commands")

if fix_mode:
    print("\n🔧 FIX MODE: Creating fix report...")
    with open('/tmp/structure_fix_report.json', 'w') as f:
        import json
        json.dump({
            'needs_fixing': [(f, a['missing_elements']) for f, a in needs_fixing],
            'scores': {f: a['structure_score'] for f, a in results.items()}
        }, f, indent=2)
    print("Fix report saved to /tmp/structure_fix_report.json")
else:
    print("\nRun with --fix to generate a fix report")
    print("Run with --verbose to see detailed issues")

# Exit with error if any command is not excellent
non_excellent = needs_work + poor + good
if non_excellent > 0:
    print(f"\n❌ Structure validation failed: {non_excellent} commands are not excellent (90%+ required)")
    print(f"Commands must achieve excellent scores to ensure quality and consistency.")
    sys.exit(1)
elif avg_score < 90:
    print(f"\n❌ Structure validation failed: Average score {avg_score:.1f}% is below 90%")
    sys.exit(1)
else:
    print(f"\n✅ Structure validation passed with average score {avg_score:.1f}%")
    print(f"All {len(all_commands)} commands have excellent structure!")