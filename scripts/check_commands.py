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

commands_dir = '.opencode/command/ar'

# Also check legacy symlink location
legacy_commands_dir = '.claude/commands'
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

    # Check legacy location
    if os.path.exists(legacy_commands_dir):
        for filename in os.listdir(legacy_commands_dir):
            if filename.endswith('.md'):
                commands.append(os.path.join('legacy', filename))

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
    if filename.startswith('legacy/'):
        filepath = os.path.join(legacy_commands_dir, filename[7:])  # Remove 'legacy/' prefix
    else:
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
    with open('/tmp/structure-fix-report.json', 'w') as f:
        import json
        json.dump({
            'needs_fixing': [(f, a['missing_elements']) for f, a in needs_fixing],
            'scores': {f: a['structure_score'] for f, a in results.items()}
        }, f, indent=2)
    print("Fix report saved to /tmp/structure-fix-report.json")
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

# ============================================================================
# CHECKPOINT SCRIPTS VALIDATION
# ============================================================================
print("\n" + "=" * 80)
print("CHECKPOINT SCRIPTS QUALITY VALIDATION")
print("=" * 80)

def validate_checkpoint_scripts():
    """Validate checkpoint scripts for proper patterns and error handling."""
    import glob

    checkpoint_scripts = glob.glob('scripts/checkpoint*.sh') + glob.glob('scripts/*checkpoint*.sh')
    script_results = {}

    required_patterns = {
        'error_handling': {
            'patterns': [r'set -e', r'set -o pipefail'],
            'description': 'Error handling (set -e or set -o pipefail)',
            'weight': 2
        },
        'ostype_pattern_matching': {
            'patterns': [r'\[\[\s*.*OSTYPE.*==.*darwin\*\s*\]\]', r'\[\[\s*.*OSTYPE.*==\s*linux\*\s*\]\]'],
            'description': 'OSTYPE pattern matching with [[ == ]] syntax',
            'weight': 2
        },
        'sed_safe_delimiter': {
            'patterns': [r"sed.*['\"]s[@|#][^/]*[@|#]"],
            'description': 'Safe sed delimiter (@ or | instead of /)',
            'weight': 2
        }
    }

    for script_path in sorted(checkpoint_scripts):
        with open(script_path, 'r') as f:
            content = f.read()

        issues = []
        score = 100

        # Check for required patterns
        for pattern_type, pattern_data in required_patterns.items():
            has_pattern = any(re.search(p, content, re.MULTILINE) for p in pattern_data['patterns'])

            # Only flag as issue if script contains sed or OSTYPE references
            if not has_pattern:
                if pattern_type == 'ostype_pattern_matching' and 'OSTYPE' in content:
                    # Check if it's using the WRONG pattern [ = ] instead of [[ == ]]
                    if re.search(r'\[\s*"\$OSTYPE"\s*=\s*"darwin\*"', content):
                        issues.append(f"Issue: Using [ = ] for OSTYPE (use [[ == ]] for pattern matching)")
                        score -= pattern_data['weight'] * 5
                    elif not re.search(r'\[\[.*OSTYPE.*==.*darwin', content):
                        # Only warn if no proper pattern found
                        issues.append(f"Missing: {pattern_data['description']}")
                        score -= pattern_data['weight'] * 2
                elif pattern_type == 'sed_safe_delimiter' and 'sed' in content:
                    # Check if using unsafe delimiter - look for s/ pattern
                    unsafe_sed = re.findall(r"sed[^|]*['\"]s/[^'\"]*['\"]", content)
                    if unsafe_sed:
                        issues.append(f"Issue: Using / delimiter with sed (use @ or | instead)")
                        score -= pattern_data['weight'] * 5
                elif pattern_type == 'error_handling' and any(x in script_path for x in ['init', 'update', 'status', 'gate', 'cleanup']):
                    # Core checkpoint scripts should have error handling
                    issues.append(f"Missing: {pattern_data['description']}")
                    score -= pattern_data['weight'] * 3

        script_results[script_path] = {
            'score': max(0, score),
            'issues': issues
        }

    # Report results
    print("\n📊 CHECKPOINT SCRIPTS QUALITY")
    print("-" * 80)

    script_scores = []
    for script_path in sorted(script_results.keys()):
        result = script_results[script_path]
        score = result['score']
        script_scores.append(score)

        if score >= 90:
            status = "✅ Pass"
        elif score >= 70:
            status = "⚠️  Warn"
        else:
            status = "❌ Fail"

        script_name = script_path.replace('scripts/', '')
        print(f"{status} {script_name:45} {score:3}%")

        if verbose and result['issues']:
            for issue in result['issues']:
                print(f"     - {issue}")

    if script_scores:
        avg_script_score = sum(script_scores) / len(script_scores)
        failed_scripts = sum(1 for s in script_scores if s < 70)

        print(f"\nAverage Script Score: {avg_script_score:.1f}%")
        if failed_scripts > 0:
            print(f"⚠️  {failed_scripts} scripts need attention")
            return False
        else:
            print("✅ All checkpoint scripts pass validation")
            return True

    return True

# Run checkpoint script validation
scripts_pass = validate_checkpoint_scripts()

# ============================================================================
# CHECKPOINT WRAPPER SCRIPT USAGE VALIDATION
# ============================================================================
print("\n" + "=" * 80)
print("CHECKPOINT WRAPPER SCRIPT USAGE VALIDATION")
print("=" * 80)

def validate_wrapper_script_usage():
    """Validate that commands using checkpoints follow recommended wrapper script patterns."""
    import glob

    wrapper_issues = []
    commands_with_checkpoints = []

    # Find all commands that use checkpoint features
    command_files = glob.glob('.opencode/command/ar/*.md')

    for cmd_file in sorted(command_files):
        with open(cmd_file, 'r') as f:
            content = f.read()

        # Check if command uses checkpoint features
        uses_checkpoint = any(pattern in content for pattern in [
            'make checkpoint-init',
            'make checkpoint-update',
            'make checkpoint-gate',
            'make checkpoint-status',
            'make checkpoint-cleanup'
        ])

        if not uses_checkpoint:
            continue

        cmd_name = os.path.basename(cmd_file)
        commands_with_checkpoints.append(cmd_name)

        # Check for recommended wrapper script usage
        uses_init_wrapper = './scripts/init-checkpoint.sh' in content
        uses_require_wrapper = './scripts/require-checkpoint.sh' in content
        uses_gate_wrapper = './scripts/gate-checkpoint.sh' in content
        uses_complete_wrapper = './scripts/complete-checkpoint.sh' in content

        # Check for anti-patterns (direct Makefile targets that should use wrappers)
        direct_init = 'make checkpoint-init' in content and not uses_init_wrapper
        direct_gate = re.search(r'if\s+.*make checkpoint-gate', content) and not uses_gate_wrapper
        direct_cleanup_status = ('make checkpoint-status' in content and
                                'make checkpoint-cleanup' in content and
                                not uses_complete_wrapper)

        # Also check for inline precondition checks that should use require-checkpoint.sh
        inline_precondition = (
            ('if [ ! -f' in content and 'progress' in content and not uses_require_wrapper) or
            ('if [ -z "$PROGRESS_FILE' in content and not uses_require_wrapper)
        )

        # Collect issues (not using wrappers when recommended)
        issues = []
        if direct_init:
            issues.append("Uses 'make checkpoint-init' instead of './scripts/init-checkpoint.sh'")
        if direct_gate:
            issues.append("Uses 'if make checkpoint-gate' instead of './scripts/gate-checkpoint.sh'")
        if direct_cleanup_status:
            issues.append("Uses separate checkpoint-status + checkpoint-cleanup instead of './scripts/complete-checkpoint.sh'")
        if inline_precondition:
            issues.append("Has inline precondition check instead of './scripts/require-checkpoint.sh'")

        if issues:
            wrapper_issues.append((cmd_name, issues))

    return commands_with_checkpoints, wrapper_issues

commands_with_checkpoints, wrapper_issues = validate_wrapper_script_usage()

print(f"\n📊 WRAPPER SCRIPT USAGE ANALYSIS")
print(f"-" * 80)
print(f"Commands using checkpoint features: {len(commands_with_checkpoints)}")
print(f"Commands following wrapper script recommendations: {len(commands_with_checkpoints) - len(wrapper_issues)}")
print(f"Commands using anti-patterns (direct targets): {len(wrapper_issues)}")

if wrapper_issues:
    print(f"\n⚠️  {len(wrapper_issues)} commands not using recommended wrapper scripts:")
    print("-" * 80)

    for cmd_name, issues in sorted(wrapper_issues)[:10]:  # Show first 10
        print(f"\n{cmd_name}:")
        for issue in issues:
            print(f"  - {issue}")

    if len(wrapper_issues) > 10:
        print(f"\n... and {len(wrapper_issues) - 10} more commands")

    print(f"\n💡 Recommendation: Use wrapper scripts for cleaner, more maintainable checkpoint integration")
    print(f"   See kb/checkpoint-based-workflow-pattern.md for details")

    if verbose:
        print(f"\nAll commands needing wrapper script updates:")
        for cmd_name, _ in sorted(wrapper_issues):
            print(f"  - {cmd_name}")
else:
    print(f"✅ All {len(commands_with_checkpoints)} checkpoint-using commands follow best practices!")

wrapper_validation_pass = len(wrapper_issues) == 0

# Final exit code
print("\n" + "=" * 80)
print("FINAL VALIDATION SUMMARY")
print("=" * 80)

validation_failures = []

if not scripts_pass:
    validation_failures.append("Checkpoint scripts need improvements")
    print("❌ Checkpoint scripts need improvements before CI can pass")

if non_excellent > 0 or avg_score < 90:
    validation_failures.append("Command structure validation failed")
    print(f"❌ Command structure validation failed: {non_excellent} commands not excellent")

if wrapper_issues and not verbose:
    print(f"⚠️  {len(wrapper_issues)} commands not using recommended wrapper scripts (use --verbose to see list)")

if validation_failures:
    print("\n" + "=" * 80)
    for failure in validation_failures:
        print(f"❌ {failure}")
    sys.exit(1)
else:
    print("✅ ALL VALIDATIONS PASSED: Commands and checkpoint scripts are excellent!")
    if wrapper_issues:
        print(f"💡 Note: {len(wrapper_issues)} commands could benefit from wrapper script migration")
    sys.exit(0)